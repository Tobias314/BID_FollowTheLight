
#define STATE_DRIVING_STRAIGHT 0
#define STATE_TURNING_LEFT 1
#define STATE_TURINING_RIGHT 2
#define STATE_BIG_TURN_LEFT 3
#define STATE_BIG_TURN_RIGHT 4
#define STATE_BACKWARDS 5

#define NO_CHANGE 0
#define HALF_BRIGHTER 1
#define WHITE_BLACK 2
#define WHITE_GREY 3
#define BLACK_WHITE 4

const int LED_LEFT = 3;
const int LED_RIGHT = 4;

const int THRESHOLD_BRIGHTER = -20;
const float THRESH_WHITE_BLACK = 0.75;
const int CALIBRATE_TIME = 1000;
const int UPDATE_CYCLE = 1000; //gut zum testen, später wahrscheinlich geringer

bool calibrated = false;

long lastBrightnessLeft = 0;
long lastBrightnessRight = 0;
long lastlastBrightnessLeft = 0;
long lastlastBrightnessRight = 0;
long brightnessLeft = 0;
long brightnessRight = 0;
int minBrightnessLeft = 255;
int maxBrightnessLeft = 0;
int minBrightnessRight = 255;
int maxBrightnessRight = 0;

long lastUpdate;

int state = STATE_DRIVING_STRAIGHT;
// 0 ... drive forward
// 1 ... turn left
// 2 ... turn right

int counter = 0;

// the setup function runs once when you press reset or power the board
void setup() {
  lastUpdate = millis();
  // initialize digital pin LED_BUILTIN as an output.
  Serial.begin(9600);
  pinMode(LED_LEFT, OUTPUT);
  pinMode(LED_RIGHT, OUTPUT);
  pinMode(A0, INPUT);
  digitalWrite(A0, HIGH);
  pinMode(A5, INPUT);
  digitalWrite(A5, HIGH);

 lastBrightnessLeft = analogRead(A5);
 lastBrightnessRight = analogRead(A0);
}

// the loop function runs over and over again forever
void loop() {
  
  int tempBrightLeft = analogRead(A5);
  int tempBrightRight = analogRead(A0);
  long currentTime = millis();

  if(!calibrated){
    if(currentTime - lastUpdate < CALIBRATE_TIME){
      state = STATE_DRIVING_STRAIGHT;
      updateMinMaxBrightness(tempBrightLeft, tempBrightRight);
    }else if(currentTime - lastUpdate < 2 * CALIBRATE_TIME){
      state = STATE_BACKWARDS;
      updateMinMaxBrightness(tempBrightLeft, tempBrightRight);
    }else{
      calibrated = true;
      lastUpdate = currentTime;
      state = STATE_DRIVING_STRAIGHT;
    }
  }

 brightnessLeft += tempBrightLeft;
 brightnessRight += tempBrightRight;

  if(currentTime - lastUpdate > UPDATE_CYCLE && calibrated){
    lastUpdate = currentTime;
    brightnessLeft = brightnessLeft / counter;
    brightnessRight = brightnessRight / counter;
    counter = 0;
    Serial.println(state);
    Serial.print(1.0 * brightnessLeft / (maxBrightnessLeft - minBrightnessLeft));
    Serial.println(1.0 * brightnessRight / (maxBrightnessRight - minBrightnessRight));
    
    int transitionLeft = checkTransition(brightnessLeft, lastlastBrightnessLeft, minBrightnessLeft, maxBrightnessLeft);
    int transitionRight = checkTransition(brightnessRight, lastlastBrightnessRight, minBrightnessRight, maxBrightnessRight);
    Serial.println(transitionLeft);
    Serial.println(transitionRight);
    if( transitionLeft != NO_CHANGE || transitionRight != NO_CHANGE){
      lastBrightnessLeft = brightnessLeft;
      lastBrightnessRight = brightnessRight;
      lastlastBrightnessLeft = brightnessLeft;
      lastlastBrightnessRight = brightnessRight;
    } else {
       lastlastBrightnessLeft = lastBrightnessLeft;
      lastlastBrightnessRight = lastBrightnessRight;
      lastBrightnessLeft = brightnessLeft;
      lastBrightnessRight = brightnessRight;
    }
  
    if(state == STATE_DRIVING_STRAIGHT){
      if(transitionLeft == WHITE_BLACK){
        state = STATE_TURNING_LEFT;
      }else if(transitionRight == WHITE_BLACK){
        state = STATE_TURINING_RIGHT;
      }else if(transitionLeft == BLACK_WHITE){
        state = STATE_BIG_TURN_LEFT;
      }else if(transitionRight == BLACK_WHITE){
        state = STATE_BIG_TURN_RIGHT;
      }
    }else if(state == STATE_TURNING_LEFT){
      if(transitionRight == WHITE_BLACK){
        state = STATE_DRIVING_STRAIGHT;
      }
    }else if(state == STATE_TURINING_RIGHT){
      if(transitionLeft == WHITE_BLACK){
        state = STATE_DRIVING_STRAIGHT;
      }
    }else if(state == STATE_BIG_TURN_LEFT){
      if(transitionRight == WHITE_BLACK ){
        state = STATE_DRIVING_STRAIGHT;
      }
    }else if(state == STATE_BIG_TURN_RIGHT){
      if(transitionLeft == WHITE_BLACK){
        state = STATE_DRIVING_STRAIGHT;
      }
    }
    
  }
  counter ++;
  

  /*
  if (lastBrightnessLeft - brightnessLeft > THRESHOLD){
    Serial.println("Uebergang1");
  }
  if (lastBrightnessRight - brightnessRight > THRESHOLD){
    Serial.println("Uebergang2");
  }*/
  
  digitalWrite(LED_LEFT, LOW);
  digitalWrite(LED_RIGHT, LOW);
  if(state == STATE_DRIVING_STRAIGHT){
    digitalWrite(LED_LEFT, HIGH);
    digitalWrite(LED_RIGHT, HIGH);
  }
  if(state == STATE_TURNING_LEFT || state == STATE_BIG_TURN_LEFT){
    digitalWrite(LED_RIGHT, HIGH);
  }
  if(state == STATE_TURINING_RIGHT || state == STATE_BIG_TURN_RIGHT){
    digitalWrite(LED_LEFT, HIGH);    
  }
  if(state == STATE_BACKWARDS){
    digitalWrite(LED_LEFT, 0);
    digitalWrite(LED_RIGHT, 0);
  }
}



// return 1 ... good transition
// return 0 ... no transition
int checkTransition(int brightness, int lastBrightness, int minBrightness, int maxBrightness){
  float diff = brightness - lastBrightness;
  float relativeDiff = diff / (maxBrightness - minBrightness);
   if (relativeDiff > THRESH_WHITE_BLACK){
    Serial.println("white to black");
    //counter ++;
    return WHITE_BLACK;
   }else if(relativeDiff < -THRESH_WHITE_BLACK){
    Serial.println("black to white");
    return BLACK_WHITE;
  }else{
    return NO_CHANGE;
  }
}

void updateMinMaxBrightness(int brightnessLeft, int brightnessRight){
  if(brightnessLeft > maxBrightnessLeft){
    maxBrightnessLeft = brightnessLeft;
  }
  if(brightnessLeft < minBrightnessLeft){
    minBrightnessLeft = brightnessLeft;
  }
  if(brightnessRight > maxBrightnessRight){
    maxBrightnessRight = brightnessRight;
  }
  if(brightnessRight < minBrightnessRight){
    minBrightnessRight = brightnessRight;
  }
}
