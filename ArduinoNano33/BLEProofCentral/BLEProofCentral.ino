/*
 * BLEProofCentral.cpp
 *
 * Created by Alexander Lavrushko on 03 Jul 2022.
 *
 * @brief BLEProof Central Nano33
 * Bluetooth Low Energy Central (also called Master, Client) demo application for Arduino Nano 33 IoT
 * 1. Scans nearby advertising devices
 * 2. Automatically connects to the first device advertising the expected service
 * 3. Expects the service to contain such characteristics:
 *    - characteristic which supports read (will be used only for read)
 *    - characteristic which supports write (will be used only for write, with response)
 *    - characteristic which supports indication (will be used only to subscribe and listen for indications)
 * 4. Provides command line interface for reading/writing characteristics:
 *    - use Arduino Serial Monitor with 9600 baud, and option 'Newline' or 'Carriage return' or 'Both'
 *   
 * Known issues:
 * 1. Sometimes it takes long time to discover iPhone Peripheral
 * 2. Can take a few minutes to detect disconnection of Android Peripheral
 * 3. Issues when characteristic has empty value (ArduinoBLE library 1.3.1):
 *    - Always fails to write empty value to characteristic
 *    - When reading a value, and it's empty - first time ERROR, then empty value
 *    - When subscribed characteristic is updated, and the new value is empty,
 *      some notifications are NOT received, only every second notification arrives
 */

#include <ArduinoBLE.h>
#include <memory>
#include <string>

// --------
// Constants
// --------
#define SERVICE_UUID        "25AE1441-05D3-4C5B-8281-93D4E07420CF"
#define CHAR_READ_UUID      "25AE1442-05D3-4C5B-8281-93D4E07420CF"
#define CHAR_WRITE_UUID     "25AE1443-05D3-4C5B-8281-93D4E07420CF"
#define CHAR_INDICATE_UUID  "25AE1444-05D3-4C5B-8281-93D4E07420CF"

#define CMD_HELP "help"
#define CMD_DISCONNECT "disc"
#define CMD_READ "read"
#define CMD_WRITE "write="

static constexpr int SCAN_DURATION_SEC = 10;

// --------
// Types
// --------
enum class ELifecycleState
{
    NotInitialized = 0,
    Disconnected,
    Scanning,
    Connecting,
    ConnectedPreparing,
    Connected
};

// --------
// Global variables
// --------
static ELifecycleState g_state = ELifecycleState::NotInitialized;
static std::unique_ptr<BLEDevice> g_pDeviceToConnect;
static std::unique_ptr<BLEDevice> g_pConnectedDevice;
static BLECharacteristic g_charRead;
static BLECharacteristic g_charWrite;
static BLECharacteristic g_charIndicate;
static std::string g_cmdLine;
static uint32_t g_scanEndTimeMs = 0;

// --------
// Application lifecycle: setup & loop
// --------
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(9600);
    while (!Serial);

    UpdateState(ELifecycleState::NotInitialized);
    if (!BLE.begin())
    {
        stopWithError("BLE.begin() failed");
    }
    UpdateState(ELifecycleState::Disconnected);

    BLE.setEventHandler(BLEDisconnected, [](BLEDevice peripheral)
    {
        ResetConnectedResources();
        UpdateState(ELifecycleState::Disconnected);
    });
    BLE.setEventHandler(BLEDiscovered, [](BLEDevice peripheral)
    {
        g_pDeviceToConnect.reset(new BLEDevice(peripheral));
    });

    Serial.println("BLE Central setup done");
    Serial.println("(command line will be available after connection)");
    Serial.println("");
}

