/* -*- mode:c -*-
 *
 * Copyright 2016 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

GPIO_INT(LID_OPEN,          PIN(27),  GPIO_INT_BOTH | GPIO_PULL_UP,       lid_interrupt)            /* Lid switch */
GPIO_INT(AC_PRESENT,        PIN(30),  GPIO_INT_BOTH | GPIO_PULL_UP | GPIO_PULL_DOWN, extpower_interrupt)     /* BC_ACOK / EC_ACIN - to know if battery or AC connected */
#ifdef CONFIG_BUTTON_COUNT
GPIO_INT(VOLUME_UP,         PIN(31),  GPIO_INT_BOTH,                      button_interrupt)         /* Volume up button */
GPIO_INT(VOLUME_DOWN,       PIN(34),  GPIO_INT_BOTH,                      button_interrupt)         /* Volume down button */
#endif
GPIO_INT(POWER_BUTTON_L,    PIN(35),  GPIO_INT_BOTH,                      power_button_interrupt)   /* Power button */
GPIO_INT(RSMRST_L_PGOOD,    PIN(63),  GPIO_INT_BOTH,                      power_signal_interrupt)   /* RSMRST_N_PWRGD from power logic */
GPIO_INT(ALL_SYS_PGOOD,     PIN(130), GPIO_INT_BOTH,                      power_signal_interrupt)   /* ALL_SYS_PWRGD from power logic */
#ifdef CONFIG_LOW_POWER_IDLE
GPIO_INT(UART0_RX,          PIN(162), GPIO_INT_BOTH_DSLEEP | GPIO_PULL_UP,       uart_deepsleep_interrupt) /* UART0 RX input */
#else
GPIO_INT(UART0_RX,          PIN(162), GPIO_INPUT | GPIO_PULL_UP,          NULL)                     /* UART0 RX input */
#endif
GPIO_INT(PCH_SLP_S4_L,      PIN(200), GPIO_INT_BOTH,                      power_signal_interrupt)   /* SLP_S4# signal from PCH */
GPIO_INT(PCH_SLP_S3_L,      PIN(206), GPIO_INT_BOTH,                      power_signal_interrupt)   /* SLP_S3# signal from PCH */

GPIO(BOARD_VERSION2,    PIN(0),  GPIO_INPUT)                    /* BOARD_ID1 */
GPIO(KBD_KSO2,          PIN(1),  GPIO_KB_OUTPUT_COL2)           /* Negative edge triggered irq. */

GPIO(NC_GPIO12,         PIN(12), GPIO_INPUT | GPIO_PULL_UP)	/* NC */
GPIO(USB_ILIM_SEL,      PIN(13), GPIO_OUT_HIGH)                 /* USB current control */
GPIO(I2C_PORT0_0_SCL,   PIN(15), GPIO_INPUT)
GPIO(I2C_PORT0_0_SDA,   PIN(16), GPIO_INPUT)
GPIO(NC_GPIO134,        PIN(134), GPIO_INPUT | GPIO_PULL_UP)
GPIO(NC_GPIO17,         PIN(17), GPIO_INPUT | GPIO_PULL_UP)
GPIO(NC_GPIO22,         PIN(22), GPIO_INPUT | GPIO_PULL_UP)
GPIO(NC_GPIO23,         PIN(23), GPIO_INPUT | GPIO_PULL_UP)
GPIO(I2C_PORT2_SCL,     PIN(20), GPIO_INPUT)
GPIO(I2C_PORT2_SDA,     PIN(21), GPIO_INPUT)
GPIO(I2C_PORT3_SCL,     PIN(24), GPIO_INPUT)
GPIO(I2C_PORT3_SDA,     PIN(25), GPIO_INPUT)
GPIO(PCH_SCI_L,         PIN(26), GPIO_ODR_HIGH)                 /* SCI output */

#ifndef CONFIG_BUTTON_COUNT
GPIO(VOLUME_UP,         PIN(31), GPIO_INPUT)                    /* Volume up button */
#endif
GPIO(WP_L,              PIN(33), GPIO_INPUT)                    /* EC_SPI_WP_ME_L */
#ifndef CONFIG_BUTTON_COUNT
GPIO(VOLUME_DOWN,       PIN(34), GPIO_INPUT)                    /* Volume down button */
#endif
GPIO(USB2_ENABLE,       PIN(36), GPIO_OUT_LOW)                  /* Enable power for USB2 Port */

GPIO(ENTERING_RW,       PIN(41), GPIO_OUT_LOW)                  /* Indicate when EC is entering RW code */
GPIO(PCH_SMI_L,         PIN(44), GPIO_ODR_HIGH)                 /* SMI output */
GPIO(USB_OC1_L,         PIN(45), GPIO_INT_FALLING)              /* DB2 BC1.2 over current signal to EC */
GPIO(NC_GPIO46,         PIN(46), GPIO_INPUT | GPIO_PULL_UP)     /* NC */
GPIO(NC_GPIO47,         PIN(47), GPIO_INPUT | GPIO_PULL_UP)     /* NC */

