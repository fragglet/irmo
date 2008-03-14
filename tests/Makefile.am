
TESTS =                        \
        test-interface         \
        test-iterator          \
        test-binding           \
        test-packet            \
        test-world

noinst_PROGRAMS = $(TESTS)

AM_CFLAGS=-I../src/include -I../src -Wall
LDADD = $(top_builddir)/src/libirmo.la

test_interface_SOURCES=test-interface.c
test_iterator_SOURCES=test-iterator.c
test_world_SOURCES=test-world.c
test_packet_SOURCES=test-packet.c
test_binding_SOURCES=test-binding.c

