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

#ifndef IRMO_INTERFACE_CLASS_VAR_H
#define IRMO_INTERFACE_CLASS_VAR_H

#include <irmo/interface.h>

// Maximum variables per class.
// Imposed by the limits of the network protocol:

#define MAX_VARIABLES 256

// class member variable

struct _IrmoClassVar {

        // Parent class

	IrmoClass *klass;

        // Name of this variable.

	char *name;

        // Index in parent class's list of variables

	unsigned int index;

        // Type of value that this variable stores.

	IrmoValueType type;

        // Structure member that this variable is bound to, or 
        // NULL if it is not bound to any structure member.

        IrmoStructMember *member;
};

uint32_t irmo_class_var_hash(IrmoClassVar *class_var);
void _irmo_class_var_free(IrmoClassVar *var);

#endif /* #ifndef IRMO_INTERFACE_CLASS_VAR_H */

