
TESTS =                        \
        test-interface         \
        test-iterator          \
        test-binding           \
        test-packet            \
        test-world             \
        test-callbacks         \
        test-ipv4              \
        test-ipv6

check_PROGRAMS = $(TESTS)
check_LIBRARIES = libtestcommon.a

libtestcommon_a_SOURCES =                                  \
        loopback-test-module.c                             \
        net-module-tests.c       net-module-tests.h

AM_CFLAGS=-I../src/include -I../src -Wall
LDADD = $(top_builddir)/src/libirmo.la libtestcommon.a

