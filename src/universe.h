//
// Irmo Universe
//

#ifndef IRMO_UNIVERSE_H
#define IRMO_UNIVERSE_H

/*!
 * \addtogroup universe
 * \{
 */

//! An Irmo Universe.

typedef struct _IrmoUniverse IrmoUniverse;

/*!
 * \brief A numerical object identifier
 *
 * All objects in a \ref IrmoUniverse have a unique number assigned to
 * them. This can be used to refer to objects by their number. You can
 * search for an object by number using the 
 * \ref universe_get_object_for_id function.
 */

typedef guint irmo_objid_t;

#include <glib.h>
#include "callback.h"
#include "if_spec.h"
#include "object.h"

//! \}

// internals:

// imposed by network protocol:

#define MAX_OBJECTS 65536 

struct _IrmoUniverse {
	InterfaceSpec *spec;
	IrmoCallbackData **callbacks;
	GHashTable *objects;
	irmo_objid_t lastid;
	int refcount;
};

/*!
 * \addtogroup universe
 * \{
 */

/*!
 * \brief Create a new Universe
 *
 * Create a new Universe from an Interface Specification. The classes
 * defined in the specification can then be instantiated as objects
 * within the universe.
 *
 * \param spec	The Interface Specification to use.
 * \return	The new universe.
 */

IrmoUniverse *universe_new(InterfaceSpec *spec);

/*!
 * \brief Find an object by its ID
 *
 * All objects within an IrmoUniverse have a unique number assigned to
 * them (see \ref irmo_objid_t). This function searches for an object 
 * by its identifier.
 *
 * \param universe	The Universe to search in
 * \param id		The object number to search for
 * \return		The IrmoObject or NULL if the object is not found.
 */

IrmoObject *universe_get_object_for_id(IrmoUniverse *universe,
				       irmo_objid_t id);

/*!
 * \brief Iterate over objects in a Universe
 *
 * This function allows you to iterate over objects in a Universe.
 * For each object found, a function is called with a pointer to the
 * object. The function can be used to iterate over ALL objects or
 * just ones of a particular class.
 *
 * \param universe	The universe to iterate over
 * \param classname	The name of the class of objects to iterate over.
 * 			If you want to iterate over ALL objects, pass
 * 			NULL for this value.
 * \param func		The callback function to call.
 * \param user_data	Extra data to pass to the functions when they 
 * 			are called.
 */

void universe_foreach_object(IrmoUniverse *universe, gchar *classname,
			     IrmoObjCallback func, gpointer user_data);

/*!
 * \brief	Add a reference to a Universe.
 *
 * Universes implement reference counting. Each time you store a 
 * reference to a universe, call universe_ref to increase the reference
 * count. When you remove a reference, call \ref universe_unref. The
 * count starts at 1. When the count reaches 0, the universe is 
 * destroyed.
 *
 * \param universe	The universe to reference
 */

void universe_ref(IrmoUniverse *universe);

/*!
 * \brief	Remove a reference to a Universe.
 *
 * See \ref universe_ref.
 *
 * \param universe	The universe to unreference.
 */

void universe_unref(IrmoUniverse *universe);

#endif /* #ifndef IRMO_UNIVERSE_H */

// $Log: not supported by cvs2svn $
// Revision 1.8  2002/11/13 14:14:46  sdh300
// object iterator function
//
// Revision 1.7  2002/11/13 13:56:23  sdh300
// add some documentation
//
// Revision 1.6  2002/10/29 16:09:11  sdh300
// initial callback code
//
// Revision 1.5  2002/10/21 14:58:07  sdh300
// split off object code to a seperate module
//
// Revision 1.4  2002/10/21 14:48:54  sdh300
// oops, fix build
//
// Revision 1.3  2002/10/21 14:43:27  sdh300
// variables code
//
// Revision 1.2  2002/10/21 10:55:14  sdh300
// reference checking and object deletion
//
// Revision 1.1  2002/10/21 10:43:31  sdh300
// initial universe code
//
