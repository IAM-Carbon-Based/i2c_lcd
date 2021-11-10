#ifndef MCP23008_H
#define MCP23008_H

#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/i2c.h>
#include <libopencm3/stm32/rcc.h>

// i2c peripheral defines
#define I2C_PERIPH I2C1
#define I2C_RCC RCC_I2C1

#define I2C_GPIO_PORT GPIOB
#define I2C_GPIO_RCC RCC_GPIOB
#define I2C_SDA GPIO7
#define I2C_SCL GPIO6

// MCP23008 Register Addresses
#define IODIR 0x00
#define IPOL 0x01
#define GPINTEN 0x02
#define DEFVAL 0x03
#define INTCON 0x04
#define IOCON 0x05
#define GPPU 0x06
#define INTF 0x07
#define INTCAP 0x08
#define GPIO 0x09
#define OLAT 0x0A

// General Functions
void mcp23008_init(uint8_t addr, uint8_t rs, uint8_t en, uint8_t db4,
                   uint8_t db5, uint8_t db6, uint8_t db7);

void i2c_write_4bits(uint8_t data, uint8_t mode);
           

#endif