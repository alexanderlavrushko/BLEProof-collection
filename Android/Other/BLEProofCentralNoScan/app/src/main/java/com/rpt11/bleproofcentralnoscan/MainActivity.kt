package com.rpt11.bleproofcentralnoscan

import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.content.Context
import android.content.Intent
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View
import android.widget.Button
import android.widget.ImageButton
import android.widget.TextView
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView

class MainActivity : AppCompatActivity() {

    private val textViewHelp: TextView
        get() = findViewById(R.id.textViewPairingDescription)
    private val buttonReloadDevices: Button
        get() = findViewById(R.id.buttonReloadDevices)
    private val imageButtonInfo: ImageButton
        get() = findViewById(R.id.imageButtonToggleInfo)
    private val recyclerViewDeviceList: RecyclerView
        get() = findViewById(R.id.recyclerViewDeviceList)

    private val deviceListAdapter: DeviceListAdapter by lazy {
        DeviceListAdapter(ArrayList()) { device ->
            // onClickListener
            val intent = Intent(this, BLEDeviceActivity::class.java).apply {
                putExtra(EXTRA_BLE_DEVICE, device)
            }
            startActivity(intent)
        }
    }

    private val bluetoothAdapter: BluetoothAdapter by lazy {
        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothManager.adapter
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        textViewHelp.visibility = View.GONE
        setupButtonHandlers()
        recyclerViewDeviceList.layoutManager = LinearLayoutManager(this)
        recyclerViewDeviceList.adapter = deviceListAdapter

        reloadDevices()
    }

    private fun setupButtonHandlers() {
        buttonReloadDevices.setOnClickListener { reloadDevices() }
        imageButtonInfo.setOnClickListener { toggleShowInfo() }
    }

    private fun toggleShowInfo() {
        textViewHelp.visibility = when (textViewHelp.visibility) {
            View.VISIBLE -> View.GONE
            View.GONE -> View.VISIBLE
            else -> View.VISIBLE
        }
    }

    private fun reloadDevices() {
        val allDevices = bluetoothAdapter.bondedDevices.toList()
        val bleDevices = allDevices.filter {
            it.type == BluetoothDevice.DEVICE_TYPE_LE || it.type == BluetoothDevice.DEVICE_TYPE_DUAL
        }
        deviceListAdapter.updateList(ArrayList(bleDevices))
    }
}