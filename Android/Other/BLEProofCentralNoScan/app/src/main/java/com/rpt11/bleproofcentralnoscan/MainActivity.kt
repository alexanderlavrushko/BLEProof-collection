package com.rpt11.bleproofcentralnoscan

import android.Manifest
import android.app.Activity
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.util.Log
import android.view.View
import android.widget.Button
import android.widget.ImageButton
import android.widget.TextView
import androidx.core.app.ActivityCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView

private const val BLUETOOTH_PERMISSIONS_REQUEST_CODE = 1

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

        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            // Don't ask for permission right after application start
            if (hasPermissions(arrayOf(Manifest.permission.BLUETOOTH_CONNECT))) {
                reloadDevices()
            }
        } else {
            reloadDevices()
        }
    }

    private fun setupButtonHandlers() {
        buttonReloadDevices.setOnClickListener { grantPermissionsAndReloadDevices() }
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

    private fun grantPermissionsAndReloadDevices() {
        grantBluetoothBasicPermissions(AskType.AskOnce) { isGranted ->
            if (!isGranted) {
                Log.e("Permissions", "Bluetooth permissions denied")
                return@grantBluetoothBasicPermissions
            }
            reloadDevices()
        }
    }

    //region Permissions management
    enum class AskType {
        AskOnce,
        InsistUntilSuccess
    }

    private var permissionResultHandlers = mutableMapOf<Int, (Array<out String>, IntArray) -> Unit>()

    override fun onRequestPermissionsResult(requestCode: Int, permissions: Array<out String>, grantResults: IntArray) {
        super.onRequestPermissionsResult(requestCode, permissions, grantResults)
        permissionResultHandlers[requestCode]?.let { handler ->
            handler(permissions, grantResults)
        } ?: run {
            Log.e("Permissions", "onRequestPermissionsResult requestCode=$requestCode not handled")
        }
    }

    private fun grantBluetoothBasicPermissions(askType: AskType, completion: (Boolean) -> Unit) {
        val wantedPermissions = if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            arrayOf(
                Manifest.permission.BLUETOOTH_CONNECT
            )
        } else {
            emptyArray()
        }

        if (wantedPermissions.isEmpty() || hasPermissions(wantedPermissions)) {
            completion(true)
        } else {
            runOnUiThread {
                val requestCode = BLUETOOTH_PERMISSIONS_REQUEST_CODE

                // set permission result handler
                permissionResultHandlers[requestCode] = { _ /*permissions*/, grantResults ->
                    val isSuccess = grantResults.all { it == PackageManager.PERMISSION_GRANTED }
                    if (isSuccess || askType != AskType.InsistUntilSuccess) {
                        permissionResultHandlers.remove(requestCode)
                        completion(isSuccess)
                    } else {
                        // request again
                        requestPermissionArray(wantedPermissions, requestCode)
                    }
                }

                requestPermissionArray(wantedPermissions, requestCode)
            }
        }
    }

    private fun Context.hasPermissions(permissions: Array<String>): Boolean = permissions.all {
        ActivityCompat.checkSelfPermission(this, it) == PackageManager.PERMISSION_GRANTED
    }

    private fun Activity.requestPermissionArray(permissions: Array<String>, requestCode: Int) {
        ActivityCompat.requestPermissions(this, permissions, requestCode)
    }
    //endregion
}