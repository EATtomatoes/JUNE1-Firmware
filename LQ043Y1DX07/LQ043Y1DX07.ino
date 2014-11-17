#include <EEPROM.h>
#include <Wire.h>
#include "spi_gpio.h"
#include "lcd.h"
#include "config_hdmi.h"

/*
@buttons
*/
#define BTN_ON A1
#define BTN_UP A2
#define BTN_DN A3

void btn_init()
{
  pinMode(BTN_ON, INPUT_PULLUP);
  pinMode(BTN_UP, INPUT_PULLUP);
  pinMode(BTN_DN, INPUT_PULLUP);
}

boolean btn_down(int button)
{
    switch(button)
    {
        case BTN_ON:
            return !digitalRead(BTN_ON);
        case BTN_UP:
            return !digitalRead(BTN_UP);
        case BTN_DN:
            return !digitalRead(BTN_DN);
    }
    return 0;
}

/*
@group LED
*/

#define LED_SLEEP 13

void led_init()
{
  pinMode(LED_SLEEP, OUTPUT);
}

void blinkSleep(int times)
{
  for (int i=0; i<=times; i++)
  {
    digitalWrite(LED_SLEEP, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(30);               // wait for a second
    digitalWrite(LED_SLEEP, LOW);    // turn the LED off by making the voltage LOW
    delay(30);
  }
}

void led_en(boolean on)
{
  if (on)
  {
    digitalWrite(LED_SLEEP, HIGH);
  } else {
    digitalWrite(LED_SLEEP, LOW);
  }
}

void led_on()
{
  led_en(1);
}

void led_off()
{
  led_en(0);
}

#define BL_SEL 6

void bl_init()
{
  pinMode(BL_SEL, OUTPUT);
  bl_off();
}

void bl_en(boolean on)
{
  if (on)
  {
    digitalWrite(BL_SEL, HIGH);
  } else {
    digitalWrite(BL_SEL, LOW);
  }
}

void bl_off()
{
    bl_en(0);
}
 
void bl_on()
{
    bl_en(1);
}

void bl_brightness(uint8_t brightness)
{
  if (brightness == 2)
  {
    bl_off();
  }else if (brightness == 255) {
    bl_on();
  }
  else
  {  
    analogWrite(BL_SEL, brightness);
  }
}

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  
  btn_init();
  led_init();
  
  edid_init();
  delay(10);
  edid_wp(0); //@EEPROM Write Enabled
  delay(10);
  edidWrite(EDID);
  delay(10);
  //edid_wp(1); //@EEPROM Write Protected
  
  blinkSleep(10);
  
  tfp401_init();
  
  hpd_init();
  delay(20);
  
  hpd_on();
  
  delay(100);
  
  spi_gpio.init();
  
  bl_init();
  
}

/*
@group EEPROM Settings Persistence
*/
#define EE_MODE         0
#define EE_BACK         1
#define EE_FREE         3

uint8_t CURRENT_MODE;
uint16_t BRIGHTNESS;

#define MODE_INIT       0
#define MODE_GO_SLEEP   1
#define MODE_SLEEP      2
#define MODE_WAKEUP     3
#define MODE_AWAKE      4


static void save_mode()
{
  EEPROM.write(EE_MODE, CURRENT_MODE);
}

static void save_brightness()
{
  EEPROM.write(EE_BACK, BRIGHTNESS);
}

void load_all()
{
  CURRENT_MODE = EEPROM.read(EE_MODE);
  BRIGHTNESS = EEPROM.read(EE_BACK);
}

