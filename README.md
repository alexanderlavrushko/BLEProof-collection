# BLEProof-collection
Collection of sample Bluetooth Low Energy applications for iOS, Android and ESP32.

Each BLE Central is compatible with each BLE Peripheral, because they use the same service and characteristics UUIDs.

## Table of UUIDs
Name | UUID
----- | ---------------
Service | 25AE1441-05D3-4C5B-8281-93D4E07420CF
Characteristic for read | 25AE1442-05D3-4C5B-8281-93D4E07420CF
Characteristic for write | 25AE1443-05D3-4C5B-8281-93D4E07420CF
Characteristic for indicate | 25AE1444-05D3-4C5B-8281-93D4E07420CF

## BLE Peripheral (all platforms)
Peripheral (also called Slave, Server) works similarly on all platforms:
* Advertises the service with our UUID (see Table of UUIDs)
* Service contains 3 characteristics:
  * for read - has only read permission (see Table of UUIDs)
  * for write - has only write permission (see Table of UUIDs)
  * for indication - supports only indications (see Table of UUIDs)
* Allows a user to change the value of characteristic for read
* Allows a user to change the value of characteristic for indicate
* Allows a user to send an indication to the connected Central

Note 1: technically characteristics can have any amount of permissions (read, write default, write without response, notify, indicate), but in this project each characteristic has only one permission for simplicity.

Note 2: indication is a notification with response - Peripheral notifies, Central confirms that notification received.

## BLE Central (all platforms)
Central (also called Master, Client) works similarly on all platforms:
* Scans for Peripherals which have a service with our UUID (see Table of UUIDs)
* Connects to the first found Peripheral
* Discovers services and characteristics of the connected Peripheral
* Subscribes to indications of the "Characteristic for indicate" (see Table of UUIDs)
* Allows a user to disconnect from the Peripheral
* Allows a user to read a characteristic value from Peripheral
* Allows a user to write a characteristic value to Peripheral
