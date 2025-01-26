# SPDX-License-Identifier: Apache-2.0

zephyr_library()

zephyr_library_sources(
  nsp.c
  nsp_utils.c
  nsp_sockets.c

  nsp_simple_ep.c

  nsp_shell.c
)

