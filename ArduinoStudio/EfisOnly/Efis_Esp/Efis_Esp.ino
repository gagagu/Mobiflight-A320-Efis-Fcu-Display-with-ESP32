
#include <Wire.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSans9pt7b.h>
#include "Fonts/DSEG7Classic_Regular20pt7b.h"  //https://github.com/keshikan/DSEG and https://rop.nl/truetype2gfx/
#include "Fonts/DSEG7Classic_Regular22pt7b.h"

#define I2C_MOBIFLIGHT_ADDR 0x27
#define I2C_MOBIFLIGHT_SDA 21
#define I2C_MOBIFLIGHT_SCL 22

#define I2C_DISPLAY_SDA 17
#define I2C_DISPLAY_SCL 16

TwoWire I2Ctwo = TwoWire(1);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);

bool isStd = false;
bool isHpa = true;
String valHpa="8888";
String valHg="88,88";


void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  Wire.onReceive(onReceive);

  Wire.begin((uint8_t)I2C_MOBIFLIGHT_ADDR,I2C_MOBIFLIGHT_SDA,I2C_MOBIFLIGHT_SCL,400000);
  I2Ctwo.begin(I2C_DISPLAY_SDA,I2C_DISPLAY_SCL,400000); // SDA pin 16, SCL pin 17, 400kHz frequency

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SCREEN_ADDRESS, true)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(2000); // Pause for 2 seconds

  updateDisplay();

}

void loop() {
  // updateDisplay();
  //  delay(3000);
  //  Serial.println("Loop");
}

void onReceive(int len){
  // Serial.println("OnReceive");
  
  char msgArray[9]="";
  // Serial.println(Wire.available());
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



void handleCommand(String command){
    
    // Serial.println("handleCommand");
    // Serial.println(command);

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

   updateDisplay();
}

void updateDisplay(void)
{

 // Clear the buffer
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);        // Draw white text

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
