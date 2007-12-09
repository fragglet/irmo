lib_LTLIBRARIES=                                           \
        libirmo.la                                         \
        libirmo-interface-parser.la

SUBDIRS=                                                   \
        algo                                               \
        arch                                               \
        base                                               \
        include                                            \
        interface                                          \
        interface-parser                                   \
        net                                                \
        netbase                                            \
        world

libirmo_la_SOURCES= 
libirmo_la_LIBADD=                                         \
        arch/libirmoarch.la                                \
        algo/libirmoalgo.la                                \
        base/libirmobase.la                                \
        interface/libirmointerface.la                      \
        net/libirmonet.la                                  \
        netbase/libirmonetbase.la                          \
        world/libirmoworld.la

libirmo_interface_parser_la_SOURCES=
libirmo_interface_parser_la_LIBADD=                        \
        interface-parser/libirmointerfaceparser.la         \
        libirmo.la                                         \
        @LEXLIB@

