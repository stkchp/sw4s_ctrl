/*
 * U2H-SW4S power on/off utility in linux
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

#include <linux/limits.h>

#include <libudev.h>

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "sw4s_ctrl.h"

/* store search device */
static size_t sw4s_count = 0;
static bool sw4s_searched = false;
static char sw4s_devices[SW4S_LIST_MAX][PATH_MAX] = {{0}};

static void sw4s_search_device(void)
{
	size_t count = 0;

	/* search hidraw device */
	struct udev *udev;
	struct udev_enumerate *enumerate;
	struct udev_list_entry *devices, *dev_list_entry;
	struct udev_device *dev;

	if(sw4s_searched)
		return;
	sw4s_searched = true;

	udev = udev_new();
	if (!udev) {
		fprintf(stderr, "cannot allocate udev object.");
		return;
	}

	enumerate = udev_enumerate_new(udev);
	udev_enumerate_add_match_subsystem(enumerate, "hidraw");
	udev_enumerate_scan_devices(enumerate);
	devices = udev_enumerate_get_list_entry(enumerate);

	udev_list_entry_foreach(dev_list_entry, devices) {

		const char *path;
		const char *vid_str;
		const char *pid_str;
		const char *dev_node;
		struct udev_device *usb_dev;

		/* path     -> /sys/devices/.../hidrawX
		 * dev_node -> /dev/hidrawX
		 */
		path = udev_list_entry_get_name(dev_list_entry);
		dev = udev_device_new_from_syspath(udev, path);
		dev_node = udev_device_get_devnode(dev);

		usb_dev = udev_device_get_parent_with_subsystem_devtype(
				dev,
				"usb",
				"usb_device");
		if (!usb_dev) goto unref_dev;

		vid_str = udev_device_get_sysattr_value(usb_dev, "idVendor");
		pid_str = udev_device_get_sysattr_value(usb_dev, "idProduct");

		if(strcmp(vid_str, SW4S_VID_STR)) goto unref_dev;
		if(strcmp(pid_str, SW4S_PID_STR)) goto unref_dev;

		memcpy(sw4s_devices[count], dev_node, PATH_MAX);

		count++;

unref_dev:
		udev_device_unref(dev);

		if(count >= SW4S_LIST_MAX) break;
	}
	/* Free the enumerator object */
	udev_enumerate_unref(enumerate);
	udev_unref(udev);

	sw4s_count = count;

	return;
}

/* return 0 when success. */
int sw4s_get_device(const char *search, char *buff, size_t len)
{
	size_t i;

	sw4s_search_device();

	if(sw4s_count == 0) return -1;

	if(search == NULL) {
		// get first hiddev
		memcpy(buff, sw4s_devices[0], len);
	} else {
		// search device
		for(i = 0; i < sw4s_count; i++) {
			if(strcmp(search, sw4s_devices[i]))
				continue;
			memcpy(buff, sw4s_devices[i], len);
			break;
		}
	}
	return 0;
}

void sw4s_list(void)
{
	size_t i;

	sw4s_search_device();

	for(i = 0; i < sw4s_count; i++) {
		printf("%s\n", sw4s_devices[i]);
	}
}

size_t sw4s_get_count(void)
{
	sw4s_search_device();

	return sw4s_count;
}

