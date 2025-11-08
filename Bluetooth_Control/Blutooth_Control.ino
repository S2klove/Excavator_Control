#include <Bluepad32.h>
#include <Arduino.h>

// Define DRV8833 pins for one motor
const int IN1_PIN = 26; // Motor A input 1 (adjust as needed)
const int IN2_PIN = 25; // Motor A input 2 (adjust as needed)

// Setting PWM properties
const int freq = 1000;         // PWM frequency in Hz
const int pwmChannelA = 0;     // PWM channel (0-15)
const int pwmChannelB = 1;     // PWM channel (0-15)
const int resolution = 8;      // PWM resolution (0-255 range)
const int MAX_SPEED = 255;     // Maximum PWM value

// Bluepad32 controller callback function
ControllerPtr myControllers[BP32_MAX_GAMEPADS];

void onConnectedController(ControllerPtr ctl) {
    // Callback when a new controller is connected
    // Use getModelName() instead of name()
    Serial.printf("CONNECTED: %s\n", ctl->getModelName().c_str());
    // Store the controller pointer in an array
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == nullptr) {
            myControllers[i] = ctl;
            break;
        }
    }
}

void onDisconnectedController(ControllerPtr ctl) {
    // Callback when a controller is disconnected
    Serial.printf("DISCONNECTED: %s\n", ctl->getModelName().c_str());
    // Remove the controller pointer from the array
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        if (myControllers[i] == ctl) {
            myControllers[i] = nullptr;
            break;
        }
    }
}

// Function to control the motor speed and direction
void setMotorSpeed(int speed) {
    // Constrain speed to the valid range
    speed = constrain(speed, -MAX_SPEED, MAX_SPEED);

    if (speed == 0) {
        // Stop the motor
        ledcWrite(pwmChannelA, 0);
        ledcWrite(pwmChannelB, 0);
    } else if (speed > 0) {
        // Forward direction (adjust pins/channels if reversed)
        ledcWrite(pwmChannelA, speed);
        ledcWrite(pwmChannelB, 0);
    } else {
        // Reverse direction
        ledcWrite(pwmChannelA, 0);
        ledcWrite(pwmChannelB, abs(speed));
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Starting Bluepad32 motor control...");

    // Setup PWM channels
    ledcSetup(pwmChannelA, freq, resolution);
    ledcAttachPin(IN1_PIN, pwmChannelA);
    ledcSetup(pwmChannelB, freq, resolution);
    ledcAttachPin(IN2_PIN, pwmChannelB);

    // Set up Bluepad32 using the updated callback function names
    BP32.setup(&onConnectedController, &onDisconnectedController);
    // Optional: Forget all keys to force new pairing if needed
    // BP32.forgetBluetoothKeys(); 
}

void loop() {
    // This is required for Bluepad32 to work
    BP32.update();

    // Iterate over all connected controllers
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++) {
        ControllerPtr controller = myControllers[i];
        if (controller && controller->isConnected()) {

            // Get the Y-axis value from the left joystick
            // Range is typically -512 to 511.
            int joy_left_y = controller->axisY();
            
            // Map the joystick value to motor speed (0-255)
            int motorSpeed = map(joy_left_y, -512, 511, -MAX_SPEED, MAX_SPEED);
            
            // Add a deadzone to prevent drift
            if (abs(motorSpeed) < 20) { // Adjust deadzone value as needed
                motorSpeed = 0;
            }

            setMotorSpeed(motorSpeed);
            
            // You can also use buttons for specific actions.
            // Check for specific button presses using the buttons() bitmask or individual functions if available.
            // Many modern controllers map "Start" to button 10 and "Select" to button 9 (0x0200 and 0x0100 in the bitmask)
            if (controller->buttons() & (1 << 9) || controller->buttons() & (1 << 8)) { // Assuming 'select' is 9 and 'start' is 8 (adjust based on your controller mapping if needed)
                 setMotorSpeed(0);
            }
            // A more modern way might be to check misc buttons depending on the controller
            // if (controller->miscButtons() & (1 << 0)) { ... } // Example for specific misc button

            // Optional: Print data for debugging
            // Serial.printf("Motor Speed: %d\n", motorSpeed);
        }
    }
    delay(10); // Small delay to prevent watchdog issues and excessive serial output
}
