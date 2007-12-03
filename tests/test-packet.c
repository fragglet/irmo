//
// Copyright (C) 2007 Simon Howard
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <irmo.h>
#include <irmo/packet.h>

static unsigned char expected_result[] = {
        0x12, 0x34, 0x56, 0x78, 0x90, 0x12, 0x34,
        'h', 'e', 'l', 'l', 'o', ' ', 'w', 'o', 'r', 'l', 'd', '\0'
};

static struct {
        IrmoValueType value_type;
        IrmoValue value;
} expected_values[] = {
        { IRMO_TYPE_INT8, { 0x12 } },
        { IRMO_TYPE_INT16, { 0x3456 } },
        { IRMO_TYPE_INT32, { 0x78901234 } },
        { IRMO_TYPE_STRING, { 0 } },
};

static void test_create_destroy(void)
{
        IrmoPacket *packet;

        packet = irmo_packet_new();

        irmo_packet_free(packet);
}

static void test_write(void)
{
        IrmoPacket *packet;
        unsigned char *packet_data;

        packet = irmo_packet_new();

        irmo_packet_writei8(packet, 0x12);                // 1 byte
        irmo_packet_writei16(packet, 0x3456);             // 2 bytes
        irmo_packet_writei32(packet, 0x78901234);         // 4 bytes
        irmo_packet_writestring(packet, "hello world");   // 12 bytes

        // 19 bytes in total

        assert(irmo_packet_get_length(packet) == 19);
        assert(irmo_packet_get_position(packet) == 19);
        packet_data = irmo_packet_get_buffer(packet);

        assert(memcmp(packet_data, expected_result, 19) == 0);

        irmo_packet_free(packet);
}

static void test_read(void)
{
        IrmoPacket *packet;
        unsigned int value;
        char *strvalue;

        packet = irmo_packet_new_from(expected_result, 19);

        assert(irmo_packet_get_length(packet) == 19);
        assert(irmo_packet_get_position(packet) == 0);

        // Read back all values

        assert(irmo_packet_readi8(packet, &value) != 0);
        assert(value == 0x12);
        assert(irmo_packet_readi16(packet, &value) != 0);
        assert(value == 0x3456);
        assert(irmo_packet_readi32(packet, &value) != 0);
        assert(value == 0x78901234);
        strvalue = irmo_packet_readstring(packet);
        assert(strvalue != NULL);
        assert(strcmp(strvalue, "hello world") == 0);

        // Check that we cannot read past the end of the packet

        assert(irmo_packet_readi8(packet, &value) == 0);
        assert(irmo_packet_readi16(packet, &value) == 0);
        assert(irmo_packet_readi32(packet, &value) == 0);
        assert(irmo_packet_readstring(packet) == NULL);

        irmo_packet_free(packet);
}

static void test_set_position(void)
{
        IrmoPacket *packet;

        packet = irmo_packet_new_from(expected_result, 19);

        assert(irmo_packet_set_position(packet, 0) != 0);
        assert(irmo_packet_set_position(packet, 5) != 0);
        assert(irmo_packet_set_position(packet, 19) != 0);

        // Check invalid positions

        assert(irmo_packet_set_position(packet, -1) == 0);
        assert(irmo_packet_get_position(packet) <= 19);

        assert(irmo_packet_set_position(packet, 20) == 0);
        assert(irmo_packet_get_position(packet) <= 19);

        irmo_packet_free(packet);
}

static void test_verify(void)
{
        IrmoPacket *packet;

        packet = irmo_packet_new_from(expected_result, 19);

        assert(irmo_packet_verify_value(packet, IRMO_TYPE_INT8) != 0);
        assert(irmo_packet_verify_value(packet, IRMO_TYPE_INT16) != 0);
        assert(irmo_packet_verify_value(packet, IRMO_TYPE_INT32) != 0);
        assert(irmo_packet_verify_value(packet, IRMO_TYPE_STRING) != 0);
        
        assert(irmo_packet_verify_value(packet, IRMO_TYPE_INT8) == 0);
        assert(irmo_packet_verify_value(packet, IRMO_TYPE_INT16) == 0);
        assert(irmo_packet_verify_value(packet, IRMO_TYPE_INT32) == 0);
        assert(irmo_packet_verify_value(packet, IRMO_TYPE_STRING) == 0);
        
        irmo_packet_free(packet);
}

static void test_read_value(void)
{
        IrmoPacket *packet;
        IrmoValue value;
        IrmoValue *expected;
        IrmoValueType expected_type;
        int i;

        packet = irmo_packet_new_from(expected_result, 19);

        assert(irmo_packet_get_length(packet) == 19);
        assert(irmo_packet_get_position(packet) == 0);

        // Read back all values

        for (i=0; i<4; ++i) {
                expected = &expected_values[i].value;
                expected_type = expected_values[i].value_type;

                assert(irmo_packet_read_value(packet, &value, expected_type)
                         != 0);

                if (expected_type == IRMO_TYPE_STRING) {
                        assert(strcmp(expected->s, value.s) == 0);
                } else {
                        assert(expected->i == value.i);
                }
        }

        // Check that we cannot read past the end of the packet

        for (i=0; i<4; ++i) {
                expected_type = expected_values[i].value_type;

                assert(irmo_packet_read_value(packet, &value, expected_type)
                        == 0);
        }

        irmo_packet_free(packet);
}

static void test_write_value(void)
{
        IrmoPacket *packet;
        unsigned char *packet_data;
        int i;

        packet = irmo_packet_new();

        expected_values[3].value.s = "hello world";

        for (i=0; i<4; ++i) {
                irmo_packet_write_value(packet, &expected_values[i].value,
                                        expected_values[i].value_type);
        }

        // 19 bytes in total

        assert(irmo_packet_get_length(packet) == 19);
        assert(irmo_packet_get_position(packet) == 19);
        packet_data = irmo_packet_get_buffer(packet);

        assert(memcmp(packet_data, expected_result, 19) == 0);

        irmo_packet_free(packet);
}

int main(int argc, char *argv[])
{
        test_create_destroy();
        test_write();
        test_read();
        test_write_value();
        test_read_value();
        test_set_position();
        test_verify();

        return 0;
}

