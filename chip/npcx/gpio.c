/* Copyright (c) 2014 The Chromium OS Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

/* GPIO module for Chrome EC */

#include "clock.h"
#include "common.h"
#include "gpio.h"
#include "gpio_wui.h"
#include "keyboard_config.h"
#include "hooks.h"
#include "registers.h"
#include "switch.h"
#include "task.h"
#include "timer.h"
#include "util.h"
#include "system.h"
#include "system_chip.h"
#include "lpc_chip.h"
#include "ec_commands.h"
#include "host_command.h"

struct npcx_wui {
	uint8_t table : 2;
	uint8_t group : 3;
	uint8_t bit   : 3;
};

/* Constants for GPIO interrupt mapping */
#define GPIO_INT(name, pin, flags, signal) NPCX_WUI_GPIO_##pin,
static const struct npcx_wui gpio_wui_table[] = {
	#include "gpio.wrap"
};

struct npcx_gpio {
	uint8_t port  : 4;
	uint8_t bit   : 3;
	uint8_t valid : 1;
};

BUILD_ASSERT(sizeof(struct npcx_gpio) == 1);

#define NPCX_GPIO_NONE       {               0,   0, 0}
#define NPCX_GPIO(port, pin) {GPIO_PORT_##port, pin, 1}

struct npcx_alt {
	uint8_t group     : 4;
	uint8_t bit       : 3;
	uint8_t inverted  : 1;
};

struct gpio_alt_map {
	struct npcx_gpio gpio;
	struct npcx_alt  alt;
};

BUILD_ASSERT(sizeof(struct gpio_alt_map) == 2);

/* Convenient macros to initialize the gpio_alt_table */
#define NPCX_ALT(grp, pin)     { ALT_GROUP_##grp, NPCX_DEVALT##grp##_##pin, 0 }
#define NPCX_ALT_INV(grp, pin) { ALT_GROUP_##grp, NPCX_DEVALT##grp##_##pin, 1 }

