#ifndef IRMO_NETLIB_H
#define IRMO_NETLIB_H

#include <glib.h>
#include <netinet/in.h>

// various network util functions

int sockaddr_len(int domain);

guint sockaddr_in_hash(struct sockaddr_in *addr);
gint sockaddr_in_cmp(struct sockaddr_in *a, struct sockaddr_in *b);

#ifdef USE_IPV6
guint sockaddr_in6_hash(struct sockaddr_in6 *addr);
gint sockaddr_in6_cmp(struct sockaddr_in6 *a, struct sockaddr_in6 *b);
#endif

guint sockaddr_hash(struct sockaddr *addr);
gint sockaddr_cmp(struct sockaddr *a, struct sockaddr *b);

struct sockaddr *sockaddr_copy(struct sockaddr *addr);

struct sockaddr *sockaddr_for_name(int domain, gchar *name, int port);

// time functions

// add two timevals (result = a + b)

void irmo_timeval_add(struct timeval *a, struct timeval *b,
		      struct timeval *result);

// subtract timevals (result = a - b)

void irmo_timeval_sub(struct timeval *a, struct timeval *b,
		      struct timeval *result);

// compare timevals

int irmo_timeval_cmp(struct timeval *a, struct timeval *b);

// conversion to milliseconds

int irmo_timeval_to_ms(struct timeval *a);
void irmo_timeval_from_ms(int ms, struct timeval *a);

#endif /* #ifndef IRMO_NETLIB_H */

// $Log: not supported by cvs2svn $
// Revision 1.6  2003/02/06 01:58:16  sdh300
// Abstracted address resolve function
//
// Revision 1.5  2003/02/03 20:54:01  sdh300
// sockaddr copy function
//
// Revision 1.4  2003/02/03 20:44:20  sdh300
// move sockaddr_len into netlib
//
// Revision 1.3  2002/11/26 16:30:44  sdh300
// need ipv6 check around ipv6 sockaddr functions
//
// Revision 1.2  2002/11/26 16:26:15  sdh300
// missing glib.h include
//
// Revision 1.1  2002/11/26 16:23:28  sdh300
// Split off sockaddr hash functions to a seperate netlib module
//
