package com.rpt11.bleproofcentral

import android.bluetooth.BluetoothGattCharacteristic
import androidx.appcompat.app.AppCompatActivity
import android.os.Bundle
import android.view.View

class MainActivity : AppCompatActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)
    }

    fun onTapRead(view: View) {
    }

    fun onTapWrite(view: View) {
    }

    fun onTapClearLog(view: View) {
    }
}