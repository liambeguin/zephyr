/*
 * Copyright (c) 2024 Liam Beguin <liambeguin@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <stdio.h>
#include <zephyr/drivers/uart.h>

#include <zephyr/logging/log.h>
LOG_MODULE_REGISTER(main, LOG_LEVEL_DBG);

// Devicetree
#define CONSOLE_DEVICE DEVICE_DT_GET(DT_CHOSEN(zephyr_console))


int main(void)
{
	const struct device *const console_dev = CONSOLE_DEVICE;
	uint32_t dtr_line = 0;

	while (!dtr_line) {
		uart_line_ctrl_get(console_dev, UART_LINE_CTRL_DTR, &dtr_line);
		k_sleep(K_MSEC(100));
	}

	return 0;
}
