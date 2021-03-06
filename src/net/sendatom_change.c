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

#include <irmo/packet.h>

#include "world/object.h"

#include "sendatom.h"

//
// IrmoChangeAtom
//
// Announce a change to one or more variables in an object.
//
// format:
// 
// <int8>	object class number
// <int16> 	object id
// <int8>[]	bitmap; one bit for each class variable,
// 		each bit is 1 if an update to that variable
//		follows. low bits to high bits. enough
// 		int8s are included for the bitfield.
// <field>[]	a field for each changed variable.
//		data depends on the variable type.
//

static int irmo_change_atom_verify(IrmoPacket *packet, IrmoClient *client)
{
	IrmoClass *objclass;
	unsigned int i;
        unsigned int n, b;
	int result;
	int *changed;
	
	if (client->world == NULL) {
		return 0;
        }

	// class

	if (!irmo_packet_readi8(packet, &i)) {
		return 0;
        }

	if (i >= client->world->iface->nclasses) {
		return 0;
        }

	objclass = client->world->iface->classes[i];
	
	// object id

	if (!irmo_packet_readi16(packet, &i)) {
		return 0;
        }

	// read object changed bitmap
	
	result = 1;

	changed = irmo_new0(int, objclass->nvariables);

	for (i=0, n=0; result && i<(objclass->nvariables+7) / 8; ++i) {
		unsigned int byte;

		if (!irmo_packet_readi8(packet, &byte)) {
			result = 0;
			break;
		}

		for (b=0; b<8 && n<objclass->nvariables; ++b, ++n) {
			if (byte & (1 << b)) {
				changed[n] = 1;
                        }
                }
	}

	// check new variable values

	if (result) {
		for (i=0; i<objclass->nvariables; ++i) {
			if (!changed[i]) {
				continue;
                        }
			
			if (!irmo_packet_verify_value
				(packet, objclass->variables[i]->type)) {
				result = 0;
				break;
			}
		}
	}
	
	free(changed);

	return result;
}

static IrmoSendAtom *irmo_change_atom_read(IrmoPacket *packet, 
                                           IrmoClient *client)
{
	IrmoChangeAtom *atom;
	IrmoClass *objclass;
	int *changed;
	IrmoValue *newvalues;
	unsigned int i;
        unsigned int b, n;

	atom = irmo_new0(IrmoChangeAtom, 1);
	atom->sendatom.klass = &irmo_change_atom;

	// read class
	
	irmo_packet_readi8(packet, &i);

	objclass = client->world->iface->classes[i];
	atom->objclass = objclass;

	// read object id
	
	irmo_packet_readi16(packet, &atom->id);
	
	// read the changed object bitmap

	changed = irmo_new0(int, objclass->nvariables);
	atom->changed = changed;
	
	for (i=0, n=0; i<(objclass->nvariables+7) / 8; ++i) {
		unsigned int byte;

		// read the bits out of this byte in the bitmap into the
		// changed array
		
		irmo_packet_readi8(packet, &byte);

		for (b=0; b<8 && n<objclass->nvariables; ++b,++n) {
			if (byte & (1 << b)) {
				changed[n] = 1;
                        }
                }
	}

	// read the new values

	newvalues = irmo_new0(IrmoValue, objclass->nvariables);
	atom->newvalues = newvalues;

	for (i=0; i<objclass->nvariables; ++i) {
		if (!changed[i]) {
			continue;
                }

		irmo_packet_read_value(packet, &newvalues[i],
				       objclass->variables[i]->type);
	}

	return IRMO_SENDATOM(atom);
}

