/*
This is only a Test and has to be set in an usable state
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSans9pt7b.h>
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
Adafruit_SSD1306 dEfisLeft(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
Adafruit_SSD1306 dEfisRight(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);

// fcu displays
Adafruit_SH1106G dFcuSpd = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
Adafruit_SH1106G dFcuHdg = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
Adafruit_SH1106G dFcuFpa = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
Adafruit_SH1106G dFcuAlt = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
Adafruit_SH1106G dFcuVs = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);

bool isStd = false;
bool isHpa = true;
String valHpa="8888";
String valHg="88,88";

// Efis left
bool bEfisLeftStd = false;
bool bEfisLeftHpa = true;
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
  Wire.onReceive(onReceive);

  // init i2c busses
  Wire.begin((uint8_t)I2C_MOBIFLIGHT_ADDR,I2C_MOBIFLIGHT_SDA,I2C_MOBIFLIGHT_SCL,400000);
  I2Ctwo.begin(I2C_DISPLAY_SDA,I2C_DISPLAY_SCL,400000); // SDA pin , SCL pin , 400kHz frequency
  
  //**************************
  // Efis left
  //**************************
  setTCAChannel(TCA9548A_CHANNEL_EFIS_LEFT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!dEfisLeft.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  dEfisLeft.display();
  delay(250); // Pause for 2 seconds

  updateDisplayEfisLeft();

  //**************************
  // Efis right
  //**************************
  setTCAChannel(TCA9548A_CHANNEL_EFIS_RIGHT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!dEfisRight.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }
  dEfisRight.begin(SCREEN_ADDRESS, true);

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  dEfisRight.display();
  delay(250); // Pause for 2 seconds

  updateDisplayEfisRight();

//**********************************************
// FCU SPD
//**********************************************
  setTCAChannel(TCA9548A_CHANNEL_FCU_SPD);
  dFcuSpd.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  dFcuSpd.display();
  delay(250); // Pause for 2 seconds
  updateDisplayFcuSpd();

//**********************************************
// FCU HDG
//**********************************************
  setTCAChannel(TCA9548A_CHANNEL_FCU_HDG);
  dFcuHdg.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  dFcuHdg.display();
  delay(250); // Pause for 2 seconds
  updateDisplayFcuHdg();

//**********************************************
// FCU HDG and V/S or TRK and FPA.
//**********************************************
  setTCAChannel(TCA9548A_CHANNEL_FCU_FPA);
  dFcuFpa.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  dFcuFpa.display();
  delay(250); // Pause for 2 seconds
  updateDisplayFcuFpa();

//**********************************************
// FCU ALT
//**********************************************
  setTCAChannel(TCA9548A_CHANNEL_FCU_ALT);
  dFcuAlt.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  dFcuAlt.display();
  delay(250); // Pause for 2 seconds
  updateDisplayFcuAlt();

//**********************************************
// FCU Vs
//**********************************************
  setTCAChannel(TCA9548A_CHANNEL_FCU_VS);
  dFcuVs.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  dFcuVs.display();
  delay(250); // Pause for 2 seconds
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
}

/*
  receive data from Mobiflight
*/
void onReceive(int len){
  char msgArray[9]="";

  // // if smaller than 32 ignore
   if(Wire.available()==32)
   {
     for (int i=0; i <= 8; i++){
       uint8_t hibits = (uint8_t)Wire.read();      
       Wire.read(); // ignore
       uint8_t lowbits = (uint8_t)Wire.read(); 
       Wire.read(); // ignore

       msgArray[i] = msgArray[i] | (hibits & 0xf0);
       msgArray[i] = msgArray[i] | ((lowbits & 0xf0)>>4);
       if(msgArray[i] == 0x20){
         msgArray[i]='\0';
       }
     }
     if(msgArray[7] != '\0'){
         msgArray[8]='\0';
     }

    //Serial.println(msgArray);
    handleCommand(String(msgArray));

   }else{
    while(Wire.available()){
      Wire.read();
    }
   }
}

