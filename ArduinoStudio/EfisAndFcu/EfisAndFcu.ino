/*
This is only a Test and has to be set in an usable state
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include "Fonts/FreeSans8pt7b.h"
#include "Fonts/FreeSans7pt7b.h"
#include "Fonts/FreeSans6pt7b.h"
#include "Fonts/DSEG7Classic_Regular14pt7b.h"
#include "Fonts/DSEG7Classic_Regular15pt7b.h"
#include "Fonts/DSEG7Classic_Regular16pt7b.h"
#include "Fonts/DSEG7Classic_Regular18pt7b.h"
#include "Fonts/DSEG7Classic_Regular20pt7b.h"  //https://github.com/keshikan/DSEG and https://rop.nl/truetype2gfx/
#include "Fonts/DSEG7Classic_Regular22pt7b.h"

// Address and communication for Mobiflight
#define I2C_MOBIFLIGHT_ADDR 0x27
#define I2C_MOBIFLIGHT_SDA 21
#define I2C_MOBIFLIGHT_SCL 22

// I2c ports for communication with displays
#define I2C_DISPLAY_SDA 17
#define I2C_DISPLAY_SCL 16

#define SCREEN_WIDTH 128    // OLED display width, in pixels
#define SCREEN_HEIGHT 64    // OLED display height, in pixels
#define OLED_RESET     -1   // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3c // address of the displays. All displays uses the same address

// address of the multiplexer to change the channels
#define TCA9548A_I2C_ADDRESS  0x70
#define TCA9548A_CHANNEL_EFIS_LEFT  0
#define TCA9548A_CHANNEL_EFIS_RIGHT 1
#define TCA9548A_CHANNEL_FCU_SPD    2
#define TCA9548A_CHANNEL_FCU_HDG    3
#define TCA9548A_CHANNEL_FCU_FPA    4
#define TCA9548A_CHANNEL_FCU_ALT    5
#define TCA9548A_CHANNEL_FCU_VS     6
#define TCA9548A_CHANNEL_UNUSED     7

TwoWire I2Ctwo = TwoWire(1);  // init second i2c bus

// Efis displays
Adafruit_SSD1306 dEfis(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);

// fcu displays
Adafruit_SH1106G dFcu = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);


// Efis left
char efisLeftBaroMode = 0x00;
char efisLeftBaroSelect = 0x00;
String efisLeftBaroValueHpa = "1013";
String efisLeftBaroValueHg = "2992";

// Efis right
char efisRightBaroMode = 0x00;
char efisRightBaroSelect = 0x00;
String efisRightBaroValueHpa = "1013";
String efisRightBaroValueHg = "2992";

// FCU Speed
char fcuSpeedManagedMode = 0x00;
String fcuSpeedValue = "0.26";

// FCU Hdg
char fcuHdgManagedMode = 0x00;
String fcuHdgValue = "000";

// FCU Trk Mode
char fcuTrkMode = 0x00;

// FCU Alt
char fcuAltManagedMode = 0x00;
String fcuAltValue = "00000";

// FCU VS
char fcuVsManagedMode = 0x00;
String fcuVsValue = "00000";
String fcuVsValueFpa = "-0.0";

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  // setup i2c receive callback
  Wire.onReceive(onReceive);

  // init i2c busses
  Wire.begin((uint8_t)I2C_MOBIFLIGHT_ADDR,I2C_MOBIFLIGHT_SDA,I2C_MOBIFLIGHT_SCL,400000);
  I2Ctwo.begin(I2C_DISPLAY_SDA,I2C_DISPLAY_SCL,400000); // SDA pin , SCL pin , 400kHz frequency
  
  // //**************************
  // // Efis left
  // //**************************
  setTCAChannel(TCA9548A_CHANNEL_EFIS_LEFT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!dEfis.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  dEfis.display();
  updateDisplayEfisLeft();

  //**************************
  // Efis right
  //**************************
  setTCAChannel(TCA9548A_CHANNEL_EFIS_RIGHT);
  if(!dEfis.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }  
  dEfis.display();
  updateDisplayEfisRight();

//**********************************************
// FCU SPD
//**********************************************
  setTCAChannel(TCA9548A_CHANNEL_FCU_SPD);
  dFcu.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  dFcu.display();
  delay(50); // Pause
  updateDisplayFcuSpd();

//**********************************************
// FCU HDG
//**********************************************
  setTCAChannel(TCA9548A_CHANNEL_FCU_HDG);
  dFcu.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  dFcu.display();
  delay(50); // Pause 
  updateDisplayFcuHdg();

//**********************************************
// FCU HDG and V/S or TRK and FPA.
//**********************************************
  setTCAChannel(TCA9548A_CHANNEL_FCU_FPA);
  dFcu.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  dFcu.display();
  delay(50); // Pause 
  updateDisplayFcuFpa();

//**********************************************
// FCU ALT
//**********************************************
  setTCAChannel(TCA9548A_CHANNEL_FCU_ALT);
  dFcu.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  dFcu.display();
  delay(50); // Pause 
  updateDisplayFcuAlt();

//**********************************************
// FCU Vs
//**********************************************
  setTCAChannel(TCA9548A_CHANNEL_FCU_VS);
  dFcu.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  dFcu.display();
  delay(50); // Pause
  updateDisplayFcuVs();      
}

void loop() {
  updateDisplayEfisLeft();
  updateDisplayEfisRight();
  updateDisplayFcuSpd();  
  updateDisplayFcuHdg();  
  updateDisplayFcuAlt();  
  updateDisplayFcuFpa();
  updateDisplayFcuVs();  
  delay(50);
}

/*
  switch multiplexer channel
*/
void setTCAChannel(byte i){
  I2Ctwo.beginTransmission(TCA9548A_I2C_ADDRESS);
  I2Ctwo.write(1 << i);
  I2Ctwo.endTransmission();  
    delay(5); // Pause
}

