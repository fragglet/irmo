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

#include <glib.h>
#include <string.h>

#include "object.h"
#include "packet.h"

G_INLINE_FUNC gboolean verify_field(IrmoPacket *packet,
				    IrmoValueType type)
{
	guint i;
	
	switch (type) {
	case IRMO_TYPE_INT8:
		return irmo_packet_readi8(packet, &i);
	case IRMO_TYPE_INT16:
		return irmo_packet_readi16(packet, &i);
	case IRMO_TYPE_INT32:
		return irmo_packet_readi32(packet, &i);
	case IRMO_TYPE_STRING:
		return irmo_packet_readstring(packet) != NULL;
	}
}

G_INLINE_FUNC void read_field(IrmoPacket *packet,
			      IrmoValue *value,
			      IrmoValueType type)
{
	switch (type) {
	case IRMO_TYPE_INT8:
		irmo_packet_readi8(packet, &value->i);
		break;
	case IRMO_TYPE_INT16:
		irmo_packet_readi16(packet, &value->i);
		break;
	case IRMO_TYPE_INT32:
		irmo_packet_readi32(packet, &value->i);
		break;
	case IRMO_TYPE_STRING:
		value->s = strdup(irmo_packet_readstring(packet));
		break;
	}
}

G_INLINE_FUNC void write_field(IrmoPacket *packet, IrmoValue *value, 
			       IrmoValueType type)
{
	switch (type) {
	case IRMO_TYPE_INT8:
		irmo_packet_writei8(packet, value->i);
		break;
	case IRMO_TYPE_INT16:
		irmo_packet_writei16(packet, value->i);
		break;
	case IRMO_TYPE_INT32:
		irmo_packet_writei32(packet, value->i);
		break;
	case IRMO_TYPE_STRING:
		irmo_packet_writestring(packet, value->s);
		break;
	}
}

//
// Null atom.
//
// does nothing.
//
// format:
// 
//  (empty)
//

static gboolean irmo_null_atom_verify(IrmoPacket *packet)
{
	return TRUE;
}

static IrmoSendAtom *irmo_null_atom_read(IrmoPacket *packet)
{
	IrmoSendAtom *atom;
	
	atom = g_new0(IrmoSendAtom, 1);
	atom->klass = &irmo_null_atom;

	return atom;
}

static void irmo_null_atom_write(IrmoSendAtom *atom, IrmoPacket *packet)
{
	return;
}

static gsize irmo_null_atom_length(IrmoSendAtom *atom)
{
	return 0;
}

static void irmo_null_atom_run(IrmoSendAtom *atom)
{
	// does nothing
}

IrmoSendAtomClass irmo_null_atom = {
	ATOM_NULL,
	irmo_null_atom_verify,
	irmo_null_atom_read,
	irmo_null_atom_write,
	irmo_null_atom_run,
	irmo_null_atom_length,
	NULL,
};

//
// IrmoNewObjectAtom
//
// Announce the creation of a new object.
//
// format:
// 
// <int16>	object id
// <int8>	object class number
// 

static gboolean irmo_newobject_atom_verify(IrmoPacket *packet)
{
	guint i;

	if (!packet->client->world)
		return FALSE;

	// object id

	if (!irmo_packet_readi16(packet, &i))
		return FALSE;

	// class of new object

	if (!irmo_packet_readi8(packet, &i))
		return FALSE;

	// check valid class
	
	if (i >= packet->client->world->spec->nclasses)
		return FALSE;

	return TRUE;
}

static IrmoSendAtom *irmo_newobject_atom_read(IrmoPacket *packet)
{
	IrmoNewObjectAtom *atom;

	atom = g_new0(IrmoNewObjectAtom, 1);
	atom->sendatom.klass = &irmo_newobject_atom;

	// object id of new object
		
	irmo_packet_readi16(packet, &atom->id);

	// class of new object
		
	irmo_packet_readi8(packet, &atom->classnum);

	return IRMO_SENDATOM(atom);
}

