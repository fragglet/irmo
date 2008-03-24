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

#ifndef IRMO_ERROR_H
#define IRMO_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Some functions report errors. All error messages are echoed to the 
 * standard error output (stderr) and the error message can be retrieved
 * by the program with @ref irmo_error_get.
 *
 * @addtogroup error
 * \{
 */

/*!
 * Get an error message.
 *
 * Returns the string containing the last reported error message
 * by the library.
 *
 * @return          A constant string containing the error.
 *
 */

char *irmo_error_get(void);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_ERROR_H */