GPIO(NC_GPIO50,         PIN(50), GPIO_INPUT | GPIO_PULL_UP)     /* NC */
GPIO(PCH_SUS_STAT_L,    PIN(51), GPIO_INT_FALLING)              /* Signal to inform EC that SOC is entering low power state */
GPIO(TS_RST_L,          PIN(52), GPIO_OUT_LOW)                  /* Touch screen reset pin */
GPIO(TRACKPAD_PWREN,    PIN(53), GPIO_OUT_HIGH)                 /* Enable power for Track Pad */
GPIO(USB_OC0_L,         PIN(55), GPIO_INT_FALLING)              /* Over current signal of the BC1.2 charger to EC */

GPIO(NC_GPIO61,         PIN(61), GPIO_INPUT | GPIO_PULL_UP)     /* NC */
GPIO(EC_HIB_L,          PIN(64), GPIO_OUT_LOW)                  /* Set to high before Pseduo G3 */
GPIO(PCH_SYS_PWROK,     PIN(65), GPIO_OUT_LOW)                  /* EC thinks everything is up and ready (DELAY_ALL_SYS_PWRGD) */
GPIO(PCH_WAKE_L,        PIN(66), GPIO_ODR_HIGH)                 /* PCH wake pin */
GPIO(USB1_ENABLE,       PIN(67), GPIO_OUT_LOW)                  /* Enable power for USB3 Port */

GPIO(NC_GPIO100,        PIN(100), GPIO_INPUT | GPIO_PULL_UP)    /* NC */
GPIO(NC_GPIO101,        PIN(101), GPIO_INPUT | GPIO_PULL_UP)    /* NC */
GPIO(BOARD_VERSION3,    PIN(102), GPIO_INPUT)                   /* BOARD_ID2 */
GPIO(USB_CTL1,          PIN(105), GPIO_OUT_HIGH)                /* USB charging mode control */

GPIO(PCH_RCIN_L,        PIN(110), GPIO_ODR_HIGH)                /* Reset line to PCH (for 8042 emulation) */
GPIO(NC_115,            PIN(115), GPIO_INPUT | GPIO_PULL_UP)    /* NC */
GPIO(EC_VNN_VCLK,       PIN(122), GPIO_INPUT | GPIO_PULL_UP)    /* Interrupt from USB PD Controller to EC */
GPIO(STRAP_L,           PIN(123), GPIO_OUT_LOW)
GPIO(EC_VNN_ALERT_L,    PIN(124), GPIO_INPUT | GPIO_PULL_UP)
GPIO(GYRO_INT2,         PIN(127), GPIO_INPUT | GPIO_PULL_DOWN)   /* Gyro sensor interrupt 2 to EC */

GPIO(NC_GPIO132,        PIN(132), GPIO_INPUT | GPIO_PULL_UP)	 /* NC */
GPIO(NC_GPIO135,        PIN(135), GPIO_INPUT | GPIO_PULL_UP)	 /* NC */
GPIO(BAT_LED_AMBER,     PIN(136), GPIO_OUT_HIGH)	         /* LED color is amber */

GPIO(NC_GPIO140,        PIN(140), GPIO_INPUT | GPIO_PULL_UP)	 /* NC */
GPIO(BAT_LED_WHITE,     PIN(141), GPIO_OUT_HIGH)	         /* LED color is white */
GPIO(PCH_RSMRST_L,      PIN(143), GPIO_OUT_LOW)                  /* RSMRST_N to PCH */
GPIO(EC_KBD_ALERT,      PIN(145), GPIO_OUT_LOW)                  /* EC_KBD_ALERT */
GPIO(PVT_CS0,           PIN(146), GPIO_ODR_HIGH)                 /* SPI PVT Chip select */
GPIO(BOARD_VERSION4,    PIN(147), GPIO_INPUT)			 /* BOARD_ID3 */

GPIO(WLAN_OFF_L,        PIN(150), GPIO_ODR_HIGH)                 /* Wireless LAN */
GPIO(CPU_PROCHOT,       PIN(151), GPIO_OUT_LOW)
GPIO(KBD_IRQ_L,         PIN(152), GPIO_ODR_HIGH)                 /* Negative edge triggered irq. */
GPIO(BOARD_VERSION1,    PIN(154), GPIO_INPUT)                    /* BOARD_ID0 */
GPIO(NC_GPIO155,        PIN(155), GPIO_INPUT | GPIO_PULL_UP)     /* NC */
GPIO(PWR_BTN_SELECT,    PIN(156), GPIO_ODR_HIGH)                 /* HIGH in clamshell mode and LOW in tablet mode */
GPIO(PCH_SUSPWRDNACK,   PIN(157), GPIO_INT_FALLING)              /* PMC SUSPWRDNACK signal from SOC to EC */

GPIO(PCH_PWRBTN_L,      PIN(160), GPIO_OUT_HIGH)                 /* Power button output to PCH */
GPIO(GYRO_INT1,         PIN(161), GPIO_INPUT | GPIO_PULL_DOWN)   /* Gyro sensor interrupt 1 to EC */
GPIO(NC_GPIO163,        PIN(163), GPIO_INPUT | GPIO_PULL_UP)     /* NC */