/* TODO: Index this table on GPIO# */
const struct gpio_alt_map gpio_alt_table[] = {
	/* I2C Module */
	{ NPCX_GPIO(B, 2),  NPCX_ALT(2, I2C0_1_SL)}, /* SMB0SDA1 */
	{ NPCX_GPIO(B, 3),  NPCX_ALT(2, I2C0_1_SL)}, /* SMB0SCL1 */
	{ NPCX_GPIO(B, 4),  NPCX_ALT(2, I2C0_0_SL)}, /* SMB0SDA0 */
	{ NPCX_GPIO(B, 5),  NPCX_ALT(2, I2C0_0_SL)}, /* SMB0SCL0 */
	{ NPCX_GPIO(8, 7),  NPCX_ALT(2, I2C1_0_SL)}, /* SMB1SDA */
	{ NPCX_GPIO(9, 0),  NPCX_ALT(2, I2C1_0_SL)}, /* SMB1SCL */
	{ NPCX_GPIO(9, 1),  NPCX_ALT(2, I2C2_0_SL)}, /* SMB2SDA */
	{ NPCX_GPIO(9, 2),  NPCX_ALT(2, I2C2_0_SL)}, /* SMB2SCL */
	{ NPCX_GPIO(D, 0),  NPCX_ALT(2, I2C3_0_SL)}, /* SMB3SDA */
	{ NPCX_GPIO(D, 1),  NPCX_ALT(2, I2C3_0_SL)}, /* SMB3SCL */
	/* ADC Module */
	{ NPCX_GPIO(4, 5),  NPCX_ALT(6, ADC0_SL)}, /* ADC0  */
	{ NPCX_GPIO(4, 4),  NPCX_ALT(6, ADC1_SL)}, /* ADC1  */
	{ NPCX_GPIO(4, 3),  NPCX_ALT(6, ADC2_SL)}, /* ADC2  */
	{ NPCX_GPIO(4, 2),  NPCX_ALT(6, ADC3_SL)}, /* ADC3  */
	{ NPCX_GPIO(4, 1),  NPCX_ALT(6, ADC4_SL)}, /* ADC4  */
	/* UART Module 1/2 */
#if NPCX_UART_MODULE2
	{ NPCX_GPIO(6, 4),  NPCX_ALT(C, UART_SL2)}, /* CR_SIN */
	{ NPCX_GPIO(6, 5),  NPCX_ALT(C, UART_SL2)}, /* CR_SOUT */
#else
	{ NPCX_GPIO(1, 0),  NPCX_ALT(9, NO_KSO08_SL)}, /* CR_SIN/KSO09 */
	{ NPCX_GPIO(1, 1),  NPCX_ALT(9, NO_KSO09_SL)}, /* CR_SOUT/KSO10 */
#endif
	/* SPI Module */
	{ NPCX_GPIO(9, 5),  NPCX_ALT(0, SPIP_SL)}, /* SPIP_MISO */
	{ NPCX_GPIO(A, 5),  NPCX_ALT(0, SPIP_SL)}, /* SPIP_CS1  */
	{ NPCX_GPIO(A, 3),  NPCX_ALT(0, SPIP_SL)}, /* SPIP_MOSI */
	{ NPCX_GPIO(A, 1),  NPCX_ALT(0, SPIP_SL)}, /* SPIP_SCLK */
	/* PWM Module */
	{ NPCX_GPIO(C, 3),  NPCX_ALT(4, PWM0_SL)}, /* PWM0 */
	{ NPCX_GPIO(C, 2),  NPCX_ALT(4, PWM1_SL)}, /* PWM1 */
	{ NPCX_GPIO(C, 4),  NPCX_ALT(4, PWM2_SL)}, /* PWM2 */
	{ NPCX_GPIO(8, 0),  NPCX_ALT(4, PWM3_SL)}, /* PWM3 */
	{ NPCX_GPIO(B, 6),  NPCX_ALT(4, PWM4_SL)}, /* PWM4 */
	{ NPCX_GPIO(B, 7),  NPCX_ALT(4, PWM5_SL)}, /* PWM5 */
	{ NPCX_GPIO(C, 0),  NPCX_ALT(4, PWM6_SL)}, /* PWM6 */
	{ NPCX_GPIO(6, 0),  NPCX_ALT(4, PWM7_SL)}, /* PWM7 */
	/* MFT Module */
#if NPCX_TACH_SEL2
	{ NPCX_GPIO(9, 3),  NPCX_ALT(C, TA1_TACH1_SL2)},/* TA1_TACH1 */
	{ NPCX_GPIO(D, 3),  NPCX_ALT(C, TB1_TACH2_SL2)},/* TB1_TACH2 */
#else
	{ NPCX_GPIO(4, 0),  NPCX_ALT(3, TA1_TACH1_SL1)},/* TA1_TACH1 */
	{ NPCX_GPIO(A, 4),  NPCX_ALT(3, TB1_TACH2_SL1)},/* TB1_TACH2 */
#endif
	/* Keyboard Scan Module (Inputs) */
	{ NPCX_GPIO(3, 1),  NPCX_ALT_INV(7, NO_KSI0_SL)},/* KSI0 */
	{ NPCX_GPIO(3, 0),  NPCX_ALT_INV(7, NO_KSI1_SL)},/* KSI1 */
	{ NPCX_GPIO(2, 7),  NPCX_ALT_INV(7, NO_KSI2_SL)},/* KSI2 */
	{ NPCX_GPIO(2, 6),  NPCX_ALT_INV(7, NO_KSI3_SL)},/* KSI3 */
	{ NPCX_GPIO(2, 5),  NPCX_ALT_INV(7, NO_KSI4_SL)},/* KSI4 */
	{ NPCX_GPIO(2, 4),  NPCX_ALT_INV(7, NO_KSI5_SL)},/* KSI5 */
	{ NPCX_GPIO(2, 3),  NPCX_ALT_INV(7, NO_KSI6_SL)},/* KSI6 */
	{ NPCX_GPIO(2, 2),  NPCX_ALT_INV(7, NO_KSI7_SL)},/* KSI7 */
	/* Keyboard Scan Module (Outputs) */
	{ NPCX_GPIO(2, 1),  NPCX_ALT_INV(8, NO_KSO00_SL)},/* KSO00 */
	{ NPCX_GPIO(2, 0),  NPCX_ALT_INV(8, NO_KSO01_SL)},/* KSO01 */
	{ NPCX_GPIO(1, 7),  NPCX_ALT_INV(8, NO_KSO02_SL)},/* KSO02 */
	{ NPCX_GPIO(1, 6),  NPCX_ALT_INV(8, NO_KSO03_SL)},/* KSO03 */
	{ NPCX_GPIO(1, 5),  NPCX_ALT_INV(8, NO_KSO04_SL)},/* KSO04 */
	{ NPCX_GPIO(1, 4),  NPCX_ALT_INV(8, NO_KSO05_SL)},/* KSO05 */
	{ NPCX_GPIO(1, 3),  NPCX_ALT_INV(8, NO_KSO06_SL)},/* KSO06 */
	{ NPCX_GPIO(1, 2),  NPCX_ALT_INV(8, NO_KSO07_SL)},/* KSO07 */
	{ NPCX_GPIO(1, 1),  NPCX_ALT_INV(9, NO_KSO08_SL)},/* KSO08 */
	{ NPCX_GPIO(1, 0),  NPCX_ALT_INV(9, NO_KSO09_SL)},/* KSO09 */
	{ NPCX_GPIO(0, 7),  NPCX_ALT_INV(9, NO_KSO10_SL)},/* KSO10 */
	{ NPCX_GPIO(0, 6),  NPCX_ALT_INV(9, NO_KSO11_SL)},/* KSO11 */
	{ NPCX_GPIO(0, 5),  NPCX_ALT_INV(9, NO_KSO12_SL)},/* KSO12 */
	{ NPCX_GPIO(0, 4),  NPCX_ALT_INV(9, NO_KSO13_SL)},/* KSO13 */
	{ NPCX_GPIO(8, 2),  NPCX_ALT_INV(9, NO_KSO14_SL)},/* KSO14 */
	{ NPCX_GPIO(8, 3),  NPCX_ALT_INV(9, NO_KSO15_SL)},/* KSO15 */
	{ NPCX_GPIO(0, 3),  NPCX_ALT_INV(A, NO_KSO16_SL)},/* KSO16 */
	{ NPCX_GPIO(B, 1),  NPCX_ALT_INV(A, NO_KSO17_SL)},/* KSO17 */
	/* Clock module */
	{ NPCX_GPIO(7, 5),  NPCX_ALT(A, 32K_OUT_SL)},     /* 32KHZ_OUT */
	{ NPCX_GPIO(E, 7),  NPCX_ALT(A, 32KCLKIN_SL)},    /* 32KCLKIN */
};

