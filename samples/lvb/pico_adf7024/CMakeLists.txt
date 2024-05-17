# Copyright (c) 2024 Liam Beguin <liambeguin@gmail.com>
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(BOARD rpi_pico)

list(APPEND CONF_FILE "usb-cdc-acm.conf")

find_package(Zephyr)

project(pico_adf7024)
file(GLOB app_src src/*.c)
target_sources(app PRIVATE ${app_src})
