#include <ArduinoBLE.h>

// UUIDs for the services and characteristics
const char* imuServiceUUID1 = "5d2af01a-439d-45ca-9dc7-edc49cf2a539";
const char* imuCharUUID1 = "ea1404e2-5466-4a03-a921-3786d1642e5d";
const char* imuServiceUUID2 = "df541fef-d2a9-42b4-a730-8e258cbc97f9";
const char* imuCharUUID2 = "d6b2f500-5868-4404-a2da-a38ce3b3cdd4";

BLEDevice peripheral1;
BLEDevice peripheral2;
BLECharacteristic imuChar1;
BLECharacteristic imuChar2;

unsigned long lastScanTime = 0;
const unsigned long scanInterval = 5000; // 5 seconds

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    //while (!Serial); // Uncomment for native USB boards

    // Initialize BLE
    if (!BLE.begin()) {
        Serial.println("starting BLE failed!");
        while (1);
    }

    Serial.println("BLE Central - IMU Receiver");

    // Start scanning for peripherals
    BLE.scan();
}

void loop() {
    // Check if a peripheral has been discovered
    BLEDevice peripheral = BLE.available();

    if (peripheral) {
        // Check if the discovered peripheral is the right one for the first board
        if (peripheral.localName() == "XIAO BLE Sense" && !peripheral1) {
            connectToPeripheral(peripheral, 1);
        }
        // Check if the discovered peripheral is the right one for the second board
        else if (peripheral.localName() == "XIAO BLE Sense no2" && !peripheral2) {
            connectToPeripheral(peripheral, 2);
        }
    }

    // Check for updates from the first peripheral
    if (peripheral1 && peripheral1.connected()) {
        if (imuChar1.valueUpdated()) {
            // Read the characteristic value
            int length = imuChar1.valueLength();
            char imuData1[length + 1];
            memcpy(imuData1, imuChar1.value(), length);
            imuData1[length] = '\0';

            // Print the data to the Serial Monitor
            Serial.print("NO.1: ");
            Serial.println(imuData1);
        }
    } else if (peripheral1 && !peripheral1.connected()) {
        Serial.println("NO.1 disconnected");
        peripheral1 = BLEDevice();
        BLE.scan(); // Start scanning again
    }

    // Check for updates from the second peripheral
    if (peripheral2 && peripheral2.connected()) {
        if (imuChar2.valueUpdated()) {
            // Read the characteristic value
            int length = imuChar2.valueLength();
            char imuData2[length + 1];
            memcpy(imuData2, imuChar2.value(), length);
            imuData2[length] = '\0';

            // Print the data to the Serial Monitor
            Serial.print("NO.2: ");
            Serial.println(imuData2);
        }
    } else if (peripheral2 && !peripheral2.connected()) {
        Serial.println("NO.2 disconnected");
        peripheral2 = BLEDevice();
        BLE.scan(); // Start scanning again
    }

    // Periodically restart scanning to ensure we reconnect if a device is lost
    if (millis() - lastScanTime > scanInterval) {
        if (!peripheral1 && !peripheral2) {
            BLE.scan(); // Restart scanning if both peripherals are disconnected
            lastScanTime = millis();
        }
    }
}

void connectToPeripheral(BLEDevice peripheral, int num) {
    Serial.print("Connecting to peripheral: ");
    Serial.println(peripheral.localName());

    // Stop scanning
    BLE.stopScan();

    // Connect to the peripheral
    if (peripheral.connect()) {
        Serial.println("Connected to peripheral");

        // Discover the service
        if (num == 1 && peripheral.discoverService(imuServiceUUID1)) {
            // Discover the characteristic
            imuChar1 = peripheral.characteristic(imuCharUUID1);

            if (imuChar1) {
                Serial.println("Found IMU characteristic");
                // Subscribe to the characteristic to get updates
                imuChar1.subscribe();
                peripheral1 = peripheral;
            } else {
                Serial.println("IMU characteristic not found");
                peripheral.disconnect();
            }
        } else if (num == 2 && peripheral.discoverService(imuServiceUUID2)) {
            // Discover the characteristic
            imuChar2 = peripheral.characteristic(imuCharUUID2);

            if (imuChar2) {
                Serial.println("Found IMU characteristic");
                // Subscribe to the characteristic to get updates
                imuChar2.subscribe();
                peripheral2 = peripheral;
            } else {
                Serial.println("IMU characteristic not found");
                peripheral.disconnect();
            }
        } else {
            Serial.println("IMU service not found");
            peripheral.disconnect();
        }
    } else {
        Serial.println("Failed to connect to peripheral");
    }

    // Start scanning again
    BLE.scan();
}

