enum BallDirection {
  right,
  left
};

// pin definitions
int ledPins[] = {11,10,9,8,7,6,5};
int leftButton = 3;
int rightButton = 2;

volatile uint8_t ledStatus; // tracks (bitwise) which LEDs should be lit
volatile int leftEndPoint = -1; //keeps track of how far left we should move the ball
volatile int rightEndPoint = 7; //how far right
const int midPoint = 3;

int currentIndex; // where the ball is currently
int ledPinsLength = 7;
int ballSpeed = 250; // delay in ms (changed by potentiometer)

//Debounce
volatile long leftLastTime = 0;
volatile long rightLastTime = 0;
const long debounceThreshold = ballSpeed + 50;

BallDirection ballDir; // tracks which direction the "ball" (light) is moving currently

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup() {
  Serial.begin(9600);
  
  // LED outputs
  for (int i=0; i < ledPinsLength; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  // Button inputs 
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);

  // button interrupt definitions
  attachInterrupt(digitalPinToInterrupt(leftButton), leftPressed, RISING);
  attachInterrupt(digitalPinToInterrupt(rightButton), rightPressed, RISING);

  currentIndex = ledPinsLength / 2; // start in the middle
  ledStatus = 0x01 << (currentIndex); // set the middle LED to on
  updateLeds();

  // if analog input pin 0 is unconnected, random analog
  // noise will cause the call to randomSeed() to generate
  // different seed numbers each time the sketch runs.
  // randomSeed() will then shuffle the random function.
  randomSeed(analogRead(0));
  
  // determine starting direction randomly
  if (random(0, 100) < 50) {
    ballDir = left;
  } else {
    ballDir = right;
  }
  delay(1500); // for dramatic tension
}

void loop() {
  if (rightEndPoint == midPoint) {
    
    rightWinSequence();
    
  } else if (leftEndPoint == midPoint) {
    
    leftWinSequence();
    
  } else {

    BallDirection currentDir = ballDir; // to reference against for detecting direction change
    ballSpeed = getSpeedFromKnob();
    moveBall(); //This will update currentIndex
    updateLeds();
    if (currentDir == ballDir) {
      delay(ballSpeed); //only delay if we haven't just changed direction
    }
  }
}

int getSpeedFromKnob() {
  return (int) map(analogRead(A1), 0, 1024, 500, 15);
}

void moveBall() {
  // clear the current LED position
  ledStatus &= ~(0x01 << (currentIndex));
  
  if (ballDir == right) {
    //if out of bounds
    if (++currentIndex == rightEndPoint) {
      ballDir = left;
      --currentIndex; //start moving back the other way
    }
    
  } else if (ballDir == left) {
    //if out of bounds
    if (--currentIndex == leftEndPoint) {
      ballDir = right;
      ++currentIndex; //start moving back the other way
    }
  }

  ledStatus |= (0x01 << (currentIndex));
}

void updateLeds() {
  // clear all LEDs first
  for (int i = 0; i < ledPinsLength; i++) {
    digitalWrite(ledPins[i], LOW);
  }
  // Light the proper ones
  for (int j = 0; j < ledPinsLength; j++) {
    if (ledStatus & (0x01 << j)) {
      digitalWrite(ledPins[j], HIGH);
    }
  }
  
}

void rightWinSequence() {
  // flash right lights five times
  for (int n = 0; n < 5; n++) {
    
    for (int i=midPoint; i < ledPinsLength; i++) {
        digitalWrite(ledPins[i], LOW);
      }
      delay(100);
      for (int i=midPoint; i < ledPinsLength; i++) {
        digitalWrite(ledPins[i], HIGH);
      }
      delay(200);
      
  }

  resetFunc();
}

void leftWinSequence() {
  // flash right lights five times
  for (int n = 0; n < 5; n++) {
    
    for (int i=0; i < midPoint + 1; i++) {
        digitalWrite(ledPins[i], LOW);
      }
      delay(100);
      for (int i=0; i < midPoint + 1; i++) {
        digitalWrite(ledPins[i], HIGH);
      }
      delay(200);
      
  }
  
  resetFunc();
}

// ISR for left Button
void leftPressed() {
  // debounce logic
  long currentTime = millis();
  if (currentTime - leftLastTime < debounceThreshold) {
    return;
  } else {
    leftLastTime = currentTime;
  }
  
  if ((currentIndex > ledPinsLength / 2) || ballDir == right) {
    // only allow capture attempt when ball is on your side and moving towards your end point
    return;
  } else {
    if ((currentIndex - 1) == leftEndPoint) {
      ledStatus |= (0x01 << currentIndex); //keep this point lit
      //digitalWrite(ledPins[currentIndex], HIGH);
      ++leftEndPoint;
      ++currentIndex;
      ballDir = right;
    }
  }
}

// ISR for right Button
void rightPressed() {
  // debounce logic
  long currentTime = millis();
  if (currentTime - rightLastTime < debounceThreshold) {
    return;
  } else {
    rightLastTime = currentTime;
  }
  
  if ((currentIndex < ledPinsLength / 2) || ballDir == left) {
    // only allow capture attempt when ball is on your side and moving towards your end point
    return;
  } else {
    if ((currentIndex + 1) == rightEndPoint) {
      ledStatus |= (0x01 << currentIndex); //keep this point lit
      //digitalWrite(ledPins[currentIndex], HIGH);
      --rightEndPoint;
      --currentIndex;
      ballDir = left;
    }
  }
}

