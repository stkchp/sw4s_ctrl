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

/*
 * Communicate with read/write 8byte.
 * Thanks for iteclub!
 *
 *       ** Port ON/OFF **
 * 　    WR 03 5D 00 05(Port1) 01(ON) 00 00 00
 * 　                02(Port2) 00(OFF)
 * 　                03(Port3)
 * 　                04(Port4)
 *       ** Get Status **
 *       WR 03 5D 02 00 00 00 00 00
 *       RD 03 5D XX 00 75 00 00 00
 *
 *       XX = 00xxxx11b (ON:1/OFF:2)
 *       xxxx order is 1,4,3,2
 *
 * */

#include <linux/limits.h>

#include <unistd.h>
#include <fcntl.h>

#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "sw4s_ctrl.h"

#define CMD_SHOW 1
#define CMD_ON   2
#define CMD_OFF  3

/* get status related code */
const char sw4s_get_status_code[8] = {
	0x03, 0x5d, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00
};
const char sw4s_get_status_resp[8] = {
	0x03, 0x5d, 0x00, 0x00, 0x75, 0x00, 0x00, 0x00
};
const char sw4s_get_status_mask[SW4S_PORTNUM] = {
	0x20, 0x04, 0x08, 0x10
};
/* set power related code */
const char sw4s_set_power_code[8] = {
	0x03, 0x5d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};
const char sw4s_set_power_value[SW4S_PORTNUM] = {
	0x05, 0x02, 0x03, 0x04
};

int sw4s_get_status(int fd)
{
	int res = 0, status;
	char buf[8];

	/* request get status */
	res = write(fd, sw4s_get_status_code, 8);
	if (res < 0) {
		fprintf(stderr, "Error: %s cannot write . (errno=%d)\n",
				__func__, errno);
		return -1;
	}

	/* Get a report from the device */
	res = read(fd, buf, 8);
	if (res < 0) {
		fprintf(stderr, "Error: %s cannot read. (errno=%d)\n",
				__func__, errno);
		return -1;
	}

	/* check that reply is status */
	if (memcmp(&buf[0], &sw4s_get_status_resp[0], 2) ||
	    memcmp(&buf[3], &sw4s_get_status_resp[3], 5) ||
	    (buf[2] & 0x3) != 0x3) {
		fprintf(stderr, "Error: %s response is not status.\n",
				__func__);
#if 0
		/* dump response */
		size_t i;
		fprintf(stderr, "response:");
		for(i = 0; i < sizeof(buf); i++) {

			fprintf(stderr, " %02X", (unsigned int)buf[i]);
		}
		fprintf(stderr, "\n");
#endif
		return -1;
	}

	status = 0;
	if(buf[2] & sw4s_get_status_mask[0]) status |= SW4S_PORT1;
	if(buf[2] & sw4s_get_status_mask[1]) status |= SW4S_PORT2;
	if(buf[2] & sw4s_get_status_mask[2]) status |= SW4S_PORT3;
	if(buf[2] & sw4s_get_status_mask[3]) status |= SW4S_PORT4;

	return status;
}

int sw4s_set_power(int fd, int target, bool on)
{
	int res = 0;
	char buf[8];
	size_t i;
	fprintf(stderr, "target: %02X\n", target);

	for(i = 0; i < SW4S_PORTNUM; i++) {

		if((target & (1 << i)) == 0) continue;

		memcpy(buf, sw4s_set_power_code, 8);
		buf[3] = sw4s_set_power_value[i];
		buf[4] = on ? 0x01 : 0x00;

#if 0
		res = write(fd, buf, 8);
		if (res < 0) {
			fprintf(stderr, "Error: %s cannot write. (port=%zu)(errno=%d)\n",
					__func__, i + 1, errno);
			return -1;
		}
#endif
	}

	return 0;
}

