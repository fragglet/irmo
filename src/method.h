#ifndef IRMO_INTERNAL_METHOD_H
#define IRMO_INTERNAL_METHOD_H

#include "public/method.h"

#include "if_spec.h"
#include "object.h"

struct _IrmoMethodData {
	MethodSpec *spec;
	IrmoClient *src;
	IrmoVariable *args;
};


#endif /* #ifndef IRMO_INTERNAL_METHOD_H */

// $Log: not supported by cvs2svn $
