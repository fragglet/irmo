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

#ifndef IRMO_INTERFACE_PARSER_H
#define IRMO_INTERFACE_PARSER_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * The external irmo-interface-parser library allows interfaces to be
 * parsed from external text interface description files, with
 * a syntax similar to C syntax.  The @ref irmo_interface_parse
 * function parses an interface description file, creating a new
 * @ref IrmoInterface object.
 *
 * @addtogroup ifaceparser
 * \{
 */

/*!
 * Function type used by the interface parser to read input data.
 *
 * @param buf           Pointer to buffer to read into.
 * @param buf_len       Length of the buffer, in bytes.
 * @param               Handle passed to @ref irmo_interface_parse.
 * @return              Number of bytes read into the buffer,
 *                      zero for end of file, or negative value
 *                      for error.
 */

typedef int (*IrmoInterfaceReadFunc)(void *buf, unsigned int buf_len,
                                     void *handle);

/*!
 * Create a new @ref IrmoInterface object, from parsing an interface
 * file.
 *
 * @param read_func     Function used to read input data to the parser.
 * @param handle        Handle to pass to the read function.
 * @return              A new IrmoInterface object or NULL for failure.
 */

IrmoInterface *irmo_interface_parse(IrmoInterfaceReadFunc read_func,
                                    void *handle);

/*!
 * Create a new @ref IrmoInterface object, from parsing an interface file.
 *
 * A file is parsed with the interface described in a simple
 * C-style syntax.
 *
 * @param filename	The filename of the interface file.
 * @return		A new IrmoInterface object or NULL for failure.
 */

IrmoInterface *irmo_interface_parse_from_file(char *filename);

/*!
 * Create a new @ref IrmoInterface object, parsing from a buffer.
 *
 * A buffer is parsed with the interface described in a simple
 * C-style syntax.
 *
 * @param data          Pointer to the data buffer.
 * @param data_len      Length of the data buffer.
 * @return		A new IrmoInterface object or NULL for failure.
 */

IrmoInterface *irmo_interface_parse_from_buffer(void *data, 
                                                size_t data_len);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_INTERFACE_PARSER_H */

