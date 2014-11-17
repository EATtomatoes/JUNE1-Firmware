#include <Arduino.h>

#ifndef SPI_h
#define SPI_h

#define LCD_SCK SCK
#define LCD_MOSI MOSI
#define LCD_MISO MISO
#define LCD_CS 4

class SPI_GPIO
{
  public:
  
    SPI_GPIO();
    void init();
    void cmd(byte);
    void data(byte);
    char read(byte);

};

extern SPI_GPIO spi_gpio;

#endif
