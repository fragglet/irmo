#ifndef IRMO_NETLIB_H
#define IRMO_NETLIB_H

#include <glib.h>
#include <netinet/in.h>

// various network util functions

guint sockaddr_in_hash(struct sockaddr_in *addr);
gint sockaddr_in_cmp(struct sockaddr_in *a, struct sockaddr_in *b);

guint sockaddr_in6_hash(struct sockaddr_in6 *addr);
gint sockaddr_in6_cmp(struct sockaddr_in6 *a, struct sockaddr_in6 *b);

guint sockaddr_hash(struct sockaddr *addr);
gint sockaddr_cmp(struct sockaddr *a, struct sockaddr *b);

#endif /* #ifndef IRMO_NETLIB_H */

// $Log: not supported by cvs2svn $
// Revision 1.1  2002/11/26 16:23:28  sdh300
// Split off sockaddr hash functions to a seperate netlib module
//
