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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "connection.h"
#include "if_spec.h"
#include "universe.h"
#include "socket.h"
#include "server.h"

void my_callback(IrmoObject *object, gchar *variable, gpointer user_data)
{
	char *message = (char *) user_data;
	
	printf("callback invoked!\n");

	printf("message: '%s'\n", message);
}

void my_destroy_callback(IrmoObject *object, gpointer user_data)
{
	printf("destroy callback invoked! message: %s\n", (char *) user_data);
}

void my_callback_2(IrmoObject *object, gchar *variable, gpointer user_data)
{
	puts("this should never be called!");
	exit(-1);
}

void my_destroy_callback_2(IrmoObject *object, gpointer user_data)
{
	puts("this should never be called!");
	exit(-1);
}

void my_iterator_callback(IrmoObject *object, gpointer user_data)
{
	printf("iterator function invoked!\n");
	printf("this is object number: %i\n", irmo_object_get_id(object));
}

void test_universe()
{
	IrmoInterfaceSpec *spec;
	IrmoUniverse *universe;
	IrmoObject *object;
	
	spec = irmo_interface_spec_new("test.if");
	
//	printf("hash: %u\n", spec->hash);

	printf("creating universe\n");

	universe = irmo_universe_new(spec);

	if (!universe) {
		printf("failed.\n");
		exit(-1);
	}

	printf("trying to set callback on creation of new objects\n");

	irmo_universe_watch_new(universe, "my_class",
				my_destroy_callback, "new object callback");
	
	printf("creating object\n");

	object = irmo_object_new(universe, "my_class");

	if (universe)
		printf("successful!\n");
	else
		printf("failed\n");

	printf("trying to set callback on object\n");

	irmo_object_watch(object, NULL,
			  my_callback, "object watch callback");

	printf("trying to set callback on object variable\n");

	irmo_object_watch(object, "my_int",
			  my_callback, "variable watch callback");

	printf("trying to set callback on class\n");

	irmo_universe_watch_class(universe,
				  "my_class", NULL,
				  my_callback, "class watch callback");

	printf("trying to set callback on class variable\n");

	irmo_universe_watch_class(universe,
				  "my_class", "my_int",
				  my_callback, "class variable watch callback");

	printf("trying to set destroy callback on class\n");

	irmo_universe_watch_destroy(universe, "my_class",
				    my_destroy_callback, 
				    "class destroy callback");

	printf("trying to set destroy callback on object\n");

	irmo_object_watch_destroy(object,
				  my_destroy_callback, 
				  "object destroy callback");

	printf("trying to set and remove some callbacks\n");

	printf("\tnew: \n");
	irmo_universe_watch_new(universe, "my_class", my_destroy_callback_2, NULL);
	irmo_universe_unwatch_new(universe, "my_class", my_destroy_callback_2, NULL);

	printf("\tdestroy: \n");
	irmo_universe_watch_destroy(universe, "my_class", my_destroy_callback_2, NULL);
	irmo_universe_unwatch_destroy(universe, "my_class", my_destroy_callback_2, NULL);

	printf("\tvariable watch:\n");
	irmo_universe_watch_class(universe, "my_class", "my_int",
			     my_callback_2, NULL);
	irmo_universe_unwatch_class(universe, "my_class", "my_int",
			       my_callback_2, NULL);
			     
	printf("\tobject destroy: \n");
	irmo_object_watch_destroy(object, my_destroy_callback_2, NULL);
	irmo_object_unwatch_destroy(object, my_destroy_callback_2, NULL);

	printf("\tobject variable watch:\n");
	irmo_object_watch(object, "my_int", my_callback_2, NULL);
	irmo_object_unwatch(object, "my_int", my_callback_2, NULL);

	printf("\ttesting invalid unwatch calls:\n");

	irmo_universe_unwatch_new(universe, "my_class",
				  my_destroy_callback_2, NULL);
	irmo_universe_unwatch_destroy(universe, "my_class",
				      my_destroy_callback_2, NULL);
	irmo_universe_unwatch_class(universe, "my_class", "my_int",
				    my_callback_2, NULL);
	irmo_object_unwatch_destroy(object, my_destroy_callback_2, NULL);
	irmo_object_unwatch(object, "my_int", my_callback_2, NULL);
	
	printf("looking for object in universe\n");

	if (irmo_universe_get_object_for_id(universe, 
					    irmo_object_get_id(object)))
		printf("successful!\n");
	else
		printf("failed!\n");

	printf("trying to set variable value\n");
	
	irmo_object_set_int(object, "my_int", 1234);

	printf("trying to set variable value (invalid)\n");
	
	irmo_object_set_int(object, "my_string", 1234);

	printf("trying to set string variable value\n");
	
	irmo_object_set_string(object, "my_string", "yoyodyne");

	printf("trying to get variable value\n");

	printf("value: %i\n", irmo_object_get_int(object, "my_int"));

	printf("trying to get variable string value\n");

	printf("value: %s\n", irmo_object_get_string(object, "my_string"));

	printf("trying to get variable value (invalid)\n");

	irmo_object_get_string(object, "my_int");
	
	printf("trying to iterate over objects of class my_class\n");

	irmo_universe_foreach_object(universe, "my_class",
				     my_iterator_callback, NULL);

	printf("trying to iterate over all objects\n");

	irmo_universe_foreach_object(universe, NULL,
				     my_iterator_callback, NULL);
	
	printf("destroying object\n");

	irmo_object_destroy(object);

	printf("done\n");

	irmo_universe_unref(universe);
	irmo_interface_spec_unref(spec);
}

