#include "i2c_lcd.h"

#include "util/delay/delay.h"
#include "util/mcp23008/mcp23008.h"
#include <string.h>

uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;

// uint8_t _initialized;

uint8_t _numlines;
uint8_t _row_offsets[4];

void lcd_init(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  mcp23008_init(I2C_ADDR, RS, EN, DB4, DB5, DB6, DB7);

  _displayfunction = LCD_4BITMODE | LCD_1LINE | LCD_5x8DOTS;  // default config

  if (lines > 1) {
    _displayfunction |= LCD_2LINE;
  }

  _numlines = lines;

  setRowOffsets(0x00, 0x40, 0x00 + cols, 0x40 + cols);

  // for some 1 line displays you can select a 10 pixel high font
  if ((dotsize != LCD_5x8DOTS) && (lines == 1)) {
    _displayfunction |= LCD_5x10DOTS;
  }

  i2c_write_4bits(0x03, 0);
  delay_us(4500);

  i2c_write_4bits(0x03, 0);
  delay_us(4500);

  i2c_write_4bits(0x03, 0);
  delay_us(150);

  i2c_write_4bits(0x02, 0);
  delay_ms(200);

  // set # lines, font size, etc..
  command(LCD_FUNCTIONSET | _displayfunction);
  delay_us(100);

  _displaycontrol |= LCD_DISPLAYON | LCD_CURSORON | LCD_BLINKON;
  display();
  delay_us(100);

  clear();
  delay_us(500);

  _displaymode |= LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
  delay_us(100);
}

void setRowOffsets(int row0, int row1, int row2, int row3) {
  _row_offsets[0] = row0;
  _row_offsets[1] = row1;
  _row_offsets[2] = row2;
  _row_offsets[3] = row3;
}

/********** high level commands, for the user! */
void clear() {
  command(LCD_CLEARDISPLAY);  // clear display, set cursor position to zero
  delay_us(2000);             // this command takes a long time!
}

void home() {
  command(LCD_RETURNHOME);  // set cursor position to zero
  delay_us(2000);           // this command takes a long time!
}

void setCursor(uint8_t col, uint8_t row) {
  const size_t max_lines = sizeof(_row_offsets) / sizeof(*_row_offsets);
  if (row >= max_lines) {
    row = max_lines - 1;  // we count rows starting w/ 0
  }
  if (row >= _numlines) {
    row = _numlines - 1;  // we count rows starting w/ 0
  }

  command(LCD_SETDDRAMADDR | (col + _row_offsets[row]));
}

// Turn the display on/off (quickly)
void noDisplay() {
  _displaycontrol &= ~LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void display() {
  _displaycontrol |= LCD_DISPLAYON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turns the underline cursor on/off
void noCursor() {
  _displaycontrol &= ~LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void cursor() {
  _displaycontrol |= LCD_CURSORON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// Turn on and off the blinking cursor
void noBlink() {
  _displaycontrol &= ~LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}
void blink() {
  _displaycontrol |= LCD_BLINKON;
  command(LCD_DISPLAYCONTROL | _displaycontrol);
}

// These commands scroll the display without changing the RAM
void scrollDisplayLeft(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVELEFT);
}
void scrollDisplayRight(void) {
  command(LCD_CURSORSHIFT | LCD_DISPLAYMOVE | LCD_MOVERIGHT);
}

// This is for text that flows Left to Right
void leftToRight(void) {
  _displaymode |= LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This is for text that flows Right to Left
void rightToLeft(void) {
  _displaymode &= ~LCD_ENTRYLEFT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'right justify' text from the cursor
void autoscroll(void) {
  _displaymode |= LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// This will 'left justify' text from the cursor
void noAutoscroll(void) {
  _displaymode &= ~LCD_ENTRYSHIFTINCREMENT;
  command(LCD_ENTRYMODESET | _displaymode);
}

// Allows us to fill the first 8 CGRAM locations
// with custom characters
void createChar(uint8_t location, uint8_t charmap[]) {
  location &= 0x7;  // we only have 8 locations 0-7
  command(LCD_SETCGRAMADDR | (location << 3));
  for (int i = 0; i < 8; i++) {
    write(charmap[i]);
  }
}

// This will print character string to the LCD
size_t print(const char str[]) {
  if (str == NULL) return 0;

  const uint8_t *buffer = (const uint8_t *)str;
  size_t size = strlen(str);

  while (size--) {
    write(*buffer++);
  }
  return size;

  // size_t n = 0;

  // while (size--) {
  //   if (write(*buffer++))
  //     n++;
  //   else
  //     break;
  // }
  // return n;
}

void command(uint8_t data) {
  i2c_write_4bits(data >> 4, 0x00);  // set RS pin LOW - send command
  i2c_write_4bits(data, 0x00);
}

void write(uint8_t data) {
  i2c_write_4bits(data >> 4, 0x01);  // Set RS pin HIGH - Write to DDRAM/CGRAM
  i2c_write_4bits(data, 0x01);
}

