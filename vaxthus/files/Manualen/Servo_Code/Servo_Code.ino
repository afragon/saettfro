#include <Servo.h>
Servo myservo;

//Servo variables and constants
const int windowDegreeOpen = 120; //Degrees specified to open the window
const int windowDegreeClosed = 70; //Degrees specified to close the window
int servoPosition = 0;   //Variable to store the servo position
boolean windowOpen = false; //To check if window is closed or open

void setup() {
}

void loop() {
  openWindow();
  delay(2000);
  closeWindow();
  delay(2000);
}

/*
   This method turns the servo up or down
*/
void topos(int gotopos) {
  myservo.attach(9);// Makes the servo ready for use
  delay(3000);
  if (servoPosition <= gotopos) {
    for (servoPosition; servoPosition <= gotopos; servoPosition += 1) { //Turns in steps of 1 degree
      myservo.write(servoPosition);//Tell servo to go to position in variable 'servoPosition'
      delay(100); //Adjusts the speed of the servo when it moves into position
    }
  }
  else {
    for (servoPosition; servoPosition >= gotopos; servoPosition -= 1) { //Turns in steps of 1 degree
      myservo.write(servoPosition); //Tell servo to go to position in variable 'servoPosition'
      delay(100); //Adjusts the speed of the servo when it moves into position
    }
  }
  myservo.detach(); //Detaches servo so it won’t respond to current
}

void openWindow() {
  if (!windowOpen) {
    topos(windowDegreeOpen);
    windowOpen = true;
  }
}

void closeWindow() {
  if (windowOpen) {
    topos(windowDegreeClosed);
    windowOpen = false;
  }
}