GPIO(STARTUP_LATCH_SET, PIN(201), GPIO_INPUT)			 /* Not used in BCRD2. Programmed as Input to fix EC power leakage in S3*/
GPIO(EC_BL_DISABLE_L,   PIN(202), GPIO_OUT_HIGH)                 /* EDP backligh disable signal from EC */
GPIO(SMC_SHUTDOWN,      PIN(203), GPIO_OUT_LOW)                  /* Shutdown signal from EC to power sequencing PLD */
GPIO(NC_GPIO204,        PIN(204), GPIO_INPUT | GPIO_PULL_UP)	 /* NC */

GPIO(BAT_PRESENT_L,    PIN(210), GPIO_INPUT)                     /* HW detection signal from battery to EC */
GPIO(GPIO_3_EC,         PIN(211), GPIO_OUT_LOW)                  /* Sleep SOIX signal from SOC to EC */

/* Alternate functions GPIO definition */
ALTERNATE(PIN_MASK(16, 0x24),   1,    MODULE_UART,            0)                    /* UART0 */

ALTERNATE(PIN_MASK(0,  0xfc),   3,    MODULE_KEYBOARD_SCAN,   GPIO_KB_OUTPUT)
ALTERNATE(PIN_MASK(1,  0x03),   3,    MODULE_KEYBOARD_SCAN,   GPIO_KB_OUTPUT)
ALTERNATE(PIN_MASK(10, 0xd8),   3,    MODULE_KEYBOARD_SCAN,   GPIO_KB_OUTPUT)
ALTERNATE(PIN_MASK(3,  0x04),   3,    MODULE_KEYBOARD_SCAN,   GPIO_KB_INPUT)
ALTERNATE(PIN_MASK(4,  0x0d),   3,    MODULE_KEYBOARD_SCAN,   GPIO_KB_INPUT)
ALTERNATE(PIN_MASK(12, 0x60),   2,    MODULE_KEYBOARD_SCAN,   GPIO_KB_INPUT)
ALTERNATE(PIN_MASK(14, 0x14),   3,    MODULE_KEYBOARD_SCAN,   GPIO_KB_INPUT)

ALTERNATE(PIN_MASK(1,  0x10),   0,    MODULE_LPC,             GPIO_ODR_HIGH)         /* 14: LPC CLKRUN - Program as GPIO for power saving */
ALTERNATE(PIN_MASK(11, 0x9e),   1,    MODULE_LPC,             0)                     /* 111~114:LAD[0:3],  117:PCI_CLK */
ALTERNATE(PIN_MASK(11, 0x40),   1,    MODULE_LPC,             GPIO_INT_BOTH)         /* 116: LRESET# */
ALTERNATE(PIN_MASK(12, 0x01),   1,    MODULE_LPC,             0)                     /* 120: LFRAME# */

ALTERNATE(PIN_MASK(5,  0x10),   1,    MODULE_SPI,             0)
ALTERNATE(PIN_MASK(16, 0x10),   1,    MODULE_SPI,             0)
ALTERNATE(PIN_MASK(15, 0x08),   1,    MODULE_SPI,             0)                     /* 153: CLK */

ALTERNATE(PIN_MASK(13, 0x48),   1,    MODULE_PWM_LED,         GPIO_OUTPUT)           /* 133: PWM0, 136: PWM1 */
ALTERNATE(PIN_MASK(14, 0x02),   1,    MODULE_PWM_LED,         GPIO_OUTPUT)           /* 141: PWM3 */
ALTERNATE(PIN_MASK(5,  0xc0),   1,    MODULE_ADC,             0)                     /* 56: temperature sensor 1, 57: temperature sensor 2 */
ALTERNATE(PIN_MASK(6,  0x05),   1,    MODULE_ADC,             0)                     /* 60: PC_MON, 62: temperature sensor 3 */

/* Re-Config LPC Pins to GPIO Open Drain for SOC G3 (EC - POWER_G3) state */
ALTERNATE(PIN_MASK(1,  0x10),   0,      MODULE_GPIO,          GPIO_ODR_HIGH)         /* 14: LPC CLKRUN */
ALTERNATE(PIN_MASK(11, 0x9e),   0,      MODULE_GPIO,          GPIO_ODR_HIGH)         /* 111~114:LAD[0:3], 117:PCI_CLK */
ALTERNATE(PIN_MASK(11, 0x40),   0,      MODULE_GPIO,          GPIO_ODR_HIGH)         /* 116: LRESET# */
ALTERNATE(PIN_MASK(12, 0x01),   0,      MODULE_GPIO,          GPIO_ODR_HIGH)         /* 120: LFRAME# */

/* I2C pins */
/* I2C0_0 CLK - GPIO015, I2C0_0 DAT - GPIO016 */
ALTERNATE(PIN_MASK(1,  0x60), 2, MODULE_I2C, GPIO_ODR_HIGH)
/* I2C2_0 CLK - GPIO020, I2C2_0 DAT - GPIO021, I2C3_0 CLK - GPIO024, I2C3_0 DAT - GPIO025 */
ALTERNATE(PIN_MASK(2,  0x33), 2, MODULE_I2C, GPIO_ODR_HIGH)