void loop()
{

  
  while(1)
  {
    switch(CURRENT_MODE)
    {
      case MODE_INIT:
        Serial.println("MODE_INIT");
        load_all();
        
        if (CURRENT_MODE == 0xFF) CURRENT_MODE = MODE_GO_SLEEP;
        if (BRIGHTNESS == 0xFF) BRIGHTNESS = 100;
        
      
        delay(50);
        
      break;
      
      case MODE_GO_SLEEP:
        Serial.println("MODE_GO_SLEEP");
        
        led_off();
        
        lcd_sleep();
        
        hpd_off();
        
        tfp401_off();
        
        bl_off();
        
        delay(50);
        CURRENT_MODE = MODE_SLEEP;
        delay(100);
      break;
      
      case MODE_SLEEP:
      
        Serial.println("MODE_SLEEP");
        
        if ( btn_down(BTN_ON) )
        {
          while(btn_down(BTN_ON));
          CURRENT_MODE = MODE_WAKEUP;
          save_mode();
        }
        
        Serial.print("EEPROM Mode is");
        Serial.println(EEPROM.read(EE_MODE));
        
        delay(50);
      break;
      
      case MODE_WAKEUP:
      
        Serial.println("MODE_WAKEUP");
        
        hpd_on();
        delay(100);
        
        tfp401_on();
        delay(100);
        
        lcd_rst();
        lcd_init();
        led_on();
        delay(100);
        
        bl_on();
        bl_brightness(BRIGHTNESS);
        
        delay(100); //@Wait until backlight on
        
        CURRENT_MODE = MODE_AWAKE;
        
      break;
      
      case MODE_AWAKE:  //@Normal Operation
        
        Serial.println("MODE_AWAKE");
        
        if (get_hpd() == false)
        {
          bl_brightness(2);
        }
        else
        {
          bl_brightness(BRIGHTNESS);
        }
       
        //@Turn Off
        if (btn_down(BTN_ON))
        {
          while(btn_down(BTN_ON));
          CURRENT_MODE = MODE_GO_SLEEP;
          save_mode();
        }
        
        if (btn_down(BTN_UP))
        {
          if (BRIGHTNESS != 200)
          {
            BRIGHTNESS ++;
            bl_brightness(BRIGHTNESS);
          }
          save_brightness();
          delay(10);
        }
        
        if (btn_down(BTN_DN))
        {
          // Don't let it get dimmer than 2/255
          if (BRIGHTNESS > 2)
          {
            BRIGHTNESS --;
            bl_brightness(BRIGHTNESS);
          }
          save_brightness();
          delay(10);
        }
        
        Serial.print("Brightness is: ");
        Serial.println(BRIGHTNESS);
        
      break;
    }//@end Switch
    
    delay(10);
    
  }//@end while
  
}

void lcd_rst()
{
  digitalWrite(LCD_RST, HIGH); // Reset high
  delay(100); // Wait for power to become stable.
  digitalWrite(LCD_RST, LOW);
  delay(10); // Page 13 in manual for LCD
  digitalWrite(LCD_RST, HIGH); // Reset high
  delay(6); // Page 13 in manual for LCD
}

void lcd_init(void)
{
  
  Serial.println("LCD Init");
  
  // Sleep out command
  digitalWrite(LCD_CS, LOW);
  spi_gpio.cmd(0x11);
  digitalWrite(LCD_CS, HIGH);
  delay(100);
  
  // Display on command
  digitalWrite(LCD_CS, LOW);
  spi_gpio.cmd(0x29);
  digitalWrite(LCD_CS, HIGH);
  
  // Interface pixel format
  digitalWrite(LCD_CS, LOW);
  spi_gpio.cmd(0x3A);
  spi_gpio.data(0x70);
  digitalWrite(LCD_CS, HIGH);
  
  // Memory Access Control
  digitalWrite(LCD_CS, LOW);
  spi_gpio.cmd(0x36);
  spi_gpio.data(0x00);
  digitalWrite(LCD_CS, HIGH);
}

void lcd_sleep(void)
{
  // Sleep out command
  digitalWrite(LCD_CS, LOW);
  spi_gpio.cmd(0x28);
  digitalWrite(LCD_CS, HIGH);
  delay(110);
  // Display on command
  digitalWrite(LCD_CS, LOW);
  spi_gpio.cmd(0x10);
  digitalWrite(LCD_CS, HIGH);
  delay(60);
}

void lcd_ext(void)
{
  digitalWrite(LCD_CS, LOW);
  spi_gpio.cmd(0xB9);
  spi_gpio.data(0xFF);
  spi_gpio.data(0x83);
  spi_gpio.data(0x63);
  digitalWrite(LCD_CS, HIGH);
}

