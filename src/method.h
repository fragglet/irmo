// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id: method.h,v 1.3 2003-06-09 21:06:51 sdh300 Exp $
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
// Revision 1.2  2003/03/16 01:54:24  sdh300
// Method calls over network protocol
//
// Revision 1.1  2003/03/15 02:21:16  sdh300
// Initial method code
//
