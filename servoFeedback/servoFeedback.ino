#include <Servo.h>

#define SERVO_OUT_PIN 9
#define SERVO_IN_PIN 10

Servo servo;

int angle = 50;
boolean rise = true;
int value;
float avg = 0;

void setup() {
  servo.attach( SERVO_OUT_PIN );
  Serial.begin( 57600 );
}

void loop() {
  if( rise ) {
    angle++;
    if( angle >= 150 ) {
      rise = false;
    }
  }
  else {
    angle--;
    if( angle <= 30 ) {
      rise = true;
    }
  }
  delay( 500 );
  servo.write( angle );
  value = analogRead( SERVO_IN_PIN );
  int prediction = 12 * value - 3111;
  if( prediction > 0 ) {
    avg = .6 * avg + .4 * prediction;
  }
  Serial.print( angle );
  Serial.print(", ");
  Serial.println( avg );
}
