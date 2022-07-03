# BLEProof-collection
Collection of sample Bluetooth Low Energy applications for iOS, Android, ESP32 and Arduino Nano 33 IoT.

### What they do
The apps implement basic BLE communication:
* Central scans for Peripheral and connects to it
* Central reads a value from Peripheral
* Central writes a value to Peripheral
* Peripheral notifies Central that a value has changed

Each BLE Central is compatible with each BLE Peripheral, because they use the same service and characteristics UUIDs.

### Screenshots

Android BLE Central | iOS BLE Peripheral
----- | ---------------
![Screenshot Android Central](/images/Screenshot-Android-Central.jpg) | ![Screenshot iOS Peripheral](/images/Screenshot-iOS-Peripheral.jpg)

#### ESP32 BLE Peripheral (Arduino Serial Monitor)

![Screenshot of ESP32 BLEProofPeripheral](/images/Screenshot-ESP32-Peripheral.png) 

### How to run
To run and see it working, you need 2 physical devices supporting Bluetooth Low Energy:
* one device for BLE Central app - Android, iOS, ESP32 or [another supported device](#supported-devices)
* another device for BLE Peripheral app - Android, iOS, ESP32 or [another supported device](#supported-devices)

...and some development tools:
* Android Studio - for Android project
* Xcode - for iOS project
* Arduino IDE [with ESP32 board installed](https://randomnerdtutorials.com/installing-the-esp32-board-in-arduino-ide-windows-instructions/) - for ESP32
* Arduino IDE with ArduinoBLE library - [for Arduino Nano 33 IoT](https://www.arduino.cc/en/Guide/NANO33IoT)

## Supported devices
Platform | Peripheral | Central | Notes
----- | ----- | ----- | -----
Android | YES | YES | Android 5.0 and newer, including Android 12
iOS | YES | YES | iOS 11.0 and newer
ESP32 | YES | YES |  |
Arduino Nano 33 | YES | YES | Tested on Arduino Nano 33 IoT, but should work on any board compatible with ArduinoBLE library

## Table of UUIDs
Name | UUID
----- | ---------------
Service | 25AE1441-05D3-4C5B-8281-93D4E07420CF
Characteristic for read | 25AE1442-05D3-4C5B-8281-93D4E07420CF
Characteristic for write | 25AE1443-05D3-4C5B-8281-93D4E07420CF
Characteristic for indicate | 25AE1444-05D3-4C5B-8281-93D4E07420CF

## BLE Peripheral (all platforms)
Peripheral (also called Slave or Server) works similarly on all platforms:
* Advertises a service with our UUID (see Table of UUIDs)
* The service contains 3 characteristics:
  * for read - has only read permission (see Table of UUIDs)
  * for write - has only write permission (see Table of UUIDs)
  * for indication - supports only indications (see Table of UUIDs)
* Allows the user to change the string value of the characteristic for read
* Allows the user to change the string value of the characteristic for indicate
* Allows the user to send an indication with updated string value to the connected Central

Note 1: technically characteristics can have any amount of permissions (read, write default, write without response, notify, indicate), but in this project each characteristic has only one permission for simplicity.

Note 2: indication is a notification with response - Peripheral notifies, Central confirms that notification received.

## BLE Central (all platforms)
Central (also called Master or Client) works similarly on all platforms:
* Scans for Peripherals which have a service with our UUID (see Table of UUIDs)
* Connects to the first found Peripheral
* Discovers services and characteristics of the connected Peripheral
* Subscribes to indications of the "Characteristic for indicate" (see Table of UUIDs)
* Allows the user to disconnect from Peripheral
* Allows the user to read a characteristic value (string) from Peripheral
* Allows the user to write a characteristic value (string) to Peripheral

## Recommended resources
Android BLE (usually articles cover only BLE Central role):
* [The Ultimate Guide to Android Bluetooth Low Energy](https://punchthrough.com/android-ble-guide/) by PunchThrough
* [Making Android BLE work](https://medium.com/@martijn.van.welie/making-android-ble-work-part-1-a736dcd53b02) by Martijn van Welie
* [(Talk) Bluetooth Low Energy On Android](https://www.stkent.com/2017/09/18/ble-on-android.html) by Stuart Kent

Android BLE Peripheral role (less popular topic):
* [How to Advertise Android as a Bluetooth LE Peripheral](https://code.tutsplus.com/tutorials/how-to-advertise-android-as-a-bluetooth-le-peripheral--cms-25426) by Paul Trebilcox-Ruiz
* [Bluetooth GATT Server Sample](https://github.com/androidthings/sample-bluetooth-le-gattserver) by Android Things

iOS:
* [Transferring Data Between Bluetooth Low Energy Devices](https://developer.apple.com/documentation/corebluetooth/transferring_data_between_bluetooth_low_energy_devices) by Apple (with sample code, Central and Peripheral)
* [The Ultimate Guide to Apple’s Core Bluetooth](https://punchthrough.com/core-bluetooth-basics/) by PunchThrough

## BLE utility apps
LightBlue: [for iOS](https://apps.apple.com/us/app/lightblue/id557428110#?platform=iphone), [for Android](https://play.google.com/store/apps/details?&hl=en&id=com.punchthrough.lightblueexplorer)

BLE Scanner: [for iOS](https://apps.apple.com/us/app/ble-scanner-4-0/id1221763603#?platform=iphone), [for Android](https://play.google.com/store/apps/details?hl=en&id=com.macdom.ble.blescanner)