struct gpio_lvol_item {
	struct npcx_gpio lvol_gpio[8];
};

const struct gpio_lvol_item gpio_lvol_table[] = {
	/* Low-Voltage GPIO Control 0 */
	{ { NPCX_GPIO(B, 5),
	    NPCX_GPIO(B, 4),
	    NPCX_GPIO(B, 3),
	    NPCX_GPIO(B, 2),
	    NPCX_GPIO(9, 0),
	    NPCX_GPIO(8, 7),
	    NPCX_GPIO(0, 0),
	    NPCX_GPIO(3, 3), }, },
	/* Low-Voltage GPIO Control 1 */
	{ { NPCX_GPIO(9, 2),
	    NPCX_GPIO(9, 1),
	    NPCX_GPIO(D, 1),
	    NPCX_GPIO(D, 0),
	    NPCX_GPIO(3, 6),
	    NPCX_GPIO(6, 4),
	    NPCX_GPIO(6, 5),
	    NPCX_GPIO_NONE , }, },
	/* Low-Voltage GPIO Control 2 */
	{ { NPCX_GPIO(7, 4),
	    NPCX_GPIO(8, 4),
	    NPCX_GPIO(8, 5),
	    NPCX_GPIO(7, 3),
	    NPCX_GPIO(C, 1),
	    NPCX_GPIO(C, 7),
	    NPCX_GPIO(E, 7),
	    NPCX_GPIO(3, 4), }, },
	/* Low-Voltage GPIO Control 3 */
	{ { NPCX_GPIO(C, 6),
	    NPCX_GPIO(3, 7),
	    NPCX_GPIO(4, 0),
	    NPCX_GPIO(7, 1),
	    NPCX_GPIO(8, 2),
	    NPCX_GPIO(7, 5),
	    NPCX_GPIO(8, 0),
	    NPCX_GPIO(C, 5), }, },
};

