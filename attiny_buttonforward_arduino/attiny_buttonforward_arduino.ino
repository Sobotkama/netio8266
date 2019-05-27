#include <SoftwareSerial.h>

#define RX    11   // *** D1, Pin 2
#define TX    10   // *** D2, Pin 1 I hope

const int butt1 = 7;     // the number of the pushbutton pin
const int butt2 = 6;     // the number of the LED pin

int butt1state;
int butt2state;
bool butt1pressed;
bool butt2pressed;
SoftwareSerial Serial1(RX, TX);

// variables will change:
void setup() {
  // initialize the pushbutton pin as an input:
  pinMode(butt1, INPUT);
  pinMode(butt2, INPUT);
  Serial1.begin(9600);
  Serial.begin(9600);
}

void loop() {
  // read the state of the pushbutton value:
  butt1state = digitalRead(butt1);
  butt2state = digitalRead(butt2);
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (butt1state == LOW) {
    // send report
    if (butt1pressed == false) {
      Serial.print(1);
      Serial1.print(1,BIN);
      butt1pressed = true;
    }
  } else if (butt2state == LOW) {
    // send report
    if (butt2pressed == false) {
      Serial.print(0);
      Serial1.print(0,BIN);
      butt2pressed = true;
    }
  } else {
    butt1pressed = false;
    butt2pressed = false;
  }
}
