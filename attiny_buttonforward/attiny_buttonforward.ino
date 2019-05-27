#include <SoftwareSerial.h>

#define RX    1   // *** D1, Pin 2
#define TX    2   // *** D2, Pin 1 I hope

const int butt1 = 3;     // the number of the pushbutton pin
const int butt2 = 4;     // the number of the LED pin

int butt1state;
int butt2state;

SoftwareSerial Serial(RX, TX);

// variables will change:
void setup() {      
  // initialize the pushbutton pin as an input:
  pinMode(butt1, INPUT);
  pinMode(butt2, INPUT);  
  Serial.begin(9600);
}

void loop(){
  // read the state of the pushbutton value:
  butt1state = digitalRead(butt1);
  butt2state = digitalRead(butt2);
  // check if the pushbutton is pressed.
  // if it is, the buttonState is HIGH:
  if (butt1state == LOW) {     
    // send report    
    Serial.print(1);  
  } 
  if (butt2state == LOW) {     
    // send report    
    Serial.print(0);  
  } 
}