/*****************************************************************************/
/* Internal functions */

static int gpio_match(uint8_t port, uint8_t mask, struct npcx_gpio gpio)
{
	return (gpio.valid && (gpio.port == port) && ((1 << gpio.bit) == mask));
}

static int gpio_alt_sel(uint8_t port, uint8_t bit, int8_t func)
{
	struct gpio_alt_map const *map;

	for (map = ARRAY_BEGIN(gpio_alt_table);
	     map < ARRAY_END(gpio_alt_table);
	     map++) {
		if (gpio_match(port, 1 << bit, map->gpio)) {
			uint8_t alt_mask = 1 << map->alt.bit;

			/*
			 * func < 0          -> GPIO functionality
			 * map->alt.inverted -> Set DEVALT bit for GPIO
			 */
			if ((func < 0) ^ map->alt.inverted)
				NPCX_DEVALT(map->alt.group) &= ~alt_mask;
			else
				NPCX_DEVALT(map->alt.group) |=  alt_mask;

			return 1;
		}
	}

	return -1;
}

/* Set interrupt type for GPIO input */
static void gpio_interrupt_type_sel(enum gpio_signal signal, uint32_t flags)
{
	uint8_t table, group, pmask;

	if (signal >= ARRAY_SIZE(gpio_wui_table))
		return;

	table = gpio_wui_table[signal].table;
	group = gpio_wui_table[signal].group;
	pmask = 1 << gpio_wui_table[signal].bit;

	/* Handle interrupt for level trigger */
	if ((flags & GPIO_INT_F_HIGH) || (flags & GPIO_INT_F_LOW)) {
		/* Set detection mode to level */
		NPCX_WKMOD(table, group) |= pmask;
		/* Handle interrupting on level high */
		if (flags & GPIO_INT_F_HIGH)
			NPCX_WKEDG(table, group) &= ~pmask;
		/* Handle interrupting on level low */
		else if (flags & GPIO_INT_F_LOW)
			NPCX_WKEDG(table, group) |= pmask;

		/* Enable wake-up input sources */
		NPCX_WKINEN(table, group) |= pmask;
		/*
		 * Clear pending bit since it might be set
		 * if WKINEN bit is changed.
		 */
		NPCX_WKPCL(table, group) |= pmask;
	}
	/* Handle interrupt for edge trigger */
	else if ((flags & GPIO_INT_F_RISING) || (flags & GPIO_INT_F_FALLING)) {
		/* Set detection mode to edge */
		NPCX_WKMOD(table, group) &= ~pmask;
		/* Handle interrupting on both edges */
		if ((flags & GPIO_INT_F_RISING) &&
			(flags & GPIO_INT_F_FALLING)) {
			/* Enable any edge */
			NPCX_WKAEDG(table, group) |= pmask;
		}
		/* Handle interrupting on rising edge */
		else if (flags & GPIO_INT_F_RISING) {
			/* Disable any edge */
			NPCX_WKAEDG(table, group) &= ~pmask;
			NPCX_WKEDG(table, group) &= ~pmask;
		}
		/* Handle interrupting on falling edge */
		else if (flags & GPIO_INT_F_FALLING) {
			/* Disable any edge */
			NPCX_WKAEDG(table, group) &= ~pmask;
			NPCX_WKEDG(table, group) |= pmask;
		}

		/* Enable wake-up input sources */
		NPCX_WKINEN(table, group) |= pmask;
		/*
		 * Clear pending bit since it might be set
		 * if WKINEN bit is changed.
		 */
		NPCX_WKPCL(table, group) |= pmask;
	} else
		/* Disable wake-up input sources */
		NPCX_WKEN(table, group) &= ~pmask;

	/* No support analog mode */
}

