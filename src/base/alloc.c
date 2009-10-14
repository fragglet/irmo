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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "alloc.h"

void irmo_alloc_fail(size_t bytes)
{
        if (bytes > 0)
        {
                fprintf(stderr, "irmo_alloc_fail: Failed on allocation of %u bytes\n",
                                bytes);
        }
        else
        {
                fprintf(stderr, "irmo_alloc_fail: Out of memory error\n");
        }

        abort();
}

void *irmo_malloc0(size_t bytes)
{
        void *result;

        result = malloc(bytes);

        if (result == NULL)
        {
                irmo_alloc_fail(bytes);
        }

        memset(result, 0, bytes);

        return result;
}

void *irmo_realloc(void *ptr, size_t bytes)
{
        void *result;

        result = realloc(ptr, bytes);

        if (result == NULL)
        {
                irmo_alloc_fail(bytes);
        }

        return result;
}

