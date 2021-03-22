/*
 * BLEProofCentral.cpp
 *
 * Created by Alexander Lavrushko on 22/03/2021.
 *
 * @brief BLEProof Central ESP32
 * Bluetooth Low Energy Central (also called Master, Client) demo application for ESP32
 * 1. Scans nearby advertising devices
 * 2. Automatically connects to the first device advertising the expected service
 * 3. Expects the service to contain such characteristics:
 *    - characteristic which supports read (will be used only for read)
 *    - characteristic which supports write (will be used only for write, with response)
 *    - characteristic which supports indication (will be used only to subscribe and listen for indications)
 * 4. Provides command line interface for reading/writing characteristics:
 *    - use Arduino Serial Monitor with 115200 baud, and option 'Newline' or 'Carriage return' or 'Both'
 */

#include <string>
#include <functional>
#include <memory>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

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

static const int SCAN_DURATION_SEC = 10;

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

typedef std::function<void(BLEAdvertisedDevice device)> DeviceFoundCallback;

// --------
// Forward declarations
// --------
void UpdateState(ELifecycleState newState);

// --------
// Global variables
// --------
static ELifecycleState g_state = ELifecycleState::NotInitialized;
static std::unique_ptr<BLEAdvertisedDevice> g_pDeviceToConnect;
static std::unique_ptr<BLEClient> g_pClient;
static std::unique_ptr<BLEClientCallbacks> g_pClientCallbacks;
static BLERemoteCharacteristic* g_pCharRead = nullptr;
static BLERemoteCharacteristic* g_pCharWrite = nullptr;
static BLERemoteCharacteristic* g_pCharIndicate = nullptr;
static std::string g_cmdLine;
static uint32_t g_scanEndTimeMs = 0;

// --------
// Bluetooth event callbacks
// --------
class MyClientCallbacks : public BLEClientCallbacks
{
    void onConnect(BLEClient* pClient) override
    {
        Serial.println("onConnect");
    }

    void onDisconnect(BLEClient* pClient) override
    {
        Serial.println("onDisconnect");
        UpdateState(ELifecycleState::Disconnected);
    }
};

class MyScanFilterCallbacks: public BLEAdvertisedDeviceCallbacks
{
public:
    explicit MyScanFilterCallbacks(const BLEUUID& wantedServiceUUID, DeviceFoundCallback&& deviceFoundCallback)
    : m_serviceUUID(wantedServiceUUID)
    , m_deviceFoundCallback(deviceFoundCallback)
    {
    }

private:
    // Called for each advertising BLE Server
    void onResult(BLEAdvertisedDevice advertisedDevice) override
    {
//        Serial.print("BLE Advertised device found: ");
//        Serial.println(advertisedDevice.toString().c_str());

        if (advertisedDevice.haveServiceUUID() && advertisedDevice.getServiceUUID().equals(m_serviceUUID))
        {
            m_deviceFoundCallback(advertisedDevice);
        }
    }

private:
    BLEUUID m_serviceUUID;
    DeviceFoundCallback m_deviceFoundCallback;
};

static void NotifyCallback(BLERemoteCharacteristic* pRemoteCharacteristic,
                           uint8_t* pData,
                           size_t length,
                           bool isNotify)
{
    Serial.print("NotifyCallback ");
    Serial.print(isNotify ? "type=notify" : "type=indicate");
    Serial.print(" length=");
    Serial.print(String(length).c_str());
    Serial.print(" value='");
    Serial.print(std::string((char*)pData, length).c_str());
    Serial.println("'");
}

// --------
// Application lifecycle: setup & loop
// --------
void setup()
{
    Serial.begin(115200);
    Serial.println("BLE Central setup started");

    BLEDevice::init("");
    UpdateState(ELifecycleState::Disconnected);

    DeviceFoundCallback onDeviceFound = [](BLEAdvertisedDevice device)
    {
        Serial.println("Advertising data contains our service, will stop scanning and connect");
        device.getScan()->stop();
        g_pDeviceToConnect.reset(new BLEAdvertisedDevice(device));
    };

    BLEScan* pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyScanFilterCallbacks(BLEUUID(SERVICE_UUID), std::move(onDeviceFound)));
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);

    Serial.println("BLE Central setup done");
}