/* Select low voltage detection level */
void gpio_low_voltage_level_sel(uint8_t port, uint8_t mask, uint8_t low_voltage)
{
	int i, j;

	for (i = 0; i < ARRAY_SIZE(gpio_lvol_table); i++) {
		const struct npcx_gpio *gpio = gpio_lvol_table[i].lvol_gpio;

		for (j = 0; j < ARRAY_SIZE(gpio_lvol_table[0].lvol_gpio); j++)
			if (gpio_match(port, mask, gpio[j])) {
				if (low_voltage)
					/* Select vol-detect level for 1.8V */
					SET_BIT(NPCX_LV_GPIO_CTL(i), j);
				else
					/* Select vol-detect level for 3.3V */
					CLEAR_BIT(NPCX_LV_GPIO_CTL(i), j);
				return;
			}

	}
}
/*
 * Make sure the bit depth of low voltage register.
 */
BUILD_ASSERT(ARRAY_SIZE(gpio_lvol_table[0].lvol_gpio) == 8);

/*****************************************************************************/
/* IC specific low-level driver */

void gpio_set_alternate_function(uint32_t port, uint32_t mask, int func)
{
	/* Enable alternative pins by func*/
	int pin;

	/* check each bit from mask  */
	for (pin = 0; pin < 8; pin++)
		if (mask & (1 << pin))
			gpio_alt_sel(port, pin, func);
}

test_mockable int gpio_get_level(enum gpio_signal signal)
{
	return !!(NPCX_PDIN(gpio_list[signal].port) & gpio_list[signal].mask);
}

void gpio_set_level(enum gpio_signal signal, int value)
{
	if (value)
		NPCX_PDOUT(gpio_list[signal].port) |=  gpio_list[signal].mask;
	else
		NPCX_PDOUT(gpio_list[signal].port) &= ~gpio_list[signal].mask;
}

void gpio_set_flags_by_mask(uint32_t port, uint32_t mask, uint32_t flags)
{
	/*
	 * Configure pin as input, if requested. Output is configured only
	 * after setting all other attributes, so as not to create a
	 * temporary incorrect logic state 0:input 1:output
	 */
	if (!(flags & GPIO_OUTPUT))
		NPCX_PDIR(port) &= ~mask;

	/* Select open drain 0:push-pull 1:open-drain */
	if (flags & GPIO_OPEN_DRAIN)
		NPCX_PTYPE(port) |= mask;
	else
		NPCX_PTYPE(port) &= ~mask;

	/* Select pull-up/down of GPIO 0:pull-up 1:pull-down */
	if (flags & GPIO_PULL_UP) {
		NPCX_PPUD(port)  &= ~mask;
		NPCX_PPULL(port) |= mask; /* enable pull down/up */
	} else if (flags & GPIO_PULL_DOWN) {
		NPCX_PPUD(port)  |= mask;
		NPCX_PPULL(port) |= mask; /* enable pull down/up */
	} else {
		/* No pull up/down */
		NPCX_PPULL(port) &= ~mask; /* disable pull down/up */
	}

	/* 1.8V low voltage select */
	if (flags & GPIO_SEL_1P8V) {
		/*
		 * Set IO type to open-drain & disable internal pulling
		 * before selecting low-voltage level
		 */
		NPCX_PTYPE(port) |= mask;
		NPCX_PPULL(port) &= ~mask;
		gpio_low_voltage_level_sel(port, mask, 1);
	} else
		gpio_low_voltage_level_sel(port, mask, 0);

	/* Set up interrupt type */
	if (flags & GPIO_INT_ANY) {
		const struct gpio_info *g = gpio_list;
		enum gpio_signal gpio_int;

		/* Find gpio signal in GPIO_INTs by port and mask */
		for (gpio_int = 0; gpio_int < GPIO_IH_COUNT; gpio_int++, g++)
			if ((g->port == port) && (g->mask & mask))
				gpio_interrupt_type_sel(gpio_int, flags);
	}

	/* Set level 0:low 1:high*/
	if (flags & GPIO_HIGH)
		NPCX_PDOUT(port) |= mask;
	else if (flags & GPIO_LOW)
		NPCX_PDOUT(port) &= ~mask;

	/* Configure pin as output, if requested 0:input 1:output */
	if (flags & GPIO_OUTPUT)
		NPCX_PDIR(port) |= mask;
}

int gpio_enable_interrupt(enum gpio_signal signal)
{
	struct npcx_wui wui;

	/* Fail if not an interrupt handler */
	if (signal >= GPIO_IH_COUNT)
		return EC_ERROR_PARAM1;

	wui = gpio_wui_table[signal];
	/* Set MIWU enable bit */
	NPCX_WKEN(wui.table, wui.group) |= 1 << wui.bit;

	return EC_SUCCESS;
}