int command_onoff(int fd, int cur, int req, bool on)
{
	int res = 0;
	int needed = 0;

	if (on) {
		needed = ((~cur) & req);
	} else {
		needed = (cur & req);
	}

	if(needed == 0) {
		fprintf(stderr, "Info: Already port is setted desired value.\n");
		return 0;
	}

	res = sw4s_set_power(fd, needed, on);
	if(res < 0) {
		fprintf(stderr, "Error: Fail to set power %s.\n",
				on ? "on" : "off");
		return -1;
	}

	res = sw4s_get_status(fd);
	if (on) {
		needed = ((~res) & req);
	} else {
		needed = (res & req);
	}
	if(needed != 0) {
		fprintf(stderr, "Error: Post check fail.\n");
		return -1;
	}

	return 0;
}

void usage(char *prog) {
	printf(
"Usage: %s [options] show\n"
"   or: %s [options] on\n"
"   or: %s [options] off\n"
"\n"
"Options: [ d1234h ]\n"
"  -d device		target hidraw device\n"
"  -1,-2,-3,-4		target port(on/off only)\n"
, prog, prog, prog);
}

int main(int argc, char **argv) {
	int fd, res;
	size_t i;
	char *req_target = NULL;
	char *commandstr = NULL;
	int command = 0;
	char target[PATH_MAX];
	int port = 0;

	if(argc == 1) {
		usage(argv[0]);
		return 1;
	}
	while((res = getopt(argc, argv, "1234d:")) != -1) {
		switch(res) {
			case '1':
				port |= SW4S_PORT1;
				break;
			case '2':
				port |= SW4S_PORT2;
				break;
			case '3':
				port |= SW4S_PORT3;
				break;
			case '4':
				port |= SW4S_PORT4;
				break;
			case 'd':
				req_target = optarg;
				if(req_target == NULL) {
					fprintf(stderr, "Error: device not specified.\n");
					return 1;
				}
				break;
			case 'h':
				usage(argv[0]);
				return 0;
			default:
				usage(argv[0]);
				return 1;
		}
	}

	// get normal argument.
	if((argc - 1) != optind) {
		usage(argv[0]);
		return 1;
	}
	commandstr = argv[optind];

	// command check
	if (!strcmp(commandstr, "show")) command = CMD_SHOW;
	if (!strcmp(commandstr, "on"))   command = CMD_ON;
	if (!strcmp(commandstr, "off"))  command = CMD_OFF;
	if(!command) {
		fprintf(stderr, "Error: command only allow show, on, off.\n");
		return 1;
	}
	if(command == CMD_ON || command == CMD_OFF) {
		if(port == 0) {
			fprintf(stderr, "Info: target port not specified."
					" Nothing todo.\n");
			return 0;
		}
	}

	if (sw4s_get_count() != 0) {
		printf("available device:\n");
		sw4s_list();
		printf("\n");
	}

	/* get target device */
	res = sw4s_get_device(req_target, target, sizeof(target));
	if(res) {
		fprintf(stderr, "Error: Cannot get U2H-SW4S device.\n");
		return 1;
	}
	printf("target device:\n%s\n\n", target);


	/* open target device */
	fd = open(target, O_RDWR);
	if(fd < 0) {
		fprintf(stderr, "Error: Cannot open device '%s'. (e=%d)\n",
				target, errno);
		return 1;
	}

	/* get current status */
	res = sw4s_get_status(fd);
	if(res < 0) {
		fprintf(stderr, "Error: Cannot get status '%s'. (e=%d)\n",
				target, errno);
		close(fd);
		return 1;
	}

	switch(command) {
		case CMD_SHOW:
			printf("status:\n");
			for(i = 0; i < SW4S_PORTNUM; i++) {
				printf("port%zu=%s\n", i + 1,
						(res & (1 << i)) ? "on" : "off");
			}
			break;
		case CMD_ON:
		case CMD_OFF:
			res = command_onoff(fd, res, port, command == CMD_ON);
			if(res < 0) {
				close(fd);
				return 1;
			}
			printf("Command successful.\n");
			break;
	}

	close(fd);
	return 0;
}

