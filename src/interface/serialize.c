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
//
// Load and save IrmoInterface structures to binary blobs.
//

#include "arch/sysheaders.h"
#include "base/error.h"

#include <irmo/iterator.h>
#include <irmo/packet.h>

#include "interface.h"

#define HEADER_SIGNATURE "Irmo Interface Blob"
#define BLOB_VERSION 1

//#define DEBUG 1

#ifdef DEBUG
#define DEBUGMSG(args)  printf args
#else
#define DEBUGMSG(args)
#endif

//
// Blob header
//

// Write the header into the packet

static void write_header(IrmoPacket *packet)
{
        DEBUGMSG(("Write header\n"));

        irmo_packet_writestring(packet, HEADER_SIGNATURE);
        irmo_packet_writei32(packet, BLOB_VERSION);
}

// Read and verify the header
// Returns 0 for failure.

static int read_header(IrmoPacket *packet)
{
        char *sig;
        unsigned int ver;

        DEBUGMSG(("Read header\n"));

        sig = irmo_packet_readstring(packet);

        if (sig == NULL || strcmp(sig, HEADER_SIGNATURE) != 0) {
                irmo_error_report("irmo_interface_load",
                                  "Invalid header signature");
                return 0;
        }

        if (!irmo_packet_readi32(packet, &ver)) {
                irmo_error_report("irmo_interface_load",
                                  "Invalid header signature");
                return 0;
        }

        if (ver != BLOB_VERSION) {
                irmo_error_report("irmo_interface_load",
                                  "Wrong interface version");
                return 0;
        }

        DEBUGMSG(("Header read successfully\n"));

        return 1;
}

//
// IrmoClassVar
//

static void write_class_var(IrmoClassVar *var, IrmoPacket *packet)
{
        DEBUGMSG(("\t\tWrite class var: '%s'\n", irmo_class_var_get_name(var)));

        irmo_packet_writestring(packet, irmo_class_var_get_name(var));
        irmo_packet_writei8(packet, irmo_class_var_get_type(var));
}

static int read_class_var(IrmoPacket *packet, IrmoClass *klass)
{
        char *name;
        unsigned int type;

        DEBUGMSG(("\t\tRead class var\n"));

        name = irmo_packet_readstring(packet);

        if (name == NULL) {
                return 0;
        }

        if (!irmo_packet_readi8(packet, &type)) {
                return 0;
        }

        if (irmo_class_new_variable(klass, name, type) == NULL) {
                return 0;
        }

        DEBUGMSG(("\t\tClass var '%s' read successfully\n", name));

        return 1;
}

//
// IrmoClass
//

static void write_class(IrmoClass *klass, IrmoPacket *packet)
{
        IrmoIterator *iter;
        IrmoClass *parent_class;
        IrmoClassVar *var;
        int unique_vars;

        DEBUGMSG(("\tWrite class: '%s'\n", irmo_class_get_name(klass)));

        parent_class = irmo_class_parent_class(klass);

        // name

        irmo_packet_writestring(packet, irmo_class_get_name(klass));

        // parent class
  
        irmo_packet_writei8(packet, parent_class != NULL);

        if (parent_class != NULL) {

                irmo_packet_writestring(packet, 
                                        irmo_class_get_name(parent_class));
        }

        // How many unique variables?

        unique_vars = irmo_class_num_variables(klass);

        if (parent_class != NULL) {
                unique_vars -= irmo_class_num_variables(parent_class);
        }

        irmo_packet_writei8(packet, unique_vars);

        // Write all variables
        
        iter = irmo_class_iterate_variables(klass, 0);

        while (irmo_iterator_has_more(iter)) {
                var = irmo_iterator_next(iter);

                write_class_var(var, packet);
        }

        irmo_iterator_free(iter);
}

