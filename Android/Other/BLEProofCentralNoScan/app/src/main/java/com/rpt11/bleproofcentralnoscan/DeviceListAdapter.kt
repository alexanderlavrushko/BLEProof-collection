package com.rpt11.bleproofcentralnoscan

import android.annotation.SuppressLint
import android.bluetooth.BluetoothDevice
import android.view.LayoutInflater
import android.view.View
import android.view.ViewGroup
import android.widget.TextView
import androidx.recyclerview.widget.RecyclerView

class DeviceListAdapter(
    private val devices: ArrayList<BluetoothDevice>,
    onClickHandler: (BluetoothDevice) -> Unit
) : RecyclerView.Adapter<DeviceListAdapter.ViewHolder>() {

    interface OnClickListener {
        fun onClick(device: BluetoothDevice)
    }
    private val onClickListener: OnClickListener

    init {
        onClickListener = object: OnClickListener {
            override fun onClick(device: BluetoothDevice) {
                onClickHandler(device)
            }
        }
    }

    @SuppressLint("NotifyDataSetChanged")
    fun updateList(newDevices: ArrayList<BluetoothDevice>) {
        devices.clear()
        devices.addAll(newDevices)
        notifyDataSetChanged()
    }

    inner class ViewHolder(itemView: View, private val onClickListener: OnClickListener) : RecyclerView.ViewHolder(itemView) {
        private var device: BluetoothDevice? = null
        private val textViewTitle: TextView
            get() = itemView.findViewById(R.id.textViewDeviceTitle)
        private val textViewAddress: TextView
            get() = itemView.findViewById(R.id.textViewDeviceAddress)

        init {
            super.itemView.setOnClickListener {
                device?.let {
                    onClickListener.onClick(it)
                }
            }
        }

        @SuppressLint("SetTextI18n")
        fun bindDevice(device: BluetoothDevice?) {
            this.device = device
            if (device == null) {
                textViewTitle.text = "<null>"
                textViewAddress.text = "Address: <null>"
            } else {
                textViewTitle.text = device.name ?: "<no name>"
                textViewAddress.text = "Address: ${device.address ?: "<null>"}"
            }
        }
    }

    override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): ViewHolder {
        val view = LayoutInflater.from(parent.context).inflate(R.layout.device_list_item, parent, false)
        return ViewHolder(view, onClickListener)
    }

    override fun onBindViewHolder(holder: ViewHolder, position: Int) {
        holder.bindDevice(devices[position])
    }

    override fun getItemCount(): Int {
        return devices.size
    }
}