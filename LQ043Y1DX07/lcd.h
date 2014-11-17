#include <Arduino.h>

#define LCD_RST 9

#define CMP_NOP          0x00
#define CMD_SWRESET      0x01
#define CMD_RDRED        0x06
#define CMD_RDGREEN      0x07
#define CMD_RDBLUE       0x08
#define CMD_RDDPM        0x0A
#define CMD_RDDMADCTL    0x0B
#define CMD_RDDCOLMOD    0x0C

