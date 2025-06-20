#include <Arduino.h>


// Change the below values if desired
#define BUTTON_PIN 0
#define RELAY_PIN 25 // Pin to control the relay
#define DEBOUND_INTERVEL 350 // ms

// Read analog input on GPIO33 and print to Serial

const int analogPin = 33;       // ADC1_CH5 on ESP32
const float referenceVoltage = 3.3;  // ESP32 Vref
const int resolution = 4095;        // 12-bit ADC: 0–4095


void setup() {
    Serial.begin(115200);

    // Configure pin for button
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    // Configure pin for relay
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, LOW);

    analogSetPinAttenuation(analogPin, ADC_11db);
}

bool isButtonPressed () {
    int raw = analogRead(analogPin);
    float voltage = (raw / float(resolution)) * referenceVoltage;
    if ( voltage > 2.0) return true;
    return false;
}



void loop() {
       // 5 clicks within the 2 seconds of first click and then no clicks for atleast 3 seconds from the last click and then one last click  after 3 seconds and within 2 secs
    int click_count = 0;
    int first_epoch = 0;
    int second_epoch = 0;
    int third_epoch = 0;
    char str_buff[100];
    while (1) {

        // If the second timer has started and there's a click
        if ( isButtonPressed() && second_epoch > 0) {
            delay(DEBOUND_INTERVEL);
            // Serial.print("IF#1");
            // Check if the click is after 3 seconds other reset
            if (millis() - second_epoch > 3000) {
                // Wait for two seconds to make sure there aren't any clicks
             third_epoch = millis();
                bool unlocked = true;
                while (millis() - third_epoch < 2000) {
                    if ( isButtonPressed()) unlocked = false;
                }

                if (unlocked) {
                    Serial.println("Looking for device"); // Sucessful unlocking
                    click_count =  0;
                    first_epoch =  0;
                    second_epoch = 0;
                    
                    // Enable Bluetooth task
                    // xTaskCreate(bluetoothTask, "bluetooth", 20000, NULL, 5, NULL);// &btTaskHandle);
                    bool device_found = false;
                    // while ( millis() - third_epoch < 50000) {
                        // if () { // Unlock the shit
                            Serial.println("Unlocking the door!!");
                            // Set pin 25 high to unlock the door
                            digitalWrite(RELAY_PIN, HIGH);
                            // device_found = true;
                            // Print the signal strength RSSI of the host device
                            // int rssi = server->getConnId();

                        // }
                    // }
                    // if (!device_found) Serial.println("Device wan't found;");

                    // stopBluetooth();
                    // Set pin 25 low to lock the door
                    delay(2000);
                    digitalWrite(RELAY_PIN, LOW);

                    // Serial.println("Killing BLE");

                 third_epoch = 0;
                }
                else {
                    // Reset stuff and continue
                    Serial.println("Resetting-Button pressed more than once during the 2sec period");
                    first_epoch  = 0;
                    click_count  = 0;
                    second_epoch = 0;
                    third_epoch  = 0;
                    continue;
                }
                
                
            }

            else {
                Serial.println("Resetting-because-button-prssed-before-the-3secs-after-5th-click");
                first_epoch  = 0;
                click_count  = 0;
                second_epoch = 0;
             third_epoch  = 0;
                continue;
            }
        }

        // If it has been more than 7 secs
        if (second_epoch && millis() - first_epoch > 7300) {
            Serial.println("Resetting-7sec-timer-out");
            click_count =  0;
            first_epoch =  0;
            second_epoch = 0;
            third_epoch = 0;
            continue;
        }

        // Reset the timer if there has been less than 5 clicks in the 2 second window from epoch
        if ( click_count > 0 && click_count < 5 && (millis() - first_epoch > 2000) ) {
            // Serial.print("IF#2");
            Serial.println("Resetting-2sec-timer-out");
            click_count =  0;
            first_epoch =  0;
            second_epoch = 0;
            third_epoch = 0;
            continue;
        }

        // First click of the sequence
        if (click_count == 0 &&  isButtonPressed()) {
            // Serial.print("IF#3");
            delay(DEBOUND_INTERVEL); // Debounce 50ms
            Serial.println("Click: 1");
            first_epoch = millis();  // Start a counter for 2 seconds
            click_count++;
            continue;
        }

        // Second to 5th click 
        if (  isButtonPressed() && click_count > 0 && click_count < 5 && first_epoch > 0 ) {
            // Serial.print("IF#4");
            delay(DEBOUND_INTERVEL);
            
            

            if (click_count == 4) {
                second_epoch = millis();
                Serial.println("Starting second epoch");
            }
            
            click_count++;

            sprintf(str_buff, "Click: %d", click_count);
            Serial.println(str_buff);
            
            continue;
        }
    }


    delay(100);
}