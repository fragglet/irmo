#ifndef IRMO_INTERNAL_IF_SPEC_H
#define IRMO_INTERNAL_IF_SPEC_H

#include <glib.h>

#include "public/if_spec.h"

typedef struct _ClassSpec ClassSpec;
typedef struct _ClassVarSpec ClassVarSpec;
typedef struct _MethodSpec MethodSpec;
typedef struct _MethodArgSpec MethodArgSpec;

// these are imposed by the size restrictions
// of the network protocol 

#define MAX_METHODS 256
#define MAX_CLASSES 256
#define MAX_VARIABLES 256
#define MAX_ARGUMENTS 256

typedef enum {
	TYPE_INT8,
	TYPE_INT16,
	TYPE_INT32,
	TYPE_STRING,
} TypeSpec;

// an argument to a method

struct _MethodArgSpec {
	TypeSpec type;
	char *name;
};

// method declaration

struct _MethodSpec {
	char *name;			// method name
	
	MethodArgSpec **arguments;
	int narguments;

	GHashTable *argument_hash;
};

// class member variable

struct _ClassVarSpec {
	int index;                      // index in variable list
	
	TypeSpec type;
	char *name;
};

// class

struct _ClassSpec {
	int index;                      // index in class list

	char *name;			// class name
	
	ClassVarSpec **variables;	// class member variables
	int nvariables;
	
	GHashTable *variable_hash;
};

// overall interface

struct _IrmoInterfaceSpec {

	int refcount;
	
	// classes:
	
	ClassSpec **classes;
	int nclasses;

	GHashTable *class_hash;

	// methods:
	
	MethodSpec **methods;
	int nmethods;

	GHashTable *method_hash;

	// unique (or should be) hash value

	guint hash;
};

#endif /* #ifndef IRMO_INTERNAL_IF_SPEC_H */

// $Log: not supported by cvs2svn $
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
