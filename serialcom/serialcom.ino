#include "Mouse.h"

void setup() {
   Serial.begin(9600); //set up serial library baud rate to 9600
   Mouse.begin();
}

void loop() {  
  char arr[3] = { 0, 0, 0 };
  if (Serial.available()) {    
    for (int i = 0; i < 3; i++)
      arr[i] = Serial.read();

    Mouse.move(arr[0], arr[1]);
    Mouse.click(arr[2] ? MOUSE_LEFT : 0);
  }
}