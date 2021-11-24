#include "i2c_lcd.h"

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/timer.h>
#include <string.h>

#include "util/delay/delay.h"

uint8_t _displayfunction;
uint8_t _displaycontrol;
uint8_t _displaymode;

// uint8_t _initialized;

uint8_t _numlines;
uint8_t _row_offsets[4];

static uint8_t _i2c_addr;

static uint8_t _rs;
static uint8_t _en;
static uint8_t _data_pins[4];

/**
 * @brief initialize i2c
 * Only Tested on STM32F411CE
 */
void _init_i2c(uint8_t addr) {
  rcc_periph_clock_enable(I2C_RCC);
  rcc_periph_clock_enable(I2C_GPIO_RCC);

  _i2c_addr = addr;

  // Set Alternate Function, Disable Pull-up/down on PB6,7 for i2c
  gpio_mode_setup(I2C_GPIO_PORT, GPIO_MODE_AF, GPIO_PUPD_NONE,
                  I2C_SDA | I2C_SCL);
  // Set Open-Drain on PB6,7
  gpio_set_output_options(I2C_GPIO_PORT, GPIO_OTYPE_OD, GPIO_OSPEED_50MHZ,
                          I2C_SDA | I2C_SCL);
  // Set i2c alternat fuction
  gpio_set_af(I2C_GPIO_PORT, GPIO_AF4, I2C_SDA | I2C_SCL);

  // Set i2c bus speed and enable i2c1
  i2c_set_speed(I2C_PERIPH, i2c_speed_sm_100k, 50);
  i2c_peripheral_enable(I2C_PERIPH);
}

void _mcp23008_init(uint8_t addr, uint8_t rs, uint8_t en, uint8_t db4,
                    uint8_t db5, uint8_t db6, uint8_t db7) {
  _i2c_addr = addr;
  _rs = rs;
  _en = en;
  _data_pins[0] = db4;
  _data_pins[1] = db5;
  _data_pins[2] = db6;
  _data_pins[3] = db7;

  _init_i2c(_i2c_addr);
  _init_mcp23008_io();
}

void _init_mcp23008_io(void) {
  // Initialize MCP23008, Disable sequential operation, Set all pins output/low
  uint8_t disable_SEQOP[] = {IOCON, GPIO5};
  uint8_t set_all_output[] = {IODIR, 0x00};
  uint8_t set_output_low[] = {GPIO, 0x00};

  i2c_transfer7(I2C_PERIPH, _i2c_addr, disable_SEQOP, 2, 0, 0);
  i2c_transfer7(I2C_PERIPH, _i2c_addr, set_all_output, 2, 0, 0);
  i2c_transfer7(I2C_PERIPH, _i2c_addr, set_output_low, 2, 0, 0);
}

void _backlight_init() {
  rcc_periph_clock_enable(RCC_GPIOB);
  rcc_periph_clock_enable(RCC_TIM3);
  rcc_periph_reset_pulse(RST_TIM3);

  gpio_mode_setup(GPIOB, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO0 | GPIO4 | GPIO5);

  gpio_set_output_options(GPIOB, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,
                          GPIO0 | GPIO4 | GPIO5);

  gpio_set_af(GPIOB, GPIO_AF2, GPIO0 | GPIO4 | GPIO5);

  timer_set_mode(TIM3, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
  timer_enable_preload(TIM3);
  timer_enable_oc_preload(TIM3, TIM_OC1);

  timer_set_prescaler(TIM3, 100000000 / 63750 - 1);
  timer_set_period(TIM3, 0xFF);

  timer_enable_oc_output(TIM3, TIM_OC1);
  timer_enable_oc_output(TIM3, TIM_OC2);
  timer_enable_oc_output(TIM3, TIM_OC3);

  timer_set_oc_mode(TIM3, TIM_OC1, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM3, TIM_OC2, TIM_OCM_PWM1);
  timer_set_oc_mode(TIM3, TIM_OC3, TIM_OCM_PWM1);

  timer_set_oc_polarity_high(TIM3, TIM_OC1);
  timer_set_oc_polarity_high(TIM3, TIM_OC2);
  timer_set_oc_polarity_high(TIM3, TIM_OC3);

  timer_set_oc_slow_mode(TIM3, TIM_OC1);
  timer_set_oc_slow_mode(TIM3, TIM_OC2);
  timer_set_oc_slow_mode(TIM3, TIM_OC3);

  // timer_set_oc_value(TIM3, TIM_OC1, 0x88);
  // timer_set_oc_value(TIM3, TIM_OC2, 0x44);
  // timer_set_oc_value(TIM3, TIM_OC3, 0x22);

  timer_generate_event(TIM3, TIM_EGR_UG);
  timer_enable_counter(TIM3);
}

void lcd_init(uint8_t cols, uint8_t lines, uint8_t dotsize) {
  delay_setup();
  _backlight_init();

  backlight_set_color(0x00FF1A);

  _mcp23008_init(I2C_ADDR, RS, EN, DB4, DB5, DB6, DB7);

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

void backlight_set_color(const uint32_t color) {
  timer_set_oc_value(TIM3, TIM_OC1, ((color >> 16) & 0xFF));
  timer_set_oc_value(TIM3, TIM_OC2, ((color >> 8) & 0xFF));
  timer_set_oc_value(TIM3, TIM_OC3, (color & 0xFF));
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

//* Delay between nibble write seems to be needed for LCD to register a write.
void i2c_write_4bits(uint8_t data, uint8_t mode) {
  uint8_t out = 0x00;

  // set data pins
  for (uint8_t i = 0; i < 4; i++) {
    out &= ~(1 << _data_pins[i]);
    out |= ((data >> i) & 0x1) << _data_pins[i];
  }

  // check if sending command(False) or writing to DDRAM/CGRAM(True)
  if (mode) {
    out |= (1 << _rs);
  }

  // TODO: Maybe make this better at some point
  uint8_t _i2c_data[4];
  _i2c_data[0] = GPIO;  // set mcp23008 reg addr

  // Set Data pins & toggle en
  _i2c_data[1] = out;
  out |= (1 << _en);
  _i2c_data[2] = out;  // set enable high
  out &= ~(1 << _en);
  _i2c_data[3] = out;  // set enable low

  // i2c_transfer7(I2C_PERIPH, I2C_ADDR, _i2c_data, 4, 0, 0);
  while ((I2C_SR2(I2C_PERIPH) & I2C_SR2_BUSY)) {
  }

  i2c_send_start(I2C_PERIPH);

  // Wait for the end of the start condition, master mode selected, and BUSY bit set
  while (!((I2C_SR1(I2C_PERIPH) & I2C_SR1_SB) &&
           (I2C_SR2(I2C_PERIPH) & I2C_SR2_MSL) &&
           (I2C_SR2(I2C_PERIPH) & I2C_SR2_BUSY)))
    ;

  i2c_send_7bit_address(I2C_PERIPH, _i2c_addr, I2C_WRITE);

  /* Waiting for address is transferred. */
  while (!(I2C_SR1(I2C_PERIPH) & I2C_SR1_ADDR))
    ;

  /* Clearing ADDR condition sequence. */
  (void)I2C_SR2(I2C_PERIPH);

  for (size_t i = 0; i < 4; i++) {
    i2c_send_data(I2C_PERIPH, _i2c_data[i]);
    while (!(I2C_SR1(I2C_PERIPH) & (I2C_SR1_BTF)))
      ;
    delay_us(1);
  }

  i2c_send_stop(I2C_PERIPH);
}
