
#include <Wire.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include <Adafruit_SH110X.h>
#include <Fonts/FreeSans9pt7b.h>
#include "Fonts/FreeSans6pt7b.h"
#include "Fonts/DSEG7Classic_Regular20pt7b.h"  //https://github.com/keshikan/DSEG and https://rop.nl/truetype2gfx/
#include "Fonts/DSEG7Classic_Regular22pt7b.h"
#include "Fonts/DSEG7Classic_Italic14pt7b.h"
#include "Fonts/DSEG14Classic_Italic14pt7b.h"

#define I2C_MOBIFLIGHT_ADDR 0x27
#define I2C_MOBIFLIGHT_SDA 22
#define I2C_MOBIFLIGHT_SCL 21

#define I2C_DISPLAY_SDA 17
#define I2C_DISPLAY_SCL 16

TwoWire I2Ctwo = TwoWire(1);

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32

// address of the multiplexer to change the channels
#define TCA9548A_I2C_ADDRESS  0x70
#define TCA9548A_CHANNEL_EFIS_LEFT  2
#define TCA9548A_CHANNEL_EFIS_CENTRE  1
#define TCA9548A_CHANNEL_EFIS_RIGHT 0

Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &I2Ctwo, OLED_RESET);

bool isStd = false;
bool isHpa = true;
String valHpa="8888";
String valHg="88,88";
String RightBlock = "8888";
int RightBlockMode = 0;
bool BaroMode = 0;
bool BaroDisplay = 0;
bool AltArm = false;
bool displayAll = true;

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  Wire.onReceive(onReceive);

  Wire.begin((uint8_t)I2C_MOBIFLIGHT_ADDR,I2C_MOBIFLIGHT_SDA,I2C_MOBIFLIGHT_SCL,400000);
  I2Ctwo.begin(I2C_DISPLAY_SDA,I2C_DISPLAY_SCL,400000); // SDA pin 16, SCL pin 17, 400kHz frequency


  setTCAChannel(TCA9548A_CHANNEL_EFIS_LEFT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SCREEN_ADDRESS, true)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  updateDisplayLeft();

  setTCAChannel(TCA9548A_CHANNEL_EFIS_CENTRE);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SCREEN_ADDRESS, true)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  updateDisplayCentre();

  setTCAChannel(TCA9548A_CHANNEL_EFIS_RIGHT);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SCREEN_ADDRESS, true)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  updateDisplayRight();

}

void loop() {
  if (displayAll)
  {
    showAllRight();
    showAllCentre();
    showAllLeft();
  } else {   
    updateDisplayRight();
    updateDisplayCentre();
    updateDisplayLeft();
  }
  delay(50);

  //  delay(3000);
  //  Serial.println("Loop");
}

void showAllRight(void)
{
  setTCAChannel(TCA9548A_CHANNEL_EFIS_RIGHT);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);        // Draw white text

  display.setFont(&FreeSans6pt7b);
  display.setTextSize(1);     
  
  display_alert();
  display_ft();
  display_hpa();
  display_inhg();
  display_fpm();
  display_rightblock("12345");

  display.display();

//   delay(3000);
//   display.clearDisplay();
//   display_rightblock("4321");
  
//   display.display();

//  delay(3000);
//   display.clearDisplay();
//   display_rightblock("789");
  
//   display.display();
  
//  delay(3000);
}

void showAllCentre(void)
{ 
  setTCAChannel(TCA9548A_CHANNEL_EFIS_CENTRE);
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);        // Draw white text

  display.setFont(&FreeSans6pt7b);
  display.setTextSize(1);     

  display_VS();
  display_ALT();
  display_ALTARM();
  
  display.display();
}

void showAllLeft(void)
{ 
  setTCAChannel(TCA9548A_CHANNEL_EFIS_LEFT);
   display.clearDisplay();
  display.setTextColor(SH110X_WHITE);        // Draw white text

  display_ROLLMODE_APR();
  display_AP_Symbol();
  display_NAV();
  display_ALTARM();
  
  display.display();
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
    
  if(command.startsWith("#0")){
       if(command.substring(2)=="0"){
        RightBlockMode=0;
      } else if (command.substring(2)=="1") {
        RightBlockMode=1;
      } else if (command.substring(2)=="2") {
        RightBlockMode=2;
      }
  }
  else if(command.startsWith("#1")){
    Serial.println(command);
     RightBlock=command.substring(2);
  }
  else if(command.startsWith("#2")){
      if(command.substring(2)=="0"){
        BaroMode=false;
      }else{
        BaroMode = true;
      }
  }
  else if(command.startsWith("#3")){
    if(command.substring(2)=="0"){
      BaroDisplay=false;
    }else{
      BaroDisplay=true;
    }
  }
  else if(command.startsWith("#4")){
      if(command.substring(2)=="0"){
        AltArm=false;
      }else{
        AltArm=true;
      }
  }
}


