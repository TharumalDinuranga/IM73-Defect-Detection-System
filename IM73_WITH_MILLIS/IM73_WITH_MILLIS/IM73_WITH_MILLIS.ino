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

// Configuration parameters
const int velocity = 10000;      // Target constant velocity (in counts/sec)
const int acceleration = 6000; // Acceleration (in counts/sec²)
const int maxVelocity = 10000; // Maximum velocity limit


// Circular buffer for storing trigger times
const int MAX_TRIGGERS = 10; // Maximum number of triggers to store
unsigned long triggerTimes[MAX_TRIGGERS]; // Buffer to store trigger times
int head = 0; // Points to the next insertion point
int tail = 0; // Points to the next trigger to process
bool bufferFull = false;

const unsigned long shotDuration = 50; // Duration of pneumatic shot (in ms)
const unsigned long delayTime = 800;   // Delay before actuation (in ms)
unsigned long solenoidEndTime = 0;     // Tracks when the solenoid should turn off
bool solenoidActive = false;           // Tracks solenoid state
unsigned long lastSensorReadTime = 0;  // Prevents rapid duplicate sensor triggers
const unsigned long debounceTime = 50; // Debounce time for the sensor

void addTrigger(unsigned long time) {
    triggerTimes[head] = time;
    head = (head + 1) % MAX_TRIGGERS;

    if (head == tail) {
        bufferFull = true; // Buffer is full, overwrite the oldest entry
    }
}

bool hasTriggers() {
    return (head != tail) || bufferFull;
}

unsigned long getNextTrigger() {
    unsigned long time = triggerTimes[tail];
    tail = (tail + 1) % MAX_TRIGGERS;
    bufferFull = false; // Reset the buffer full flag after processing a trigger
    return time;
}

unsigned long previousMillis = 0;
int state = 0;

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
    unsigned long currentMillis = millis();

    switch (state) {
        case 0:
            digitalWrite(SUCTION_LEFT, HIGH);
            digitalWrite(SUCTION_RIGHT, HIGH);
            previousMillis = currentMillis;
            state = 1;
            break;

        case 1:
            if (currentMillis - previousMillis >= 1000) {
                digitalWrite(PUSHER_BOTTOM, HIGH);
                digitalWrite(SUCTION_BOTTOM, HIGH);
                digitalWrite(CUPROTATOR_LEFT, HIGH);
                previousMillis = currentMillis;
                state = 2;
            }
            break;

        case 2:
            if (currentMillis - previousMillis >= 600) {
                digitalWrite(SUCTION_LEFT, LOW);
                previousMillis = currentMillis;
                state = 3;
            }
            break;

        case 3:
            if (currentMillis - previousMillis >= 100) {
                digitalWrite(PUSHER_BOTTOM, LOW);
                digitalWrite(CUPROTATOR_LEFT, LOW);
                previousMillis = currentMillis;
                state = 4;
            }
            break;

        case 4:
            if (currentMillis - previousMillis >= 150) {
                digitalWrite(SUCTION_BOTTOM, LOW);
                previousMillis = currentMillis;
                state = 5;
            }
            break;

        case 5:
            if (currentMillis - previousMillis >= 1000) {
                digitalWrite(CUPROTATOR_RIGHT, HIGH);
                digitalWrite(PUSHER_BOTTOM, HIGH);
                digitalWrite(SUCTION_BOTTOM, HIGH);
                previousMillis = currentMillis;
                state = 6;
            }
            break;

        case 6:
            if (currentMillis - previousMillis >= 800) {
                digitalWrite(SUCTION_RIGHT, LOW);
                previousMillis = currentMillis;
                state = 7;
            }
            break;

        case 7:
            if (currentMillis - previousMillis >= 100) {
                digitalWrite(PUSHER_BOTTOM, LOW);
                digitalWrite(CUPROTATOR_RIGHT, LOW);
                previousMillis = currentMillis;
                state = 8;
            }
            break;

        case 8:
            if (currentMillis - previousMillis >= 150) {
                digitalWrite(SUCTION_BOTTOM, LOW);
                previousMillis = currentMillis;
                state = 9;
            }
            break;

        case 9:
            if (currentMillis - previousMillis >= 1000) {
                // Reset to initial state if needed
                state = 0;
            }
            break;
    }


    unsigned long now = millis(); // Get the current time

    // Check if the sensor is triggered (debounce to avoid rapid duplicate triggers)
    if (digitalRead(CLEARCORE_PIN_CCIOA6) == HIGH && (now - lastSensorReadTime >= debounceTime)) {
        addTrigger(now);
        lastSensorReadTime = now; // Update the last sensor read time
    }

    // Process the buffer for pneumatic shots
    if (hasTriggers()) {
        unsigned long nextTriggerTime = triggerTimes[tail];

        // Check if the trigger time + delay has been reached and the solenoid is not active
        if (!solenoidActive && now - nextTriggerTime >= delayTime) {
            getNextTrigger(); // Remove the processed trigger from the buffer

            // Activate the solenoid
            digitalWrite(CLEARCORE_PIN_CCIOA7, HIGH);
            solenoidActive = true; // Mark the solenoid as active
            solenoidEndTime = now + shotDuration; // Set the time to deactivate the solenoid
        }
    }

    // Turn off the solenoid after the shot duration
    if (solenoidActive && now >= solenoidEndTime) {
        digitalWrite(CLEARCORE_PIN_CCIOA7, LOW);
        solenoidActive = false; // Mark the solenoid as inactive
    }
}
