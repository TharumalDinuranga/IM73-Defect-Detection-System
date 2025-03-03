#include <ClearCore.h>

#define CcioPort ConnectorCOM0
#define Motor ConnectorM3 // Define motor connection
#define baudRate 9600

#define SUCTION_LEFT IO0
#define SUCTION_RIGHT IO1
#define SUCTION_BOTTOM IO2
#define CUPROTATOR_LEFT IO3
#define CUPROTATOR_RIGHT IO4
#define PUSHER_BOTTOM IO5

// // Define IO pins for sensor and pneumatic shot
// #define receiverPin  CLEARCORE_PIN_CCIOA6      // Digital input from the receiver (IO1)
// #define pneumaticPin  CLEARCORE_PIN_CCIOA7     // Digital output for activating the pneumatic shot (IO2)

// Configuration parameters
const int velocity = 10000;    // Target constant velocity (in counts/sec)
const int acceleration = 6000; // Acceleration (in counts/sec²)
const int maxVelocity = 10000; // Maximum velocity limit

// Timing variables
const unsigned long shotDuration = 500; // 500 ms shot duration
unsigned long lastTriggerTime = 0;
unsigned long lastShotTime = 0;
bool isShotActive = false;
bool isSensorTriggered = false;
const unsigned long delayTime = 800;  // 2 seconds delay for pneumatic shot

void InitOutput() {
    pinMode(IO0, OUTPUT);
    pinMode(IO1, OUTPUT);
    pinMode(IO2, OUTPUT);
    pinMode(IO3, OUTPUT);
    pinMode(IO4, OUTPUT);
    pinMode(IO5, OUTPUT);
}

void ExpansionIO() {
    pinMode(CLEARCORE_PIN_CCIOA6, INPUT);
    pinMode(CLEARCORE_PIN_CCIOA7, OUTPUT);
}

void setup() {
    InitOutput();
    ExpansionIO();
    digitalWrite(IO0, LOW);
    digitalWrite(IO1, LOW);
    digitalWrite(IO2, LOW);
    digitalWrite(IO3, LOW);
    digitalWrite(IO4, LOW);
    digitalWrite(IO5, LOW);

    // Initialize the motor manager clocking and mode
    MotorMgr.MotorInputClocking(MotorManager::CLOCK_RATE_LOW);
    MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL, Connector::CPM_MODE_STEP_AND_DIR);

    // Configure motor settings
    Motor.VelMax(maxVelocity);    // Set the maximum velocity
    Motor.AccelMax(acceleration); // Set the acceleration
    Motor.EnableRequest(true);    // Enable the motor driver

    // Wait for the motor to become enabled and ensure it's not in a fault state
    while (Motor.StatusReg().bit.MotorInFault) {
        continue; // Wait until the motor is not in fault
    }

    // Set the target velocity
    Motor.MoveVelocity(velocity);

    // Set up the CCIO-8 COM port.
    CcioPort.Mode(Connector::CCIO);
    CcioPort.PortOpen();
    
}

void loop() {
  //Cup Placer Mechanism

    digitalWrite(SUCTION_LEFT,HIGH);
    digitalWrite(SUCTION_RIGHT,HIGH);
    delay(1000); 
    digitalWrite(PUSHER_BOTTOM,HIGH);
    digitalWrite(SUCTION_BOTTOM,HIGH);
    digitalWrite(CUPROTATOR_LEFT,HIGH);
    delay(600);
    digitalWrite(SUCTION_LEFT,LOW);
    delay(100);
    digitalWrite(PUSHER_BOTTOM,LOW);
    digitalWrite(CUPROTATOR_LEFT,LOW);
    delay(150);
    digitalWrite(SUCTION_BOTTOM,LOW);
    delay(1000);
    digitalWrite(CUPROTATOR_RIGHT,HIGH);
    digitalWrite(PUSHER_BOTTOM,HIGH);
    digitalWrite(SUCTION_BOTTOM,HIGH);
    delay(800);
    digitalWrite(SUCTION_RIGHT,LOW);
    delay(100);
    digitalWrite(PUSHER_BOTTOM,LOW);
    digitalWrite(CUPROTATOR_RIGHT,LOW);
    delay(150);
    digitalWrite(SUCTION_BOTTOM,LOW);
    delay(1000);


    // Check if the sensor is triggered (HIGH state)
    if (digitalRead(CLEARCORE_PIN_CCIOA6) == HIGH) {
        isSensorTriggered = true; // Mark the sensor as triggered
        lastTriggerTime = millis(); // Update the last trigger time
    }

    // If the sensor is triggered and it's time to activate the pneumatic shot
    if (isSensorTriggered && !isShotActive && millis() - lastTriggerTime >= delayTime) {
        // Decide randomly if the pneumatic shot should be activated
        int randomChoice = random(0, 2); // Generate a random number (0 or 1)

        if (randomChoice == 1) {
            // Activate the pneumatic shot
            digitalWrite(CLEARCORE_PIN_CCIOA7, HIGH);
            lastShotTime = millis();
            isShotActive = true; // Start tracking the shot duration
        }

        isSensorTriggered = false; // Reset the sensor trigger flag
    }

    // Handle the pneumatic shot duration non-blockingly
    if (isShotActive && millis() - lastShotTime >= shotDuration) {
        digitalWrite(CLEARCORE_PIN_CCIOA7, LOW); // Deactivate the pneumatic shot
        isShotActive = false; // Reset the shot state
    }
}
