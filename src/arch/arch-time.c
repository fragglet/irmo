// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2005 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//---------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>

#include "arch-time.h"

// Function to read the current time.  This is platform specific (there
// is no way in ANSI C to find the current time in ms).

#define USEC_MAX 1000000

static struct timeval irmo_start_time = { 0, 0 };

static void irmo_timeval_sub(struct timeval *a, 
                             struct timeval *b, 
                             struct timeval *result)
{
	result->tv_sec = a->tv_sec - b->tv_sec;

	if (a->tv_usec >= b->tv_usec) {
		result->tv_usec = a->tv_usec - b->tv_usec;
	} else {
		--result->tv_sec;
		result->tv_usec = USEC_MAX - (b->tv_usec - a->tv_usec);
	}
}

unsigned int irmo_get_time(void)
{
        struct timeval now_time;
        struct timeval relative_time;

        gettimeofday(&now_time, NULL);

        // is this the first time the function has been called? if
        // so, start time from now.
 
        if (irmo_start_time.tv_sec == 0) {
                irmo_start_time = now_time;
        }

        // subtract the start time to get a time relative to it.

        irmo_timeval_sub(&now_time, &irmo_start_time, &relative_time);

        return (relative_time.tv_sec * 1000) 
             + (relative_time.tv_usec / 1000);
}


