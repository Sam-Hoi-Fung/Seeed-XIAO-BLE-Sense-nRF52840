#include <ArduinoBLE.h>
#include "LSM6DS3.h"
#include "Wire.h"

// Create an instance of class LSM6DS3
LSM6DS3 myIMU(I2C_MODE, 0x6A); // I2C device address 0x6A

// BLE Service
BLEService imuService("5d2af01a-439d-45ca-9dc7-edc49cf2a539"); // Custom UUID for the service

// BLE Characteristic with custom UUID
BLECharacteristic imuChar("ea1404e2-5466-4a03-a921-3786d1642e5d", BLERead | BLENotify, 400); // Custom UUID for IMU data

// Custom function to convert float to string
void floatToString(char* buffer, float value, int places) {
    // Handle negative numbers
    if (value < 0) {
        *buffer++ = '-';
        value = -value;
    }/Users/michaelsam

    // Round value to the specified number of decimal places
    float rounding = 0.5;
    for (int i = 0; i < places; ++i) {
        rounding /= 10.0;
    }
    value += rounding;

    // Extract integer part
    unsigned long intPart = (unsigned long)value;
    float remainder = value - (float)intPart;

    // Convert integer part to string
    itoa(intPart, buffer, 10);
    while (*buffer != '\0') {
        buffer++;
    }

    // Add decimal point
    *buffer++ = '.';

    // Extract fractional part
    while (places-- > 0) {
        remainder *= 10.0;
        int toPrint = int(remainder);
        *buffer++ = toPrint + '0';
        remainder -= toPrint;
    }

    // Null-terminate the string
    *buffer = '\0';
}

void setup() {
    // Initialize serial communication
    Serial.begin(115200);

    // Initialize the IMU
    if (myIMU.begin() != 0) {
        Serial.println("Device error");
    } else {
        Serial.println("Device OK!");
    }

    // Initialize BLE
    if (!BLE.begin()) {
        Serial.println("starting BLE failed!");
        while (1);
    }

    // Set device name and local name
    BLE.setDeviceName("XIAO BLE Sense");
    BLE.setLocalName("XIAO BLE Sense");
    BLE.setAdvertisedService(imuService);

    // Add characteristic to the service
    imuService.addCharacteristic(imuChar);

    // Add service
    BLE.addService(imuService);

    // Start advertising
    BLE.advertise();

    Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
    // Listen for BLE connections
    BLEDevice central = BLE.central();

    // If a central is connected to the peripheral:
    if (central) {
        Serial.print("Connected to central: ");
        Serial.println(central.address());

        // Check the IMU and send data periodically
        while (central.connected()) {
            // Read accelerometer and gyroscope values
            float accelX = myIMU.readFloatAccelX();
            float accelY = myIMU.readFloatAccelY();
            float accelZ = myIMU.readFloatAccelZ();
            float gyroX = myIMU.readFloatGyroX();
            float gyroY = myIMU.readFloatGyroY();
            float gyroZ = myIMU.readFloatGyroZ();

            // Create a message string for the package
            char message[400] = {0};
            char valueStr[20];

            // Add readings to the message
            floatToString(valueStr, accelX, 4);
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "%s, ", valueStr);
            floatToString(valueStr, accelY, 4);
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "%s, ", valueStr);
            floatToString(valueStr, accelZ, 4);
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "%s, ", valueStr);
            floatToString(valueStr, gyroX, 4);
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "%s, ", valueStr);
            floatToString(valueStr, gyroY, 4);
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "%s, ", valueStr);
            floatToString(valueStr, gyroZ, 4);
            snprintf(message + strlen(message), sizeof(message) - strlen(message), "%s ", valueStr);


            // Update BLE characteristic with message string
            imuChar.writeValue(message);

            // Print the message to the Serial Monitor
            Serial.println(message);

            // Delay for 0.1 second to send data at 10 Hz frequency
            delay(100);
        }

        // When the central disconnects:
        Serial.print("Disconnected from central: ");
        Serial.println(central.address());
    }
}