void loop()
{
    BLE.poll();

    if (g_state == ELifecycleState::Connected)
    {
        // if connected, read and process command line
        while (Serial.available())
        {
            char c = Serial.read();
            if (c == '\r' || c == '\n')
            {
                std::string cmdLine;
                std::swap(g_cmdLine, cmdLine);
                ProcessCommandLine(cmdLine);
            }
            else
            {
                g_cmdLine += c;
            }
        }
        return;
    }

    if (g_state == ELifecycleState::Disconnected)
    {
        StartScanForMyService();
        return;
    }

    if (g_state == ELifecycleState::Scanning)
    {
        if (g_pDeviceToConnect)
        {
            Serial.println("Discovered device with our service, will stop scanning and connect");
            BLE.stopScan();
            UpdateState(ELifecycleState::Connecting);
        }
        else if (millis() > g_scanEndTimeMs)
        {
            BLE.stopScan();
            UpdateState(ELifecycleState::Disconnected);
            Serial.println("Scan finished, device not found, will try again soon");
            delay(3000);
        }
        return;
    }

    if (g_state == ELifecycleState::Connecting)
    {
        Serial.print("Connecting");
        if (g_pDeviceToConnect->hasLocalName())
        {
            Serial.print(" to '");
            Serial.print(g_pDeviceToConnect->localName());
            Serial.println("'");
        }
        else
        {
            Serial.println();
        }
        
        if (g_pDeviceToConnect->connect())
        {
            g_pConnectedDevice = std::move(g_pDeviceToConnect);
            UpdateState(ELifecycleState::ConnectedPreparing);
        }
        else
        {
            Serial.println("ERROR: connect failed");
            UpdateState(ELifecycleState::Disconnected);
        }
        return;
    }

    if (g_state == ELifecycleState::ConnectedPreparing)
    {
        if (g_pConnectedDevice->discoverService(SERVICE_UUID))
        {
            g_charRead = g_pConnectedDevice->characteristic(CHAR_READ_UUID);
            if (!g_charRead)
            {
                Serial.println("WARN: Characteristic for read not found, UUID: "CHAR_READ_UUID);
            }

            g_charWrite = g_pConnectedDevice->characteristic(CHAR_WRITE_UUID);
            if (!g_charWrite)
            {
                Serial.println("WARN: Characteristic for write not found, UUID: "CHAR_WRITE_UUID);
            }

            g_charIndicate = g_pConnectedDevice->characteristic(CHAR_INDICATE_UUID);
            if (g_charIndicate)
            {
                if (g_charIndicate.subscribe())
                {
                    g_charIndicate.setEventHandler(BLEUpdated, [](BLEDevice device, BLECharacteristic characteristic)
                    {
                        std::string value = GetCurrentValueOfCharacteristic(g_charIndicate);
                        Serial.print("Subscribed characteristic updated: '");
                        Serial.print(value.c_str());
                        Serial.println("'");
                    });
                }
                else
                {
                    Serial.println("WARN: Failed to subscribe for indication, UUID: "CHAR_INDICATE_UUID);
                }
            }
            else
            {
                Serial.println("WARN: Characteristic for indicate not found, UUID: "CHAR_INDICATE_UUID);
            }

            g_cmdLine.clear();
            UpdateState(ELifecycleState::Connected);
            PrintHelp();
        }
        else
        {
            Serial.println("ERROR: discover service failed "SERVICE_UUID);
            UpdateState(ELifecycleState::Disconnected);
        }
        return;
    }
}

// --------
// Helper functions
// --------
void errorBlink()
{
    for (int i = 0; i < 3; ++i)
    {
        digitalWrite(LED_BUILTIN, HIGH);
        delay(100);
        digitalWrite(LED_BUILTIN, LOW);
        delay(100);
    }
}

void stopWithError(const char* errorStr)
{
    Serial.println("");
    Serial.print("ERROR: ");
    Serial.println(errorStr);
    for (;;)
    {
        errorBlink();
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
    }
}

void ResetConnectedResources()
{
    g_pDeviceToConnect.reset();
    g_pConnectedDevice.reset();
    g_charRead = BLECharacteristic();
    g_charWrite = BLECharacteristic();
    g_charIndicate = BLECharacteristic();
}

void StartScanForMyService()
{
    Serial.println("Scanning, expected service UUID: "SERVICE_UUID);
    UpdateState(ELifecycleState::Scanning);
    BLE.scanForUuid(SERVICE_UUID);
    Serial.println("Scanning...");

    // Experience shows, that if one scanning takes more than a few minutes,
    // it will never detect new devices (I was waiting 5 minutes), even if they are really advertising.
    // So let's request a NEW scan once in a while.
    g_scanEndTimeMs = millis() + (SCAN_DURATION_SEC + 1) * 1000;
}

