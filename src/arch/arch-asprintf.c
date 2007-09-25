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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"

#include "arch-asprintf.h"

#ifdef HAVE_VASPRINTF

char *irmo_vasprintf(char *format, va_list args)
{
        char *result;

        if (vasprintf(&result, format, args) < 0) {
                return NULL;
        }

        return result;
}

#else

//
// Portable replacement for vasprintf
//
// This is not actually ANSI C, as it relies on having vsnprintf, which
// is non-standard.  However, this is supported under Linux and Windows.
//

static char *vasprintf_buffer;
static int vasprintf_buffer_len = 0;

char *irmo_vasprintf(char *format, va_list args)
{
        char *result = NULL;
        int count;

        if (vasprintf_buffer_len <= 0) {
                vasprintf_buffer_len = 1024;
                vasprintf_buffer = malloc(vasprintf_buffer_len);
        }

        do {
                count = vsnprintf(vasprintf_buffer, vasprintf_buffer_len,
                                  format, args);

                if (count < 0 || count >= vasprintf_buffer_len) {
                        // Increase the buffer size and try again

                        vasprintf_buffer_len *= 2;
                        vasprintf_buffer = realloc(vasprintf_buffer,
                                                   vasprintf_buffer_len);
                } else {

                        // Successfully wrote the whole string into the
                        // buffer.  strdup and return it.

                        result = strdup(vasprintf_buffer);
                }
        } while (result == NULL);

        return result;
}

#endif

char *irmo_asprintf(char *format, ...)
{
        va_list args;
        char *result;

        va_start(args, format);

        result = irmo_vasprintf(format, args);
        
        va_end(args);

        return result;
}


