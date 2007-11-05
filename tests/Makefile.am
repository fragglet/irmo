
TESTS =                       \
       test-interface         \
       test-iterator          \
       test-world

noinst_PROGRAMS = $(TESTS)

AM_CFLAGS=-I../src/include -I../src
LDADD = $(top_builddir)/src/libirmo.la

test_interface_SOURCES=test-interface.c
test_iterator_SOURCES=test-iterator.c
test_world_SOURCES=test-world.c

