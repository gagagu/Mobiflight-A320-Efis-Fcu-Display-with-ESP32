/*
This is only a Test and has to be set in an usable state
*/

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSans9pt7b.h>
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
#define TCA9548A_CHANNEL_0    0
#define TCA9548A_CHANNEL_1    1
#define TCA9548A_CHANNEL_2    2
#define TCA9548A_CHANNEL_3    3
#define TCA9548A_CHANNEL_4    4
#define TCA9548A_CHANNEL_5    5
#define TCA9548A_CHANNEL_6    6
#define TCA9548A_CHANNEL_7    7

TwoWire I2Ctwo = TwoWire(1);  // init second i2c bus

// Efis displays
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
Adafruit_SSD1306 display2(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);

// fcu displays
Adafruit_SH1106G display3 = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
Adafruit_SH1106G display4 = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
Adafruit_SH1106G display5 = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
Adafruit_SH1106G display6 = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);
Adafruit_SH1106G display7 = Adafruit_SH1106G(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);

bool isStd = false;
bool isHpa = true;
String valHpa="8888";
String valHg="88,88";



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
  setTCAChannel(TCA9548A_CHANNEL_0);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(250); // Pause for 2 seconds

  updateDisplay();

  //**************************
  // Efis right
  //**************************
  setTCAChannel(TCA9548A_CHANNEL_1);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display2.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }display3.begin(SCREEN_ADDRESS, true);

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display2.display();
  delay(250); // Pause for 2 seconds

  updateDisplay2();

//**********************************************
// FCU 1
//**********************************************
  setTCAChannel(TCA9548A_CHANNEL_2);
  display3.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  display3.display();
  delay(250); // Pause for 2 seconds
  updateDisplay3();
//**********************************************
// FCU 2
//**********************************************
  setTCAChannel(TCA9548A_CHANNEL_3);
  display4.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  display4.display();
  delay(250); // Pause for 2 seconds
  updateDisplay4();
//**********************************************
// FCU 3
//**********************************************

  setTCAChannel(TCA9548A_CHANNEL_4);
  display5.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  display5.display();
  delay(250); // Pause for 2 seconds
  updateDisplay5();
//**********************************************
// FCU 4
//**********************************************

  setTCAChannel(TCA9548A_CHANNEL_5);
  display6.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  display6.display();
  delay(250); // Pause for 2 seconds
  updateDisplay6();
//**********************************************
// FCU 5
//**********************************************

  setTCAChannel(TCA9548A_CHANNEL_6);
  display7.begin(SCREEN_ADDRESS, true); // Address 0x3C default
  display7.display();
  delay(250); // Pause for 2 seconds
  updateDisplay7();      
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
   setTCAChannel(TCA9548A_CHANNEL_0);
   updateDisplay();
   setTCAChannel(TCA9548A_CHANNEL_1);
   updateDisplay2();
   setTCAChannel(TCA9548A_CHANNEL_2);
   updateDisplay3();
   setTCAChannel(TCA9548A_CHANNEL_3);
   updateDisplay4();
   setTCAChannel(TCA9548A_CHANNEL_4);
   updateDisplay5();
   setTCAChannel(TCA9548A_CHANNEL_5);
   updateDisplay6();
   setTCAChannel(TCA9548A_CHANNEL_6);
   updateDisplay7();
}

/*******************************************
Has to be redone, only tests
******************************************/
void updateDisplay(void)
{

 // Clear the buffer
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);        // Draw white text

  if(isStd){
      display.setFont(&DSEG7Classic_Regular22pt7b);
      display.setCursor(20,60);             
      display.println("5td");
  }else{
    display.setFont(&FreeSans9pt7b);
    display.setTextSize(1);             
    display.setCursor(85,15);             
    display.println("QNH");
    if(isHpa){
        display.setFont(&DSEG7Classic_Regular20pt7b);
        if(valHpa.length()==3)
        {
          display.setCursor(32,60);             
        }else{
          display.setCursor(0,60);     
        }
        display.println(valHpa);
    }else {
        display.setFont(&DSEG7Classic_Regular20pt7b);
        display.setCursor(0,60);             
        display.println(valHg); 
    }
  }

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();

}

void updateDisplay2(void)
{

 // Clear the buffer
  display2.clearDisplay();
  display2.setTextColor(SSD1306_WHITE);        // Draw white text

  if(isStd){
      display2.setFont(&DSEG7Classic_Regular22pt7b);
      display2.setCursor(20,60);             
      display2.println("5td");
  }else{
    display2.setFont(&FreeSans9pt7b);
    display2.setTextSize(1);             
    display2.setCursor(85,15);             
    display2.println("QNH");
    if(isHpa){
        display2.setFont(&DSEG7Classic_Regular20pt7b);
        if(valHpa.length()==3)
        {
          display2.setCursor(32,60);             
        }else{
          display2.setCursor(0,60);     
        }
        display2.println(valHpa);
    }else {
        display2.setFont(&DSEG7Classic_Regular20pt7b);
        display2.setCursor(0,60);             
        display2.println(valHg); 
    }
  }

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display2.display();

}



