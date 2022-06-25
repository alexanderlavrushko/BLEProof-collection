/*
 * BLEProofPeripheral.cpp
 *
 * Created by Alexander Lavrushko on 25/06/2022.
 *
 * @brief BLEProof Peripheral Nano33
 * Bluetooth Low Energy Peripheral (also called Slave, Server) demo application for Arduino Nano 33 IoT
 * 1. Advertises one service with 3 characteristics:
 *    - characteristic which supports read (BLE Central can only read)
 *    - characteristic which supports write (BLE Central can only write, with response)
 *    - characteristic which supports indication (BLE Central can only subscribe and listen for indications)
 * 2. Provides command line interface for changing values of characteristics:
 *    - use Arduino Serial Monitor with 9600 baud, and option 'Newline' or 'Carriage return' or 'Both'
 */

#include <ArduinoBLE.h>
#include <string>

// --------
// Constants
// --------
#define SERVICE_UUID        "25AE1441-05D3-4C5B-8281-93D4E07420CF"
#define CHAR_READ_UUID      "25AE1442-05D3-4C5B-8281-93D4E07420CF"
#define CHAR_WRITE_UUID     "25AE1443-05D3-4C5B-8281-93D4E07420CF"
#define CHAR_INDICATE_UUID  "25AE1444-05D3-4C5B-8281-93D4E07420CF"

#define CMD_HELP "help"
#define CMD_INFO "info"
#define CMD_SET_READ "setr="
#define CMD_SET_INDICATE "seti="

// --------
// Global variables
// --------
static BLEService g_service(SERVICE_UUID);
static BLEStringCharacteristic g_charRead(CHAR_READ_UUID, BLERead, 512);
static BLEStringCharacteristic g_charWrite(CHAR_WRITE_UUID, BLEWrite, 512);
static BLEStringCharacteristic g_charIndicate(CHAR_INDICATE_UUID, BLEIndicate, 512);
static bool g_isCentralConnected = false;
static std::string g_cmdLine;

// --------
// Application lifecycle: setup & loop
// --------
void setup()
{
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    Serial.begin(9600);
    while (!Serial);

    if (!BLE.begin())
    {
        stopWithError("BLE.begin() failed");
    }
    BLE.setLocalName("Nano33");
    BLE.setAdvertisedService(g_service);

    BLE.setEventHandler(BLEConnected, [](BLEDevice central)
    {
        g_isCentralConnected = true;
        Serial.println("Event: central connected");
    });

    BLE.setEventHandler(BLEDisconnected, [](BLEDevice central)
    {
        g_isCentralConnected = false;
        Serial.println("Event: central disconnected");
    });

    // characteristic for read
    {
        g_service.addCharacteristic(g_charRead);
        g_charRead.writeValue("NANO33 for read");
        g_charRead.setEventHandler(BLERead, [](BLEDevice central, BLECharacteristic characteristic)
        {
            Serial.print("Event: characteristic read, value='");
            Serial.print(g_charRead.value());
            Serial.println("'");
        });
    }

    // characteristic for write
    {
        g_service.addCharacteristic(g_charWrite);
        g_charWrite.setEventHandler(BLEWritten, [](BLEDevice central, BLECharacteristic characteristic)
        {
            Serial.print("Event: characteristic write, value='");
            Serial.print(g_charWrite.value());
            Serial.println("'");
        });
    }

    // characteristic for indicate
    {
        g_service.addCharacteristic(g_charIndicate);
        g_charIndicate.setEventHandler(BLESubscribed, [](BLEDevice central, BLECharacteristic characteristic)
        {
            Serial.println("Event: central subscribed to characteristic");
        });
        g_charIndicate.setEventHandler(BLEUnsubscribed, [](BLEDevice central, BLECharacteristic characteristic)
        {
            Serial.println("Event: central unsubscribed from characteristic");
        });
    }

    BLE.addService(g_service);
    BLE.advertise();

    Serial.println("BLE setup done, advertising...");
    Serial.println("");
    PrintInfo();
    PrintHelp();
}

void loop()
{
    BLE.poll();
    if (!Serial.available())
    {
        return;
    }

    char c = Serial.read();
    if (c != '\r' && c != '\n')
    {
        g_cmdLine += c;
        return;
    }

    std::string cmdLine;
    std::swap(g_cmdLine, cmdLine);
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

    if (ParseCommand(cmdLine, CMD_INFO, commandData))
    {
        PrintInfo();
        return;
    }

    if (ParseCommand(cmdLine, CMD_SET_READ, commandData))
    {
        Serial.print("Setting read characteristic: '");
        Serial.print(commandData.c_str());
        Serial.println("'");
        g_charRead.writeValue(commandData.c_str());
        return;
    }

    if (ParseCommand(cmdLine, CMD_SET_INDICATE, commandData))
    {
        Serial.print("Setting indicate characteristic: '");
        Serial.print(commandData.c_str());
        Serial.println("'");
        g_charIndicate.writeValue(commandData.c_str());
        return;
    }

    Serial.print("ERROR: command not recognized: '");
    Serial.print(cmdLine.c_str());
    Serial.println("'");
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

bool ParseCommand(const std::string& cmdLine, const std::string& commandKeyword, std::string& outputCommandData)
{
    size_t commandPosition = cmdLine.find(commandKeyword);
    if (commandPosition == std::string::npos)
    {
        return false;
    }
    outputCommandData = cmdLine.substr(commandPosition + commandKeyword.length());
    return true;
}

void PrintInfo()
{
    Serial.println("-------------------------------");
    Serial.println("  Service UUID: "SERVICE_UUID);
    Serial.println(g_isCentralConnected ? "  Central connected" : "  Central not connected");
    Serial.println("  Characteristics:");

    Serial.print("  Readable: value='");
    Serial.print(g_charRead.value());
    Serial.println("' UUID="CHAR_READ_UUID);

    Serial.print("  Writeable: value='");
    Serial.print(g_charWrite.value());
    Serial.println("' UUID="CHAR_WRITE_UUID);

    Serial.print("  Indication: value='");
    Serial.print(g_charIndicate.value());
    Serial.println("' UUID="CHAR_INDICATE_UUID);
    Serial.println("-------------------------------");
}

void PrintHelp()
{
    Serial.println("-------------------------------");
    Serial.println("  Command line interface:");
    Serial.println("  1. "CMD_HELP" - print this description of command line interface");
    Serial.println("  2. "CMD_INFO" - print current state of BLE Peripheral");
    Serial.println("  3. "CMD_SET_READ"<value> - set value to readable characteristic");
    Serial.println("       Set 'abc def': "CMD_SET_READ"abc def");
    Serial.println("       Set empty value: "CMD_SET_READ);
    Serial.println("  4. "CMD_SET_INDICATE"<value> - set value to indication characteristic, and send indication to Central");
    Serial.println("       Set 'abc def': "CMD_SET_INDICATE"abc def");
    Serial.println("       Set empty value: "CMD_SET_INDICATE);
    Serial.println("-------------------------------");
    Serial.println("Waiting for command line input...");
}
