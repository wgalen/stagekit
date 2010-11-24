
/*
 *
 * This code is modified version of the fftest.c program
 * which Tests the force feedback driver by Johan Deneux.
 * Modifications to incorporate into Word War vi
 * by Stephen M.Cameron
 *
 * Copyright 2001-2002 Johann Deneux <deneux@ifrance.com>
 * Copyright 2008 Stephen M. Cameron <smcameron@yahoo.com>
 */

/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * You can contact the author by email at this address:
 * Johann Deneux <deneux@ifrance.com>
 */
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "rumble.h"
#include "stagekit.h"

#ifdef __linux__
#define HAS_LINUX_JOYSTICK_INTERFACE 1
#endif

#ifdef HAS_LINUX_JOYSTICK_INTERFACE
#include <linux/input.h>
#endif

#define BITS_PER_LONG (sizeof(long) * 8)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)    ((array[LONG(bit)] >> OFF(bit)) & 1)

#define N_EFFECTS 5

char* effect_names[] = {
	"Sine vibration",
	"Constant Force",
	"Spring Condition",
	"Damping Condition",
	"Strong Rumble",
	"Weak Rumble"
};

#ifdef HAS_LINUX_JOYSTICK_INTERFACE


static int event_fd;
static char *default_event_file = "/dev/input/event6";
static int n_effects;	/* Number of effects the device can play at the same time */
static unsigned long features[4];
static struct ff_effect effects[N_EFFECTS];

#endif /* HAS_LINUX_JOYSTICK_INTERFACE */

int stop_all_rumble_effects(void)
{
#ifdef HAS_LINUX_JOYSTICK_INTERFACE
	int i;
	struct input_event stop;

	for (i=0; i<N_EFFECTS; ++i) {
		stop.type = EV_FF;
		stop.code =  effects[i].id;
		stop.value = 0;

		if (write(event_fd, (const void*) &stop, sizeof(stop)) == -1) {
			perror("Stop effect");
			exit(1);
		}
	}
#endif
	return 0;
}

int play_rumble_effect(int effect)
{
#ifdef HAS_LINUX_JOYSTICK_INTERFACE
	struct input_event play;

	if (effect < 0 || effect >= N_EFFECTS)
		return -1;

	play.type = EV_FF;
	play.code = effects[effect].id;
	play.value = 1;

	if (write(event_fd, (const void*) &play, sizeof(play)) == -1)
		return -1;
#endif
	return 0;
}

int send_raw_value(unsigned short left, unsigned short right)
{
#ifdef HAS_LINUX_JOYSTICK_INTERFACE
    struct input_event play;
    /* download the effect */
    effects[0].u.rumble.strong_magnitude = left;
    effects[0].u.rumble.weak_magnitude = right;

	if (ioctl(event_fd, EVIOCSFF, &effects[0]) == -1) {
		printf("failed to upload effect: %s\n", strerror(errno));
		;
	}
    play.type=EV_FF;
    play.code=effects[0].id;
    play.value=1;

    if (write(event_fd,(const void*) &play, sizeof(play)) == -1)
        return -1;
#endif
    return 0;
}

void close_rumble_fd(void)
{
#ifdef HAS_LINUX_JOYSTICK_INTERFACE
	close(event_fd);
#endif
}

