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

#include "arch/sysheaders.h"

#include "interface.h"

//
// IrmoInterface
//

IrmoClass *irmo_interface_get_class(IrmoInterface *iface, 
					 char *class_name)
{
	irmo_return_val_if_fail(iface != NULL, NULL);
	irmo_return_val_if_fail(class_name != NULL, NULL);

	return irmo_hash_table_lookup(iface->class_hash, class_name);
}

IrmoMethod *irmo_interface_get_method(IrmoInterface *iface, 
					   char *method_name)
{
	irmo_return_val_if_fail(iface != NULL, NULL);
	irmo_return_val_if_fail(method_name != NULL, NULL);

	return irmo_hash_table_lookup(iface->method_hash, method_name);
}

void irmo_interface_foreach_class(IrmoInterface *iface, 
				       IrmoClassCallback func, 
				       void *user_data)
{
	int i;

	irmo_return_if_fail(iface != NULL);
	irmo_return_if_fail(func != NULL);

	for (i=0; i<iface->nclasses; ++i) 
		func(iface->classes[i], user_data);
}

void irmo_interface_foreach_method(IrmoInterface *iface, 
					IrmoMethodCallback func, 
					void *user_data)
{
	int i;

	irmo_return_if_fail(iface != NULL);
	irmo_return_if_fail(func != NULL);

	for (i=0; i<iface->nmethods; ++i) 
		func(iface->methods[i], user_data);
}