/*
  receive data from Mobiflight
*/
void onReceive(int len){
  char data[9]="";
  int count=0;
  char command=0;

  if(Wire.available()>=1 && Wire.available() <=9)
  {
    command=Wire.read(); 
  
    // Serial.print("command:");
    // Serial.println(command,HEX);
    handleCommand(command, data);
    // more?
    while(Wire.available()){
      Wire.read();
    } // while
  } // if
  else{
    // trash
    while(Wire.available()){
      Wire.read();
    } // while
  }
} //onReceive


void handleCommand(char command, char data[8]){

  switch(command){
    case 0: // send in ASCII
      //Efis Left Baro Select
      //inHg=0, hPa=1
      if(Wire.available()>=1)
      {
        efisLeftBaroSelect=Wire.read();

      }
      break;
    case 1:
      //Efis Left Baro Value Hpa
      if(Wire.available()>=1)
      {
        for(char x=0; x<4;x++)
        {
          if(Wire.available()==0)
            efisLeftBaroValueHpa[x]=0x00;
          else
            efisLeftBaroValueHpa[x]=Wire.read();
        }
      }
      break;
    case 2:
      //Efis Left Baro Value Hg
      if(Wire.available()>=1)
      {
        for(char x=0; x<4;x++)
        {
          if(Wire.available()==0)
            efisLeftBaroValueHg[x]=0x00;
          else
            efisLeftBaroValueHg[x]=Wire.read();
        }
      }      
      break;
    case 3:
      //Efis Left Baro Mode
      //0 = QFE; 1 = QNH; 2 = STD
      if(Wire.available()>=1)
      {
        efisLeftBaroMode=Wire.read();
      }      
      break;
    case 4: // send in ASCII
      //Efis Right Baro Select
      //inHg=0, hPa=1
      if(Wire.available()>=1)
      {
        efisRightBaroSelect=Wire.read();

      }   
      break;
    case 5:
      //Efis Right Baro Value Hpa
      if(Wire.available()>=1)
      {
        for(char x=0; x<4;x++)
        {
          if(Wire.available()==0)
            efisRightBaroValueHpa[x]=0x00;
          else
            efisRightBaroValueHpa[x]=Wire.read();
        }
      }          
      break;
    case 6:
      //Efis Right Baro Value Hg
      if(Wire.available()>=1)
      {
        for(char x=0; x<4;x++)
        {
          if(Wire.available()==0)
            efisRightBaroValueHg[x]=0x00;
          else
            efisRightBaroValueHg[x]=Wire.read();
        }
      }        
      break;
    case 7:
      //Efis Right Baro Mode
      //0 = QFE; 1 = QNH; 2 = STD
      if(Wire.available()>=1)
      {
        efisRightBaroMode=Wire.read();
      }         
      break;      
    case 8:
      //Fcu Speed Value
      if(Wire.available()>=1)
      {
        for(char x=0; x<4;x++)
        {
          if(Wire.available()==0)
            fcuSpeedValue[x]=0x00;
          else
            fcuSpeedValue[x]=Wire.read();
        }

      }           
      break;
    case 9:
      //Fcu Speed Managed
      //0 = No; 1 = Yes
      if(Wire.available()>=1)
      {
        fcuSpeedManagedMode=Wire.read();
      }          
      break;
    case 10:
      //Fcu Hdg Value
      if(Wire.available()>=1)
      {
        for(char x=0; x<3;x++)
        {
          if(Wire.available()==0)
            fcuHdgValue[x]=0x00;
          else
            fcuHdgValue[x]=Wire.read();
        }
      }       
      break;
    case 11:
      //Fcu Hdg Managed
      //0 = No; 1 = Yes
      if(Wire.available()>=1)
      {
        fcuHdgManagedMode=Wire.read();
      }         
      break;
    case 12:
      //Fcu Trk Mode
      //0 = No; 1 = Yes
      if(Wire.available()>=1)
      {
        fcuTrkMode=Wire.read();
      }   
      break;
    case 13:
      if(Wire.available()>=1)
      {
        for(char x=0; x<5;x++)
        {
          if(Wire.available()==0)
            fcuAltValue[x]=0x00;
          else
            fcuAltValue[x]=Wire.read();
        }

      }      
      break;
    case 14:
      if(Wire.available()>=1)
      {
        fcuAltManagedMode=Wire.read();
      }        
      break;
    case 15:
      if(Wire.available()>=1)
      {
        for(char x=0; x<5;x++)
        {
          if(Wire.available()==0)
            fcuVsValue[x]=0x00;
          else
            fcuVsValue[x]=Wire.read();
        }
      }   
      break;    
    case 16:
      if(Wire.available()>=1)
      {
        for(char x=0; x<4;x++)
        {

          if(Wire.available()==0)
            fcuVsValueFpa[x]=0x00;
          else
            fcuVsValueFpa[x]=Wire.read();
        }
      }      
      break;
    case 17:
      if(Wire.available()>=1)
      {
        fcuVsManagedMode=Wire.read();
      }       
      break;
  }

} //handleCommand


