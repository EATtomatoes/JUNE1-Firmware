#include "spi_gpio.h"

SPI_GPIO::SPI_GPIO()
{
}

void SPI_GPIO::init()
{
  pinMode(LCD_CS, OUTPUT);
  pinMode(LCD_MOSI, OUTPUT);
  pinMode(LCD_MISO, OUTPUT);
  pinMode(LCD_SCK, OUTPUT);
  digitalWrite(LCD_CS, HIGH);
  digitalWrite(LCD_MOSI, LOW);
  digitalWrite(LCD_MISO, LOW);
  digitalWrite(LCD_SCK, LOW);
  delay(100);
}

void SPI_GPIO::cmd(byte cmd)
{
  unsigned int  i;
  digitalWrite(LCD_MOSI, 0); //Set DC=0, for writing to Command register
  digitalWrite(LCD_SCK, 0);
  digitalWrite(LCD_SCK, 1);
  digitalWrite(LCD_SCK, 0);
  for(i=0;i<8;i++)
  {
    if((cmd&0x80)==0x80) digitalWrite(LCD_MOSI, 1);
    else  digitalWrite(LCD_MOSI, 0);
    digitalWrite(LCD_SCK, 1);
    digitalWrite(LCD_SCK, 0);
    cmd = cmd<<1;	
  }
}

void SPI_GPIO::data(byte value)
{
  unsigned int  i;
  digitalWrite(LCD_MOSI, HIGH);
  digitalWrite(LCD_SCK, LOW);
  digitalWrite(LCD_SCK, HIGH);
  digitalWrite(LCD_SCK, LOW);

  for(i=0;i<8;i++)
  {
    if((value&0x80)==0x80) digitalWrite(LCD_MOSI, HIGH);
    else         digitalWrite(LCD_MOSI, LOW);
    digitalWrite(LCD_SCK, HIGH);
    digitalWrite(LCD_SCK, LOW);
    value = value<<1;	
  }
}

char SPI_GPIO::read(byte addr)
{
  int  i;
  char data = 0;
  
  digitalWrite(LCD_MOSI, HIGH);
  digitalWrite(LCD_SCK, LOW);
  digitalWrite(LCD_SCK, HIGH);
  digitalWrite(LCD_SCK, LOW);

  for(i=0;i<8;i++)
  {
    if((addr&0x80)==0x80) digitalWrite(LCD_MOSI, HIGH);
    else         digitalWrite(LCD_MOSI, LOW);
    digitalWrite(LCD_SCK, HIGH);
    digitalWrite(LCD_SCK, LOW);
    addr = addr<<1;
  }
}

SPI_GPIO spi_gpio = SPI_GPIO();
