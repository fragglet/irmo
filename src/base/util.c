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

#include "arch/sysheaders.h"

#include <stdio.h>
#include <stdlib.h>

#include "util.h"

void *irmo_malloc0(int bytes)
{
        void *p;

        p = malloc(bytes);
        memset(p, 0, bytes);

        return p;
}

void irmo_assert_fail_message(const char *function_name, char *assertation)
{
        fprintf(stderr, "%s: Warning: assertation '%s' failed.\n", 
                        function_name, assertation);
}

void irmo_bug_abort(char *file, int line)
{
        fprintf(stderr, "Irmo BUG at %s:%i!\n", file, line);
        abort();
}

uint32_t irmo_rotate_int(uint32_t i)
{
	return (i << 1) | (i >> 31);
}

