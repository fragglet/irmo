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

#ifndef IRMO_BASE_ERROR_H
#define IRMO_BASE_ERROR_H

#include <irmo/error.h>

/*!
 * Save the reason for an error occurring; the error message can be retrieved
 * using @param irmo_error_get.
 *
 * @param function_name    The function in which the error occurred.
 * @param format           printf format string for the error message.
 */

void irmo_error_report(char *function_name, char *format, ...);

#endif /* #ifndef IRMO_BASE_ERROR_H */