void lcd_gamma()
{
  int i;
  digitalWrite(LCD_CS, LOW);
  spi_gpio.cmd(0xC1);// Memory Access Control
  spi_gpio.data(0x00);// DCG_EN = 0
  
  for(i=0; i<128; i++)
  {
    spi_gpio.data(i*0x00);// Gamma shit
  }
  
  
  for(i=0; i<=33; i++)
  {
    spi_gpio.data(i*0x08);// Gamma shit
  }
  for(i=34; i<=42; i++)
  {
    spi_gpio.data(i*0x00);// Gamma shit
  }
  for(i=43; i<=75; i++)
  {
    spi_gpio.data((i-0x43)*0x08);// Gamma shit
  }
  digitalWrite(LCD_CS, HIGH);
}

void lcd_test(void)
{
  char data;
  Serial.print("Reading LCD version ID: 0x");
  digitalWrite(LCD_CS, LOW);
  data =spi_gpio.read(0xDB);
  digitalWrite(LCD_CS, HIGH);
  Serial.println(data, HEX);
  
  Serial.print("Display power mode:");
  digitalWrite(LCD_CS, LOW); // Select Chip Enable low
  data =spi_gpio.read(0x0A);
  digitalWrite(LCD_CS, HIGH);
  Serial.println(data, HEX);
  
  if(data & (1<<7))
    Serial.println("Booster OK");
  else
    Serial.println("Booster Error");

  if(data & (1<<4))
    Serial.println("Sleep out mode");
  else
    Serial.println("Sleep in mode");
  
  if(data & (1<<3))
    Serial.println("Display mode normal");
  else
    Serial.println("Display mode not normal");

  if(data & (1<<2))
    Serial.println("Display is on");
  else
    Serial.println("Display is off");

  Serial.print("Display pixel format: ");
  digitalWrite(LCD_CS, LOW); // Select Chip Enable low
  data =spi_gpio.read(0x0C);
  digitalWrite(LCD_CS, HIGH);
  Serial.println(data, HEX);
  
  if(data == 0x70)
    Serial.println("24 bit pixel format");
  else if(data == 0x50)
    Serial.println("16 bit pixel format");
  else if(data == 0x60)
    Serial.println("18 bit pixel format");
  else
    Serial.println("Unknown pixel format");

  Serial.print("Display image mode: ");
  digitalWrite(LCD_CS, LOW); // Select Chip Enable low
  data =spi_gpio.read(0x0D);
  digitalWrite(LCD_CS, HIGH);
  Serial.println(data, HEX);

  Serial.print("Display signal mode: ");
  digitalWrite(LCD_CS, LOW); // Select Chip Enable low
  data =spi_gpio.read(0x0E);
  digitalWrite(LCD_CS, HIGH);
  Serial.println(data, HEX);

  Serial.print("Display self diagnostic: 0x");
  digitalWrite(LCD_CS, LOW); // Select Chip Enable LOW
  data =spi_gpio.read(0x0F);
  digitalWrite(LCD_CS, HIGH);  // Select Chip Enable HIGH
  Serial.println(data, HEX);
  
  if(data & (1<<7))
    Serial.println("OTP match");
  else
    Serial.println("OTP mismatch");

  if(data & (1<<6))
    Serial.println("Booster level and timings OK");
  else
    Serial.println("Booster shit error");
  
  Serial.print("Read RGB Red: 0x");
  digitalWrite(LCD_CS, LOW); // Select Chip Enable low
  data =spi_gpio.read(CMD_RDRED);
  digitalWrite(LCD_CS, HIGH);
  Serial.println(data, HEX);
  
  Serial.print("Read RGB Blue: 0x");
  digitalWrite(LCD_CS, LOW); // Select Chip Enable low
  data =spi_gpio.read(CMD_RDBLUE);
  digitalWrite(LCD_CS, HIGH);
  Serial.println(data, HEX);
  
  Serial.print("Read RGB Green: 0x");
  digitalWrite(LCD_CS, LOW); // Select Chip Enable low
  data =spi_gpio.read(CMD_RDGREEN);
  digitalWrite(LCD_CS, HIGH);
  Serial.println(data, HEX);

  digitalWrite(LCD_CS, LOW); // Select Chip Enable low
  spi_gpio.cmd(0xB3); // Set RGB interface related register (B3h)
  spi_gpio.data(0x01);  // EPL + VSPL + HSPL + DPL
  digitalWrite(LCD_CS, HIGH);
}
