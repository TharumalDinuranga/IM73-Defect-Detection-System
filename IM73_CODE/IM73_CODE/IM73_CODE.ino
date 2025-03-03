#include <ClearCore.h>

#define CcioPort ConnectorCOM0
#define Motor ConnectorM3
#define baudRate 9600

#define SUCTION_LEFT IO0
#define SUCTION_RIGHT IO1
#define SUCTION_BOTTOM IO2
#define CUPROTATOR_LEFT IO3
#define CUPROTATOR_RIGHT IO4
#define PUSHER_BOTTOM IO5

#define CONVEYOR2_LIFT CLEARCORE_PIN_CCIOA0
#define ALIGNER_TOP CLEARCORE_PIN_CCIOA1
#define ALIGNER_BOTTOM CLEARCORE_PIN_CCIOA2
#define PNEUMATIC_SHOT CLEARCORE_PIN_CCIOA4
#define PHTELECTRIC_INPUT CLEARCORE_PIN_CCIOA6
#define TRIGGER_INPUT CLEARCORE_PIN_CCIOA7
#define PROXIMITY_SENSOR CLEARCORE_PIN_CCIOA3

const int velocity = 10000;
const int acceleration = 6000;
const int maxVelocity = 10000;

const int MAX_TRIGGERS = 10;
unsigned long triggerTimes[MAX_TRIGGERS];
int head = 0, tail = 0;
bool bufferFull = false;

const unsigned long shotDuration = 50;
const unsigned long delayTime = 900;
unsigned long solenoidEndTime = 0;
bool solenoidActive = false;
unsigned long lastSensorReadTime = 0;
const unsigned long debounceTime = 50;

int acceptedCupCount = 0; // Tracks count of accepted cups
unsigned long previousMillis = 0;
unsigned long previousMillis2 = 0;
const long interval = 1000;
int state = 0;
int step = 10;
bool mechanismTriggered = false; // Flag to track sensor trigger


void addTrigger(unsigned long time) {
    triggerTimes[head] = time;
    head = (head + 1) % MAX_TRIGGERS;
    if (head == tail) bufferFull = true;
}

bool hasTriggers() {
    return (head != tail) || bufferFull;
}

unsigned long getNextTrigger() {
    unsigned long time = triggerTimes[tail];
    tail = (tail + 1) % MAX_TRIGGERS;
    bufferFull = false;
    return time;
}

void InitOutput() {
    pinMode(IO0, OUTPUT);
    pinMode(IO1, OUTPUT);
    pinMode(IO2, OUTPUT);
    pinMode(IO3, OUTPUT);
    pinMode(IO4, OUTPUT);
    pinMode(IO5, OUTPUT);
}

void ExpansionIO() {
    pinMode(CLEARCORE_PIN_CCIOA0, OUTPUT);
    pinMode(CLEARCORE_PIN_CCIOA1, OUTPUT);
    pinMode(CLEARCORE_PIN_CCIOA2, OUTPUT);
    pinMode(CLEARCORE_PIN_CCIOA4, OUTPUT);
    pinMode(CLEARCORE_PIN_CCIOA6, INPUT);
    pinMode(CLEARCORE_PIN_CCIOA3, INPUT);
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
    digitalWrite(CONVEYOR2_LIFT, LOW);
    digitalWrite(ALIGNER_TOP, LOW);
    digitalWrite(ALIGNER_BOTTOM, LOW);
    digitalWrite(PNEUMATIC_SHOT, LOW);



    MotorMgr.MotorInputClocking(MotorManager::CLOCK_RATE_LOW);
    MotorMgr.MotorModeSet(MotorManager::MOTOR_ALL, Connector::CPM_MODE_STEP_AND_DIR);

    Motor.VelMax(maxVelocity);
    Motor.AccelMax(acceleration);
    Motor.EnableRequest(true);

    while (Motor.StatusReg().bit.MotorInFault) continue;

    Motor.MoveVelocity(velocity);

    CcioPort.Mode(Connector::CCIO);
    CcioPort.PortOpen();
}

void runMechanism() {


    // if (loopEnabled) {
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
              state = 0;
          }
          break;
  }
    
    
    // }
} 

void loop() {

  unsigned long currentMillis = millis();
  
  // Detect the rising edge of the proximity sensor
  if (digitalRead(PROXIMITY_SENSOR) == HIGH && !mechanismTriggered) {
      mechanismTriggered = true;  // Set flag when sensor is triggered
      runMechanism();  // Run the mechanism once
  }

  // Reset the flag when the mechanism completes
  if (state == 0) {
      mechanismTriggered = false;
  }
     // Random rejection system
  unsigned long now = millis();
  if (digitalRead(PHTELECTRIC_INPUT) == HIGH && (now - lastSensorReadTime >= debounceTime)) {
      digitalWrite(TRIGGER_INPUT, PHTELECTRIC_INPUT);
      int reject = random(0, 2);
      if (reject == 1) { // Reject the cup
          addTrigger(now);
      } else { // Accept the cup
          acceptedCupCount++;
      }
      lastSensorReadTime = now;
  }

  // Pneumatic shot handling
  if (hasTriggers()) {
      unsigned long nextTriggerTime = triggerTimes[tail];
      if (!solenoidActive && now - nextTriggerTime >= delayTime) {
          getNextTrigger();
          digitalWrite(PNEUMATIC_SHOT, HIGH);
          solenoidActive = true;
          solenoidEndTime = now + shotDuration;
      }
  }

  if (solenoidActive && now >= solenoidEndTime) {
      digitalWrite(PNEUMATIC_SHOT, LOW);
      solenoidActive = false;
  }

  // Step Machine - Runs only when acceptedCupCount reaches 6
  switch (step) {
      case 10:
          digitalWrite(ALIGNER_BOTTOM, HIGH);
          if (acceptedCupCount >= 6) {
              step = 11;
          }
          break;
      case 11:
          if (currentMillis - previousMillis2 >= interval) {
              digitalWrite(ALIGNER_TOP, HIGH);
              previousMillis2 = currentMillis;
              step = 12;
          }
          break;
      case 12:
          if (currentMillis - previousMillis2 >= interval) {
              digitalWrite(ALIGNER_TOP, LOW);
              previousMillis2 = currentMillis;
              step = 13;
          }
          break;
      case 13:
          if (currentMillis - previousMillis2 >= interval) {
              digitalWrite(ALIGNER_BOTTOM, LOW);
              previousMillis2 = currentMillis;
              step = 10;
              acceptedCupCount = 0; // Reset count
          }
        break;
    }
}  // <--- Add this missing closing bracket


