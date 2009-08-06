//
// Copyright (C) 2002-2008 Simon Howard
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
#include "base/alloc.h"
#include "base/assert.h"

#include "interface/interface.h"

#include "binding.h"
#include "world.h"

static void update_var_from_member(IrmoObject *obj,
                                   IrmoClassVar *class_var,
                                   IrmoStructMember *member)
{
        IrmoValue new_value;
        IrmoValue *variable;

        variable = &obj->variables[class_var->index];

        // Read the value from the structure, depending on the variable
        // type.  If the value hasn't changed, return.

        switch (class_var->type) {
        case IRMO_TYPE_INT8:
        case IRMO_TYPE_INT16:
        case IRMO_TYPE_INT32:
                new_value.i = irmo_struct_member_get_int(member,
                                                         obj->binding);
                if (new_value.i == variable->i) {
                        return;
                }
                break;

        case IRMO_TYPE_STRING:
                new_value.s = irmo_struct_member_get_string(member,
                                                            obj->binding);

                // NULL string values are not allowed.

                if (new_value.s == NULL) {
                        return;
                }

                // If the string is the same, no update needed.

                if (!strcmp(new_value.s, variable->s)) {
                        return;
                }

                break;

        default:
                irmo_bug();
        }

        // Set the new value:

        irmo_object_internal_set(obj, class_var, &new_value);
}

static void irmo_object_internal_update(IrmoObject *obj)
{
        IrmoClass *objclass = obj->objclass;
        IrmoClassVar *class_var;
        unsigned int i;

        // Check each variable in turn:

        for (i=0; i<objclass->nvariables; ++i) {
                class_var = objclass->variables[i];

                if (class_var->member != NULL) {
                        update_var_from_member(obj, class_var,
                                               class_var->member);
                }
        }
}

// Read the contents of the structure fields in the structure bound to
// the specified object, and update its instance variables as
// neccessary.

void irmo_object_update(IrmoObject *obj)
{
        irmo_return_if_fail(obj != NULL);
        irmo_return_if_fail(obj->binding != NULL);
        irmo_return_if_fail(!obj->world->remote);

        irmo_object_internal_update(obj);
}

// Update all objects in a world.

void irmo_world_update(IrmoWorld *world)
{
        IrmoHashTableIterator iter;
        IrmoObject *obj;

        irmo_return_if_fail(world != NULL);
        irmo_return_if_fail(!world->remote);

        irmo_hash_table_iterate(world->objects, &iter);

        while (irmo_hash_table_iter_has_more(&iter)) {
                obj = irmo_hash_table_iter_next(&iter);

                if (obj->binding != NULL) {
                        irmo_object_internal_update(obj);
                }
        }
}

// Bind an object to a structure.

void irmo_object_bind(IrmoObject *obj, void *cstruct)
{
       IrmoClass *objclass;

       irmo_return_if_fail(obj != NULL);

       // To bind to a structure, the class of this object must have
       // a corresponding C structure.  This may have been determined
       // already (by explicitly specifying the corresponding C structure).
       // Otherwise, we may be able to infer the structure if the name is
       // the same as the class name.

       objclass = obj->objclass;

       if (objclass->structure == NULL) {
                irmo_return_if_fail(irmo_class_get_default_binding(objclass));
       }

       obj->binding = cstruct;
}

// Update the bound variable for the specified object:

void irmo_object_update_binding(IrmoObject *obj, IrmoClassVar *class_var)
{
        IrmoValue *variable;

        // Does this variable have a structure member that it is bound to?

        if (class_var->member == NULL) {
                return;
        }

        variable = &obj->variables[class_var->index];

        // Set the new structure member value:

        switch (class_var->type) {
        case IRMO_TYPE_INT8:
        case IRMO_TYPE_INT16:
        case IRMO_TYPE_INT32:
                irmo_struct_member_set_int(class_var->member,
                                           obj->binding,
                                           variable->i);
                break;

        case IRMO_TYPE_STRING:
                irmo_struct_member_set_string(class_var->member,
                                              obj->binding,
                                              variable->s);
                break;

        default:
                irmo_bug();
        }

}

