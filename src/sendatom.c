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

#include "sysheaders.h"

#include "sendatom.h"

IrmoSendAtomClass *irmo_sendatom_types[NUM_SENDATOM_TYPES] = {
	&irmo_null_atom,
	&irmo_newobject_atom,
	&irmo_change_atom,
	&irmo_destroy_atom,
	&irmo_method_atom,
	&irmo_sendwindow_atom,
};

//---------------------------------------------------------------------
//
// $Log$
// Revision 1.14  2005/12/23 22:47:50  fraggle
// Add algorithm implementations from libcalg.   Use these instead of
// the glib equivalents.  This is the first stage in removing the dependency
// on glib.
//
// Revision 1.13  2003/12/01 13:07:30  fraggle
// Split off system headers to sysheaders.h for common portability stuff
//
// Revision 1.12  2003/10/22 16:13:10  fraggle
// Split off sendatom classes into separate files
//
// Revision 1.11  2003/10/22 16:05:01  fraggle
// Move field reading routines into packet.c
//
// Revision 1.10  2003/10/22 15:32:20  fraggle
// Some documentation
//
// Revision 1.9  2003/10/14 22:12:50  fraggle
// Major internal refactoring:
//  - API for packet functions now uses straight integers rather than
//    unsigned int8/unsigned int16/unsigned int32/etc.
//  - What was sendatom.c is now client_sendq.c.
//  - IrmoSendAtoms are now in an object oriented model. Functions
//    to do with particular "classes" of sendatom are now grouped together
//    in (the new) sendatom.c. This groups things together that seem to
//    logically belong together and cleans up the code a lot.
//
//
//---------------------------------------------------------------------