#define TEST_PORT 7000

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("usage: %s [client|server]\n", argv[0]);
		exit(0);
	}

	if (!strcmp(argv[1], "client")) {
		IrmoInterfaceSpec *spec;
		IrmoConnection *conn;

		spec = irmo_interface_spec_new("test.if");
		
		conn = irmo_connect(AF_INET, "localhost", TEST_PORT,
				    spec, NULL);

		if (conn) {
			IrmoUniverse *universe;
			
			printf("connected to server!\n");

			universe = irmo_connection_get_universe(conn);
			
			irmo_universe_watch_new(universe, "my_class",
						my_destroy_callback,
						"new object callback");
			irmo_universe_watch_destroy(universe, "my_class",
						    my_destroy_callback,
						    "object destroyed");
			for (;;)
				irmo_connection_run(conn);
		} else {
			printf("connection failed.\n");
		}
		
	} else if (!strcmp(argv[1], "server")) {
		IrmoInterfaceSpec *spec;
		IrmoUniverse *universe;
		IrmoSocket *socket;
		IrmoServer *server;
		IrmoObject *obj;
		time_t t;
		
		socket = irmo_socket_new(AF_INET, TEST_PORT);

		if (!socket) {
			printf("couldnt bind to test port\n");
			return;
		}

		spec = irmo_interface_spec_new("test.if");

		universe = irmo_universe_new(spec);

		server = irmo_server_new(socket, NULL, universe, NULL);

		t = time(NULL);

		obj = irmo_object_new(universe, "my_class");

		while (1) {
			irmo_socket_run(socket);
			usleep(100);

			// every 4 seconds create a new object
			
			if (time(NULL) > t + 4) {
				// destroy the previously created object
				
				irmo_object_destroy(obj);
				
				printf("create new object\n");
				obj = irmo_object_new(universe, "my_class");
				irmo_object_set_int(obj, "my_int", 3);
				irmo_object_set_string(obj, "my_string", "hi");
				
				t = time(NULL);
			}
		}
	}
}

// $Log$
// Revision 1.1  2003/06/09 21:33:24  fraggle
// Initial revision
//
// Revision 1.25  2003/06/09 21:06:51  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.24  2003/03/07 12:17:16  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.23  2003/03/06 20:43:45  sdh300
// Delay before deleting objects in server harness
// This is so that the object change atoms go through and are tested
//
// Revision 1.22  2003/03/06 19:35:24  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec
// Improve client test harness to add callbacks for new object creation
//
// Revision 1.21  2003/03/05 17:38:05  sdh300
// Test harness for network code
//
// Revision 1.20  2003/02/23 00:05:02  sdh300
// Fix compile after header changes
//
// Revision 1.19  2003/02/11 19:19:02  sdh300
// Test code for connection
//
// Revision 1.18  2002/11/26 16:38:17  sdh300
// some tests for the socket and server constructor functions
//
// Revision 1.17  2002/11/13 14:14:46  sdh300
// object iterator function
//
// Revision 1.16  2002/11/12 23:13:05  sdh300
// add some clarity
//
// Revision 1.15  2002/11/12 23:04:31  sdh300
// callback removal, and code cleanup/generalisation
//
// Revision 1.14  2002/11/05 16:28:10  sdh300
// new object callbacks
//
// Revision 1.13  2002/11/05 15:55:13  sdh300
// object destroy callbacks
//
// Revision 1.12  2002/11/05 15:04:12  sdh300
// more warnings!
//
// Revision 1.11  2002/11/05 15:01:06  sdh300
// change callback function names
// initial destroy callback variables
//
// Revision 1.10  2002/10/29 16:28:50  sdh300
// functioning callbacks
//
// Revision 1.9  2002/10/29 14:53:34  sdh300
// sensible test variable names for clarity
//
// Revision 1.8  2002/10/29 14:48:16  sdh300
// variable value retrieval
//
// Revision 1.7  2002/10/21 15:39:35  sdh300
// setting string values
//
// Revision 1.6  2002/10/21 15:32:35  sdh300
// variable value setting
//
// Revision 1.5  2002/10/21 15:09:01  sdh300
// object destruction
//
// Revision 1.4  2002/10/21 14:58:08  sdh300
// split off object code to a seperate module
//
// Revision 1.3  2002/10/21 10:55:14  sdh300
// reference checking and object deletion
//
// Revision 1.2  2002/10/21 10:43:32  sdh300
// initial universe code
//
// Revision 1.1.1.1  2002/10/19 18:53:24  sdh300
// initial import
//
