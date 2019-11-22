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
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, 
OLED_CS);
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
bool processData = false;
bool dispFlag = false;
bool errFlag = false;

#include "ThingSpeak.h"
TCPClient client;
const char * apiKey = "82B7CKNM40K0PCX0";
long channelNum = 788264 ;
bool uploadFlag= false;

int heightOfTank = 230; //in cm

char val[3]; char buf[7]; String data; float level = 0, levelPer =0; 
char levelMark[10];

void setup(){
    
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1);
  Time.zone(+5.5);
  
  ThingSpeak.begin(client);
  display.begin(SSD1306_SWITCHCAPVCC);
  display.clearDisplay();// clears the screen and buffer
  dispLogo();
  wfCing();
  wfCtd();
  
  
  if(WiFi.ready())
  {
      Particle.publish("WiFi Status","Connected Successfully now!" 
,PUBLIC);
  }
  
}

int c=0;
void loop()  //Compatible with nodemcu, if serials are changed 
accordingly
{


    if(Serial1.available())
    {
      c++;
      data = "";
      memset(val, 0, sizeof(val));
      memset(buf, 0, sizeof(buf));
      data = Serial1.readString();
      data.trim();
      data.toCharArray(buf, 8);
      Serial.print("Data buf is: ");
      Serial.println(data);
      processData = true;
    }

    //myString.charAt(4)

    if(processData)  
    {
        procData(); //will return error status <errFlag> - true/false > 
true if received data is okay
        Particle.publish("Level:", String (level) ,PUBLIC); //see device 
events in particle device dashboard
        Particle.publish("Level(%):", String (levelPer) ,PUBLIC);
        dispFlag = true;
    }
    
    if(dispFlag)
    {
        procDisp();
        uploadFlag=true;
        dispFlag = false;
        processData = false;
    }

    if(uploadFlag && !errFlag)
    {
      int x = ThingSpeak.writeFields(channelNum, apiKey);  
      if(x==200)
      {
        Serial.println("Upload Success!");
        Particle.publish("Thingspeak upload","Sts:Success" ,PUBLIC);
      }
      else
      {
        Serial.println("Upload status unknown");
        Particle.publish("Thingspeak upload","Sts:Unknown/Failure" 
,PUBLIC);
      }
      uploadFlag = false;
    }

}


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
}

void dispLogo(void)
{
  display.clearDisplay();
  display.drawBitmap(0, 0, tkm_logo, 128, 64, 1);
  display.display(); // show splashscreen
  delay(6000);
  Serial.println("*************USB OKAY***************");
  display.clearDisplay(); 
}

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
      display.print("Pls check connectivity.\nEnsure the antenna\nis 
attached.");
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
    display.print(c);
    display.display();
}

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
      Serial.print("val is: ");
      Serial.println(val);
    }
    else
    {
      val[0] = 'E';
      val[1] = 'R';
      val[2] = 'R';
      errFlag = true;
    }
    
        level = atof(val);
        level = heightOfTank - level;
        if(level<0)
        {
            level =0;
        }
        levelPer = float ((level/heightOfTank)*100);
        ThingSpeak.setField(1,level);
        Serial.print("level float: ");
        Serial.println(level);
        
        ThingSpeak.setField(2,levelPer);
        Serial.print("level per: ");
        Serial.print(levelPer);
        Serial.println("%");
        
        //Serial.print("\n");
        //memset(levelMark, 0, sizeof(levelMark));
        //for(int i=0; i<=(((float (level)/203))*10) ; i++)
        //{
          //levelMark[i] = '|';
        //}
        return errFlag;
}