void updateDisplay3(void)
{

 // Clear the buffer
  display3.clearDisplay();
  display3.setTextColor(SSD1306_WHITE);        // Draw white text

  if(isStd){
      display3.setFont(&DSEG7Classic_Regular22pt7b);
      display3.setCursor(20,60);             
      display3.println("5td");
  }else{
    display3.setFont(&FreeSans9pt7b);
    display3.setTextSize(1);             
    display3.setCursor(85,15);             
    display3.println("QNH");
    if(isHpa){
        display3.setFont(&DSEG7Classic_Regular20pt7b);
        if(valHpa.length()==3)
        {
          display3.setCursor(32,60);             
        }else{
          display3.setCursor(0,60);     
        }
        display3.println(valHpa);
    }else {
        display3.setFont(&DSEG7Classic_Regular20pt7b);
        display3.setCursor(0,60);             
        display3.println(valHg); 
    }
  }

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display3.display();

}


void updateDisplay4(void)
{

 // Clear the buffer
  display4.clearDisplay();
  display4.setTextColor(SSD1306_WHITE);        // Draw white text

  if(isStd){
      display4.setFont(&DSEG7Classic_Regular22pt7b);
      display4.setCursor(20,60);             
      display4.println("5td");
  }else{
    display4.setFont(&FreeSans9pt7b);
    display4.setTextSize(1);             
    display4.setCursor(85,15);             
    display4.println("QNH");
    if(isHpa){
        display4.setFont(&DSEG7Classic_Regular20pt7b);
        if(valHpa.length()==3)
        {
          display4.setCursor(32,60);             
        }else{
          display4.setCursor(0,60);     
        }
        display4.println(valHpa);
    }else {
        display4.setFont(&DSEG7Classic_Regular20pt7b);
        display4.setCursor(0,60);             
        display4.println(valHg); 
    }
  }

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display4.display();

}

void updateDisplay5(void)
{

 // Clear the buffer
  display5.clearDisplay();
  display5.setTextColor(SSD1306_WHITE);        // Draw white text

  if(isStd){
      display5.setFont(&DSEG7Classic_Regular22pt7b);
      display5.setCursor(20,60);             
      display5.println("5td");
  }else{
    display5.setFont(&FreeSans9pt7b);
    display5.setTextSize(1);             
    display5.setCursor(85,15);             
    display5.println("QNH");
    if(isHpa){
        display5.setFont(&DSEG7Classic_Regular20pt7b);
        if(valHpa.length()==3)
        {
          display5.setCursor(32,60);             
        }else{
          display5.setCursor(0,60);     
        }
        display5.println(valHpa);
    }else {
        display5.setFont(&DSEG7Classic_Regular20pt7b);
        display5.setCursor(0,60);             
        display5.println(valHg); 
    }
  }

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display5.display();

}

void updateDisplay6(void)
{

 // Clear the buffer
  display6.clearDisplay();
  display6.setTextColor(SSD1306_WHITE);        // Draw white text

  if(isStd){
      display6.setFont(&DSEG7Classic_Regular22pt7b);
      display6.setCursor(20,60);             
      display6.println("5td");
  }else{
    display6.setFont(&FreeSans9pt7b);
    display6.setTextSize(1);             
    display6.setCursor(85,15);             
    display6.println("QNH");
    if(isHpa){
        display6.setFont(&DSEG7Classic_Regular20pt7b);
        if(valHpa.length()==3)
        {
          display6.setCursor(32,60);             
        }else{
          display6.setCursor(0,60);     
        }
        display6.println(valHpa);
    }else {
        display6.setFont(&DSEG7Classic_Regular20pt7b);
        display6.setCursor(0,60);             
        display6.println(valHg); 
    }
  }

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display6.display();

}

void updateDisplay7(void)
{

 // Clear the buffer
  display7.clearDisplay();
  display7.setTextColor(SSD1306_WHITE);        // Draw white text

  if(isStd){
      display7.setFont(&DSEG7Classic_Regular22pt7b);
      display7.setCursor(20,60);             
      display7.println("5td");
  }else{
    display7.setFont(&FreeSans9pt7b);
    display7.setTextSize(1);             
    display7.setCursor(85,15);             
    display7.println("QNH");
    if(isHpa){
        display7.setFont(&DSEG7Classic_Regular20pt7b);
        if(valHpa.length()==3)
        {
          display7.setCursor(32,60);             
        }else{
          display7.setCursor(0,60);     
        }
        display7.println(valHpa);
    }else {
        display7.setFont(&DSEG7Classic_Regular20pt7b);
        display7.setCursor(0,60);             
        display7.println(valHg); 
    }
  }

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display7.display();

}