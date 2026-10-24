#ifndef __STM32_U8G2_H
#define __STM32_U8G2_H

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "u8g2.h"

/* 0.96 inch SSD1306 OLED module on I2C1 (PB6 = SCL, PB7 = SDA) */
#define OLED_ADDRESS  0x78  /* 7-bit address 0x3C shifted left for HAL */
#define OLED_CMD      0x00  /* control byte: following bytes are commands */
#define OLED_DATA     0x40  /* control byte: following bytes are pixel data */

/* I2C diagnostics -- watch these in the Keil debugger */
extern volatile uint32_t i2c_tx_ok;     /* number of successful I2C transfers */
extern volatile uint32_t i2c_tx_fail;   /* number of failed I2C transfers */
extern volatile uint32_t i2c_err_code;  /* hi2c1.ErrorCode of the last failure */
extern volatile uint8_t  oled_ack_78;   /* 1 = a device ACKs at 0x78 */
extern volatile uint8_t  oled_ack_7a;   /* 1 = a device ACKs at 0x7A */
extern uint8_t oled_i2c_addr;           /* 8-bit address actually in use */

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
void u8g2Init(u8g2_t *u8g2);
void draw(u8g2_t *u8g2);

#endif
