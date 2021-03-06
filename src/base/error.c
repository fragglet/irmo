//
// Copyright (C) 2002-2008 Simon Howard
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

#include "arch/sysheaders.h"

#include "error.h"

static char *last_error = NULL;

void irmo_error_report(char *function_name, char *format, ...)
{
	va_list args;

	va_start(args, format);

        free(last_error);
	last_error = irmo_vasprintf(format, args);

	va_end(args);

	fprintf(stderr, "%s: %s\n", function_name, last_error);
}

char *irmo_error_get(void)
{
	return last_error;
}

