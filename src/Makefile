PKGS=glib-2.0
CFLAGS=`pkg-config --cflags $(PKGS)`
LDFLAGS = `pkg-config --libs $(PKGS)` -lfl
OBJS=if_spec.o main.o

irmo: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

%.c : %.lex
	flex -o$@ $<

# $Log: not supported by cvs2svn $
