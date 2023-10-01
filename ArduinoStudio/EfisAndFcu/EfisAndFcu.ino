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
// Adafruit_SH1106G dFcuHdg = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
// Adafruit_SH1106G dFcuFpa = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
// Adafruit_SH1106G dFcuAlt = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
// Adafruit_SH1106G dFcuVs = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);

// Efis left
bool bEfisLeftStd = false;
bool bEfisLeftHpa = false;
String sEfisLeftHpa = "8888";
String sEfisLeftHg = "8888";

// Efis right
bool bEfisRightStd = false;
bool bEfisRightHpa = true;
String sEfisRightHpa = "8888";
String sEfisRightHg = "8888";

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  // setup i2c receive callback
  //Wire.onReceive(onReceive);

  // init i2c busses
  Wire.begin((uint8_t)I2C_MOBIFLIGHT_ADDR,I2C_MOBIFLIGHT_SDA,I2C_MOBIFLIGHT_SCL,400000);
  I2Ctwo.begin(I2C_DISPLAY_SDA,I2C_DISPLAY_SCL,400000); // SDA pin , SCL pin , 400kHz frequency
  
  // //**************************
  // // Efis left
  // //**************************
  setTCAChannel(0);

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
  setTCAChannel(1);
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

// /*
//   receive data from Mobiflight
// */
// void onReceive(int len){
//   char msgArray[9]="";

//   // // if smaller than 32 ignore
//    if(Wire.available()==32)
//    {
//      for (int i=0; i <= 8; i++){
//        uint8_t hibits = (uint8_t)Wire.read();      
//        Wire.read(); // ignore
//        uint8_t lowbits = (uint8_t)Wire.read(); 
//        Wire.read(); // ignore

//        msgArray[i] = msgArray[i] | (hibits & 0xf0);
//        msgArray[i] = msgArray[i] | ((lowbits & 0xf0)>>4);
//        if(msgArray[i] == 0x20){
//          msgArray[i]='\0';
//        }
//      }
//      if(msgArray[7] != '\0'){
//          msgArray[8]='\0';
//      }

//     //Serial.println(msgArray);
//     handleCommand(String(msgArray));

//    }else{
//     while(Wire.available()){
//       Wire.read();
//     }
//    }
// }

// /*
//   handle data from mobiflight
// */
// void handleCommand(String command){
    
//       Serial.println(command);

//    if(command.startsWith("#0")){
//     valHpa=command.substring(2);
//     if(valHpa.substring(0,1)=="5"){
//       valHpa="0"+ valHpa.substring(1);
//     }
//    }
//    if(command.startsWith("#1")){
//     valHg=command.substring(2);
//    }
//    if(command.startsWith("#2")){
//      if(command.substring(2)=="0"){
//       isHpa=false;
//      }else{
//        isHpa=true;
//      }
//    }
//    if(command.startsWith("#3")){
//      if(command.substring(2)=="1"){
//       isStd=true;
//      }else{
//        isStd=false;
//      }
//    } 

//    // Update displays
//    // has to be redone!! only tests
//    setTCAChannel(TCA9548A_CHANNEL_EFIS_LEFT);
//    updateDisplayEfisLeft();

//    setTCAChannel(TCA9548A_CHANNEL_EFIS_RIGHT);
//    updateDisplayEfisRight();

//    setTCAChannel(TCA9548A_CHANNEL_FCU_SPD);
//    updateDisplayFcuSpd();

//    setTCAChannel(TCA9548A_CHANNEL_FCU_HDG);
//    updateDisplayFcuHdg();

//    setTCAChannel(TCA9548A_CHANNEL_FCU_FPA);
//    updateDisplayFcuFpa();

//    setTCAChannel(TCA9548A_CHANNEL_FCU_ALT);
//    updateDisplayFcuAlt();

//    setTCAChannel(TCA9548A_CHANNEL_FCU_VS);
//    updateDisplayFcuVs();
// }

/*******************************************
Has to be redone, only tests
******************************************/
void updateDisplayEfisLeft(void)
{

 // Clear the buffer
  dEfis.clearDisplay();
  dEfis.setTextColor(SSD1306_WHITE);        // Draw white text

  if(bEfisLeftStd){
      dEfis.setFont(&DSEG7Classic_Regular22pt7b);
      dEfis.setCursor(20,60);             
      dEfis.println("5td");
  }else{

    if(bEfisLeftHpa){
      dEfis.setFont(&FreeSans9pt7b);
      dEfis.setTextSize(1);             
      dEfis.setCursor(85,15);             
      dEfis.println("QNH");
      dEfis.setFont(&DSEG7Classic_Regular20pt7b);
      if(sEfisLeftHpa.length()==3)
      {
        dEfis.setCursor(32,60);             
      }else{
        dEfis.setCursor(0,60);     
      }
      dEfis.println(sEfisLeftHpa);
    }else {
      dEfis.setFont(&FreeSans9pt7b);
      dEfis.setTextSize(1);             
      dEfis.setCursor(0,15);             
      dEfis.println("BARO");      
      dEfis.setFont(&DSEG7Classic_Regular20pt7b);
      dEfis.setCursor(0,60);             
      dEfis.println(sEfisLeftHg); 
    }
  }

  dEfis.display();
}

