// This #include statement was automatically added by the Particle IDE.
#include <ThingSpeak.h>

// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_SSD1306.h"

// This #include statement was automatically added by the Particle IDE.
#include "Adafruit_GFX.h"

#include "logo.h"


SYSTEM_THREAD(ENABLED);
SYSTEM_MODE(AUTOMATIC);

/* Uncomment this block to use hardware SPI
// If using software SPI (the default case):
#define OLED_MOSI   D0
#define OLED_CLK    D1
#define OLED_DC     D2
#define OLED_CS     D3
#define OLED_RESET  D4
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
*/

// use hardware SPI
//SDA - D12
//SCL - D13
#define OLED_DC     D3
#define OLED_CS     D4 //Leave open D4
#define OLED_RESET  D5
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);


#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2



#define LOGO_HEIGHT 16
#define LOGO_WIDTH  16



#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


/* All flag declarations*/
bool processDtaFlag = false;
bool dispFlag = false;
bool errFlag = false;

/*For alert handler*/
double alertTimeHold = 0;
bool alertFlag = false;
#define ALTR_INTVAL 490

/*Scheduled reboot interval*/
#define REBOOT_INTVAL 21600 //21600 - Every 6 hours Argon will perform a self reboot.


#include "ThingSpeak.h"
TCPClient client;
const char * apiKey = "82B7CKNM40K0PCX0";
long channelNum = 788264 ;
bool uploadFlag= false;

int heightOfTank = 230; //in cm

char val[3]; char buf[7]; String data; float level = 0, levelPer =0; char levelMark[10];

/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
void setup(){

  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1);

  ThingSpeak.begin(client);
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();// clears the screen and buffer
  dispLogo();
  wfCing();
  wfCtd();

  if(WiFi.ready())
  {
      Particle.publish("WiFi Status","Connected Successfully now!" ,PUBLIC);
  }

}

/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
int cnt = 0; int upTime = 0;
void loop()  //Compatible with nodemcu, if serials are changed accordingly
{

    if(Serial1.available() > 0)
    {
      cnt++;
      data = " ";
      memset(val, 0, sizeof(val));
      memset(buf, 0, sizeof(buf));
      data = Serial1.readString();
      data.trim();
      data.toCharArray(buf, 8);
      Serial.print("Data buf is: ");
      Serial.println(data);
      processDtaFlag = true;
    }

    if(processDtaFlag)
    {
        if(!procData()) //will return <errFlag> - true/false : false if received data is expected
        {
            Particle.publish("Level:", String (level) ,PUBLIC); //Upload to particle device log
            Particle.publish("Level(%):", String (levelPer) ,PUBLIC);

            if(sendToTspk()) { //Uploading to thingspeak
                Serial.println("Upload Success!");
                Particle.publish("Thingspeak upload","sts:Success" ,PUBLIC);
            }
            else {
                Serial.println("Upload status unknown");
                Particle.publish("Thingspeak upload","sts:Unknown/Failure" ,PUBLIC);
            }

            procDisp(); //Will process and display values on OLED - returns nothing

            /*Calling handleAlert to handle if any alert condition exists. Returns zero if no condition.
            Check error code in the function for further implementation*/
            if(handleAlerts() == 0) {
               Serial.println("No alert condition.");
            }
            else {
               Serial.println("Alert condition: Check particle web log for details!");
            }

        }

        else
        {
            errFlag = true;
        }

        processDtaFlag = false;
    }

    upTime = System.uptime(); //Returns system uptime in seconds
    if(upTime > REBOOT_INTVAL) //Scheduled Reset - Every 6 hours
    {
       Particle.publish("Scheduled Reboot", "System will reboot now. Bootup time appr. 2 min!" ,PUBLIC);
       System.reset();
    }

}

/*Supporting functions following*/

/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */

