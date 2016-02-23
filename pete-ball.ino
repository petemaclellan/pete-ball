int ledPins[] = {11,10,9,8,7,6,5};
volatile uint8_t ledStatus;
int leftButton = 3;
int rightButton = 2;

volatile int leftEndPoint = -1; //keeps track of how far left we should move the ball
volatile int rightEndPoint = 7; //how far right
const int midPoint = 3;
int currentIndex;
int ledPinsLength = 7;
int ballSpeed = 250;

//Debounce
volatile long leftLastTime = 0;
volatile long rightLastTime = 0;
const long debounceThreshold = ballSpeed + 50;

enum BallDirection {
  right,
  left
};

BallDirection ballDir;

void setup() {
  Serial.begin(9600);
  
  // LED outputs
  for (int i=0; i < ledPinsLength; i++) {
    pinMode(ledPins[i], OUTPUT);
  }

  // Button inputs 
  pinMode(leftButton, INPUT);
  pinMode(rightButton, INPUT);

  attachInterrupt(digitalPinToInterrupt(leftButton), leftPressed, RISING);
  attachInterrupt(digitalPinToInterrupt(rightButton), rightPressed, RISING);

  currentIndex = ledPinsLength / 2;
  ledStatus = 0x01 << (currentIndex);
  updateLeds();

  // determine starting direction
  if (rand() < 0.5) {
    ballDir = left;
  } else {
    ballDir = right;
  }
  delay(200);
}

void loop() {
  if (rightEndPoint == midPoint) {
    
    rightWinSequence();
    
  } else if (leftEndPoint == midPoint) {
    
    leftWinSequence();
    
  } else {

    BallDirection currentDir = ballDir;
    moveBall(); //This will update currentIndex
    updateLeds();
    if (currentDir == ballDir) {
      delay(ballSpeed); //only delay if we haven't just changed direction
    }
  }
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
  Serial.println(ledStatus);
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
}

// ISR for left Button
void leftPressed() {
  long currentTime = millis();
  if (currentTime - leftLastTime < debounceThreshold) {
    return;
  } else {
    leftLastTime = currentTime;
  }
  
  if ((currentIndex > ledPinsLength / 2) || ballDir == right) {
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
  long currentTime = millis();
  if (currentTime - rightLastTime < debounceThreshold) {
    return;
  } else {
    rightLastTime = currentTime;
  }
  
  if (currentIndex < ledPinsLength / 2) {
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

