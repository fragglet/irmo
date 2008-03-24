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

#ifndef IRMO_ARCH_ASPRINTF_H
#define IRMO_ARCH_ASPRINTF_H

#include <stdarg.h>

/*!
 * Printf to string, allocating the string automatically to the size 
 * needed.
 *
 * @param format            Format string (normal printf-style).
 * @return                  Pointer to a newly-allocated string containing
 *                          the expanded format string.
 */

char *irmo_asprintf(char *format, ...);

/*!
 * Printf to a string, allocating the string automatically to the size
 * needed, using a varargs list.
 *
 * @param format            Format string (normal printf-style).
 * @param args              List of arguments.
 * @return                  Pointer to a newly-allocated string containing
 *                          the expanded format string.
 */

char *irmo_vasprintf(char *format, va_list args);

#endif /* #ifndef IRMO_ARCH_ASPRINTF_H */

