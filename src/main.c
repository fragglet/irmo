#include <stdio.h>
#include <stdlib.h>

#include "if_spec.h"
#include "universe.h"

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

int main(int argc, char *argv[])
{
	InterfaceSpec *spec;
	IrmoUniverse *universe;
	IrmoObject *object;
	
	spec = interface_spec_new("test.if");
	
	printf("hash: %u\n", spec->hash);

	printf("creating universe\n");

	universe = universe_new(spec);

	if (!universe) {
		printf("failed.\n");
		exit(-1);
	}
	
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
	
	printf("looking for object in universe\n");

	if (universe_get_object_for_id(universe, object->id))
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
	
	printf("destroying object\n");

	object_destroy(object);

	printf("done\n");
	
	universe_unref(universe);
	interface_spec_unref(spec);

	return 0;
}

// $Log: not supported by cvs2svn $
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
