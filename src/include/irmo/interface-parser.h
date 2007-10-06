//
// Copyright (C) 2002-3 Simon Howard
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
 * a syntax similar to C syntax.  The \ref irmo_interface_parse
 * function parses an interface description file, creating a new
 * \ref IrmoInterface object.
 *
 * \addtogroup ifaceparser
 * \{
 */

/*!
 * Create a new IrmoInterface object, from parsing an interface file.
 *
 * A file is parsed with the interface described in a simple
 * C-style syntax.
 *
 * \param filename	The filename of the interface file.
 * \return		A new IrmoInterface object or NULL for failure.
 */

IrmoInterface *irmo_interface_parse(char *filename);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_INTERFACE_PARSER_H */