int gpio_disable_interrupt(enum gpio_signal signal)
{
	struct npcx_wui wui;

	/* Fail if not an interrupt handler */
	if (signal >= GPIO_IH_COUNT)
		return EC_ERROR_PARAM1;

	wui = gpio_wui_table[signal];
	NPCX_WKEN(wui.table, wui.group) &= ~(1 << wui.bit);

	return EC_SUCCESS;
}

int gpio_clear_pending_interrupt(enum gpio_signal signal)
{
	struct npcx_wui wui;

	/* Fail if not an interrupt handler */
	if (signal >= GPIO_IH_COUNT)
		return EC_ERROR_PARAM1;

	wui = gpio_wui_table[signal];
	NPCX_WKPCL(wui.table, wui.group) |= 1 << wui.bit;

	return EC_SUCCESS;
}

void gpio_pre_init(void)
{
	const struct gpio_info *g = gpio_list;
	int is_warm = system_is_reboot_warm();
	int flags;
	int i, j;

	/* Pin_Mux for FIU/SPI (set to GPIO) */
	SET_BIT(NPCX_DEVALT(0), NPCX_DEVALT0_GPIO_NO_SPIP);
	SET_BIT(NPCX_DEVALT(0), NPCX_DEVALT0_NO_F_SPI);

	/* Pin_Mux for PWRGD */
	SET_BIT(NPCX_DEVALT(1), NPCX_DEVALT1_NO_PWRGD);

	/* Pin_Mux for PECI */
#ifndef CONFIG_PECI
	SET_BIT(NPCX_DEVALT(0xA), NPCX_DEVALTA_NO_PECI_EN);
#endif

	/* Pin_Mux for LPC & SHI */
#ifdef CONFIG_HOSTCMD_SPS
	/* Switching to eSPI mode for SHI interface */
	NPCX_DEVCNT |= 0x08;
	/* Alternate Intel bus interface LPC/eSPI to GPIOs first */
	SET_BIT(NPCX_DEVALT(ALT_GROUP_1), NPCX_DEVALT1_NO_LPC_ESPI);
#endif

	/* Clear all interrupt pending and enable bits of GPIOS */
	for (i = 0; i < 2; i++) {
		for (j = 0; j < 8; j++) {
			NPCX_WKPCL(i, j) = 0xFF;
			NPCX_WKEN(i, j) = 0;
		}
	}

	/* No support enable clock for the GPIO port in run and sleep. */
	/* Set flag for each GPIO pin in gpio_list */
	for (i = 0; i < GPIO_COUNT; i++, g++) {
		flags = g->flags;

		if (flags & GPIO_DEFAULT)
			continue;
		/*
		 * If this is a warm reboot, don't set the output levels or
		 * we'll shut off the AP.
		 */
		if (is_warm)
			flags &= ~(GPIO_LOW | GPIO_HIGH);

		/* Set up GPIO based on flags */
		gpio_set_flags_by_mask(g->port, g->mask, flags);

		/*
		 * Ensure that any GPIO defined in gpio.inc is actually
		 * configured as a GPIO, and not left in its default state,
		 * which may or may not be as a GPIO.
		 */
		gpio_set_alternate_function(g->port, g->mask, -1);
	}
}

/* List of GPIO IRQs to enable. Don't automatically enable interrupts for
 * the keyboard input GPIO bank - that's handled separately. Of course the
 * bank is different for different systems. */
static void gpio_init(void)
{
	/* Enable IRQs now that pins are set up */
	task_enable_irq(NPCX_IRQ_MTC_WKINTAD_0);
	task_enable_irq(NPCX_IRQ_WKINTEFGH_0);
	task_enable_irq(NPCX_IRQ_WKINTC_0);
	task_enable_irq(NPCX_IRQ_TWD_WKINTB_0);
	task_enable_irq(NPCX_IRQ_WKINTA_1);
	task_enable_irq(NPCX_IRQ_WKINTB_1);
#ifndef HAS_TASK_KEYSCAN
	task_enable_irq(NPCX_IRQ_KSI_WKINTC_1);
#endif
	task_enable_irq(NPCX_IRQ_WKINTD_1);
	task_enable_irq(NPCX_IRQ_WKINTE_1);
	task_enable_irq(NPCX_IRQ_WKINTF_1);
	task_enable_irq(NPCX_IRQ_WKINTG_1);
	task_enable_irq(NPCX_IRQ_WKINTH_1);
}
DECLARE_HOOK(HOOK_INIT, gpio_init, HOOK_PRIO_DEFAULT);