void updateDisplayRight(void)
{
  setTCAChannel(TCA9548A_CHANNEL_EFIS_RIGHT);

 // Clear the buffer
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);        // Draw white text

  display.setFont(&FreeSans6pt7b);
  display.setTextSize(1);     
  // if(isAlert)
  // {
  //   displayAlert();
  // }

  if(BaroDisplay && BaroMode)
  {
    display_inhg();
  }

  if(BaroDisplay && !BaroMode)
  {
    display_hpa();
  }

  // if(isFPM)
  // { 
  //   display_fpm();
  // }

  if(!BaroDisplay)
  {
    display_ft();
  }

  display_rightblock(RightBlock);

  // display.setFont(&DSEG7Classic_Italic14pt7b); //&DSEG7Classic_Regular9pt7b);
  // display.setCursor(58,30);             
  // display.println("500");
  
  // display.setFont(&DSEG14Classic_Italic14pt7b); //&DSEG7Classic_Regular9pt7b);
  // display.setCursor(40,38);             
  // display.println(",");

  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();
}


void updateDisplayCentre(void)
{
  setTCAChannel(TCA9548A_CHANNEL_EFIS_CENTRE);
  
 // Clear the buffer
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);        // Draw white text

  display_VS();
  display_ALT();
  display_ALTARM();
 
  // Show the display buffer on the screen. You MUST call display() after
  // drawing commands to make them visible on screen!
  display.display();

}

void updateDisplayLeft(void)
{
  setTCAChannel(TCA9548A_CHANNEL_EFIS_LEFT);

  display.display();
}


// Right hand section

void display_inhg(void)
{
  display.setCursor(80,61);             
  display.println("IN  HG");
}

void display_hpa(void)
{
  display.setCursor(25,61);             
  display.println("HPA");
}

void display_ft(void)
{
  display.setCursor(102,47);             
  display.println("FT");
}

void display_fpm(void)
{
  display.setCursor(66,47);             
  display.println("FPM");
}

void display_alert(void)
{
  display.setCursor(10,47);             
  display.println("ALERT");
}

void display_rightblock(String rightBlockValue)
{
  String rightString = rightBlockValue.substring(rightBlockValue.length()-3);
  String leftString = rightBlockValue.substring(0,rightBlockValue.length()-3);

  display.setFont(&DSEG7Classic_Italic14pt7b); //&DSEG7Classic_Regular9pt7b);
  display.setCursor(55,30);             
  display.println(rightString);
    
  if (leftString.length() > 0) {
    display.setFont(&DSEG7Classic_Italic14pt7b); //&DSEG7Classic_Regular9pt7b);
    if (leftString.length()==2){  
      display.setCursor(4,30); 
    } else if (leftString.length()==1) {
      display.setCursor(25,30); 
    }            
    display.println(leftString);

    display.setFont(&DSEG14Classic_Italic14pt7b); //&DSEG7Classic_Regular9pt7b);
    display.setCursor(43,38);             
    display.println(",");
  }
}


// Middle section

void display_VS(void)
{
  display.setFont(&DSEG14Classic_Italic14pt7b);
  display.setCursor(45,25);             
  display.println("VS");
}

void display_ALT(void)
{
  
  display.setFont(&DSEG14Classic_Italic14pt7b);
  display.setCursor(20,61);             
  display.println("ALT");
}

void display_ALTARM(void)
{
  display.setFont(&FreeSans6pt7b);
  display.setCursor(90,36);
  display.println("A");
  display.setCursor(90,48);
  display.println("R");
  display.setCursor(90,60);
  display.println("M");
}


// Middle section

void display_ROLLMODE_ROL(void)
{
  display.setFont(&DSEG14Classic_Italic14pt7b);
  display.setCursor(20,25);             
  display.println("ROL");
}

void display_ROLLMODE_HDG(void)
{
  display.setFont(&DSEG14Classic_Italic14pt7b);
  display.setCursor(20,25);             
  display.println("HDG");
}

void display_ROLLMODE_REV(void)
{
  display.setFont(&DSEG14Classic_Italic14pt7b);
  display.setCursor(20,25);             
  display.println("REV");
}

void display_ROLLMODE_APR(void)
{
  display.setFont(&DSEG14Classic_Italic14pt7b);
  display.setCursor(20,25);             
  display.println("APR");
}

void display_AP(void)
{
  display.setFont(&DSEG14Classic_Italic14pt7b);
  display.setCursor(20,25);             
  display.println("AP");
}

void display_AP_Symbol(void)
{
  display.setFont(&FreeSans6pt7b);
  display.setCursor(107,14);             
  display.println("AP");
  display.drawFastHLine(105, 2, 19, SH110X_WHITE);
  display.drawFastVLine(103, 4, 13, SH110X_WHITE);
  display.drawFastHLine(105, 18, 19, SH110X_WHITE);
  display.drawFastVLine(125, 4, 13, SH110X_WHITE);
}

void display_NAV(void)
{
  display.setFont(&DSEG14Classic_Italic14pt7b);
  display.setCursor(20,61);             
  display.println("NAV");
}
void display_REV(void)
{
  display.setFont(&DSEG14Classic_Italic14pt7b);
  display.setCursor(20,61);             
  display.println("REV");
}
void display_GS(void)
{
  display.setFont(&DSEG14Classic_Italic14pt7b);
  display.setCursor(45,61);             
  display.println("GS");
}
void display_LEFTARM(void)
{  
  display.setFont(&FreeSans6pt7b);
  display.setCursor(90,36);
  display.println("A");
  display.setCursor(90,48);
  display.println("R");
  display.setCursor(90,60);
  display.println("M");
}
