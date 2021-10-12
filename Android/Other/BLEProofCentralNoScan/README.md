# Android BLEProofCentralNoScan
Android application which acts as BLE Central with minimum permissions: only `android.permission.BLUETOOTH`.
It means the app cannot scan for devices, it can only connect and communicate with already paired devices.

### What it does
* Shows a list of paired devices (BluetoothDevice.DEVICE_TYPE_LE, BluetoothDevice.DEVICE_TYPE_DUAL)
* Connects to the chosen device
* Shows services and characteristics UUIDs of the connected device

Communication like read-write-notify is possible but not implemented, because it's already shown in BLEProofCentral project.

### Screenshots

Device List | Connected
----- | ---------------
![Screenshot Device List](/images/Screenshot-Android-CentralNoScan-List.jpg) | ![Screenshot Connected](/images/Screenshot-Android-CentralNoScan-Connect.jpg)
