//
// Copyright (C) 2002-3 Simon Howard
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//

//
// Type definitions
//
// All typedefs are defined in this file.
//

#ifndef IRMO_TYPES_H
#define IRMO_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------
//
// IrmoIterator
//
//---------------------------------------------------------------------

/*!
 * @addtogroup iterator
 * \{
 */

/*!
 * An iterator for iterating over values in a collection.
 */

typedef struct _IrmoIterator IrmoIterator;

//! \}

//---------------------------------------------------------------------
//
// IrmoInterface
//
//---------------------------------------------------------------------

/*!
 * @addtogroup iface
 * \{
 */

/*!
 * An interface is a collection of classes and methods that form
 * an @ref IrmoWorld.
 */

typedef struct _IrmoInterface IrmoInterface;

/*!
 * An object representing a class in an interface.
 */

typedef struct _IrmoClass IrmoClass;

/*!
 * An object representing a variable in a class.
 */

typedef struct _IrmoClassVar IrmoClassVar;

/*!
 * An object representing a method in an interface.
 */

typedef struct _IrmoMethod IrmoMethod;

/*!
 * An object representing an argument to a method.
 */ 

typedef struct _IrmoMethodArg IrmoMethodArg;

/*!
 * Network address of a remote machine.
 */

typedef struct _IrmoNetAddress IrmoNetAddress;

/*!
 * Variable types.
 */

typedef enum {
	IRMO_TYPE_UNKNOWN,
	IRMO_TYPE_INT8,
	IRMO_TYPE_INT16,
	IRMO_TYPE_INT32,
	IRMO_TYPE_STRING,
	IRMO_NUM_TYPES,
} IrmoValueType;

typedef void (*IrmoClassCallback)(IrmoClass *klass, void *user_data);
typedef void (*IrmoClassVarCallback)(IrmoClassVar *var, void *user_data);
typedef void (*IrmoMethodCallback)(IrmoMethod *method, void *user_data);
typedef void (*IrmoMethodArgCallback)(IrmoMethodArg *arg, void *user_data);

//! \}

//---------------------------------------------------------------------
//
// IrmoNetModule
//
//---------------------------------------------------------------------

/*!
 * @addtogroup net_module
 * \{
 */

/*!
 * A network module, defining callback functions to use for network
 * communications.
 */

typedef struct _IrmoNetModule IrmoNetModule;

//! \}

//---------------------------------------------------------------------
//
// IrmoWorld
//
//---------------------------------------------------------------------

/*!
 * @addtogroup world
 * \{
 */

//! An Irmo World.

typedef struct _IrmoWorld IrmoWorld;

/*!
 * A numerical object identifier.
 *
 * All objects in a @ref IrmoWorld have a unique number assigned to
 * them. This can be used to refer to objects by their number. An
 * object can be obtained from its identifier through the 
 * @ref irmo_world_get_object_for_id function.
 */

typedef unsigned int IrmoObjectID;

//! \}

//---------------------------------------------------------------------
//
// Method Callbacks
//
//---------------------------------------------------------------------

/*!
 * @addtogroup method
 * \{
 */

/*!
 * Method callback data.
 *
 * When a callback function attached to a method is invoked, it is passed
 * an object of this type containing data relevant to the method. For
 * example, the object can be used to retrieve the values of method
 * arguments, and the client which invoked the method.
 */

typedef struct _IrmoMethodData IrmoMethodData;

/*!
 * Callback function for methods.
 *
 * When a method invocation is received from a client, a callback function
 * of this type is invoked.
 *
 * @param data      A @ref IrmoMethodData object holding data about the
 *                  invoked method.
 * @param user_data A pointer to user defined piece of memory specified in
 *                  the @ref irmo_world_method_watch call used to set
 *                  the callback.
 */

typedef void (*IrmoInvokeCallback)(IrmoMethodData *data, 
				   void *user_data);

//! \}

//---------------------------------------------------------------------
//
// IrmoObject
//
//---------------------------------------------------------------------

/*!
 * @addtogroup object
 * \{
 */

/*!
 * A union structure that can hold an integer or a string pointer.
 */

typedef union {
	unsigned int i;
	char *s;
} IrmoValue;

//! An Irmo Object

typedef struct _IrmoObject IrmoObject;

//! Callback functions for operations on variables

typedef void (*IrmoVarCallback) (IrmoObject *object, char *variable,
				 void *user_data);

//! Callback functions for operations on objects

typedef void (*IrmoObjCallback) (IrmoObject *object, void *user_data);

//! \}

//---------------------------------------------------------------------
//
// IrmoServer
//
//---------------------------------------------------------------------

/*!
 * @addtogroup server
 * \{
 */

/*!
 * A server.
 */

typedef struct _IrmoServer IrmoServer;

//! \}

//---------------------------------------------------------------------
//
// IrmoClient
//
//---------------------------------------------------------------------

/*!
 * @addtogroup client
 * \{
 */

/*!
 * A client connected to an @ref IrmoServer server.
 */

typedef struct _IrmoClient IrmoClient;

/*! 
 * Callback function for client actions.
 *
 * Functions of this type are used for client connect and
 * disconnect callbacks.
 */

typedef void (*IrmoClientCallback) (IrmoClient *client, void *user_data);

//! \}

//---------------------------------------------------------------------
//
// IrmoConnection
//
//---------------------------------------------------------------------

/*!
 * @addtogroup connection
 * \{
 */

/*!
 * A connection to a remote server.
 *
 * This represents a connection to an @ref IrmoServer server running
 * on a remote machine.
 *
 * This is exactly the same as an @ref IrmoClient. Internally the remote
 * server is seen as a client connected back to the local machine.
 */

typedef IrmoClient IrmoConnection;

//! \}

//---------------------------------------------------------------------
//
// IrmoCallback
//
//---------------------------------------------------------------------

/*!
 * @addtogroup callback
 * \{
 */

/*!
 * Callback object.
 * 
 * When a new callback watch is created, an IrmoCallback object is
 * returned representing the watch. The watch can be unset with
 * @ref irmo_callback_unset.
 */

typedef struct _IrmoCallback IrmoCallback;

/*!
 * Function invoked for watching callbacks.
 *
 * See @ref irmo_callback_watch_destroy.
 */

typedef void (*IrmoCallbackCallback)(IrmoCallback *callback,
				     void *user_data);

//! \}

#ifdef __cplusplus
}
#endif
	
#endif /* #ifndef IRMO_TYPES_H */