/*******************************************
Has to be redone, only tests
******************************************/
void updateDisplayEfisLeft(void)
{
  setTCAChannel(TCA9548A_CHANNEL_EFIS_LEFT);
 // Clear the buffer
  dEfis.clearDisplay();
  dEfis.setTextColor(SSD1306_WHITE);        // Draw white text

  if(efisLeftBaroMode=='3'){
       dEfis.setFont(&DSEG7Classic_Regular22pt7b);
       dEfis.setCursor(10,60);             
       dEfis.println("5td");
  }else{
    if(efisLeftBaroMode=='0'){
      dEfis.setFont(&FreeSans9pt7b);
      dEfis.setTextSize(1);             
      dEfis.setCursor(0,15);             
      dEfis.println("QFE");  
    }else{
      dEfis.setFont(&FreeSans9pt7b);
      dEfis.setTextSize(1);             
      dEfis.setCursor(85,15);             
      dEfis.println("QNH");
    }
    if(efisLeftBaroSelect=='0'){  // send in ASCII
      dEfis.setFont(&DSEG7Classic_Regular20pt7b);
      dEfis.setCursor(0,60);             
      dEfis.println(efisLeftBaroValueHg); 
      dEfis.fillCircle(64, 60, 2, SSD1306_WHITE);
    }else {
      dEfis.setFont(&DSEG7Classic_Regular20pt7b);
      dEfis.setCursor(0,60);   
      dEfis.println(efisLeftBaroValueHpa);
    }
  }

  dEfis.display();
} //updateDisplayEfisLeft

