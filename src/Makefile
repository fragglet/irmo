PKGS=glib-2.0
CFLAGS=`pkg-config --cflags $(PKGS)`
LDFLAGS = `pkg-config --libs $(PKGS)` -lfl
OBJS=if_spec.o main.o universe.o object.o

irmo: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.c : %.lex
	flex -o$@ $<

# $Log: not supported by cvs2svn $
# Revision 1.2  2002/10/21 10:55:13  sdh300
# reference checking and object deletion
#
# Revision 1.1.1.1  2002/10/19 18:53:23  sdh300
# initial import
#