/*****************************************************************************/
/* Interrupt handlers */

/**
 * Handle a GPIO interrupt.
 *
 * @param wui_int wui table & group for GPIO interrupt no.
 */

static void gpio_interrupt(struct npcx_wui wui_int)
{
	int i;
	uint8_t wui_mask;
	uint8_t table = wui_int.table;
	uint8_t group = wui_int.group;

	/* Get pending mask */
	wui_mask = NPCX_WKPND(table, group) & NPCX_WKEN(table, group);

	/* Find GPIOs and execute interrupt service routine */
	for (i = 0; i < GPIO_IH_COUNT && wui_mask; i++) {
		uint8_t pin_mask = 1 << gpio_wui_table[i].bit;

		if ((gpio_wui_table[i].table == table) &&
			(gpio_wui_table[i].group == group) &&
			(wui_mask & pin_mask)) {
			/* Clear pending bit of GPIO */
			NPCX_WKPCL(table, group) = pin_mask;
			/* Execute GPIO's ISR */
			gpio_irq_handlers[i](i);
			/* In case declare the same GPIO in gpio_wui_table */
			wui_mask &= ~pin_mask;
		}
	}
}

/**
 * Handlers for each GPIO port.  These read and clear the interrupt bits for
 * the port, then call the master handler above.
 */

#define GPIO_IRQ_FUNC(_irq_func, wui_int)		\
void _irq_func(void)					\
{							\
	gpio_interrupt(wui_int);			\
}

/* If we need to handle the other type interrupts except GPIO, add code here */
void __gpio_wk0efgh_interrupt(void)
{
#if defined(CONFIG_LPC) || defined(CONFIG_ESPI)
	/* Pending bit 7 or 6 or 5? */
	if (IS_BIT_SET(NPCX_WKEN(MIWU_TABLE_0 , MIWU_GROUP_5), 6) &&
	    IS_BIT_SET(NPCX_WKPND(MIWU_TABLE_0 , MIWU_GROUP_5), 6)) {
		/* Disable host wake-up */
		CLEAR_BIT(NPCX_WKEN(MIWU_TABLE_0, MIWU_GROUP_5), 6);
		/* Clear pending bit of WUI */
		SET_BIT(NPCX_WKPCL(MIWU_TABLE_0, MIWU_GROUP_5), 6);
	}
#ifdef CONFIG_ESPI
	else if (IS_BIT_SET(NPCX_WKEN(MIWU_TABLE_0, MIWU_GROUP_5), 5) &&
		 IS_BIT_SET(NPCX_WKPND(MIWU_TABLE_0, MIWU_GROUP_5), 5))
		espi_espirst_handler();
#else
	else if (IS_BIT_SET(NPCX_WKEN(MIWU_TABLE_0, MIWU_GROUP_5), 7) &&
		 IS_BIT_SET(NPCX_WKPND(MIWU_TABLE_0, MIWU_GROUP_5), 7))
		lpc_lreset_pltrst_handler();
#endif
	else
#endif
	{
		gpio_interrupt(WUI_INT(MIWU_TABLE_0, MIWU_GROUP_5));
		gpio_interrupt(WUI_INT(MIWU_TABLE_0, MIWU_GROUP_6));
		gpio_interrupt(WUI_INT(MIWU_TABLE_0, MIWU_GROUP_7));
		gpio_interrupt(WUI_INT(MIWU_TABLE_0, MIWU_GROUP_8));
	}
}