static void irmo_newobject_atom_write(IrmoSendAtom *_atom, 
				      IrmoPacket *packet)
{
	IrmoNewObjectAtom *atom = (IrmoNewObjectAtom *) _atom;

	irmo_packet_writei16(packet, atom->id);
	irmo_packet_writei8(packet, atom->classnum);
}

static void irmo_newobject_atom_run(IrmoSendAtom *_atom)
{
	IrmoNewObjectAtom *atom = (IrmoNewObjectAtom *) _atom;
	IrmoClient *client = atom->sendatom.client;
	IrmoInterfaceSpec *spec = client->world->spec;
	IrmoClass *objclass = spec->classes[atom->classnum];
	
	// sanity check

	if (irmo_world_get_object_for_id(client->world,
					 atom->id)) {
		irmo_error_report("client_run_new",
				  "new object id of %i but an object with "
				  "that id already exists!",
				  atom->id);
		return;
	}

	// create new object
							  
	irmo_object_internal_new(client->world, objclass, atom->id);
}

static gsize irmo_newobject_atom_length(IrmoSendAtom *atom)
{
	// object id, class number

	return 2 + 1;
}


IrmoSendAtomClass irmo_newobject_atom = {
	ATOM_NEW,
	irmo_newobject_atom_verify,
	irmo_newobject_atom_read,
	irmo_newobject_atom_write,
	irmo_newobject_atom_run,
	irmo_newobject_atom_length,
	NULL,
};

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
			
			if (!verify_field(packet,
					  objclass->variables[i]->type)) {
				result = FALSE;
				break;
			}
		}
	}
	
	free(changed);

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

		read_field(packet, &newvalues[i],
			   objclass->variables[i]->type);
	}

	return IRMO_SENDATOM(atom);
}

static void irmo_change_atom_write(IrmoSendAtom *_atom, IrmoPacket *packet)
{
	IrmoChangeAtom *atom = (IrmoChangeAtom *) _atom;
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
			write_field(packet, &obj->variables[i], 
				    obj->objclass->variables[i]->type);
	}
}

static void irmo_change_atom_destroy(IrmoSendAtom *_atom)
{
	IrmoChangeAtom *atom = (IrmoChangeAtom *) _atom;
        int i;
                                                                                
        if (atom->newvalues) {
                IrmoClass *objclass = atom->objclass;
                                                                                
                for (i=0; i<objclass->nvariables; ++i) {
                        // only changed values are stored
                                                                                
                        if (!atom->changed[i])
                                continue;
                                                                                
                        // free strings
                                                                                
                        if (objclass->variables[i]->type == IRMO_TYPE_STRING)
                                free(atom->newvalues[i].s);
                }
                                                                                
                free(atom->newvalues);
        }
                                                                                
        free(atom->changed);
}

static void irmo_change_atom_run(IrmoSendAtom *_atom)
{
	IrmoChangeAtom *atom = (IrmoChangeAtom *) _atom;
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
			free(obj->variables[i].s);
			obj->variables[i].s = strdup(newvalues[i].s);
			break;
		}

		irmo_object_set_raise(obj, i);

		obj->variable_time[i] = seq;
	}

	// mark as executed
	
	atom->executed = TRUE;
}

static gsize irmo_change_atom_length(IrmoSendAtom *_atom)
{
	IrmoChangeAtom *atom = (IrmoChangeAtom *) _atom;
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
	irmo_change_atom_write,
	irmo_change_atom_run,
	irmo_change_atom_length,
	irmo_change_atom_destroy,
};

//
// IrmoMethodAtom
//
// Invoke a method on the remote machine.
//
// format:
//
// <int8>	method number
// <field>[]	a field for each method argument
//		data depends on the argument type
//

