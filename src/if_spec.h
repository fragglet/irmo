#ifndef IFSPEC_H
#define IFSPEC_H

#include <glib.h>

typedef struct _InterfaceSpec InterfaceSpec;
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

struct _InterfaceSpec {

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

InterfaceSpec *interface_spec_new(char *filename);
void interface_spec_ref(InterfaceSpec *spec);
void interface_spec_unref(InterfaceSpec *spec);

#endif /* #ifndef IFSPEC_H */

// $Log: not supported by cvs2svn $
// Revision 1.2  2002/10/21 15:32:34  sdh300
// variable value setting
//
// Revision 1.1.1.1  2002/10/19 18:53:23  sdh300
// initial import
//