int get_ready_to_rumble(char *filename)
{
#ifdef HAS_LINUX_JOYSTICK_INTERFACE
	if (filename == NULL)
		filename = default_event_file;

	event_fd = open(filename, O_RDWR);
	if (event_fd < 0) {
		fprintf(stderr, "Can't open %s: %s\n",
			filename, strerror(errno));
		return -1;
	}

	printf("Device %s opened\n", filename);

	/* Query device */
	if (ioctl(event_fd, EVIOCGBIT(EV_FF, sizeof(unsigned long) * 4), features) == -1) {
		fprintf(stderr, "Query of rumble device failed: %s:%s\n",
			filename, strerror(errno));
		return -1;
	}

	printf("Axes query: ");

	if (test_bit(ABS_X, features)) printf("Axis X ");
	if (test_bit(ABS_Y, features)) printf("Axis Y ");
	if (test_bit(ABS_WHEEL, features)) printf("Wheel ");

	printf("\nEffects: ");

	if (test_bit(FF_CONSTANT, features)) printf("Constant ");
	if (test_bit(FF_PERIODIC, features)) printf("Periodic ");
	if (test_bit(FF_SPRING, features)) printf("Spring ");
	if (test_bit(FF_FRICTION, features)) printf("Friction ");
	if (test_bit(FF_RUMBLE, features)) printf("Rumble ");

	printf("\nNumber of simultaneous effects: ");

	if (ioctl(event_fd, EVIOCGEFFECTS, &n_effects) == -1) {
		fprintf(stderr, "Query of number of simultaneous "
			"effects failed, assuming 1. %s:%s\n",
			filename, strerror(errno));
		n_effects = 1;	 /* assume 1. */
	}

	printf("%d\n", n_effects);


	/* download a lights out effect */
	effects[0].type = FF_RUMBLE;
	effects[0].id = -1;
    effects[0].u.rumble.strong_magnitude = 0x0;
    effects[0].u.rumble.weak_magnitude = STAGEKIT_ALLOFF;
	effects[0].direction = 0;
	effects[0].trigger.button = 0;
	effects[0].trigger.interval = 0;
	effects[0].replay.length = EFFECT_LENGTH;
	effects[0].replay.delay = EFFECT_DELAY;
		if (ioctl(event_fd, EVIOCSFF, &effects[0]) == -1) {
		fprintf(stdout, "%s: failed to upload AllOff effect: %s\n",
				filename, strerror(errno));
		;
	}

	/* Load a slow strobe */
    effects[1].type = FF_RUMBLE;
	effects[1].id = -1;
    effects[1].u.rumble.strong_magnitude = 0x0;
    effects[1].u.rumble.weak_magnitude = STAGEKIT_SLOW_STROBE;
	effects[1].direction = 0;
	effects[1].trigger.button = 0;
	effects[1].trigger.interval = 0;
	effects[1].replay.length = EFFECT_LENGTH;
	effects[1].replay.delay = EFFECT_DELAY;
       	if (ioctl(event_fd, EVIOCSFF, &effects[1]) == -1) {
		fprintf(stdout, "%s: failed to upload SlowStrobe effect: %s\n",
				filename, strerror(errno));
		;
	}

	/* Load a medium strobe */
	effects[2].type = FF_RUMBLE;
	effects[2].id = -1;
    effects[2].u.rumble.strong_magnitude = 0x0;
    effects[2].u.rumble.weak_magnitude = STAGEKIT_MEDIUM_STROBE;
	effects[2].direction = 0;
	effects[2].trigger.button = 0;
	effects[2].trigger.interval = 0;
	effects[2].replay.length = EFFECT_LENGTH;
	effects[2].replay.delay = EFFECT_DELAY;
	if (ioctl(event_fd, EVIOCSFF, &effects[2]) == -1) {
		fprintf(stdout, "%s: failed to upload MediumStrobe effect: %s\n",
				filename, strerror(errno));
		;
	}

    /*Load a fast strobe */
    effects[3].type = FF_RUMBLE;
	effects[3].id = -1;
    effects[3].u.rumble.strong_magnitude = 0x0;
    effects[3].u.rumble.weak_magnitude = STAGEKIT_FAST_STROBE;
	effects[3].direction = 0;
	effects[3].trigger.button = 0;
	effects[3].trigger.interval = 0;
	effects[3].replay.length = EFFECT_LENGTH;
	effects[3].replay.delay = EFFECT_DELAY;
    if (ioctl(event_fd, EVIOCSFF, &effects[3]) == -1) {
		fprintf(stdout, "%s: failed to upload FastStrobe effect: %s\n",
				filename, strerror(errno));
		;
	}

	/* Load the fastest strobe */
	effects[4].type = FF_RUMBLE;
	effects[4].id = -1;
    effects[4].u.rumble.strong_magnitude = 0x0;
    effects[4].u.rumble.weak_magnitude = STAGEKIT_FASTEST_STROBE;
	effects[4].direction = 0;
	effects[4].trigger.button = 0;
	effects[4].trigger.interval = 0;
	effects[4].replay.length = EFFECT_LENGTH;
	effects[4].replay.delay = EFFECT_DELAY;
	if (ioctl(event_fd, EVIOCSFF, &effects[4]) == -1) {
		fprintf(stdout, "%s: failed to upload FastestStrobe effect: %s\n",
				filename, strerror(errno));
		;
	}




	return 0;
#else
	return -1;
#endif
}
