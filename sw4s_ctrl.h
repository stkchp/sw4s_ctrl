/*
 * U2C-SW4S power on/off utility in linux
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * The full GNU General Public License is included in this distribution in
 * the file called "COPYING".
 *
 */
#ifndef _SW4S_CTRL_H
#define _SW4S_CTRL_H

/*
 * target device info
 * 	VendorID:  0x2101
 * 	ProductID: 0x8501
 */

#define SW4S_VID       0x2101
#define SW4S_PID       0x8501
#define SW4S_VID_STR   "2101"
#define SW4S_PID_STR   "8501"
#define SW4S_PORTNUM   4
#define SW4S_LIST_MAX  16

#define SW4S_PORT1     0x01
#define SW4S_PORT2     0x02
#define SW4S_PORT3     0x04
#define SW4S_PORT4     0x08

/* sw4s_udev.c */
int sw4s_get_device(const char *search, char *buff, size_t len);
void sw4s_list(void);
size_t sw4s_get_count(void);

#endif
