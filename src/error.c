// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2002-2003 University of Southampton
// Copyright (C) 2003 Simon Howard
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include "error.h"

static char *last_error = NULL;

void irmo_error_report(char *function_name, char *format, ...)
{
	va_list args;

	va_start(args, format);

	free(last_error);

	last_error = g_strdup_vprintf(format, args);

	va_end(args);

	fprintf(stderr, "%s: %s\n", function_name, last_error);
}

char *irmo_error_get(void)
{
	return last_error;
}

// $Log$
// Revision 1.2  2003/08/13 18:32:40  fraggle
// Convert () in function definitions to the more explicit (void)
//
// Revision 1.1  2003/07/24 01:32:08  fraggle
// Add error reporting modules
//

