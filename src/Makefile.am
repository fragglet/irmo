lib_LTLIBRARIES=libirmo.la

INCLUDES=$(MODULES_CFLAGS)

SUBDIRS=public

SRC= \
  callback.c    connection.c  netlib.c   packet.h       server.c	\
  callback.h    connection.h  netlib.h   proto_build.c  server.h	\
  client.c      if_spec.h     object.c   proto_parse.c  socket.c	\
  client.h      if_spec.l     object.h   sendatom.c     socket.h	\
  client_run.c  packet.c      sendatom.h universe.c			\
  universe.h    protocol.h    proto_verify.c            method.c	\
  method.h

libirmo_la_SOURCES= $(SRC)
libirmo_la_LIBADD=$(MODULES_LIBS) -lfl

