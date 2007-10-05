
bindir = $(prefix)/bin
bin_PROGRAMS =                                             \
       irmo-interface-compiler

irmo_interface_compiler_SOURCES=                           \
       interface-compiler.c
irmo_interface_compiler_CFLAGS=                            \
	-I../src/include
irmo_interface_compiler_LDADD=                             \
	$(top_builddir)/src/libirmo.la                     \
	$(top_builddir)/src/libirmo-interface-parser.la