void updateDisplayEfisRight(void)
{
  setTCAChannel(TCA9548A_CHANNEL_EFIS_RIGHT);
 // Clear the buffer
  dEfis.clearDisplay();
  dEfis.setTextColor(SSD1306_WHITE);        // Draw white text

  if(efisRightBaroMode=='3'){
       dEfis.setFont(&DSEG7Classic_Regular22pt7b);
       dEfis.setCursor(10,60);             
       dEfis.println("5td");
  }else{
    if(efisRightBaroMode=='0'){
      dEfis.setFont(&FreeSans9pt7b);
      dEfis.setTextSize(1);             
      dEfis.setCursor(0,15);             
      dEfis.println("QFE");  
    }else{
      dEfis.setFont(&FreeSans9pt7b);
      dEfis.setTextSize(1);             
      dEfis.setCursor(85,15);             
      dEfis.println("QNH");
    }
    if(efisRightBaroSelect=='0'){  // send in ASCII
      dEfis.setFont(&DSEG7Classic_Regular20pt7b);
      dEfis.setCursor(0,60);             
      dEfis.println(efisRightBaroValueHg); 
      dEfis.fillCircle(64, 60, 2, SSD1306_WHITE);
    }else {
      dEfis.setFont(&DSEG7Classic_Regular20pt7b);
      dEfis.setCursor(0,60);   
      dEfis.println(efisRightBaroValueHpa);
    }
  }

  dEfis.display();
} //updateDisplayEfisRight



void updateDisplayFcuSpd(void)
{

  setTCAChannel(TCA9548A_CHANNEL_FCU_SPD);

  // Clear the buffer
  dFcu.clearDisplay();
  dFcu.setTextColor(SSD1306_WHITE);        // Draw white text

  dFcu.setFont(&FreeSans8pt7b);
  dFcu.setTextSize(1); 

  if(fcuSpeedValue[0]=='0')
  {
    dFcu.setCursor(65,20);       
    dFcu.println("MACH");
    if(fcuSpeedValue[3]==0x00)
    {
      fcuSpeedValue[3]='0';
    }
  }else{
    dFcu.setCursor(25,20);             
    dFcu.println("SPD");
  }

  if(fcuSpeedManagedMode=='1')
  {
    dFcu.setFont(&DSEG7Classic_Regular15pt7b);
    dFcu.setCursor(28,55);             
    dFcu.println("---"); 
    dFcu.fillCircle(104, 40, 3, SSD1306_WHITE);
  }
  else{
    dFcu.setFont(&DSEG7Classic_Regular15pt7b);
    dFcu.setCursor(28,55);             
    dFcu.println(fcuSpeedValue); 
  }

  dFcu.display();
}


