# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources(
  nsp.c
  nsp_sockets.c
)

zephyr_library_sources_ifdef(CONFIG_NSP_BACKEND_UART nsp_uart.c)

zephyr_library_sources_ifdef(CONFIG_NSP_SIMPLE_EP nsp_simple_ep.c)
zephyr_library_sources_ifdef(CONFIG_NSP_SHELL nsp_shell.c)
