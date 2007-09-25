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

#ifndef IRMO_INTERNAL_IF_SPEC_H
#define IRMO_INTERNAL_IF_SPEC_H

#include "public/if_spec.h"

// these are imposed by the size restrictions
// of the network protocol 

#define MAX_METHODS 256
#define MAX_CLASSES 256
#define MAX_VARIABLES 256
#define MAX_ARGUMENTS 256

// an argument to a method

struct _IrmoMethodArg {
	IrmoMethod *parent;

	int index;
	IrmoValueType type;
	char *name;
};

// method declaration

struct _IrmoMethod {
	IrmoInterfaceSpec *parent;

	int index;
	char *name;			// method name
	
	IrmoMethodArg **arguments;
	int narguments;

	IrmoHashTable *argument_hash;
};

// class member variable

struct _IrmoClassVar {
	IrmoClass *parent;

	int index;                      // index in variable list
	IrmoValueType type;
	char *name;
};

// class

struct _IrmoClass {

	// parent object

	IrmoInterfaceSpec *parent;

	// parent class or NULL for base class
	
	IrmoClass *parent_class;

	int index;                      // index in class list

	char *name;			// class name
	
	IrmoClassVar **variables;	// class member variables
	int nvariables;
	
	IrmoHashTable *variable_hash;
};

// overall interface

struct _IrmoInterfaceSpec {

	int refcount;
	
	// classes:
	
	IrmoClass **classes;
	int nclasses;

	IrmoHashTable *class_hash;

	// methods:
	
	IrmoMethod **methods;
	int nmethods;

	IrmoHashTable *method_hash;

	// unique (or should be) hash value

	unsigned int hash;
};

#endif /* #ifndef IRMO_INTERNAL_IF_SPEC_H */