static int read_class(IrmoPacket *packet, IrmoInterface *iface)
{
        IrmoClass *klass;
        IrmoClass *parent_class;
        char *name;
        unsigned int has_parent;
        char *parent_name;
        unsigned int num_vars;
        unsigned int i;

        DEBUGMSG(("\tRead class\n"));

        // Class name

        name = irmo_packet_readstring(packet);

        if (name == NULL) {
                return 0;
        }

        // Parent class?

        if (!irmo_packet_readi8(packet, &has_parent)) {
                return 0;
        }

        if (has_parent) {
                parent_name = irmo_packet_readstring(packet);

                if (parent_name == NULL) {
                        return 0;
                }

                parent_class = irmo_interface_get_class(iface, parent_name);

                if (parent_class == NULL) {
                        return 0;
                }
        } else {
                parent_class = NULL;
        }

        // Create the class

        klass = irmo_interface_new_class(iface, name, parent_class);

        if (klass == NULL) {
                return 0;
        }

        // Read variables

        if (!irmo_packet_readi8(packet, &num_vars)) {
                return 0;
        }

        for (i=0; i<num_vars; ++i) {
                if (!read_class_var(packet, klass)) {
                        printf("failed when loading class var\n");
                        return 0;
                }
        }

        // Read all variables successfully

        DEBUGMSG(("\tClass '%s' read successfully\n", name));

        return 1;
}

// 
// IrmoMethodArg
//

static void write_method_arg(IrmoMethodArg *arg, IrmoPacket *packet)
{
        DEBUGMSG(("\t\tWrite method arg: '%s'\n", irmo_method_arg_get_name(arg)));

        irmo_packet_writestring(packet, irmo_method_arg_get_name(arg));
        irmo_packet_writei8(packet, irmo_method_arg_get_type(arg));
}

static int read_method_arg(IrmoPacket *packet, IrmoMethod *method)
{
        char *name;
        unsigned int type;

        DEBUGMSG(("\t\tRead method arg\n"));

        name = irmo_packet_readstring(packet);

        if (name == NULL) {
                return 0;
        }

        if (!irmo_packet_readi8(packet, &type)) {
                return 0;
        }

        if (irmo_method_new_argument(method, name, type) == NULL) {
                return 0;
        }

        DEBUGMSG(("\t\tMethod arg '%s' read successfully\n", name));

        return 1;
}

//
// IrmoMethod
//

static void write_method(IrmoMethod *method, IrmoPacket *packet)
{
        IrmoIterator *iter;
        IrmoMethodArg *arg;

        DEBUGMSG(("\tWrite method: '%s'\n", irmo_method_get_name(method)));

        irmo_packet_writestring(packet, irmo_method_get_name(method));

        // Write all arguments
     
        irmo_packet_writei8(packet, irmo_method_num_arguments(method));

        iter = irmo_method_iterate_arguments(method);

        while (irmo_iterator_has_more(iter)) {
                arg = irmo_iterator_next(iter);

                write_method_arg(arg, packet);
        }
}

static int read_method(IrmoPacket *packet, IrmoInterface *iface)
{
        IrmoMethod *method;
        char *name;
        unsigned int num_args;
        unsigned int i;

        DEBUGMSG(("\tRead method\n"));

        name = irmo_packet_readstring(packet);

        if (name == NULL) {
                return 0;
        }

        method = irmo_interface_new_method(iface, name);

        if (method == NULL) {
                return 0;
        }

        if (!irmo_packet_readi8(packet, &num_args)) {
                return 0;
        }

        for (i=0; i<num_args; ++i) {
                if (!read_method_arg(packet, method)) {
                        return 0;
                }
        }

        DEBUGMSG(("\tMethod '%s' read successfully\n", name));

        return 1;
}

//
// Include hash checksum of the interface at the end of the file.
//

static void write_checksum(IrmoInterface *iface, IrmoPacket *packet)
{
        DEBUGMSG(("Write checksum\n"));

        irmo_packet_writei32(packet, irmo_interface_hash(iface));
}

static int read_checksum(IrmoInterface *iface, IrmoPacket *packet)
{
        unsigned int checksum;
        unsigned int iface_checksum;

        DEBUGMSG(("Read checksum\n"));

        if (!irmo_packet_readi32(packet, &checksum)) {
                DEBUGMSG(("\tFailed to read checksum\n"));
                return 0;
        }

        iface_checksum = irmo_interface_hash(iface);

        if (checksum != iface_checksum) {
                DEBUGMSG(("\tChecksum failed: %i != %i\n", iface_checksum, 
                          checksum));
                return 0;
        }

        DEBUGMSG(("Checksum ok\n"));

        return 1;
}

