/*tkmtank@gmail.com*/

#define TRIG 10 //Module pins
#define ECHO 11

#include <SoftwareSerial.h>
SoftwareSerial swSer(8, 9); //D8(RX)  ,D9(TX)

float prevDist = 0;
bool firstValue = true;

void setup() {
  Serial.begin(9600); // Serial monitoring
  swSer.begin(115200); // lora
  pinMode(TRIG, OUTPUT); // Initializing Trigger Output and Echo Input
  pinMode(ECHO, INPUT_PULLUP);
  }

  void loop() {

    float distance = 0;


    for (int i=0; i<15; i++)
    {
    digitalWrite(TRIG, LOW); // Set the trigger pin to low for 2uS
    delayMicroseconds(2);

    digitalWrite(TRIG, HIGH); // Send a 10uS high to trigger ranging
    delayMicroseconds(15);

    digitalWrite(TRIG, LOW); // Send pin low again
    float duration = pulseIn(ECHO, HIGH ,26000); // Read in times pulse

    distance += (duration*0.0348)/2; //Convert the pulse duration to distance// speed of sound at 28 deg C = 347.8m/s

    delay(100);
    }

    distance /= 15; //mean
    if(!firstValue)
    {
        if((abs (prevDist - distance) )> 15 && (abs (prevDist - distance) )<20) //error tolerance
        {
          distance = (prevDist + distance)/2;
        }
        else if((abs (prevDist - distance) )> 20) //error tolerance
        {
          distance = prevDist;
        }
    }
    prevDist = distance;
    firstValue = false;


    //distance = 101;
    swSer.print("##");
    swSer.print(int(distance));
    swSer.print("##");

    Serial.print("##");
    Serial.print(int(distance));
    Serial.print("##");

    if( distance > 185 || distance < 50 ) {
        delay(150000); //2.5min
    }

    else {
        delay(240000); //4min
    }

}
