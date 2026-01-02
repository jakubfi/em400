//  Copyright (c) 2025 Jakub Filipowicz <jakubf@gmail.com>
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc.,
//  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

#ifndef DEV2_H
#define DEV2_H

#include <stdbool.h>

enum em400_device_types {
	EM400_DEV_TERMINAL,
	EM400_DEV_SP45DE,
	EM400_DEV_WINCHESTER,
	EM400_DEV_FLOP5,
};

enum dev_cmd_status {
	DEV_STATUS_OK = 0,
	DEV_STATUS_ERR,
	DEV_STATUS_BUSY,
	DEV_STATUS_INVALID,
	DEV_STATUS_NOMEDIUM,
	DEV_STATUS_SEEKERR,
	DEV_STATUS_WRPROTECT,
	DEV_STATUS_WRERR,
	DEV_STATUS_RDERR,
};

typedef struct em400_dev em400_dev_t;

typedef void (*dev_noarg_f)(em400_dev_t *dev);
typedef int (*dev_write_f)(em400_dev_t *dev, char c);

typedef unsigned (*dev_slot_cnt_f)(em400_dev_t *dev);
typedef bool (*dev_is_ejectable_f)(em400_dev_t *dev, unsigned slot);
typedef int (*dev_load_f)(em400_dev_t *dev, unsigned slot, const char *image_name);
typedef int (*dev_eject_f)(em400_dev_t *dev, unsigned slot);
typedef const char * (*dev_image_f)(em400_dev_t *dev, unsigned slot);


struct em400_dev {
	int type;
	char *name;

	dev_noarg_f reset;
	dev_noarg_f shutdown;
	dev_write_f write;

	dev_slot_cnt_f slot_cnt;			// number of image slots in the device
	dev_is_ejectable_f is_ejectable;	// can the image be ejected now
	dev_load_f load;					// load new image into a slot
	dev_eject_f eject;					// eject image from the slot
	dev_image_f image;					// get the name of image loaded into a slot

};

#endif
