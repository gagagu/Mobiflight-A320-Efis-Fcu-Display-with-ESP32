/*
This is only a Test and has to be set in an usable state
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
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
  char data[8]="";
  int count=0;
  String command="00";

  if(Wire.available()>=3 && Wire.available() <=9)
  {
    command[0]=Wire.read(); 
    command[1]=Wire.read(); 
   // Serial.print(command);
   // Serial.print(",");
    while(Wire.available()){
      data[count] = Wire.read();
     // Serial.print(data[count]);
      //Serial.print(",");
      count++;
    } // while
   // Serial.println("");
    handleCommand(command, data);
  } // if
  else{
    // trash
    while(Wire.available()){
      Wire.read();
    } // while
  }
} //onReceive


void handleCommand(String command, char data[8]){

  switch(command[0]){
    case '1':
      switch(command[1]){
        case '0': // send in ASCII
          //Efis Left Baro Select
          //inHg=0, hPa=1
          efisLeftBaroSelect=data[0];
          updateDisplayEfisLeft();
          break;
        case '1':
          //Efis Left Baro Value Hpa
          efisLeftBaroValueHpa[0]=data[0];
          efisLeftBaroValueHpa[1]=data[1];
          efisLeftBaroValueHpa[2]=data[2];
          efisLeftBaroValueHpa[3]=data[3];
          updateDisplayEfisLeft();
          break;
        case '2':
          //Efis Left Baro Value Hg
          efisLeftBaroValueHg[0]=data[0];
          efisLeftBaroValueHg[1]=data[1];
          efisLeftBaroValueHg[2]=data[2];
          efisLeftBaroValueHg[3]=data[3];      
          updateDisplayEfisLeft();
          break;
        case '3':
          //Efis Left Baro Mode
          //0 = QFE; 1 = QNH; 2 = STD
          efisLeftBaroMode=data[0];
          updateDisplayEfisLeft();
          break;
        case '4': // send in ASCII
          //Efis Right Baro Select
          //inHg=0, hPa=1
          efisRightBaroSelect=data[0];
          updateDisplayEfisRight();
          break;
        case '5':
          //Efis Right Baro Value Hpa
          efisRightBaroValueHpa[0]=data[0];
          efisRightBaroValueHpa[1]=data[1];
          efisRightBaroValueHpa[2]=data[2];
          efisRightBaroValueHpa[3]=data[3];
          updateDisplayEfisRight();
          break;
        case '6':
          //Efis Right Baro Value Hg
          efisRightBaroValueHg[0]=data[0];
          efisRightBaroValueHg[1]=data[1];
          efisRightBaroValueHg[2]=data[2];
          efisRightBaroValueHg[3]=data[3];      
          updateDisplayEfisRight();
          break;
        case '7':
          //Efis Right Baro Mode
          //0 = QFE; 1 = QNH; 2 = STD
          efisRightBaroMode=data[0];
          updateDisplayEfisRight();
          break;      
        case '8':
          //Fcu Speed Value
          fcuSpeedValue[0]=data[0];
          fcuSpeedValue[1]=data[1];
          fcuSpeedValue[2]=data[2]; 
          fcuSpeedValue[3]=data[3]; 
          updateDisplayFcuSpd();
          break;
        case '9':
          //Fcu Speed Managed
          //0 = No; 1 = Yes
          fcuSpeedManagedMode=data[0];
          updateDisplayFcuSpd();
          break;
        }
      break;
    case '2':
      switch(command[1]){
        case '0':
          //Fcu Hdg Value
          fcuHdgValue[0]=data[0];
          fcuHdgValue[1]=data[1];
          fcuHdgValue[2]=data[2];      
          updateDisplayFcuHdg();
          break;
        case '1':
          //Fcu Hdg Managed
          //0 = No; 1 = Yes
          fcuHdgManagedMode=data[0];      
          updateDisplayFcuHdg();
          break;
        case '2':
          //Fcu Trk Mode
          //0 = No; 1 = Yes
          fcuTrkMode=data[0]; 
          updateDisplayFcuHdg();
          updateDisplayFcuFpa();
          updateDisplayFcuVs();
          break;
        case '3':
          fcuAltValue[0]=data[0];
          fcuAltValue[1]=data[1];
          fcuAltValue[2]=data[2]; 
          fcuAltValue[3]=data[3]; 
          fcuAltValue[4]=data[4];           
          updateDisplayFcuAlt();
          break;
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

  if(efisLeftBaroMode=='2'){
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

  if(efisRightBaroMode=='2'){
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

  // FCU Hdg
//char fcuHdgManagedMode = 0x00;
//String fcuHdgValue = "000";

// FCU Trk Mode
//char fcuTrkMode = 0x00;
  
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
    dFcu.println(fcuHdgValue); 
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

  dFcu.setFont(&DSEG7Classic_Regular15pt7b);
  dFcu.setCursor(0,55);             
  dFcu.print(fcuAltValue); 

  dFcu.fillCircle(124, 39, 3, SSD1306_WHITE);

   dFcu.display();
}


void updateDisplayFcuVs(void)
{
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

  dFcu.setFont(&DSEG7Classic_Regular15pt7b);
  dFcu.setCursor(0,55);             
  dFcu.println("88888"); 

   dFcu.display();
}