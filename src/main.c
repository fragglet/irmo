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

	object = universe_object_new(universe, "myclass");

	if (universe)
		printf("successful!\n");
	else
		printf("failed\n");

	printf("looking for object in universe\n");

	if (universe_get_object_for_id(universe, object->id))
		printf("successful!\n");
	else
		printf("failed!\n");
}

// $Log: not supported by cvs2svn $
// Revision 1.1.1.1  2002/10/19 18:53:24  sdh300
// initial import
//
