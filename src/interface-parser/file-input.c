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

static int from_file_input(void *buf, unsigned int buf_len, void *handle)
{
	FILE *fstream = handle;
	int result;

	result = fread(buf, 1, buf_len, fstream);

	if (result <= 0) {
		if (feof(fstream)) {
			return 0;
		} else {
			return -1;
		}
	} else {
		return result;
	}
}

IrmoInterface *irmo_interface_parse_from_file(char *filename)
{
	FILE *instream;
	IrmoInterface *result;

        irmo_return_val_if_fail(filename != NULL, NULL);

	instream = fopen(filename, "r");

        if (instream == NULL) {
                irmo_error_report("irmo_interface_parse_from_file",
                                  "cannot open '%s'", filename);
                return NULL;
        }

	result = irmo_interface_parse(from_file_input, instream);

	fclose(instream);

	return result;
}