std::string GetCurrentValueOfCharacteristic(BLECharacteristic characteristic)
{
    const int valueLength = characteristic.valueLength();
    const char* rawValue = (const char*)characteristic.value();
    return std::string(rawValue, rawValue + valueLength);
}

void PrintState(ELifecycleState state)
{
    Serial.print("state = ");
    switch (state)
    {
        case ELifecycleState::NotInitialized:     Serial.print("NotInitialized"); break;
        case ELifecycleState::Disconnected:       Serial.print("Disconnected"); break;
        case ELifecycleState::Scanning:           Serial.print("Scanning"); break;
        case ELifecycleState::Connecting:         Serial.print("Connecting"); break;
        case ELifecycleState::ConnectedPreparing: Serial.print("ConnectedPreparing"); break;
        case ELifecycleState::Connected:          Serial.print("Connected"); break;
        default: Serial.print(String((int)g_state).c_str()); break;
    }
    Serial.println("");
}

void UpdateState(ELifecycleState newState)
{
    PrintState(newState);
    g_state = newState;
}

void ProcessCommandLine(const std::string& cmdLine)
{
    if (cmdLine.empty())
    {
        return;
    }

    std::string commandData;
    if (ParseCommand(cmdLine, CMD_HELP, commandData))
    {
        PrintHelp();
        return;
    }

    if (ParseCommand(cmdLine, CMD_DISCONNECT, commandData))
    {
        Serial.println("Disconnecting because of command");
        g_pConnectedDevice->disconnect();
        return;
    }

    if (ParseCommand(cmdLine, CMD_READ, commandData))
    {
        Serial.print("Reading characteristic: ");
        if (g_charRead.read())
        {
            std::string value = GetCurrentValueOfCharacteristic(g_charRead);
            Serial.print("value='");
            Serial.print(value.c_str());
            Serial.println("'");
        }
        else
        {
            Serial.println("ERROR");
        }
        return;
    }

    if (ParseCommand(cmdLine, CMD_WRITE, commandData))
    {
        Serial.print("Writing characteristic: value='");
        Serial.print(commandData.c_str());
        Serial.print("'");
        const int writeResult = g_charWrite.writeValue(commandData.c_str(), commandData.length());
        if (writeResult == 1)
        {
            Serial.println(" - OK");
        }
        else if (writeResult == 0)
        {
            if (commandData.length() == 0)
            {
                Serial.println(" - ERROR (bug in ArduinoBLE 1.3.1, fails to write zero length)");
            }
            else
            {
                Serial.println(" - ERROR");
            }
        }
        else
        {
            Serial.print(" - unknown result ");
            Serial.println(writeResult);
        }
        return;
    }

    Serial.print("ERROR: command not recognized: '");
    Serial.print(cmdLine.c_str());
    Serial.println("'");
}

bool ParseCommand(const std::string& cmdLine, const std::string& commandKeyword, std::string& commandData)
{
    size_t commandPosition = cmdLine.find(commandKeyword);
    if (commandPosition == std::string::npos)
    {
        return false;
    }
    commandData = cmdLine.substr(commandPosition + commandKeyword.length());
    return true;
}

void PrintHelp()
{
    Serial.println("-------------------------------");
    Serial.println("  Command line interface:");
    Serial.println("  1. "CMD_HELP" - print this description of command line interface");
    Serial.println("  2. "CMD_DISCONNECT" - disconnect and start scanning for new devices");
    Serial.println("  3. "CMD_READ" - read value from readable characteristic");
    Serial.println("  4. "CMD_WRITE"<value> - set value to writeable characteristic");
    Serial.println("       Set 'abc def': "CMD_WRITE"abc def");
    Serial.println("       Set empty value: "CMD_WRITE);
    Serial.println("-------------------------------");
    Serial.println("Waiting for command line input...");
}
