PKGS=glib-2.0
CFLAGS=-Wall `pkg-config --cflags $(PKGS)` -DUSE_IPV6
LDFLAGS = `pkg-config --libs $(PKGS)` -lfl
OBJS=if_spec.o main.o universe.o object.o callback.o socket.o server.o netlib.o packet.o client.o connection.o

irmo: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.c : %.lex
	flex -o$@ $<

# $Log: not supported by cvs2svn $
# Revision 1.11  2003/02/03 21:12:49  sdh300
# add client.c
#
# Revision 1.10  2002/12/02 22:24:44  sdh300
# Add packet.c to makefile build
#
# Revision 1.9  2002/11/26 16:37:23  sdh300
# add USE_IPV6 to makefile CFLAGS
#
# Revision 1.8  2002/11/26 16:23:27  sdh300
# Split off sockaddr hash functions to a seperate netlib module
#
# Revision 1.7  2002/11/26 15:46:41  sdh300
# Fix compile and possible namespace conflicts with the "socket" function
#
# Revision 1.6  2002/11/26 15:23:48  sdh300
# Initial network code
#
# Revision 1.5  2002/11/05 15:04:11  sdh300
# more warnings!
#
# Revision 1.4  2002/10/29 16:09:09  sdh300
# initial callback code
#
# Revision 1.3  2002/10/21 14:58:08  sdh300
# split off object code to a seperate module
#
# Revision 1.2  2002/10/21 10:55:13  sdh300
# reference checking and object deletion
#
# Revision 1.1.1.1  2002/10/19 18:53:23  sdh300
# initial import
#
