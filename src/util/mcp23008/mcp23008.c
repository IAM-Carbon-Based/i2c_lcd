#include "mcp23008.h"

#include "util/delay/delay.h"
#include <libopencm3/stm32/i2c.h>

static uint8_t _i2c_addr;

static uint8_t _rs;
static uint8_t _en;
static uint8_t _data_pins[4];

static void _init_i2c(uint8_t addr);
static void _init_io(void);

void mcp23008_init(uint8_t addr, uint8_t rs, uint8_t en, uint8_t db4,
                   uint8_t db5, uint8_t db6, uint8_t db7) {
  _i2c_addr = addr;
  _rs = rs;
  _en = en;
  _data_pins[0] = db4;
  _data_pins[1] = db5;
  _data_pins[2] = db6;
  _data_pins[3] = db7;

  _init_i2c(_i2c_addr);
  _init_io();
}

/**
 * @brief initialize i2c
 * Only Tested on STM32F411CE
 */
static void _init_i2c(uint8_t addr) {
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

static void _init_io(void) {
  // Initialize MCP23008, Disable sequential operation, Set all pins output/low
  uint8_t disable_SEQOP[] = {IOCON, GPIO5};
  uint8_t set_all_output[] = {IODIR, 0x00};
  uint8_t set_output_low[] = {GPIO, 0x00};

  i2c_transfer7(I2C_PERIPH, _i2c_addr, disable_SEQOP, 2, 0, 0);
  i2c_transfer7(I2C_PERIPH, _i2c_addr, set_all_output, 2, 0, 0);
  i2c_transfer7(I2C_PERIPH, _i2c_addr, set_output_low, 2, 0, 0);
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

  /* Wait for the end of the start condition, master mode selected, and BUSY bit
   * set */
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
