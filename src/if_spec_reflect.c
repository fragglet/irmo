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

#include "if_spec.h"

IrmoVarType irmo_interface_spec_get_var_type(IrmoInterfaceSpec *spec,
					     gchar *classname,
					     gchar *variable)
{
	ClassSpec *class_spec;
	ClassVarSpec *var_spec;

	g_return_val_if_fail(spec != NULL, IRMO_TYPE_UNKNOWN);
	g_return_val_if_fail(classname != NULL, IRMO_TYPE_UNKNOWN);
	g_return_val_if_fail(variable != NULL, IRMO_TYPE_UNKNOWN);

	class_spec = g_hash_table_lookup(spec->class_hash,
					 classname);

	if (class_spec == NULL) 
		return IRMO_TYPE_UNKNOWN;

	var_spec = g_hash_table_lookup(class_spec->variable_hash,
				       variable);

	if (var_spec == NULL)
		return IRMO_TYPE_UNKNOWN;

	return var_spec->type;
}

// $Log$
// Revision 1.1  2003/08/21 14:32:29  fraggle
// Initial reflection API
//
//

