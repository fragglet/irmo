// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2002-2003 University of Southampton
// Copyright (C) 2003 Simon Howard
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------

#ifndef IRMO_INTERNAL_IF_SPEC_H
#define IRMO_INTERNAL_IF_SPEC_H

#include <glib.h>

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

	GHashTable *argument_hash;
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
	
	GHashTable *variable_hash;
};

// overall interface

struct _IrmoInterfaceSpec {

	int refcount;
	
	// classes:
	
	IrmoClass **classes;
	int nclasses;

	GHashTable *class_hash;

	// methods:
	
	IrmoMethod **methods;
	int nmethods;

	GHashTable *method_hash;

	// unique (or should be) hash value

	guint hash;
};

#endif /* #ifndef IRMO_INTERNAL_IF_SPEC_H */

// $Log$
// Revision 1.6  2003/09/12 11:30:25  fraggle
// Rename IrmoVarType to IrmoValueType to be orthogonal to IrmoValue
//
// Revision 1.5  2003/09/02 20:33:55  fraggle
// Subclassing in interfaces
//
// Revision 1.4  2003/08/31 18:32:10  fraggle
// refcounting functions for the InterfaceSpec internals
//
// Revision 1.3  2003/08/28 15:24:02  fraggle
// Make types for object system part of the public API.
// *Spec renamed -> Irmo*.
// More complete reflection API and better structured.
//
// Revision 1.2  2003/08/21 14:21:25  fraggle
// TypeSpec => IrmoVarType.  TYPE_* => IRMO_TYPE_*.  Make IrmoVarType publicly
// accessible.
//
// Revision 1.1.1.1  2003/06/09 21:33:23  fraggle
// Initial sourceforge import
//
// Revision 1.9  2003/06/09 21:06:51  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.8  2003/03/14 17:36:36  sdh300
// Store index for method specs
//
// Revision 1.7  2003/03/06 19:33:50  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.6  2003/02/23 00:00:03  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.5  2002/11/17 22:01:58  sdh300
// InterfaceSpec documentation
//
// Revision 1.4  2002/11/13 13:56:24  sdh300
// add some documentation
//
// Revision 1.3  2002/10/29 15:58:26  sdh300
// add index entry for ClassSpec, use "index" instead of n for index
// member name
//
// Revision 1.2  2002/10/21 15:32:34  sdh300
// variable value setting
//
// Revision 1.1.1.1  2002/10/19 18:53:23  sdh300
// initial import
//
