#ifndef IRMO_INTERNAL_METHOD_H
#define IRMO_INTERNAL_METHOD_H

#include "public/method.h"

#include "client.h"
#include "if_spec.h"
#include "object.h"

struct _IrmoMethodData {
	MethodSpec *spec;
	IrmoClient *src;
	IrmoVariable *args;
};

// internal method invocation

void irmo_method_invoke(IrmoUniverse *universe, IrmoMethodData *data);

#endif /* #ifndef IRMO_INTERNAL_METHOD_H */

// $Log: not supported by cvs2svn $
// Revision 1.1  2003/03/15 02:21:16  sdh300
// Initial method code
//
