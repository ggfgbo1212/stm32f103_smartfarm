#include "stm32_u8g2.h"
#include "tim.h"
#include "i2c.h"

/* I2C diagnostics (visible in the debugger watch window) */
volatile uint32_t i2c_tx_ok;
volatile uint32_t i2c_tx_fail;
volatile uint32_t i2c_err_code;
volatile uint8_t  oled_ack_78;
volatile uint8_t  oled_ack_7a;
uint8_t oled_i2c_addr = OLED_ADDRESS;

/* Hardware I2C byte transfer callback used by u8x8.
 * u8g2/u8x8 never sends more than 32 bytes between START_TRANSFER and END_TRANSFER. */
uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    static uint8_t buffer[128];
    static uint8_t buf_idx;
    uint8_t *data;

    switch (msg)
    {
    case U8X8_MSG_BYTE_INIT:
        /* I2C is already set up by MX_I2C1_Init() in main(); re-init is harmless */
        MX_I2C1_Init();
        break;

    case U8X8_MSG_BYTE_START_TRANSFER:
        buf_idx = 0;
        break;

    case U8X8_MSG_BYTE_SEND:
        data = (uint8_t *)arg_ptr;
        while (arg_int > 0)
        {
            buffer[buf_idx++] = *data;
            data++;
            arg_int--;
        }
        break;

    case U8X8_MSG_BYTE_END_TRANSFER:
        if (HAL_I2C_Master_Transmit(&hi2c1, oled_i2c_addr, buffer, buf_idx, 1000) != HAL_OK)
        {
            i2c_err_code = hi2c1.ErrorCode;
            i2c_tx_fail++;
            return 0;
        }
        i2c_tx_ok++;
        break;

    case U8X8_MSG_BYTE_SET_DC:
        break;

    default:
        return 0;
    }

    return 1;
}

/* GPIO and delay callback used by u8x8.
 * The display is driven by hardware I2C, so the GPIO emulation branches do nothing. */
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
    switch (msg)
    {
    case U8X8_MSG_DELAY_100NANO: /* delay arg_int * 100 nano seconds */
        __NOP();
        break;
    case U8X8_MSG_DELAY_10MICRO: /* delay arg_int * 10 micro seconds */
        for (uint16_t n = 0; n < 320; n++)
        {
            __NOP();
        }
        break;
    case U8X8_MSG_DELAY_MILLI: /* delay arg_int * 1 milli second */
        HAL_Delay(arg_int);
        break;
    case U8X8_MSG_DELAY_I2C: /* arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz */
        Tims_delay_us(5);
        break;
    case U8X8_MSG_GPIO_I2C_CLOCK:
        break;
    case U8X8_MSG_GPIO_I2C_DATA:
        break;
    case U8X8_MSG_GPIO_MENU_SELECT:
        u8x8_SetGPIOResult(u8x8, 0);
        break;
    case U8X8_MSG_GPIO_MENU_NEXT:
        u8x8_SetGPIOResult(u8x8, 0);
        break;
    case U8X8_MSG_GPIO_MENU_PREV:
        u8x8_SetGPIOResult(u8x8, 0);
        break;
    case U8X8_MSG_GPIO_MENU_HOME:
        u8x8_SetGPIOResult(u8x8, 0);
        break;
    default:
        u8x8_SetGPIOResult(u8x8, 1);
        break;
    }
    return 1;
}

/* Set up u8g2 for a 128x64 SSD1306 panel over hardware I2C */
void u8g2Init(u8g2_t *u8g2)
{
    /* Probe both common module addresses (SA0 tied low or high) before setup */
    oled_ack_78 = (HAL_I2C_IsDeviceReady(&hi2c1, 0x78, 3, 100) == HAL_OK) ? 1 : 0;
    oled_ack_7a = (HAL_I2C_IsDeviceReady(&hi2c1, 0x7A, 3, 100) == HAL_OK) ? 1 : 0;
    if (oled_ack_78 == 0 && oled_ack_7a != 0)
    {
        oled_i2c_addr = 0x7A;
    }

    u8g2_Setup_ssd1306_i2c_128x64_noname_f(u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);
    u8g2_InitDisplay(u8g2);
    u8g2_SetPowerSave(u8g2, 0);
    u8g2_ClearBuffer(u8g2);
}

/* Draw the U8G2 logo */
void draw(u8g2_t *u8g2)
{
    u8g2_ClearBuffer(u8g2);

    u8g2_SetFontMode(u8g2, 1);
    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf);
    u8g2_DrawStr(u8g2, 0, 20, "U");

    u8g2_SetFontDirection(u8g2, 1);
    u8g2_SetFont(u8g2, u8g2_font_inb30_mn);
    u8g2_DrawStr(u8g2, 21, 8, "8");

    u8g2_SetFontDirection(u8g2, 0);
    u8g2_SetFont(u8g2, u8g2_font_inb24_mf);
    u8g2_DrawStr(u8g2, 51, 30, "g");
    u8g2_DrawStr(u8g2, 67, 30, "\xb2");

    u8g2_DrawHLine(u8g2, 2, 35, 47);
    u8g2_DrawHLine(u8g2, 3, 36, 47);
    u8g2_DrawVLine(u8g2, 45, 32, 12);
    u8g2_DrawVLine(u8g2, 46, 33, 12);

    u8g2_SetFont(u8g2, u8g2_font_4x6_tr);
    u8g2_DrawStr(u8g2, 1, 54, "github.com/olikraus/u8g2");

    u8g2_SendBuffer(u8g2);
    HAL_Delay(1000);
}
