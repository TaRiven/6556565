/* Copyright 2016 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */
/* Sweetberry board configuration */

#include "common.h"
#include "dma.h"
#include "ec_version.h"
#include "gpio.h"
#include "gpio_list.h"
#include "hooks.h"
#include "i2c.h"
#include "registers.h"
#include "stm32-dma.h"
#include "task.h"
#include "update_fw.h"
#include "usb_descriptor.h"
#include "util.h"
#include "usb_dwc_hw.h"
#include "usb_dwc_console.h"
#include "usb_power.h"
#include "usb_dwc_update.h"

/******************************************************************************
 * Define the strings used in our USB descriptors.
 */
const void *const usb_strings[] = {
	[USB_STR_DESC]		= usb_string_desc,
	[USB_STR_VENDOR]	= USB_STRING_DESC("Google Inc."),
	[USB_STR_PRODUCT]	= USB_STRING_DESC("Sweetberry"),
	[USB_STR_SERIALNO]	= USB_STRING_DESC("1234-a"),
	[USB_STR_VERSION]	= USB_STRING_DESC(CROS_EC_VERSION32),
	[USB_STR_CONSOLE_NAME]	= USB_STRING_DESC("Sweetberry EC Shell"),
	[USB_STR_UPDATE_NAME]	= USB_STRING_DESC("Firmware update"),
};

BUILD_ASSERT(ARRAY_SIZE(usb_strings) == USB_STR_COUNT);

/* USB power interface. */
USB_POWER_CONFIG(sweetberry_power, USB_IFACE_POWER, USB_EP_POWER);


struct dwc_usb usb_ctl = {
	.ep = {
		&ep0_ctl,
		&ep_console_ctl,
		&usb_update_ep_ctl,
		&sweetberry_power_ep_ctl,
	},
	.speed = USB_SPEED_FS,
	.phy_type = USB_PHY_ULPI,
	.dma_en = 1,
	.irq = STM32_IRQ_OTG_HS,
};

/* I2C ports */
const struct i2c_port_t i2c_ports[] = {
	{"i2c1", I2C_PORT_0, 800,
		GPIO_I2C1_SCL, GPIO_I2C1_SDA},
	{"i2c2", I2C_PORT_1, 800,
		GPIO_I2C2_SCL, GPIO_I2C2_SDA},
	{"i2c3", I2C_PORT_2, 800,
		GPIO_I2C3_SCL, GPIO_I2C3_SDA},
	{"fmpi2c4", FMPI2C_PORT_3, 800,
		GPIO_FMPI2C_SCL, GPIO_FMPI2C_SDA},
};
const unsigned int i2c_ports_used = ARRAY_SIZE(i2c_ports);

/******************************************************************************
 * Support firmware upgrade over USB. We can update whichever section is not
 * the current section.
 */

/*
 * This array defines possible sections available for the firmware update.
 * The section which does not map the current executing code is picked as the
 * valid update area. The values are offsets into the flash space.
 */
const struct section_descriptor board_rw_sections[] = {
	{CONFIG_RO_MEM_OFF,
	 CONFIG_RO_MEM_OFF + CONFIG_RO_SIZE},
	{CONFIG_RW_MEM_OFF,
	 CONFIG_RW_MEM_OFF + CONFIG_RW_SIZE},
};
const struct section_descriptor * const rw_sections = board_rw_sections;
const int num_rw_sections = ARRAY_SIZE(board_rw_sections);

#define GPIO_SET_HS(bank, number)	\
	(STM32_GPIO_OSPEEDR(GPIO_##bank) |= (0x3 << ((number) * 2)))

void board_config_post_gpio_init(void)
{
	/* We use MCO2 clock passthrough to provide a clock to USB HS */
	gpio_config_module(MODULE_MCO, 1);
	/* GPIO PC9 to high speed */
	GPIO_SET_HS(C,  9);

	if (usb_ctl.phy_type == USB_PHY_ULPI)
		gpio_set_level(GPIO_USB_MUX_SEL, 0);
	else
		gpio_set_level(GPIO_USB_MUX_SEL, 1);

	/* Set USB GPIO to high speed */
	GPIO_SET_HS(A, 11);
	GPIO_SET_HS(A, 12);

	GPIO_SET_HS(C,  3);
	GPIO_SET_HS(C,  2);
	GPIO_SET_HS(C,  0);
	GPIO_SET_HS(A,  5);

	GPIO_SET_HS(B,  5);
	GPIO_SET_HS(B, 13);
	GPIO_SET_HS(B, 12);
	GPIO_SET_HS(B,  2);
	GPIO_SET_HS(B, 10);
	GPIO_SET_HS(B,  1);
	GPIO_SET_HS(B,  0);
	GPIO_SET_HS(A,  3);

	/* Set I2C GPIO to HS */
	GPIO_SET_HS(B,  6);
	GPIO_SET_HS(B,  7);
	GPIO_SET_HS(F,  1);
	GPIO_SET_HS(F,  0);
	GPIO_SET_HS(A,  8);
	GPIO_SET_HS(B,  4);
	GPIO_SET_HS(C,  6);
	GPIO_SET_HS(C,  7);
}

static void board_init(void)
{
	uint8_t tmp;

	/* i2c 0 has a tendancy to get wedged. TODO(nsanders): why? */
	i2c_xfer(0, 0, NULL, 0, &tmp, 1, I2C_XFER_SINGLE);
}
DECLARE_HOOK(HOOK_INIT, board_init, HOOK_PRIO_DEFAULT);
