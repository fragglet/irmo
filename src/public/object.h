//
// Irmo Objects
//

#ifndef IRMO_OBJECT_H
#define IRMO_OBJECT_H

/*!
 * \addtogroup object
 *
 * \{
 */

//! An Irmo Object

typedef struct _IrmoObject IrmoObject;

//! Callback functions for operations on variables

typedef void (*IrmoVarCallback) (IrmoObject *object, gchar *variable,
				 gpointer user_data);

//! Callback functions for operations on objects

typedef void (*IrmoObjCallback) (IrmoObject *object, gpointer user_data);

#include "universe.h"

/*!
 * \brief create a new object of a particular class
 *
 * \param universe Universe to create the object within
 * \param typename The name of the class of object to create
 * \return 	   The created object or NULL for failure
 */

IrmoObject *object_new(IrmoUniverse *universe, char *typename);

/*!
 * \brief Destroy an object
 *
 * \param object  The object to destroy
 */

void object_destroy(IrmoObject *object);

/*!
 * \brief Get numerical object identifier
 *
 * Each object within a Universe has a unique number assigned to it. 
 * This function allows you to retrieve the number assigned to a particular
 * object.
 *
 * \param object  The object to query
 * \return 	  Object ID
 */

irmo_objid_t object_get_id(IrmoObject *object);

/*!
 * \brief Get the class of an object
 *
 * All objects have a class, which is one of the classes defined in the
 * IrmoInterfaceSpec for the Universe the object exists in. This retrieves
 * the name of the class for a particular object.
 *
 * \param object The object to query.
 * \return 	 A string with the classname
 */

gchar *object_get_class(IrmoObject *object);

/*!
 * \brief Set the value of an object variable (int type)
 *
 * Set the value of a member variable. This is for use on variables of
 * int type. To set string values, use \ref object_set_string
 *
 * \param object   The object to change
 * \param variable The name of the variable to change
 * \param value    The new value for the variable
 */

void object_set_int(IrmoObject *object, gchar *variable, int value);

/*!
 * \brief Set the value of an object variable (string type)
 *
 * Set the value of a member variable. This is for use on variables of
 * string type. To set integer values, use \ref object_set_int
 *
 * \param object   The object to change
 * \param variable The name of the variable to change
 * \param value    The new value for the variable
 */

void object_set_string(IrmoObject *object, gchar *variable, gchar *value);

/*!
 * \brief Get the value of an object variable(int type)
 *
 * Retrieve the value of an object's member variable. This function is
 * for variables of integer type. To get string values, use 
 * \ref object_get_string
 *
 * \param object   The object to query
 * \param variable The name of the member variable
 * \return         The value of the member variable
 */

gint object_get_int(IrmoObject *object, gchar *variable);

/*!
 * \brief Get the value of an object variable(string type)
 *
 * Retrieve the value of an object's member variable. This function is
 * for variables of string type. To get integer values, use 
 * \ref object_get_int
 *
 * The returned string should not be modified; to set the value of a 
 * member variable use \ref object_set_string
 *
 * \param object   The object to query
 * \param variable The name of the member variable
 * \return         The value of the member variable
 */

gchar *object_get_string(IrmoObject *object, gchar *variable);

//! \}

#endif /* #ifndef IRMO_OBJECT_H */

// $Log: not supported by cvs2svn $
// Revision 1.1  2003/02/23 00:00:06  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.11  2002/11/17 20:37:15  sdh300
// initial doxygen documentation
//
// Revision 1.10  2002/11/13 15:12:34  sdh300
// object_get_id to get identifier
//
// Revision 1.9  2002/11/13 14:14:46  sdh300
// object iterator function
//
// Revision 1.8  2002/11/13 13:57:42  sdh300
// object_get_class to get the class of an object
//
// Revision 1.7  2002/11/13 13:56:24  sdh300
// add some documentation
//
// Revision 1.6  2002/10/29 16:09:11  sdh300
// initial callback code
//
// Revision 1.5  2002/10/29 14:48:16  sdh300
// variable value retrieval
//
// Revision 1.4  2002/10/21 15:39:36  sdh300
// setting string values
//
// Revision 1.3  2002/10/21 15:10:17  sdh300
// missing cvs tags
//