void loop()
{
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
        g_pDeviceToConnect.reset();

        Serial.println("Scanning, expected service UUID: "SERVICE_UUID);
        BLEScan* pBLEScan = BLEDevice::getScan();

        // there are 2 versions of start() function, this one doesn't wait until scan is finished
        if (pBLEScan->start(SCAN_DURATION_SEC, /*finishCallback = */nullptr, /*is_continue = */false))
        {
            g_scanEndTimeMs = millis() + (SCAN_DURATION_SEC + 1) * 1000;
            UpdateState(ELifecycleState::Scanning);
        }
        else
        {
            Serial.println("ERROR: pBLEScan->start failed");
            UpdateState(ELifecycleState::Disconnected);
        }
        return;
    }

    if (g_state == ELifecycleState::Scanning)
    {
        if (g_pDeviceToConnect)
        {
            UpdateState(ELifecycleState::Connecting);
        }
        else if (millis() > g_scanEndTimeMs)
        {
            UpdateState(ELifecycleState::Disconnected);
            Serial.println("Scan finished, device not found, will try again soon");
            delay(3000);
        }
        return;
    }

    if (g_state == ELifecycleState::Connecting)
    {
        g_pClient.reset();
        g_pClientCallbacks.reset();

        Serial.print("Connecting to: ");
        Serial.println(g_pDeviceToConnect->toString().c_str());

        g_pClient.reset(BLEDevice::createClient());
        g_pClientCallbacks.reset(new MyClientCallbacks());
        g_pClient->setClientCallbacks(g_pClientCallbacks.get());
        if (g_pClient->connect(g_pDeviceToConnect.get()))
        {
            UpdateState(ELifecycleState::ConnectedPreparing);
        }
        else
        {
            Serial.println("ERROR: pClient->connect failed");
            UpdateState(ELifecycleState::Disconnected);
        }
        return;
    }

    if (g_state == ELifecycleState::ConnectedPreparing)
    {
        BLERemoteService* pRemoteService = g_pClient->getService(BLEUUID(SERVICE_UUID));
        if (pRemoteService)
        {
            g_pCharRead = pRemoteService->getCharacteristic(BLEUUID(CHAR_READ_UUID));
            if (!g_pCharRead)
            {
                Serial.println("WARN: Characteristic for read not found, UUID: "CHAR_READ_UUID);
            }

            g_pCharWrite = pRemoteService->getCharacteristic(BLEUUID(CHAR_WRITE_UUID));
            if (!g_pCharWrite)
            {
                Serial.println("WARN: Characteristic for write not found, UUID: "CHAR_WRITE_UUID);
            }

            g_pCharIndicate = pRemoteService->getCharacteristic(BLEUUID(CHAR_INDICATE_UUID));
            if (g_pCharIndicate)
            {
                if (g_pCharIndicate->canIndicate())
                {
                    bool notifications = false; // true - notification, false - indication
                    g_pCharIndicate->registerForNotify(NotifyCallback, notifications);
                }
                else
                {
                    Serial.println("WARN: Characteristic doesn't support indication, UUID: "CHAR_INDICATE_UUID);
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
            Serial.println("ERROR: Failed to find our service UUID: "SERVICE_UUID);
            g_pClient->disconnect();
            UpdateState(ELifecycleState::Disconnected);
        }
        return;
    }
}

// --------
// Helper functions
// --------
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
        g_pClient->disconnect();
        return;
    }

    if (ParseCommand(cmdLine, CMD_READ, commandData))
    {
        Serial.print("Reading characteristic: value='");
        std::string value = g_pCharRead->readValue();
        Serial.print(value.c_str());
        Serial.println("'");
        return;
    }

    if (ParseCommand(cmdLine, CMD_WRITE, commandData))
    {
        Serial.print("Writing characteristic: value='");
        Serial.print(commandData.c_str());
        Serial.println("'");
        g_pCharWrite->writeValue(commandData, /*response = */true);
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
