#include <stdio.h>

#include "if_spec.h"
#include "universe.h"

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

	object = object_new(universe, "myclass");

	if (universe)
		printf("successful!\n");
	else
		printf("failed\n");

	printf("looking for object in universe\n");

	if (universe_get_object_for_id(universe, object->id))
		printf("successful!\n");
	else
		printf("failed!\n");

	printf("destroying object\n");

	object_destroy(object);

	printf("done\n");
	
	universe_unref(universe);
	interface_spec_unref(spec);
}

// $Log: not supported by cvs2svn $
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