void updateDisplayFcuHdg(void)
{
  String strHdgValue="000";
  
  // FCU Hdg
  setTCAChannel(TCA9548A_CHANNEL_FCU_HDG);

  // Clear the buffer
  dFcu.clearDisplay();
  dFcu.setTextColor(SSD1306_WHITE);        // Draw white text

  dFcu.setFont(&FreeSans8pt7b);
  dFcu.setTextSize(1); 


  if(fcuTrkMode=='0')
  {
    dFcu.setCursor(20,20);             
    dFcu.println("HDG");
  }else{
    dFcu.setCursor(60,20);       
    dFcu.println("TRK");
  }

  dFcu.setCursor(95,20);       
  dFcu.println("LAT");

  if(fcuHdgManagedMode=='1')
  {
    dFcu.setFont(&DSEG7Classic_Regular15pt7b);
    dFcu.setCursor(28,55);             
    dFcu.println("---"); 
    dFcu.fillCircle(104, 40, 3, SSD1306_WHITE);
  }
  else{
    dFcu.setFont(&DSEG7Classic_Regular15pt7b);
    dFcu.setCursor(28,55);   
 
    if(fcuHdgValue[1]==0x00)
    {
      strHdgValue[0]='0';
      strHdgValue[1]='0';
      strHdgValue[2]=fcuHdgValue[0];
    }else{
      if(fcuHdgValue[2]==0x00)
      {
        strHdgValue[0]='0';
        strHdgValue[1]=fcuHdgValue[0];
        strHdgValue[2]=fcuHdgValue[1];
      }else{
        strHdgValue[0]=fcuHdgValue[0];
        strHdgValue[1]=fcuHdgValue[1];
        strHdgValue[2]=fcuHdgValue[2];
      }      
    }
    dFcu.println(strHdgValue); 
  }

  dFcu.display();
}

void updateDisplayFcuFpa(void)
{
  setTCAChannel(TCA9548A_CHANNEL_FCU_FPA);

  // Clear the buffer
  dFcu.clearDisplay();
  dFcu.setTextColor(SSD1306_WHITE);        // Draw white text

  dFcu.setFont(&FreeSans8pt7b);
  dFcu.setTextSize(1); 

  if(fcuTrkMode=='0')
  {
    dFcu.setCursor(30,30);             
    dFcu.println("HDG");

    dFcu.setCursor(74,30);       
    dFcu.println("V/S");
  }else{
    dFcu.setCursor(30,45);             
    dFcu.println("TRK");

    dFcu.setCursor(74,45);       
    dFcu.println("FPA");
  }
  dFcu.display();
}

void updateDisplayFcuAlt(void)
{
  String strAltValue="00000";

  setTCAChannel(TCA9548A_CHANNEL_FCU_ALT);
  // Clear the buffer
  dFcu.clearDisplay();
  dFcu.setTextColor(SSD1306_WHITE);        // Draw white text
  
  dFcu.setFont(&FreeSans8pt7b);
  dFcu.setTextSize(1); 

  dFcu.setCursor(52,20);             
  dFcu.print("ALT");

  dFcu.setCursor(94,20);   
  dFcu.print("LVL");
  dFcu.print("/");

  dFcu.drawFastVLine(82, 15, 5, SSD1306_WHITE);
  dFcu.drawFastHLine(82, 15, 10, SSD1306_WHITE);

  if(fcuAltValue[3]==0x00)
  {
    strAltValue[0]='0';
    strAltValue[1]='0';
    strAltValue[2]=fcuAltValue[0];
    strAltValue[3]=fcuAltValue[1];
    strAltValue[4]=fcuAltValue[2];
  }else{
    if(fcuAltValue[4]==0x00)
    {
      strAltValue[0]='0';
      strAltValue[1]=fcuAltValue[0];
      strAltValue[2]=fcuAltValue[1];
      strAltValue[3]=fcuAltValue[2];
      strAltValue[4]=fcuAltValue[3];        
    }else{
      strAltValue[0]=fcuAltValue[0];
      strAltValue[1]=fcuAltValue[1];
      strAltValue[2]=fcuAltValue[2];
      strAltValue[3]=fcuAltValue[3];
      strAltValue[4]=fcuAltValue[4];        
    }      
  }

  dFcu.setFont(&DSEG7Classic_Regular15pt7b);
  dFcu.setCursor(0,55);             
  dFcu.print(strAltValue); 
  
  if(fcuAltManagedMode=='1')
  {
    dFcu.fillCircle(124, 39, 3, SSD1306_WHITE);
  }

   dFcu.display();
} //updateDisplayFcuAlt


