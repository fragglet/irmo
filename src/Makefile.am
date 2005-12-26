lib_LTLIBRARIES=libirmo.la

SUBDIRS=arch common public

SRC= \
  callback.c            callback.h          \
  connection.c          connection.h        \
  client.c              client.h            \
  client_run.c                              \
  client_sendq.c                            \
  error.c               error.h             \
  if_spec.l             if_spec.h           \
  if_spec_reflect.c                         \
  method.c              method.h            \
  netlib.c              netlib.h            \
  object.c              object.h            \
  packet.c              packet.h            \
  protocol.h                                \
  proto_build.c                             \
  proto_parse.c                             \
  proto_verify.c                            \
  sendatom.c            sendatom.h          \
  sendatom_change.c                         \
  sendatom_destroy.c                        \
  sendatom_method.c                         \
  sendatom_newobject.c                      \
  sendatom_null.c                           \
  sendatom_sendwindow.c                     \
  socket.c              socket.h            \
  server.c              server.h            \
  sysheaders.h                              \
  world.c               world.h

libirmo_la_SOURCES= $(SRC)
libirmo_la_LIBADD=\
  arch/libirmoarch.a                        \
  common/libirmocommon.a                    \
  @LEXLIB@