void updateDisplayEfisRight(void)
{

 //Clear the buffer
  dEfis.clearDisplay();
  dEfis.setTextColor(SSD1306_WHITE);        // Draw white text

   if(bEfisRightStd){
      dEfis.setFont(&DSEG7Classic_Regular22pt7b);
      dEfis.setCursor(20,60);             
      dEfis.println("5td");
  }else{
    if(bEfisRightHpa){
      dEfis.setFont(&FreeSans9pt7b);
      dEfis.setTextSize(1);             
      dEfis.setCursor(85,15);             
      dEfis.println("QNH");      
      dEfis.setFont(&DSEG7Classic_Regular20pt7b);
      if(sEfisRightHpa.length()==3)
      {
        dEfis.setCursor(32,60);             
      }else{
        dEfis.setCursor(0,60);     
      }
      dEfis.println(sEfisRightHpa);
    }else {
      dEfis.setFont(&FreeSans9pt7b);
      dEfis.setTextSize(1);             
      dEfis.setCursor(0,15);             
      dEfis.println("BARO");      
      dEfis.setFont(&DSEG7Classic_Regular20pt7b);
      dEfis.setCursor(0,60);             
      dEfis.println(sEfisRightHg); 
    }
  }
  dEfis.display();
}



void updateDisplayFcuSpd(void)
{
  // Clear the buffer
  dFcu.clearDisplay();
  dFcu.setTextColor(SSD1306_WHITE);        // Draw white text

  dFcu.setFont(&FreeSans8pt7b);
  dFcu.setTextSize(1); 

  dFcu.setCursor(25,20);             
  dFcu.println("SPD");

  dFcu.setCursor(65,20);       
  dFcu.println("MACH");

  dFcu.setFont(&DSEG7Classic_Regular15pt7b);
  dFcu.setCursor(28,55);             
  dFcu.println("888"); 

  dFcu.fillCircle(104, 39, 3, SSD1306_WHITE);
  dFcu.display();
}


void updateDisplayFcuHdg(void)
{
  // Clear the buffer
  dFcu.clearDisplay();
  dFcu.setTextColor(SSD1306_WHITE);        // Draw white text

  dFcu.setFont(&FreeSans8pt7b);
  dFcu.setTextSize(1); 

  dFcu.setCursor(20,20);             
  dFcu.println("HDG");

  dFcu.setCursor(60,20);       
  dFcu.println("TRK");

  dFcu.setCursor(95,20);       
  dFcu.println("LAT");

  dFcu.setFont(&DSEG7Classic_Regular15pt7b);
  dFcu.setCursor(28,55);             
  dFcu.println("888"); 

  dFcu.fillCircle(104, 39, 3, SSD1306_WHITE);
  dFcu.display();
}

void updateDisplayFcuFpa(void)
{

//  // Clear the buffer
   dFcu.clearDisplay();
   dFcu.setTextColor(SSD1306_WHITE);        // Draw white text

  dFcu.setFont(&FreeSans8pt7b);
  dFcu.setTextSize(1); 

  dFcu.setCursor(30,30);             
  dFcu.println("HDG");

  dFcu.setCursor(74,30);       
  dFcu.println("V/S");

  dFcu.setCursor(30,45);             
  dFcu.println("TRK");

  dFcu.setCursor(74,45);       
  dFcu.println("FPA");

   dFcu.display();
}

void updateDisplayFcuAlt(void)
{

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
  dFcu.print("88888"); 

  dFcu.fillCircle(124, 39, 3, SSD1306_WHITE);

   dFcu.display();
}


void updateDisplayFcuVs(void)
{

//  // Clear the buffer
  dFcu.clearDisplay();
  dFcu.setTextColor(SSD1306_WHITE);        // Draw white text
  
  dFcu.setFont(&FreeSans8pt7b);
  dFcu.setTextSize(1); 

  dFcu.setCursor(2,20);             
  dFcu.print("CH");

  dFcu.setCursor(40,20);   
  dFcu.print("V/S");

  dFcu.drawFastHLine(26, 15, 10, SSD1306_WHITE);
  dFcu.drawFastVLine(36, 15, 5, SSD1306_WHITE);

  dFcu.setCursor(86,20);   
  dFcu.print("FPA");

  dFcu.setFont(&DSEG7Classic_Regular15pt7b);
  dFcu.setCursor(0,55);             
  dFcu.println("88888"); 

   dFcu.display();
}