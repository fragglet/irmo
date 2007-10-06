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

//
// Callbacks
//

#ifndef IRMO_CALLBACK_H
#define IRMO_CALLBACK_H

#include "types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Various parts of the library allow callback watches to be placed 
 * on particular events. When the event occurs, the callback function
 * will be invoked. These watches are represented by a 
 * \ref IrmoCallback object.
 *
 * \addtogroup callback
 * \{
 */

/*!
 * Unset a callback watch. This cancels a watch which has been set.
 *
 * \param callback   The callback to destroy.
 *
 */

void irmo_callback_unset(IrmoCallback *callback);

/*!
 * Set a callback for when a \ref IrmoCallback is destroyed.
 *
 * This sets a watch which invokes a callback when a callback object 
 * is destroyed, either from being explicitely unset or if the object 
 * it is watching is destroyed. 
 *
 * This is mostly useful when writing language bindings so that the
 * language can know when to free a callback context.
 *
 * \param callback   The callback to watch.
 * \param func       Function to invoke.
 * \param user_data  Extra data to pass to the function.
 * \return           A new callback object representing the watch.
 *
 */

IrmoCallback *irmo_callback_watch_destroy(IrmoCallback *callback,
					  IrmoCallbackCallback func,
					  void *user_data);
					  
//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_CALLBACK_H */

