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

#include <stdio.h>
#include <stdlib.h>

#include "assert.h"

void irmo_warning_message(const char *function_name, char *message, ...)
{
        va_list args;

        va_start(args, message);

        fprintf(stderr, "%s: ", function_name);
        vfprintf(stderr, message, args);
        fprintf(stderr, "\n");

        va_end(args);
}

void irmo_assert_fail_message(const char *function_name, char *assertation)
{
        irmo_warning_message(function_name, 
                             "Warning: assertation '%s' failed.\n",
                             assertation);
}

void irmo_bug_abort(char *file, int line)
{
        fprintf(stderr, "Irmo BUG at %s:%i!\n", file, line);
        abort();
}

