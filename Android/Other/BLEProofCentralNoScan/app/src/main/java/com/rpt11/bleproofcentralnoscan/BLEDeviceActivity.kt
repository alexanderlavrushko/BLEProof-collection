package com.rpt11.bleproofcentralnoscan

import android.annotation.SuppressLint
import android.bluetooth.*
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.util.Log
import android.view.View
import android.widget.ScrollView
import android.widget.TextView
import java.text.SimpleDateFormat
import java.util.*

const val EXTRA_BLE_DEVICE = "BLEDevice"

class BLEDeviceActivity : AppCompatActivity() {
    enum class BLELifecycleState {
        Disconnected,
        Connecting,
        ConnectedDiscovering,
        Connected
    }

    private var lifecycleState = BLELifecycleState.Disconnected
        set(value) {
            field = value
            appendLog("status = $value")
        }

    private val textViewDeviceName: TextView
        get() = findViewById(R.id.textViewDeviceName)
    private val textViewLog: TextView
        get() = findViewById(R.id.textViewLog)
    private val scrollViewLog: ScrollView
        get() = findViewById(R.id.scrollViewLog)

    private var device: BluetoothDevice? = null
    private var connectedGatt: BluetoothGatt? = null

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_bledevice)

        device = intent.getParcelableExtra(EXTRA_BLE_DEVICE)
        val deviceName: String = device?.let {
            "${it.name ?: "<no name"} (${it.address})"
        } ?: run {
            "<null>"
        }
        textViewDeviceName.text = deviceName

        connect()
    }

    override fun onDestroy() {
        connectedGatt?.close()
        connectedGatt = null
        super.onDestroy()
    }

    @SuppressLint("SetTextI18n")
    private fun appendLog(message: String) {
        Log.d("appendLog", message)
        runOnUiThread {
            val strTime = SimpleDateFormat("HH:mm:ss", Locale.getDefault()).format(Date())
            textViewLog.text = textViewLog.text.toString() + "\n$strTime $message"

            // scroll after delay, because textView has to be updated first
            Handler(Looper.getMainLooper()).postDelayed({
                scrollViewLog.fullScroll(View.FOCUS_DOWN)
            }, 16)
        }
    }

    private fun connect() {
        device?.let {
            appendLog("Connecting to ${it.name}")
            lifecycleState = BLELifecycleState.Connecting
            if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
                it.connectGatt(this, false, gattCallback, BluetoothDevice.TRANSPORT_LE)
            } else {
                it.connectGatt(this, false, gattCallback)
            }
        } ?: run {
            appendLog("ERROR: BluetoothDevice is null, cannot connect")
        }
    }

    private fun bleRestartLifecycle() {
        val timeoutSec = 5L
        appendLog("Will try reconnect in $timeoutSec seconds")
        Handler(Looper.getMainLooper()).postDelayed({
            connect()
        }, timeoutSec * 1000)
    }

    //region BLE events, when connected
    private val gattCallback = object : BluetoothGattCallback() {
        override fun onConnectionStateChange(gatt: BluetoothGatt, status: Int, newState: Int) {
            val deviceAddress = gatt.device.address

            if (status == BluetoothGatt.GATT_SUCCESS) {
                if (newState == BluetoothProfile.STATE_CONNECTED) {
                    appendLog("Connected to $deviceAddress")

                    // recommended on UI thread https://punchthrough.com/android-ble-guide/
                    Handler(Looper.getMainLooper()).post {
                        lifecycleState = BLELifecycleState.ConnectedDiscovering
                        gatt.discoverServices()
                    }
                } else if (newState == BluetoothProfile.STATE_DISCONNECTED) {
                    appendLog("Disconnected from $deviceAddress")
                    connectedGatt = null
                    gatt.close()
                    lifecycleState = BLELifecycleState.Disconnected
                    bleRestartLifecycle()
                }
            } else {
                // random error 133 - close() and try reconnect

                appendLog("ERROR: onConnectionStateChange status=$status deviceAddress=$deviceAddress, disconnecting")

                connectedGatt = null
                gatt.close()
                lifecycleState = BLELifecycleState.Disconnected
                bleRestartLifecycle()
            }
        }

        @Suppress("SpellCheckingInspection")
        override fun onServicesDiscovered(gatt: BluetoothGatt, status: Int) {
            appendLog("onServicesDiscovered services.count=${gatt.services.size} status=$status")

            if (status == 129 /*GATT_INTERNAL_ERROR*/) {
                // it should be a rare case, this article recommends to disconnect:
                // https://medium.com/@martijn.van.welie/making-android-ble-work-part-2-47a3cdaade07
                appendLog("ERROR: status=129 (GATT_INTERNAL_ERROR), disconnecting")
                gatt.disconnect()
                return
            }

            var strLog = "\n"
            val services = gatt.services
            services.forEach { service ->
                val characteristicsTable = service.characteristics.joinToString(
                    separator = "\n    ",
                    prefix = "    "
                ) {
                    it.uuid.toString()
                }
                strLog += "\nService\n${service.uuid}\nCharacteristics:\n$characteristicsTable\n"
            }
            appendLog(strLog)

            connectedGatt = gatt
            lifecycleState = BLELifecycleState.Connected
        }
    }
    //endregion
}