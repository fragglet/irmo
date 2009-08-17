//
// Copyright (C) 2009 Simon Howard
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

#ifndef IRMO_MODULE_IP_H
#define IRMO_MODULE_IP_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * @ref IrmoNetModule implementation that communicates using UDP on IPv6.
 *
 * @addtogroup module_ipv6
 * \{
 */

/*!
 * Convenience constant for @ref irmo_module_ipv6.
 */

#define IRMO_NET_IPV6 (&irmo_module_ipv6)

/*!
 * Network module for communication using UDP over IPv6.
 */

extern IrmoNetModule irmo_module_ipv6;

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_MODULE_IP_H */


