#ifndef IRMO_METHOD_H
#define IRMO_METHOD_H

/*!
 * \addtogroup method
 * \{
 */

/*!
 * \brief Method Callback Data
 *
 * When a callback function attached to a method is invoked, it is passed
 * an object of this type containing data relevant to the method. For
 * example, the object can be used to retrieve the values of method
 * arguments, and the client which invoked the method.
 */

typedef struct _IrmoMethodData IrmoMethodData;

#include "client.h"

/*!
 * \brief Callback function for methods
 *
 * When a method invocation is received from a client, a callback function
 * of this type is invoked.
 *
 * \param data      A \ref IrmoMethodData object holding data about the
 *                  invoked method.
 * \param user_data A pointer to user defined piece of memory specified in
 *                  the \ref irmo_universe_method_watch call used to set
 *                  the callback.
 */

typedef void (*IrmoMethodCallback)(IrmoMethodData *data, gpointer user_data);

/*!
 * \brief Set a callback function to be invoked when a method is called
 *
 * \param universe    The universe to set the watch.
 * \param method_name The name of the method to watch
 * \param method      A callback function to call when the method is invoked.
 * \param user_data   User data to be passed to the callback function
 *
 * \sa irmo_universe_method_unwatch
 */

void irmo_universe_method_watch(IrmoUniverse *universe, gchar *method_name,
				IrmoMethodCallback method, gpointer user_data);

/*!
 * \brief Unset a method watch
 *
 * Unset a watch set with \ref irmo_universe_method_watch. All parameters
 * must be identical to those used to set the watch.
 */

void irmo_universe_method_unwatch(IrmoUniverse *universe, gchar *method_name,
				  IrmoMethodCallback method,
				  gpointer user_data);

/*!
 * \brief Retrieve a method argument
 *
 * Get the value of a string argument to a method from the method callback
 * function.
 *
 * \param data    A \ref IrmoMethodData object containing information about
 *                the method call
 * \param argname The name of the method argument
 * \return        The value of the method argument (constant string)
 */

gchar *irmo_method_arg_string(IrmoMethodData *data, gchar *argname);

/*!
 * \brief Retrieve a method argument
 *
 * Get the value of an integer argument to a method from the method callback
 * function.
 *
 * \param data    A \ref IrmoMethodData object containing information about
 *                the method call
 * \param argname The name of the method argument
 * \return        The value of the method argument
 */

guint irmo_method_arg_int(IrmoMethodData *data, gchar *argname);

/*!
 * \brief Find the client which invoked a method
 *
 * This function returns a reference to the \ref IrmoClient object
 * of the client which invoked the method. If it was invoked by a
 * function in the local program, it returns NULL.
 *
 */

IrmoClient *irmo_method_get_source(IrmoMethodData *data);

/*!
 * \brief Call a method
 *
 * Invoke a method. If the universe is a local copy of a universe served from
 * a remote machine the call will be forwarded to the remote machine.
 *
 * Following the function arguments are the method arguments, in the order
 * specified in the interface specification.
 * 
 * \param universe  The universe object on which to invoke the method
 * \param method    The name of the method
 */

void irmo_universe_method_call(IrmoUniverse *universe, gchar *method, ...);

//! \}

#endif /* #ifndef IRMO_METHOD_H */

// $Log: not supported by cvs2svn $
// Revision 1.1  2003/03/15 02:21:19  sdh300
// Initial method code
//