static gboolean irmo_method_atom_verify(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	IrmoMethod *method;
	int i;

	if (!client->server->world)
		return FALSE;

	// read method index

	if (!irmo_packet_readi8(packet, &i))
		return FALSE;

	// sanity check method index

	if (i >= client->server->world->spec->nmethods)
		return FALSE;

	method = client->server->world->spec->methods[i];

	// read arguments

	for (i=0; i<method->narguments; ++i) {
		if (!verify_field(packet, method->arguments[i]->type))
			return FALSE;
	}

	return TRUE;
}

static IrmoSendAtom *irmo_method_atom_read(IrmoPacket *packet)
{
	IrmoMethodAtom *atom;
	IrmoMethod *method;
	int i;

	atom = g_new0(IrmoMethodAtom, 1);
	atom->sendatom.klass = &irmo_method_atom;
	
	// read method number
	
	irmo_packet_readi8(packet, &i);
	atom->method.spec = method 
		= packet->client->server->world->spec->methods[i];

	// read arguments
	
	atom->method.args = g_new0(IrmoValue, method->narguments);

	for (i=0; i<method->narguments; ++i) {
		read_field(packet, &atom->method.args[i],
			   method->arguments[i]->type);
	}

	return IRMO_SENDATOM(atom);
}

static void irmo_method_atom_write(IrmoSendAtom *_atom, IrmoPacket *packet)
{
	IrmoMethodAtom *atom = (IrmoMethodAtom *) _atom;
	IrmoMethod *method = atom->method.spec;
	IrmoValue *args = atom->method.args;
	int i;
	
	// send method index
	
	irmo_packet_writei8(packet, method->index);

	// send arguments

	for (i=0; i<method->narguments; ++i)
		write_field(packet, &args[i],
			    method->arguments[i]->type);
}

static void irmo_method_atom_run(IrmoSendAtom *_atom)
{
	IrmoMethodAtom *atom = (IrmoMethodAtom *) _atom;
	IrmoClient *client = atom->sendatom.client;

	atom->method.src = client;
	
	irmo_method_invoke(client->server->world, &atom->method);
}

static void irmo_method_atom_destroy(IrmoSendAtom *_atom)
{
	IrmoMethodAtom *atom = (IrmoMethodAtom *) _atom;
        IrmoMethod *method = atom->method.spec;
        int i;
 
        for (i=0; i<method->narguments; ++i) {
                if (method->arguments[i]->type == IRMO_TYPE_STRING)
                        free(atom->method.args[i].s);
        }
 
        free(atom->method.args);
}

static gsize irmo_method_atom_length(IrmoSendAtom *_atom)
{
	IrmoMethodAtom *atom = (IrmoMethodAtom *) _atom;
	IrmoMethod *method = atom->method.spec;
	int i;
	gsize len;

        // find length of atom,
 
        len = 0;
 
        // method number
         
        len += 1;
 
        // find length of arguments
        // copy strings while we are here
         
        for (i=0; i<method->narguments; ++i) {
                switch (method->arguments[i]->type) {
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
                        len += strlen(atom->method.args[i].s) + 1;
                        break;
                }
        }

	return len;
}

IrmoSendAtomClass irmo_method_atom = {
	ATOM_METHOD,
	irmo_method_atom_verify,
	irmo_method_atom_read,
	irmo_method_atom_write,
	irmo_method_atom_run,
	irmo_method_atom_length,
	irmo_method_atom_destroy,
};

//
// IrmoDestroyAtom
//
// Announce the destruction of a remote object.
//
// format:
//
// <int16>	object id of object to destroy
//

static gboolean irmo_destroy_atom_verify(IrmoPacket *packet)
{
	guint i;

	if (!packet->client->world)
		return FALSE;

	// object id

	if (!irmo_packet_readi16(packet, &i))
		return FALSE;
		
	return TRUE;
}

static IrmoSendAtom *irmo_destroy_atom_read(IrmoPacket *packet)
{
	IrmoDestroyAtom *atom;
	guint i;

	atom = g_new0(IrmoDestroyAtom, 1);
	atom->sendatom.klass = &irmo_destroy_atom;

	// object id to destroy

	irmo_packet_readi16(packet, &atom->id);

	return IRMO_SENDATOM(atom);
}

