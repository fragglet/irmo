// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
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

