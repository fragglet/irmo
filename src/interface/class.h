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

#ifndef IRMO_INTERFACE_CLASS_H
#define IRMO_INTERFACE_CLASS_H

#include <irmo/interface.h>

// Maximum classes per interface.
// Limit imposed by the network protocol.

#define MAX_CLASSES 256

// class

struct _IrmoClass {

	// parent interface

	IrmoInterface *iface;

	// parent class or NULL for base class
	
	IrmoClass *parent_class;

	int index;                      // index in class list

	char *name;			// class name
	
	IrmoClassVar **variables;	// class member variables
	int nvariables;
	
	IrmoHashTable *variable_hash;
};

uint32_t irmo_class_hash(IrmoClass *klass);
void _irmo_class_free(IrmoClass *klass);

#endif /* #ifndef IRMO_INTERFACE_CLASS_H */