static void irmo_change_atom_write(IrmoChangeAtom *atom, IrmoPacket *packet)
{
	IrmoObject *obj = atom->object;
	unsigned int bitmap_size;
	unsigned int i, j;

	// include the object class number
	// this is neccesary otherwise the packet can be ambiguous to
	// decode (if we receive a change atom for an object which has
	// not yet been received, for example)

	irmo_packet_writei8(packet, obj->objclass->index);
	
	// send object id
	
	irmo_packet_writei16(packet, obj->id);

	// build and send bitmap

	bitmap_size = (obj->objclass->nvariables + 7) / 8;

	for (i=0; i<bitmap_size; ++i) {
		uint8_t b;

		// build a byte at a time

		b = 0;

		for (j=0; j<8 && i*8+j<obj->objclass->nvariables; ++j) {
			if (atom->changed[i*8 + j]) {
				b |= (uint8_t) (1 << j);
			}
		}

		irmo_packet_writei8(packet, b);
	}

	// send variables

	for (i=0; i<obj->objclass->nvariables; ++i) {

		// check we are sending this variable

		if (atom->changed[i]) {
			irmo_packet_write_value
				(packet, &obj->variables[i], 
				 obj->objclass->variables[i]->type);
                }
	}
}

static void irmo_change_atom_destroy(IrmoChangeAtom *atom)
{
        unsigned int i;

        if (atom->newvalues) {
                IrmoClass *objclass = atom->objclass;

                for (i=0; i<objclass->nvariables; ++i) {
                        // only changed values are stored

                        if (!atom->changed[i]) {
                                continue;
                        }

                        // free strings

                        if (objclass->variables[i]->type == IRMO_TYPE_STRING) {
                                free(atom->newvalues[i].s);
                        }
                }

                free(atom->newvalues);
        }

        free(atom->changed);
}

static void irmo_change_atom_run(IrmoChangeAtom *atom)
{
        IrmoClient *client = atom->sendatom.client;
	IrmoObject *obj;
	IrmoClass *objclass;
	IrmoValue *newvalues;
	unsigned int i;
	unsigned int seq;

	if (atom->executed) {
		return;
        }
	
	// sanity checks

	obj = irmo_world_get_object_for_id(client->world,
					   atom->id);

	// if these fail, it is possibly because of dependencies on
	// previous atoms in the stream
	
	if (obj == NULL) {
		return;
        }

	if (obj->objclass != atom->objclass) {
		return;
        }

	objclass = obj->objclass;
	newvalues = atom->newvalues;

	// sequence number of this atom
	
	seq = atom->sendatom.seqnum;
	
	// run through variables and apply changes
	
	for (i=0; i<obj->objclass->nvariables; ++i) {

		// Not changed?
		
		if (!atom->changed[i]) {
			continue;
                }

		// Check if a newer change to this atom has been run
		// do not apply older changes
		// dont run the same atom twice (could conceivably
		// happen with resends)
		
		if (seq <= obj->variable_time[i]) {
			continue;
                }
		
                // Set the new value

                irmo_object_internal_set(obj, objclass->variables[i],
                                         &newvalues[i], 1);

		obj->variable_time[i] = seq;
	}

	// mark as executed
	
	atom->executed = 1;
}

static size_t irmo_change_atom_length(IrmoChangeAtom *atom)
{
        IrmoObject *obj = atom->object;
        IrmoClass *klass = obj->objclass;
        size_t len;
        unsigned int i;
 
        len = 0;
 
        // object class
 
        len += 1;
         
        // object id
 
        len += 2;
         
        // leading bitmap
         
        len += (klass->nvariables + 7) / 8;
 
        // add up sizes of variables
         
        for (i=0; i<klass->nvariables; ++i) {

                // only variables which have changed
                 
                if (!atom->changed[i]) {
                        continue;
                }
                 
                switch (klass->variables[i]->type) {
                case IRMO_TYPE_INT8:
                        len += 1;
                        break;
                case IRMO_TYPE_INT16:
                        len += 2;
                        break;
                case IRMO_TYPE_INT32:
                        len += 4;
                        break;
                case IRMO_TYPE_STRING:
                        len += strlen(obj->variables[i].s) + 1;
                        break;
                default:
                        irmo_bug();
                }
        }
 
        return len;
}

IrmoSendAtomClass irmo_change_atom = {
	ATOM_CHANGE,
	irmo_change_atom_verify,
	irmo_change_atom_read,
	(IrmoSendAtomWriteFunc) irmo_change_atom_write,
	(IrmoSendAtomRunFunc) irmo_change_atom_run,
	(IrmoSendAtomLengthFunc) irmo_change_atom_length,
        NULL,
	(IrmoSendAtomDestroyFunc) irmo_change_atom_destroy,
};

