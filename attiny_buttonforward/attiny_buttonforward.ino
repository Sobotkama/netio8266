#include <SoftwareSerial.h>

#define RX    1   // *** D1, Pin 2
#define TX    2   // *** D2, Pin 1 I hope

#define butt1 4     // the number of the pushbutton pin
#define butt2 3     // the number of the LED pin

bool butt1state;
bool butt2state;
bool butt1pressed;
bool butt2pressed;
SoftwareSerial Serial(RX, TX);

// variables will change:
void setup() {
  // initialize the pushbutton pin as an input:
  pinMode(butt1, INPUT);
  pinMode(butt2, INPUT);
  Serial.begin(9600);
}

void loop() {
  // read the state of the pushbutton value:
  butt1state = digitalRead(butt1);
  butt2state = digitalRead(butt2);
  // check if the pushbuttons are pressed.
  // if it is, the buttonState is LOW:
  if (butt1state == LOW) {
    // send report
    if (butt1pressed == false) {
      Serial.print(1,BIN);
      butt1pressed = true;
    }
  } else if (butt2state == LOW) {
    // send report
    if (butt2pressed == false) {
      Serial.print(0,BIN);
      butt2pressed = true;
    }
  } else {
    butt1pressed = false;
    butt2pressed = false;
  }
}
