#include <ArduinoBLE.h>

// UUIDs for the service and characteristic
const char* imuServiceUUID = "5d2af01a-439d-45ca-9dc7-edc49cf2a539";
const char* imuCharUUID = "ea1404e2-5466-4a03-a921-3786d1642e5d";

void setup() {
    // Initialize serial communication
    Serial.begin(115200);
    //while (!Serial);

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
        // Check if the discovered peripheral is the right one
        if (peripheral.localName() == "XIAO BLE Sense") {
            Serial.print("Connecting to peripheral: ");
            Serial.println(peripheral.localName());

            // Stop scanning
            BLE.stopScan();

            // Connect to the peripheral
            if (peripheral.connect()) {
                Serial.println("Connected to peripheral");

                // Discover the service
                if (peripheral.discoverService(imuServiceUUID)) {
                    // Discover the characteristic
                    BLECharacteristic imuChar = peripheral.characteristic(imuCharUUID);

                    if (imuChar) {
                        Serial.println("Found IMU characteristic");

                        // Subscribe to the characteristic to get updates
                        imuChar.subscribe();

                        while (peripheral.connected()) {
                            // Check if new data is available
                            if (imuChar.valueUpdated()) {
                                // Read the characteristic value
                                int length = imuChar.valueLength();
                                char imuData[length + 1];
                                memcpy(imuData, imuChar.value(), length);
                                imuData[length] = '\0';

                                // Print the data to the Serial Monitor
                                Serial.println(imuData);
                            }
                        }

                        Serial.println("Peripheral disconnected");
                    } else {
                        Serial.println("IMU characteristic not found");
                    }
                } else {
                    Serial.println("IMU service not found");
                }

                // Disconnect from the peripheral
                peripheral.disconnect();
            } else {
                Serial.println("Failed to connect to peripheral");
            }

            // Start scanning again
            BLE.scan();
        }
    }
}
