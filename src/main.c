#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "public/connection.h"
#include "public/if_spec.h"
#include "public/universe.h"
#include "public/socket.h"
#include "public/server.h"

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
	printf("this is object number: %i\n", object_get_id(object));
}

void test_universe()
{
	InterfaceSpec *spec;
	IrmoUniverse *universe;
	IrmoObject *object;
	
	spec = interface_spec_new("test.if");
	
//	printf("hash: %u\n", spec->hash);

	printf("creating universe\n");

	universe = universe_new(spec);

	if (!universe) {
		printf("failed.\n");
		exit(-1);
	}

	printf("trying to set callback on creation of new objects\n");

	universe_watch_new(universe, "my_class",
			   my_destroy_callback, "new object callback");
	
	printf("creating object\n");

	object = object_new(universe, "my_class");

	if (universe)
		printf("successful!\n");
	else
		printf("failed\n");

	printf("trying to set callback on object\n");

	object_watch(object, NULL,
		     my_callback, "object watch callback");

	printf("trying to set callback on object variable\n");

	object_watch(object, "my_int",
		     my_callback, "variable watch callback");

	printf("trying to set callback on class\n");

	universe_watch_class(universe,
			     "my_class", NULL,
			     my_callback, "class watch callback");

	printf("trying to set callback on class variable\n");

	universe_watch_class(universe,
			     "my_class", "my_int",
			     my_callback, "class variable watch callback");

	printf("trying to set destroy callback on class\n");

	universe_watch_destroy(universe, "my_class",
			       my_destroy_callback, "class destroy callback");

	printf("trying to set destroy callback on object\n");

	object_watch_destroy(object,
			     my_destroy_callback, "object destroy callback");

	printf("trying to set and remove some callbacks\n");

	printf("\tnew: \n");
	universe_watch_new(universe, "my_class", my_destroy_callback_2, NULL);
	universe_unwatch_new(universe, "my_class", my_destroy_callback_2, NULL);

	printf("\tdestroy: \n");
	universe_watch_destroy(universe, "my_class", my_destroy_callback_2, NULL);
	universe_unwatch_destroy(universe, "my_class", my_destroy_callback_2, NULL);

	printf("\tvariable watch:\n");
	universe_watch_class(universe, "my_class", "my_int",
			     my_callback_2, NULL);
	universe_unwatch_class(universe, "my_class", "my_int",
			       my_callback_2, NULL);
			     
	printf("\tobject destroy: \n");
	object_watch_destroy(object, my_destroy_callback_2, NULL);
	object_unwatch_destroy(object, my_destroy_callback_2, NULL);

	printf("\tobject variable watch:\n");
	object_watch(object, "my_int", my_callback_2, NULL);
	object_unwatch(object, "my_int", my_callback_2, NULL);

	printf("\ttesting invalid unwatch calls:\n");

	universe_unwatch_new(universe, "my_class",
			     my_destroy_callback_2, NULL);
	universe_unwatch_destroy(universe, "my_class",
				 my_destroy_callback_2, NULL);
	universe_unwatch_class(universe, "my_class", "my_int",
			       my_callback_2, NULL);
	object_unwatch_destroy(object, my_destroy_callback_2, NULL);
	object_unwatch(object, "my_int", my_callback_2, NULL);
	
	printf("looking for object in universe\n");

	if (universe_get_object_for_id(universe, object_get_id(object)))
		printf("successful!\n");
	else
		printf("failed!\n");

	printf("trying to set variable value\n");
	
	object_set_int(object, "my_int", 1234);

	printf("trying to set variable value (invalid)\n");
	
	object_set_int(object, "my_string", 1234);

	printf("trying to set string variable value\n");
	
	object_set_string(object, "my_string", "yoyodyne");

	printf("trying to get variable value\n");

	printf("value: %i\n", object_get_int(object, "my_int"));

	printf("trying to get variable string value\n");

	printf("value: %s\n", object_get_string(object, "my_string"));

	printf("trying to get variable value (invalid)\n");

	object_get_string(object, "my_int");
	
	printf("trying to iterate over objects of class my_class\n");

	universe_foreach_object(universe, "my_class",
				my_iterator_callback, NULL);

	printf("trying to iterate over all objects\n");

	universe_foreach_object(universe, NULL,
				my_iterator_callback, NULL);
	
	printf("destroying object\n");

	object_destroy(object);

	printf("done\n");

	universe_unref(universe);
	interface_spec_unref(spec);
}

#define TEST_PORT 7000

int main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("usage: %s [client|server]\n", argv[0]);
		exit(0);
	}

	if (!strcmp(argv[1], "client")) {
		InterfaceSpec *spec;
		IrmoConnection *conn;

		spec = interface_spec_new("test.if");
		
		conn = irmo_connect(AF_INET, "localhost", TEST_PORT,
				    spec, NULL);

		if (conn)
			printf("connected to server!\n");
		else
			printf("connection failed.\n");

		for (;;)
			connection_run(conn);
		
	} else if (!strcmp(argv[1], "server")) {
		InterfaceSpec *spec;
		IrmoUniverse *universe;
		IrmoSocket *socket;
		IrmoServer *server;
		time_t t;
		
		socket = socket_new(AF_INET, TEST_PORT);

		if (!socket) {
			printf("couldnt bind to test port\n");
			return;
		}

		spec = interface_spec_new("test.if");

		universe = universe_new(spec);

		server = server_new(socket, NULL, universe, NULL);

		t = time(NULL);
		
		while (1) {
			socket_run(socket);
			usleep(100);

			if (time(NULL) > t + 4) {
				IrmoObject *obj;
				printf("create new object\n");
				obj = object_new(universe, "my_class");
				object_set_int(obj, "my_int", 3);
				object_set_string(obj, "my_string", "hi");
				t = time(NULL);
				object_destroy(obj);
			}
		}
	}
}

// $Log: not supported by cvs2svn $
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