void updateDisplayFcuVs(void)
{
  String strVrValue="0000";

  setTCAChannel(TCA9548A_CHANNEL_FCU_VS);

  // Clear the buffer
  dFcu.clearDisplay();
  dFcu.setTextColor(SSD1306_WHITE);        // Draw white text
  
  dFcu.setFont(&FreeSans8pt7b);
  dFcu.setTextSize(1); 

  dFcu.setCursor(2,20);             
  dFcu.print("CH");

  if(fcuTrkMode=='0')
  {
    dFcu.setCursor(40,20);   
    dFcu.print("V/S");
  }else{
    dFcu.setCursor(86,20);   
    dFcu.print("FPA");
  }



  dFcu.drawFastHLine(26, 15, 10, SSD1306_WHITE);
  dFcu.drawFastVLine(36, 15, 5, SSD1306_WHITE);

  if(fcuVsManagedMode=='1')
  {
    dFcu.setFont(&DSEG7Classic_Regular15pt7b);
    dFcu.setCursor(0,55);   
    dFcu.print("-----"); 
  }else{
    if(fcuTrkMode=='0')
    {
      if(fcuVsValue[1]==0x00){
          dFcu.setFont(&FreeSans18pt7b);
          dFcu.setCursor(0,50);   
          dFcu.print("+"); 
          
          strVrValue[0]='0';
          strVrValue[1]='0';
          strVrValue[2]='0';
          strVrValue[3]='0';

          dFcu.setFont(&DSEG7Classic_Regular15pt7b);
          dFcu.setCursor(24,55);   
          dFcu.print(strVrValue); 
      }else{
        if(fcuVsValue[0]=='-')
        {
          dFcu.setFont(&DSEG7Classic_Regular15pt7b);
          dFcu.setCursor(0,55);   
          dFcu.print("-"); 

          if(fcuVsValue[4]==0x00){
            strVrValue[0]='0';
            strVrValue[1]=fcuVsValue[1];
            strVrValue[2]=fcuVsValue[2];
            strVrValue[3]=fcuVsValue[3];         
          }else{
            strVrValue[0]=fcuVsValue[1];
            strVrValue[1]=fcuVsValue[2];
            strVrValue[2]=fcuVsValue[3];
            strVrValue[3]=fcuVsValue[4];      
          }
          dFcu.print(strVrValue); 
        }else{
          dFcu.setFont(&FreeSans18pt7b);
          dFcu.setCursor(0,50);   
          dFcu.print("+"); 
          
          if(fcuVsValue[3]==0x00){
            strVrValue[0]='0';
            strVrValue[1]=fcuVsValue[0];
            strVrValue[2]=fcuVsValue[1];
            strVrValue[3]=fcuVsValue[2];         
          }else{
            strVrValue[0]=fcuVsValue[0];
            strVrValue[1]=fcuVsValue[1];
            strVrValue[2]=fcuVsValue[2];
            strVrValue[3]=fcuVsValue[3];      
          }

          dFcu.setFont(&DSEG7Classic_Regular15pt7b);
          dFcu.setCursor(24,55);   
          dFcu.print(strVrValue); 
        }
      }
    }else{
     
      if(fcuVsValueFpa[0]=='-')
       {
        dFcu.setFont(&DSEG7Classic_Regular15pt7b);
        dFcu.setCursor(0,55);  

        if(fcuVsValueFpa[2]==0x00)
        {
          strVrValue=fcuVsValueFpa + ".0";
        }else{
          strVrValue=fcuVsValueFpa;
        }       
        dFcu.print(strVrValue);   
      } else{
        dFcu.setFont(&FreeSans18pt7b);
        dFcu.setCursor(0,50);   
        dFcu.print("+");    

        if(fcuVsValueFpa=="0")
        {
          strVrValue="0.0";
        }else{
          if(fcuVsValueFpa[1]==0x00)
          {
            strVrValue=fcuVsValueFpa + ".0";
          } else{
            strVrValue[0]=fcuVsValueFpa[0];
            strVrValue[1]=fcuVsValueFpa[1];
            strVrValue[2]=fcuVsValueFpa[2];
            strVrValue[3]=0x00;
          }
        }

        dFcu.setFont(&DSEG7Classic_Regular15pt7b);
        dFcu.setCursor(24,55);  
        dFcu.print(strVrValue);             
      }
   
    }
  }
  dFcu.display();
}