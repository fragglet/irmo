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

#ifndef IRMO_BASE_UTIL_H
#define IRMO_BASE_UTIL_H

/*!
 * "Rotate" a 32-bit integer value: the resulting value is bitshifted to 
 * the left by one bit, with the top bit moved to the bottom bit.
 * This is useful when writing hash functions.
 *
 * @param i             Value to rotate.
 * @return              Rotated value.
 */

uint32_t irmo_rotate_int(uint32_t i);

#endif /* #ifndef IRMO_BASE_UTIL_H */

