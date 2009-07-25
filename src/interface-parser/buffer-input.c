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
#include "base/assert.h"
#include "base/iterator.h"
#include "base/error.h"

#include <irmo/interface.h>
#include <irmo/interface-parser.h>

typedef struct {
        uint8_t *data;
        unsigned int offset;
        size_t length;
} InputBuffer;

static int from_buffer_input(void *buf, unsigned int buf_len, void *handle)
{
        InputBuffer *buffer = handle;
        unsigned int remaining_bytes;
        int result;

        // How many bytes are there left to read?

        remaining_bytes = buffer->length - buffer->offset;

        // Copy as many as we can

        if (remaining_bytes < buf_len) {
                result = (int) remaining_bytes;
        } else {
                result = (int) buf_len;
        }

        memcpy(buf, buffer->data + buffer->offset, (size_t) result);

        return result;
}

IrmoInterface *irmo_interface_parse_from_buffer(void *data, 
                                                size_t data_len)
{
        InputBuffer buffer;

        irmo_return_val_if_fail(data != NULL, NULL);

        // Initialise buffer structure

        buffer.data = data;
        buffer.offset = 0;
        buffer.length = data_len;

	return irmo_interface_parse(from_buffer_input, &buffer);
}