// 
// Top-level
//

static void write_classes(IrmoInterface *iface, IrmoPacket *packet)
{
        IrmoIterator *iter;
        IrmoClass *klass;

        DEBUGMSG(("Write classes: %i\n", irmo_interface_num_classes(iface)));
        irmo_packet_writei8(packet, irmo_interface_num_classes(iface));
        
        iter = irmo_interface_iterate_classes(iface);

        while (irmo_iterator_has_more(iter)) {
                klass = irmo_iterator_next(iter);

                write_class(klass, packet);
        }

        irmo_iterator_free(iter);
}

static int read_classes(IrmoInterface *iface, IrmoPacket *packet)
{
        unsigned int num_classes;
        unsigned int i;

        DEBUGMSG(("Read classes\n"));

        if (!irmo_packet_readi8(packet, &num_classes)) {
                return 0;
        }

        for (i=0; i<num_classes; ++i) {
                if (!read_class(packet, iface)) {
                        return 0;
                }
        }

        DEBUGMSG(("Classes read successfully\n"));

        return 1;
}

static void write_methods(IrmoInterface *iface, IrmoPacket *packet)
{
        IrmoIterator *iter;
        IrmoMethod *method;

        DEBUGMSG(("Write methods: %i\n", irmo_interface_num_methods(iface)));

        irmo_packet_writei8(packet, irmo_interface_num_methods(iface));

        iter = irmo_interface_iterate_methods(iface);

        while (irmo_iterator_has_more(iter)) {
                method = irmo_iterator_next(iter);

                write_method(method, packet);
        }
}

static int read_methods(IrmoInterface *iface, IrmoPacket *packet)
{
        unsigned int num_methods;
        unsigned int i;

        DEBUGMSG(("Read methods\n"));

        if (!irmo_packet_readi8(packet, &num_methods)) {
                return 0;
        }

        for (i=0; i<num_methods; ++i) {
                if (!read_method(packet, iface)) {
                        return 0;
                }
        }

        DEBUGMSG(("Methods read successfully\n"));

        return 1;
}

void irmo_interface_dump(IrmoInterface *iface, 
                         void **data, 
                         unsigned int *data_len)
{
        IrmoPacket *packet;

        irmo_return_if_fail(iface != NULL);
        irmo_return_if_fail(data != NULL);
        irmo_return_if_fail(data_len != NULL);

        packet = irmo_packet_new();
        write_header(packet);
        write_classes(iface, packet);
        write_methods(iface, packet);
        write_checksum(iface, packet);

        *data_len = irmo_packet_get_length(packet);
        *data = malloc(*data_len);
        memcpy(*data, irmo_packet_get_buffer(packet), *data_len);

        irmo_packet_free(packet);
}

IrmoInterface *irmo_interface_load(void *data, 
                                   unsigned int data_len)
{
        IrmoPacket *packet;
        IrmoInterface *iface;
        int success;

        irmo_return_val_if_fail(data != NULL, NULL);
        irmo_return_val_if_fail(data_len > 0, NULL);

        // Create a new interface, and a packet to read the blob with.

        packet = irmo_packet_new_from(data, data_len);
        iface = irmo_interface_new();

        // Parse the blob

        success = read_header(packet);

        if (success) {
               success = read_classes(iface, packet)
                      && read_methods(iface, packet);

                if (!success) {
                        irmo_error_report("irmo_interface_load",
                                          "Error when parsing buffer");
                }
        }

        if (success) {
                success = read_checksum(iface, packet);

                if (!success) {
                        irmo_error_report("irmo_interface_load",
                                          "Checksum failed");
                }
        }

        // Finished parsing; free the packet

        irmo_packet_free(packet);

        if (success) {
                return iface;
        } else {
                irmo_interface_unref(iface);
                return NULL;
        }
}

