PKGS=glib-2.0
CFLAGS=-Wall `pkg-config --cflags $(PKGS)`
LDFLAGS = `pkg-config --libs $(PKGS)` -lfl
OBJS=if_spec.o main.o universe.o object.o callback.o socket.o

irmo: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.c : %.lex
	flex -o$@ $<

# $Log: not supported by cvs2svn $
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
