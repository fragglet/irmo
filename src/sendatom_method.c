// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
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

#include "netlib.h"
#include "object.h"
#include "packet.h"

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

static int irmo_method_atom_verify(IrmoPacket *packet)
{
	IrmoClient *client = packet->client;
	IrmoMethod *method;
	unsigned int i;

	if (!client->server->world)
		return 0;

	// read method index

	if (!irmo_packet_readi8(packet, &i))
		return 0;

	// sanity check method index

	if (i >= client->server->world->spec->nmethods)
		return 0;

	method = client->server->world->spec->methods[i];

	// read arguments

	for (i=0; i<method->narguments; ++i) {
		if (!irmo_packet_verify_value
			(packet, method->arguments[i]->type))
			return 0;
	}

	return 1;
}

static IrmoSendAtom *irmo_method_atom_read(IrmoPacket *packet)
{
	IrmoMethodAtom *atom;
	IrmoMethod *method;
	unsigned int i;

	atom = irmo_new0(IrmoMethodAtom, 1);
	atom->sendatom.klass = &irmo_method_atom;
	
	// read method number
	
	irmo_packet_readi8(packet, &i);
	atom->method.spec = method 
		= packet->client->server->world->spec->methods[i];

	// read arguments
	
	atom->method.args = irmo_new0(IrmoValue, method->narguments);

	for (i=0; i<method->narguments; ++i) {
		irmo_packet_read_value(packet, &atom->method.args[i],
				       method->arguments[i]->type);
	}

	return IRMO_SENDATOM(atom);
}

static void irmo_method_atom_write(IrmoMethodAtom *atom, IrmoPacket *packet)
{
	IrmoMethod *method = atom->method.spec;
	IrmoValue *args = atom->method.args;
	int i;
	
	// send method index
	
	irmo_packet_writei8(packet, method->index);

	// send arguments

	for (i=0; i<method->narguments; ++i)
		irmo_packet_write_value(packet, &args[i],
					method->arguments[i]->type);
}

static void irmo_method_atom_run(IrmoMethodAtom *atom)
{
	IrmoClient *client = atom->sendatom.client;

	atom->method.src = client;
	
	irmo_method_invoke(client->server->world, &atom->method);
}

static void irmo_method_atom_destroy(IrmoMethodAtom *atom)
{
        IrmoMethod *method = atom->method.spec;
        int i;
 
        for (i=0; i<method->narguments; ++i) {
                if (method->arguments[i]->type == IRMO_TYPE_STRING)
                        free(atom->method.args[i].s);
        }
 
        free(atom->method.args);
}

static size_t irmo_method_atom_length(IrmoMethodAtom *atom)
{
	IrmoMethod *method = atom->method.spec;
	int i;
	size_t len;

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
	(IrmoSendAtomWriteFunc) irmo_method_atom_write,
	(IrmoSendAtomRunFunc) irmo_method_atom_run,
	(IrmoSendAtomLengthFunc) irmo_method_atom_length,
	(IrmoSendAtomDestroyFunc) irmo_method_atom_destroy,
};

