/*
 * Open-BLDC - Open BrushLess DC Motor Controller
 * Copyright (C) 2009-2010 by Piotr Esden-Tempski <piotr@esden.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   gprot.c
 * @author Piotr Esden-Tempski <piotr@esden.net>
 * @date   Tue Aug 17 02:00:29 2010
 *
 * @brief  Application specific part of Governor PC software protocol implementation
 *
 * Governor protocol is a library implementing a very thin protocol for
 * realtime applications. In this file all the initialization and hook
 * assignement is being done that obviously can not be done in a general
 * purpose protocol library.
 */

#include "config.h"

#include <libopencm3/stm32/f1/gpio.h>

#include "types.h"

#include <lg/gpdef.h>
#include <lg/gprotc.h>

#include "gprot.h"
#include "driver/led.h"
#include "driver/usart.h"
#include "pwm/pwm.h"
#include "comm_tim.h"
#include "driver/adc.h"
#include "sensor_process.h"
#include "comm_process.h"
#include "control_process.h"
#include "main.h"

/**
 * version strings
 */
#define FIRMWARE_VERSION "\n" PROJECT_NAME " " TARGET " firmware " VERSION VERSION_SUFFIX ", build " BUILDDATE "\n"
#define FIRMWARE_COPYRIGHT COPYRIGHT "\n"
#define FIRMWARE_LICENSE LICENSE "\n"

/**
 * Commutate once trigger flag
 */
#define GPROT_FLAG_PWM_COMM (1 << 0)
/**
 * Run commutation timer flag
 */
#define GPROT_FLAG_COMM_TIM (1 << 1)
/**
 * Run closed loop control flag
 */
#define GPROT_FLAG_ADC_COMM (1 << 2)
/**
 * Pull all phases low flag
 */
#define GPROT_FLAG_ALL_LO (1 << 3)
/**
 * Pull all phases high flag
 */
#define GPROT_FLAG_ALL_HI (1 << 4)

/**
 * Dummy value...
 */
u16 gprot_dummy_val;

/**
 * Flags register
 */
static u16 gprot_flag_reg;

/**
 * PWM power register
 */
static s16 gprot_pwm_power = CP__SST_POWER;

/**
 * Previous value of the flag register for detection of flag transitions.
 */
static u16 gprot_flag_reg_old;

/**
 * Version request trigger flag.
 */
static bool gprot_get_version_triggered = false;

/* Private function declarations */
static void gprot_trigger_output(void *data);
static void gprot_register_changed(void *data, u8 addr);
static void gprot_update_flags(void);
static void gprot_update_pwm_power(void);
static void gprot_get_version(void *data);

/* Function implementations */
/**
 * Initialize all necessary registers and callback hooks.
 *
 * @todo All setup regs should be called from the respective module init
 * functions.
 */
void gprot_init()
{
	(void)gpc_init(gprot_trigger_output, NULL, gprot_register_changed, NULL);
	(void)gpc_set_get_version_callback(gprot_get_version, NULL);

	gprot_flag_reg = 0;
	gprot_flag_reg_old = 0;
	(void)gpc_setup_reg(GPROT_FLAG_REG_ADDR, &gprot_flag_reg);

	(void)gpc_setup_reg(GPROT_NEW_CYCLE_TIME, (u16 *) & new_cycle_time);

	(void)gpc_setup_reg(GPROT_PWM_VAL_REG_ADDR, (u16 *) & gprot_pwm_power);
}

/**
 * Callback from the libgovernor enabeling transmission.
 *
 * @param data Callback passed thorough data. Ignored here.
 */
void gprot_trigger_output(void *data)
{
	data = data;
	usart_enable_send();
}

/**
 * Callback from libgovernor indicating a get version event.
 *
 * @param data Passthrough data to the callback.
 */
void gprot_get_version(void *data)
{
	data = data;

	gprot_get_version_triggered = true;
}

/**
 * Userspace process handling the get version event.
 */
void run_gprot_get_version_process(void)
{
	if (gprot_get_version_triggered) {
		gprot_get_version_triggered = false;
		gpc_send_string(FIRMWARE_VERSION, sizeof(FIRMWARE_VERSION));
		gpc_send_string(FIRMWARE_COPYRIGHT, sizeof(FIRMWARE_COPYRIGHT));
		gpc_send_string(FIRMWARE_LICENSE, sizeof(FIRMWARE_LICENSE));
	}
}

/**
 * Callback from libgovernor indicating that a register content has changed.
 *
 * @param data Callback passed through data. Ignored here.
 * @param addr Address of the register that has changed.
 */
void gprot_register_changed(void *data, u8 addr)
{
	data = data;
	if (addr == GPROT_FLAG_REG_ADDR) {
		gprot_update_flags();
	}else if(addr == GPROT_PWM_VAL_REG_ADDR) {
		gprot_update_pwm_power();
	}
}

/**
 * Automatically called by @ref gprot_register_changed()
 *
 * Implements the logic associated with the application specific flag register.
 */
void gprot_update_flags(void)
{
	if ((gprot_flag_reg & GPROT_FLAG_PWM_COMM) != 0) {
		pwm_comm();
		return;
	}

        if ((gprot_flag_reg & GPROT_FLAG_COMM_TIM) != 0) {
		if ((gprot_flag_reg_old & GPROT_FLAG_COMM_TIM) == 0) {
			control_process_ignite();
		}
	} else {
		if ((gprot_flag_reg_old & GPROT_FLAG_COMM_TIM) != 0) {
			control_process_kill();
		}
	}

	if ((gprot_flag_reg & GPROT_FLAG_ADC_COMM) != 0) {
		demo = true;
	} else {
		if ((gprot_flag_reg_old & GPROT_FLAG_ADC_COMM) != 0) {
			demo = false;
		}
	}

	if ((gprot_flag_reg & GPROT_FLAG_ALL_LO) != 0) {
		pwm_all_lo();
	} else {
		if ((gprot_flag_reg_old & GPROT_FLAG_ALL_LO) != 0) {
			pwm_off();
		}
	}

	if ((gprot_flag_reg & GPROT_FLAG_ALL_HI) != 0) {
		pwm_all_hi();
	} else {
		if ((gprot_flag_reg_old & GPROT_FLAG_ALL_HI) != 0) {
			pwm_off();
		}
	}

	/* add other flags here (up to 16) */

	gprot_flag_reg_old = gprot_flag_reg;
}

/**
 * Autamatically called by @ref gprot_register_changed()
 *
 * Implements setting PWM power in the PWM subsystem
 */
void gprot_update_pwm_power(void)
{
	PWM_SET(gprot_pwm_power);
}