void __gpio_rtc_interrupt(void)
{
	/* Check pending bit 7 */
#ifdef CONFIG_HOSTCMD_RTC
	if (NPCX_WKPND(MIWU_TABLE_0, MIWU_GROUP_4) & 0x80) {
		/* Clear pending bit for WUI */
		SET_BIT(NPCX_WKPCL(MIWU_TABLE_0, MIWU_GROUP_4), 7);
		host_set_events(EC_HOST_EVENT_MASK(EC_HOST_EVENT_RTC));
	} else
#endif
	{
		gpio_interrupt(WUI_INT(MIWU_TABLE_0, MIWU_GROUP_1));
		gpio_interrupt(WUI_INT(MIWU_TABLE_0, MIWU_GROUP_4));
	}
}

GPIO_IRQ_FUNC(__gpio_wk0b_interrupt, WUI_INT(MIWU_TABLE_0, MIWU_GROUP_2));
GPIO_IRQ_FUNC(__gpio_wk0c_interrupt, WUI_INT(MIWU_TABLE_0, MIWU_GROUP_3));
GPIO_IRQ_FUNC(__gpio_wk1a_interrupt, WUI_INT(MIWU_TABLE_1, MIWU_GROUP_1));
GPIO_IRQ_FUNC(__gpio_wk1b_interrupt, WUI_INT(MIWU_TABLE_1, MIWU_GROUP_2));
#ifndef HAS_TASK_KEYSCAN
/* Delcare GPIO irq functions for KSI pins if there's no keyboard scan task, */
GPIO_IRQ_FUNC(__gpio_wk1c_interrupt, WUI_INT(MIWU_TABLE_1, MIWU_GROUP_3));
#endif
GPIO_IRQ_FUNC(__gpio_wk1d_interrupt, WUI_INT(MIWU_TABLE_1, MIWU_GROUP_4));
GPIO_IRQ_FUNC(__gpio_wk1e_interrupt, WUI_INT(MIWU_TABLE_1, MIWU_GROUP_5));
GPIO_IRQ_FUNC(__gpio_wk1f_interrupt, WUI_INT(MIWU_TABLE_1, MIWU_GROUP_6));
GPIO_IRQ_FUNC(__gpio_wk1g_interrupt, WUI_INT(MIWU_TABLE_1, MIWU_GROUP_7));
GPIO_IRQ_FUNC(__gpio_wk1h_interrupt, WUI_INT(MIWU_TABLE_1, MIWU_GROUP_8));

DECLARE_IRQ(NPCX_IRQ_MTC_WKINTAD_0, __gpio_rtc_interrupt, 2);
DECLARE_IRQ(NPCX_IRQ_TWD_WKINTB_0,  __gpio_wk0b_interrupt, 2);
DECLARE_IRQ(NPCX_IRQ_WKINTC_0,      __gpio_wk0c_interrupt, 2);
DECLARE_IRQ(NPCX_IRQ_WKINTEFGH_0,   __gpio_wk0efgh_interrupt, 2);
DECLARE_IRQ(NPCX_IRQ_WKINTA_1,      __gpio_wk1a_interrupt, 2);
DECLARE_IRQ(NPCX_IRQ_WKINTB_1,      __gpio_wk1b_interrupt, 2);
#ifndef HAS_TASK_KEYSCAN
DECLARE_IRQ(NPCX_IRQ_KSI_WKINTC_1,  __gpio_wk1c_interrupt, 2);
#endif
DECLARE_IRQ(NPCX_IRQ_WKINTD_1,      __gpio_wk1d_interrupt, 2);
DECLARE_IRQ(NPCX_IRQ_WKINTE_1,      __gpio_wk1e_interrupt, 2);
#ifdef CONFIG_HOSTCMD_SPS
/*
 * HACK: Make CS GPIO P1 to improve SHI reliability.
 * TODO: Increase CS-assertion-to-transaction-start delay on host to
 * accommodate P2 CS interrupt.
 */
DECLARE_IRQ(NPCX_IRQ_WKINTF_1,      __gpio_wk1f_interrupt, 1);
#else
DECLARE_IRQ(NPCX_IRQ_WKINTF_1,      __gpio_wk1f_interrupt, 2);
#endif
DECLARE_IRQ(NPCX_IRQ_WKINTG_1,      __gpio_wk1g_interrupt, 2);
DECLARE_IRQ(NPCX_IRQ_WKINTH_1,      __gpio_wk1h_interrupt, 2);


#undef GPIO_IRQ_FUNC
