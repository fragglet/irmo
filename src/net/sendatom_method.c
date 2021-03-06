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

static int irmo_method_atom_verify(IrmoPacket *packet, IrmoClient *client)
{
	IrmoMethod *method;
        IrmoServer *server;
	unsigned int i;

        server = client->server;

	if (server->world == NULL) {
		return 0;
        }

	// read method index

	if (!irmo_packet_readi8(packet, &i)) {
		return 0;
        }

	// sanity check method index

	if (i >= client->server->world->iface->nmethods) {
		return 0;
        }

	method = client->server->world->iface->methods[i];

	// read arguments

	for (i=0; i<method->narguments; ++i) {
		if (!irmo_packet_verify_value
			(packet, method->arguments[i]->type)) {
			return 0;
                }
	}

	return 1;
}

static IrmoSendAtom *irmo_method_atom_read(IrmoPacket *packet, 
                                           IrmoClient *client)
{
	IrmoMethodAtom *atom;
	IrmoMethod *method;
	unsigned int i;

	atom = irmo_new0(IrmoMethodAtom, 1);
	atom->sendatom.klass = &irmo_method_atom;
	
	// read method number
	
	irmo_packet_readi8(packet, &i);
        method = client->server->world->iface->methods[i];
	atom->method_data.method = method;

	// read arguments
	
	atom->method_data.args = irmo_new0(IrmoValue, method->narguments);

	for (i=0; i<method->narguments; ++i) {
		irmo_packet_read_value(packet, &atom->method_data.args[i],
				       method->arguments[i]->type);
	}

	return IRMO_SENDATOM(atom);
}

static void irmo_method_atom_write(IrmoMethodAtom *atom, IrmoPacket *packet)
{
	IrmoMethod *method = atom->method_data.method;
	IrmoValue *args = atom->method_data.args;
	unsigned int i;
	
	// send method index
	
	irmo_packet_writei8(packet, method->index);

	// send arguments

	for (i=0; i<method->narguments; ++i) {
		irmo_packet_write_value(packet, &args[i],
					method->arguments[i]->type);
        }
}

static void irmo_method_atom_run(IrmoMethodAtom *atom)
{
        IrmoClient *client = atom->sendatom.client;

	atom->method_data.src = client;
	
	irmo_method_internal_call(client->server->world, &atom->method_data);
}

static void irmo_method_atom_destroy(IrmoMethodAtom *atom)
{
        IrmoMethod *method = atom->method_data.method;
        unsigned int i;
 
        for (i=0; i<method->narguments; ++i) {
                if (method->arguments[i]->type == IRMO_TYPE_STRING) {
                        free(atom->method_data.args[i].s);
                }
        }
 
        free(atom->method_data.args);
}

static size_t irmo_method_atom_length(IrmoMethodAtom *atom)
{
	IrmoMethod *method = atom->method_data.method;
	unsigned int i;
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
                        len += strlen(atom->method_data.args[i].s) + 1;
                        break;
                default:
                        irmo_bug();
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
        NULL,
	(IrmoSendAtomDestroyFunc) irmo_method_atom_destroy,
};

