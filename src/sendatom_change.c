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

#include "netlib.h"

#include "object.h"
#include "packet.h"

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

static gboolean irmo_change_atom_verify(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	IrmoClass *objclass;
	int i, n, b;
	gboolean result;
	gboolean *changed;
	
	if (!client->world)
		return FALSE;

	// class

	if (!irmo_packet_readi8(packet, &i))
		return FALSE;

	if (i >= client->world->spec->nclasses)
		return FALSE;

	objclass = client->world->spec->classes[i];
	
	// object id

	if (!irmo_packet_readi16(packet, &i))
		return FALSE;

	// read object changed bitmap
	
	result = TRUE;

	changed = g_new0(gboolean, objclass->nvariables);

	for (i=0, n=0; result && i<(objclass->nvariables+7) / 8; ++i) {
		guint byte;

		if (!irmo_packet_readi8(packet, &byte)) {
			result = FALSE;
			break;
		}

		for (b=0; b<8 && n<objclass->nvariables; ++b, ++n)
			if (byte & (1 << b))
				changed[n] = TRUE;			
	}

	// check new variable values

	if (result) {
		for (i=0; i<objclass->nvariables; ++i) {
			if (!changed[i])
				continue;
			
			if (!irmo_packet_verify_value
				(packet, objclass->variables[i]->type)) {
				result = FALSE;
				break;
			}
		}
	}
	
	g_free(changed);

	return result;
}

static IrmoSendAtom *irmo_change_atom_read(IrmoPacket *packet)
{
	IrmoChangeAtom *atom;
	IrmoClient *client = packet->client;
	IrmoClass *objclass;
	gboolean *changed;
	IrmoValue *newvalues;
	int i, b, n;

	atom = g_new0(IrmoChangeAtom, 1);
	atom->sendatom.klass = &irmo_change_atom;

	// read class
	
	irmo_packet_readi8(packet, &i);

	objclass = client->world->spec->classes[i];
	atom->objclass = objclass;

	// read object id
	
	irmo_packet_readi16(packet, &atom->id);
	
	// read the changed object bitmap

	changed = g_new0(gboolean, objclass->nvariables);
	atom->changed = changed;
	
	for (i=0, n=0; i<(objclass->nvariables+7) / 8; ++i) {
		guint byte;

		// read the bits out of this byte in the bitmap into the
		// changed array
		
		irmo_packet_readi8(packet, &byte);

		for (b=0; b<8 && n<objclass->nvariables; ++b,++n)
			if (byte & (1 << b))
				changed[n] = TRUE;
	}

	// read the new values

	newvalues = g_new0(IrmoValue, objclass->nvariables);
	atom->newvalues = newvalues;

	for (i=0; i<objclass->nvariables; ++i) {
		if (!changed[i])
			continue;

		irmo_packet_read_value(packet, &newvalues[i],
				       objclass->variables[i]->type);
	}

	return IRMO_SENDATOM(atom);
}

static void irmo_change_atom_write(IrmoChangeAtom *atom, IrmoPacket *packet)
{
	IrmoObject *obj = atom->object;
	int bitmap_size;
	int i, j;

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
		guint8 b;

		// build a byte at a time

		b = 0;
		
		for (j=0; j<8 && i*8+j<obj->objclass->nvariables; ++j) {
			if (atom->changed[i*8 + j]) {
				b |= 1 << j;
			}
		}

		irmo_packet_writei8(packet, b);
	}

	// send variables

	for (i=0; i<obj->objclass->nvariables; ++i) {

		// check we are sending this variable

		if (atom->changed[i])
			irmo_packet_write_value
				(packet, &obj->variables[i], 
				 obj->objclass->variables[i]->type);
	}
}

static void irmo_change_atom_destroy(IrmoChangeAtom *atom)
{
        int i;
                                                                                
        if (atom->newvalues) {
                IrmoClass *objclass = atom->objclass;
                                                                                
                for (i=0; i<objclass->nvariables; ++i) {
                        // only changed values are stored
                                                                                
                        if (!atom->changed[i])
                                continue;
                                                                                
                        // free strings
                                                                                
                        if (objclass->variables[i]->type == IRMO_TYPE_STRING)
                                g_free(atom->newvalues[i].s);
                }
                                                                                
                g_free(atom->newvalues);
        }
                                                                                
        g_free(atom->changed);
}

static void irmo_change_atom_run(IrmoChangeAtom *atom)
{
	IrmoClient *client = atom->sendatom.client;
	IrmoObject *obj;
	IrmoClass *objclass;
	IrmoValue *newvalues;
	int i;
	int seq;

	if (atom->executed)
		return;
	
	// sanity checks

	obj = irmo_world_get_object_for_id(client->world,
					   atom->id);

	// if these fail, it is possibly because of dependencies on
	// previous atoms in the stream
	
	if (!obj)
		return;
	if (obj->objclass != atom->objclass)
		return;	       

	objclass = obj->objclass;
	newvalues = atom->newvalues;

	// sequence number of this atom
	
	seq = atom->sendatom.seqnum;
	
	// run through variables and apply changes
	
	for (i=0; i<obj->objclass->nvariables; ++i) {

		// not changed?
		
		if (!atom->changed[i])
			continue;

		// check if a newer change to this atom has been run
		// do not apply older changes
		// dont run the same atom twice (could conceivably
		// happen with resends)
		
		if (seq <= obj->variable_time[i])
			continue;
		
		// apply change

		switch (objclass->variables[i]->type) {
		case IRMO_TYPE_INT8:
		case IRMO_TYPE_INT16:
		case IRMO_TYPE_INT32:
			obj->variables[i].i = newvalues[i].i;
			break;
		case IRMO_TYPE_STRING:
			g_free(obj->variables[i].s);
			obj->variables[i].s = strdup(newvalues[i].s);
			break;
		}

		irmo_object_set_raise(obj, i);

		obj->variable_time[i] = seq;
	}

	// mark as executed
	
	atom->executed = TRUE;
}

static gsize irmo_change_atom_length(IrmoChangeAtom *atom)
{
        IrmoObject *obj = atom->object;
        IrmoClass *spec = obj->objclass;
        gsize len;
        int i;
 
        len = 0;
 
        // object class
 
        len += 1;
         
        // object id
 
        len += 2;
         
        // leading bitmap
         
        len += (spec->nvariables + 7) / 8;
 
        // add up sizes of variables
         
        for (i=0; i<spec->nvariables; ++i) {
                                                                                
                // only variables which have changed
                 
                if (!atom->changed[i])
                        continue;
                 
                switch (spec->variables[i]->type) {
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
	(IrmoSendAtomDestroyFunc) irmo_change_atom_destroy,
};

//---------------------------------------------------------------------
//
// $Log$
// Revision 1.5  2004/04/17 22:19:57  fraggle
// Use glib memory management functions where possible
//
// Revision 1.4  2003/12/01 13:07:30  fraggle
// Split off system headers to sysheaders.h for common portability stuff
//
// Revision 1.3  2003/12/01 12:46:05  fraggle
// Fix under NetBSD
//
// Revision 1.2  2003/11/05 04:05:44  fraggle
// Cast functions rather than casting arguments to functions
//
// Revision 1.1  2003/10/22 16:13:10  fraggle
// Split off sendatom classes into separate files
//
//
//---------------------------------------------------------------------