/*
  handle data from mobiflight
*/
void handleCommand(String command){
    
      Serial.println(command);

   if(command.startsWith("#0")){
    valHpa=command.substring(2);
    if(valHpa.substring(0,1)=="5"){
      valHpa="0"+ valHpa.substring(1);
    }
   }
   if(command.startsWith("#1")){
    valHg=command.substring(2);
   }
   if(command.startsWith("#2")){
     if(command.substring(2)=="0"){
      isHpa=false;
     }else{
       isHpa=true;
     }
   }
   if(command.startsWith("#3")){
     if(command.substring(2)=="1"){
      isStd=true;
     }else{
       isStd=false;
     }
   } 

   // Update displays
   // has to be redone!! only tests
   setTCAChannel(TCA9548A_CHANNEL_EFIS_LEFT);
   updateDisplayEfisLeft();

   setTCAChannel(TCA9548A_CHANNEL_EFIS_RIGHT);
   updateDisplayEfisRight();

   setTCAChannel(TCA9548A_CHANNEL_FCU_SPD);
   updateDisplayFcuSpd();

   setTCAChannel(TCA9548A_CHANNEL_FCU_HDG);
   updateDisplayFcuHdg();

   setTCAChannel(TCA9548A_CHANNEL_FCU_FPA);
   updateDisplayFcuFpa();

   setTCAChannel(TCA9548A_CHANNEL_FCU_ALT);
   updateDisplayFcuAlt();

   setTCAChannel(TCA9548A_CHANNEL_FCU_VS);
   updateDisplayFcuVs();
}

/*******************************************
Has to be redone, only tests
******************************************/
void updateDisplayEfisLeft(void)
{

 // Clear the buffer
  dEfisLeft.clearDisplay();
  dEfisLeft.setTextColor(SSD1306_WHITE);        // Draw white text

  if(bEfisLeftStd){
      dEfisLeft.setFont(&DSEG7Classic_Regular22pt7b);
      dEfisLeft.setCursor(20,60);             
      dEfisLeft.println("5td");
  }else{
    dEfisLeft.setFont(&FreeSans9pt7b);
    dEfisLeft.setTextSize(1);             
    dEfisLeft.setCursor(85,15);             
    dEfisLeft.println("QNH");
    if(bEfisLeftHpa){
        dEfisLeft.setFont(&DSEG7Classic_Regular20pt7b);
        if(sEfisLeftHpa.length()==3)
        {
          dEfisLeft.setCursor(32,60);             
        }else{
          dEfisLeft.setCursor(0,60);     
        }
        dEfisLeft.println(sEfisLeftHpa);
    }else {
        dEfisLeft.setFont(&DSEG7Classic_Regular20pt7b);
        dEfisLeft.setCursor(0,60);             
        dEfisLeft.println(sEfisLeftHg); 
    }
  }

  dEfisLeft.display();
}

void updateDisplayEfisRight(void)
{

 // Clear the buffer
  dEfisRight.clearDisplay();
  dEfisRight.setTextColor(SSD1306_WHITE);        // Draw white text

  if(bEfisRightStd){
      dEfisRight.setFont(&DSEG7Classic_Regular22pt7b);
      dEfisRight.setCursor(20,60);             
      dEfisRight.println("5td");
  }else{
    dEfisRight.setFont(&FreeSans9pt7b);
    dEfisRight.setTextSize(1);             
    dEfisRight.setCursor(85,15);             
    dEfisRight.println("QNH");
    if(bEfisRightHpa){
        dEfisRight.setFont(&DSEG7Classic_Regular20pt7b);
        if(sEfisRightHpa.length()==3)
        {
          dEfisRight.setCursor(32,60);             
        }else{
          dEfisRight.setCursor(0,60);     
        }
        dEfisRight.println(sEfisRightHpa);
    }else {
        dEfisRight.setFont(&DSEG7Classic_Regular20pt7b);
        dEfisRight.setCursor(0,60);             
        dEfisRight.println(sEfisRightHg); 
    }
  }
  dEfisRight.display();
}



void updateDisplayFcuSpd(void)
{

//  // Clear the buffer
   dFcuSpd.clearDisplay();
   dFcuSpd.setTextColor(SSD1306_WHITE);        // Draw white text

//   if(isStd){
//       dFcuSpd.setFont(&DSEG7Classic_Regular22pt7b);
//       dFcuSpd.setCursor(20,60);             
//       dFcuSpd.println("5td");
//   }else{
     dFcuSpd.setFont(&FreeSans9pt7b);
     dFcuSpd.setTextSize(1);             
     dFcuSpd.setCursor(5,15);             
     dFcuSpd.println("SPD");
//     if(isHpa){
//         dFcuSpd.setFont(&DSEG7Classic_Regular20pt7b);
//         if(valHpa.length()==3)
//         {
//           dFcuSpd.setCursor(32,60);             
//         }else{
//           dFcuSpd.setCursor(0,60);     
//         }
//         dFcuSpd.println(valHpa);
//     }else {
         dFcuSpd.setFont(&DSEG7Classic_Regular18pt7b);
         dFcuSpd.setCursor(0,60);             
         dFcuSpd.println("8 8 8"); 
//     }
//   }

   dFcuSpd.display();
}