static void irmo_destroy_atom_write(IrmoSendAtom *_atom, IrmoPacket *packet)
{
	IrmoDestroyAtom *atom = (IrmoDestroyAtom *) _atom;

	irmo_packet_writei16(packet, atom->id);
}

static void irmo_destroy_atom_run(IrmoSendAtom *_atom)
{
	IrmoDestroyAtom *atom = (IrmoDestroyAtom *) _atom;
	IrmoClient *client = atom->sendatom.client;
	IrmoObject *obj;

	// sanity check

	obj = irmo_world_get_object_for_id(client->world, atom->id);

	if (!obj) {
		irmo_error_report("client_run_destroy",
				  "destroy object %i, but object does not exist",
				  atom->id);
		return;
	}

	// destroy object. remove from world and call notify functions
	
	irmo_object_internal_destroy(obj, TRUE, TRUE);
}

static gsize irmo_destroy_atom_length(IrmoSendAtom *atom)
{
	return 2;
}

IrmoSendAtomClass irmo_destroy_atom = {
	ATOM_DESTROY,
	irmo_destroy_atom_verify,
	irmo_destroy_atom_read,
	irmo_destroy_atom_write,
	irmo_destroy_atom_run,
	irmo_destroy_atom_length,
	NULL,
};

//
// IrmoSendWindowAtom
//
// Send a message to set the maximum send window size, to throttle
// bandwidth
//
// format:
//
// <int16>	new send window size in bytes
//

static gboolean irmo_sendwindow_atom_verify(IrmoPacket *packet)
{
	guint i;

	// set maximum sendwindow size

	return irmo_packet_readi16(packet, &i);
}

static IrmoSendAtom *irmo_sendwindow_atom_read(IrmoPacket *packet)
{
	IrmoSendWindowAtom *atom;
	guint i;

	atom = g_new0(IrmoSendWindowAtom, 1);
	atom->sendatom.klass = &irmo_sendwindow_atom;
	
	// read window advertisement

	irmo_packet_readi16(packet, &atom->max);

	return IRMO_SENDATOM(atom);
}

static void irmo_sendwindow_atom_write(IrmoSendAtom *_atom, 
				       IrmoPacket *packet)
{
	IrmoSendWindowAtom *atom = (IrmoSendWindowAtom *) _atom;

	irmo_packet_writei16(packet, atom->max);
}

static void irmo_sendwindow_atom_run(IrmoSendAtom *_atom)
{
	IrmoSendWindowAtom *atom = (IrmoSendWindowAtom *) _atom;
	IrmoClient *client = atom->sendatom.client;

	client->remote_sendwindow_max = atom->max;
}

static gsize irmo_sendwindow_atom_length(IrmoSendAtom *atom)
{
	return 2;
}

IrmoSendAtomClass irmo_sendwindow_atom = {
	ATOM_SENDWINDOW,
	irmo_sendwindow_atom_verify,
	irmo_sendwindow_atom_read,
	irmo_sendwindow_atom_write,
	irmo_sendwindow_atom_run,
	irmo_sendwindow_atom_length,
	NULL,
};

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
// Revision 1.10  2003/10/22 15:32:20  fraggle
// Some documentation
//
// Revision 1.9  2003/10/14 22:12:50  fraggle
// Major internal refactoring:
//  - API for packet functions now uses straight integers rather than
//    guint8/guint16/guint32/etc.
//  - What was sendatom.c is now client_sendq.c.
//  - IrmoSendAtoms are now in an object oriented model. Functions
//    to do with particular "classes" of sendatom are now grouped together
//    in (the new) sendatom.c. This groups things together that seem to
//    logically belong together and cleans up the code a lot.
//
//
//---------------------------------------------------------------------