void wfCing(void)
{
    int r=0;
    display.clearDisplay();
    display.setCursor(0, 0);
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.println("Wi-Fi");
    //display.setTextSize(1);
    display.println("Connecting\n:)");
    display.display();
    while(!WiFi.ready() && r<200)
    {
      delay(200);
      r++;
    }
    Particle.syncTime();
    waitUntil(Particle.syncTimeDone);
    Time.zone(+5.5);
}
/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
void dispLogo(void)
{
  display.clearDisplay();
  display.drawBitmap(0, 0, tkm_logo, 128, 64, 1);
  display.display(); // show splashscreen
  delay(6000);
  Serial.println("*************USB OKAY***************");
  display.clearDisplay();
}
/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
void wfCtd(void)
{
    int r=0;
    display.clearDisplay();
    if(WiFi.ready())
    {
      Particle.syncTime(); delay(100);
      Particle.process();
      display.clearDisplay();
      display.setCursor(0, 0);
      display.setTextColor(WHITE);
      display.setTextSize(2);
      display.println("Wi-Fi\nConnected");
      display.setTextSize(1);
      display.print("\n");
      display.print("SSID:");
      display.println(WiFi.SSID());
      display.print("IP:");
      display.println(WiFi.localIP());
      display.print("At:");
      display.print(Time.format(Time.now(), "%I:%M:%S%p"));
      display.display();
      delay(5000);
    }
    else
    {
      display.setCursor(0, 0);
      display.setTextColor(WHITE);
      display.clearDisplay();
      display.setTextSize(1.5);
      display.println("Wi-Fi Err.");
      display.setTextSize(1);
      display.print("Pls check connectivity.\nEnsure the antenna\nis attached.");
      display.display();
      while(!WiFi.ready() && r<200)
      {
          delay(200);
          r++;
      }
      if(!WiFi.ready())
      {
      display.setCursor(0, 0);
      display.setTextColor(WHITE);
      display.setTextSize(2);
      display.println("Wi-Fi");
      //display.setTextSize(1);
      display.println("Not Connected\n:\\");
      display.display();
      }
    }
}
/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
void procDisp()
{
    display.clearDisplay();
    display.display();
    display.setCursor(0,0);
    display.setTextSize(2);
    display.println("Level:");
    display.print(levelPer);
    //Serial.print(data);
    if(!errFlag)
    {
      display.print(" %\n");
      errFlag = false;
      //display.println(levelMark);
    }
    else
    {
      display.print("\n");
      //display.println(levelMark);
    }

    if(levelPer>30 && levelPer <70)
    {
      display.println("Level:Med");
    }
    if(levelPer>70)
    {
      display.print("Level:Full"); //dont keep next line \n or ln here
    }
    if(levelPer<30)
    {
      if(levelPer<10)
      {
        display.setTextSize(2);
        display.println("Turn on\nPump");
        Particle.publish("Critical:","Turn On Pump!!" ,PUBLIC);
      }
      else
      {
        display.println("Level:Low");
      }
    }
    if(WiFi.ready())
    {
      display.setTextSize(1);
      display.print("\nWiFi:OK | ");
      Serial.println("WiFi:OK");
    }
    else
    {
      display.setTextSize(1);
      display.print("\nNO-WiFi | ");
      Serial.println("NO-WiFi");
    }
    display.setTextSize(1);
    display.print(level);
    display.print("cm ");
    display.print(cnt);
    display.display();
}
/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
bool procData()
{
    if(data.startsWith("##") && data.endsWith("##"))
    {
      val[0] = char (buf[2]);
      val[1] = char (buf[3]);
      if(isdigit(buf[4]))
      {
       val[2] = char (buf[4]);
      }
      else
      {
        val[2] = ' ';
      }
      Serial.print("Raw distance measured: ");
      Serial.print(atof(val));
      Serial.println(" cm");
      errFlag = false;
    }
    else
    {
      val[0] = 'E';
      val[1] = 'R';
      val[2] = 'R';
      errFlag = true;

      return errFlag;
    }

    level = atof(val); //will take number in the string and convert to float

    level = heightOfTank - level;
    if(level<0)
    {
        level =0;
    }

    levelPer = float ((level/heightOfTank)*100);

    return errFlag;
}
/*
 ********************************************************************
 *
 *
 *
 ********************************************************************
 */
bool sendToTspk()
{

    ThingSpeak.setField(1,level);
    Serial.print("level float: ");
    Serial.println(level);

    ThingSpeak.setField(2,levelPer);
    Serial.print("level per: ");
    Serial.print(levelPer);
    Serial.println("%");

    int x = ThingSpeak.writeFields(channelNum, apiKey);
    if(x==200)
    {
        uploadFlag = false;
        return true;
    }
    else
    {
        uploadFlag = false;
        return false;
    }

}


int handleAlerts()
{

    Particle.syncTime();
    waitUntil(Particle.syncTimeDone);
    Time.zone(+5.5);

    if (abs (alertTimeHold - System.uptime()) > ALTR_INTVAL )
    {
        alertFlag = false;
        alertTimeHold = System.uptime();
    }

    if((levelPer < 25 && levelPer > 10 && !alertFlag))
    {
        alertTimeHold = System.uptime();
        alertFlag = true;
        Particle.publish("message", ( "WATER LOW (25%) - TURN ON PUMP - " + (Time.format(Time.now(), "%b %e %a %I:%M:%S %p")) ) , PRIVATE);
        Particle.publish("email", ( "WATER LOW (25%) - TURN ON PUMP - " + (Time.format(Time.now(), "%b %e %a %I:%M:%S %p")) ), PRIVATE);
        return 1; //alert code 1: first alert given at 25%
    }

    if(levelPer < 10 && !alertFlag )
    {
        alertTimeHold = System.uptime();
        alertFlag = true;
        Particle.publish("message",( "WATER LEVEL CRITICAL (10%) - TURN ON PUMP - " + (Time.format(Time.now(), "%b %e %a %I:%M:%S %p")) ), PRIVATE);
        Particle.publish("email",( "WATER LEVEL CRITICAL (10%) - TURN ON PUMP - " + (Time.format(Time.now(), "%b %e %a %I:%M:%S %p")) ), PRIVATE);
        return 2; //alert code 2: second critical alert given at 10%
    }


    if(levelPer > 25 && levelPer <95 && alertFlag)  //aler clear
    {
        alertFlag = false;
        Particle.publish("Alert Clear", "Water level regained (25%) -  PUMP SEEMS TO BE ON" , PRIVATE);
        return 3; //alert code 3: alert clear : tank level back to  25%
    }

    if(levelPer > 95 && !alertFlag )
    {
        alertTimeHold = System.uptime();
        alertFlag = true;
        Particle.publish("message",( "WATER LEVEL FULL (95%) - TURN OFF PUMP - " + (Time.format(Time.now(), "%b %e %a %I:%M:%S %p")) ), PRIVATE);
        Particle.publish("email", ( "WATER LEVEL FULL (95%) - TURN OFF PUMP - " + (Time.format(Time.now(), "%b %e %a %I:%M:%S %p")) ), PRIVATE);
        return 4; //alert code 4: tank full
    }

    return 0;

}
