#ifndef I2C_LCD_H
#define I2C_LCD_H

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

// 7-Bit Address of mcp23008 / LCD
#define I2C_ADDR 0x20

// i2c peripheral defines
#define I2C_PERIPH I2C1
#define I2C_RCC RCC_I2C1

#define I2C_GPIO_PORT GPIOB
#define I2C_GPIO_RCC RCC_GPIOB
#define I2C_SDA GPIO7
#define I2C_SCL GPIO6

// MCP23008 Register Addresses (only uncommented registers are used)
#define IODIR 0x00
// #define IPOL 0x01
// #define GPINTEN 0x02
// #define DEFVAL 0x03
// #define INTCON 0x04
#define IOCON 0x05
// #define GPPU 0x06
// #define INTF 0x07
// #define INTCAP 0x08
#define GPIO 0x09
// #define OLAT 0x0A

// Pins used on the mcp23008
#define RS 2
#define EN 3
#define DB4 4
#define DB5 5
#define DB6 6
#define DB7 7

// Defines Related to LCD

// commands
#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

void lcd_init(uint8_t cols, uint8_t lines, uint8_t dotsize);

// LCD Functions
void clear(void);
void home(void);
void backlight_set_color(const uint32_t color);

void noDisplay(void);
void display(void);
void noBlink(void);
void blink(void);
void noCursor(void);
void cursor(void);
void scrollDisplayLeft(void);
void scrollDisplayRight(void);
void leftToRight(void);
void rightToLeft(void);
void autoscroll(void);
void noAutoscroll(void);

void setRowOffsets(int row1, int row2, int row3, int row4);
void createChar(uint8_t, uint8_t[]);
void setCursor(uint8_t, uint8_t);

void command(uint8_t data);
void write(uint8_t data);
void i2c_write_4bits(uint8_t data, uint8_t mode);

size_t print(const char str[]);

#endif