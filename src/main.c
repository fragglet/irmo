#include <stdio.h>
#include "if_spec.h"

int main(int argc, char *argv[])
{
	InterfaceSpec *spec = interface_spec_new("test.if");

	printf("hash: %u\n", spec->hash);
}

// $Log: not supported by cvs2svn $