void updateDisplayFcuHdg(void)
{
//  // Clear the buffer
   dFcuHdg.clearDisplay();
   dFcuHdg.setTextColor(SSD1306_WHITE);        // Draw white text

//   if(isStd){
//       dFcuHdg.setFont(&DSEG7Classic_Regular22pt7b);
//       dFcuHdg.setCursor(20,60);             
//       dFcuHdg.println("5td");
//   }else{
//     dFcuHdg.setFont(&FreeSans9pt7b);
//     dFcuHdg.setTextSize(1);             
//     dFcuHdg.setCursor(85,15);             
//     dFcuHdg.println("QNH");
//     if(isHpa){
//         dFcuHdg.setFont(&DSEG7Classic_Regular20pt7b);
//         if(valHpa.length()==3)
//         {
//           dFcuHdg.setCursor(32,60);             
//         }else{
//           dFcuHdg.setCursor(0,60);     
//         }
//         dFcuHdg.println(valHpa);
//     }else {
//         dFcuHdg.setFont(&DSEG7Classic_Regular20pt7b);
//         dFcuHdg.setCursor(0,60);             
//         dFcuHdg.println(valHg); 
//     }
//   }

   dFcuHdg.display();
}

void updateDisplayFcuFpa(void)
{

//  // Clear the buffer
   dFcuFpa.clearDisplay();
   dFcuFpa.setTextColor(SSD1306_WHITE);        // Draw white text

//   if(isStd){
//       dFcuFpa.setFont(&DSEG7Classic_Regular22pt7b);
//       dFcuFpa.setCursor(20,60);             
//       dFcuFpa.println("5td");
//   }else{
//     dFcuFpa.setFont(&FreeSans9pt7b);
//     dFcuFpa.setTextSize(1);             
//     dFcuFpa.setCursor(85,15);             
//     dFcuFpa.println("QNH");
//     if(isHpa){
//         dFcuFpa.setFont(&DSEG7Classic_Regular20pt7b);
//         if(valHpa.length()==3)
//         {
//           dFcuFpa.setCursor(32,60);             
//         }else{
//           dFcuFpa.setCursor(0,60);     
//         }
//         dFcuFpa.println(valHpa);
//     }else {
//         dFcuFpa.setFont(&DSEG7Classic_Regular20pt7b);
//         dFcuFpa.setCursor(0,60);             
//         dFcuFpa.println(valHg); 
//     }
//   }

   dFcuFpa.display();
}

void updateDisplayFcuAlt(void)
{

//  // Clear the buffer
   dFcuAlt.clearDisplay();
   dFcuAlt.setTextColor(SSD1306_WHITE);        // Draw white text

//   if(isStd){
//       dFcuAlt.setFont(&DSEG7Classic_Regular22pt7b);
//       dFcuAlt.setCursor(20,60);             
//       dFcuAlt.println("5td");
//   }else{
//     dFcuAlt.setFont(&FreeSans9pt7b);
//     dFcuAlt.setTextSize(1);             
//     dFcuAlt.setCursor(85,15);             
//     dFcuAlt.println("QNH");
//     if(isHpa){
//         dFcuAlt.setFont(&DSEG7Classic_Regular20pt7b);
//         if(valHpa.length()==3)
//         {
//           dFcuAlt.setCursor(32,60);             
//         }else{
//           dFcuAlt.setCursor(0,60);     
//         }
//         dFcuAlt.println(valHpa);
//     }else {
//         dFcuAlt.setFont(&DSEG7Classic_Regular20pt7b);
//         dFcuAlt.setCursor(0,60);             
//         dFcuAlt.println(valHg); 
//     }
//   }

   dFcuAlt.display();
}


void updateDisplayFcuVs(void)
{

//  // Clear the buffer
   dFcuVs.clearDisplay();
   dFcuVs.setTextColor(SSD1306_WHITE);        // Draw white text

//   if(isStd){
//       dFcuVs.setFont(&DSEG7Classic_Regular22pt7b);
//       dFcuVs.setCursor(20,60);             
//       dFcuVs.println("5td");
//   }else{
//     dFcuVs.setFont(&FreeSans9pt7b);
//     dFcuVs.setTextSize(1);             
//     dFcuVs.setCursor(85,15);             
//     dFcuVs.println("QNH");
//     if(isHpa){
//         dFcuVs.setFont(&DSEG7Classic_Regular20pt7b);
//         if(valHpa.length()==3)
//         {
//           dFcuVs.setCursor(32,60);             
//         }else{
//           dFcuVs.setCursor(0,60);     
//         }
//         dFcuVs.println(valHpa);
//     }else {
//         dFcuVs.setFont(&DSEG7Classic_Regular20pt7b);
//         dFcuVs.setCursor(0,60);             
//         dFcuVs.println(valHg); 
//     }
//   }

   dFcuVs.display();
}