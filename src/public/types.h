// Emacs style mode select -*- C++ -*-
//---------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 2002-2003 University of Southampton
// Copyright (C) 2003 Simon Howard
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation; either
// version 2 of the License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the
// Free Software Foundation, Inc., 59 Temple Place - Suite 330,
// Boston, MA 02111-1307, USA.
//
//---------------------------------------------------------------------

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

/*!
 *---------------------------------------------------------------------
 *
 * IrmoInterfaceSpec
 *
 *---------------------------------------------------------------------
 *
 * \addtogroup if_spec
 * \{
 */

/*!
 * \brief An IrmoInterfaceSpec object
 * \ingroup if_spec
 */

typedef struct _IrmoInterfaceSpec IrmoInterfaceSpec;

/*!
 * \brief An object representing a class in an interface
 */

typedef struct _IrmoClass IrmoClass;

/*!
 * \brief An object representing a variable in a class
 */

typedef struct _IrmoClassVar IrmoClassVar;

/*!
 * \brief An object representing a method in an interface
 */

typedef struct _IrmoMethod IrmoMethod;

/*!
 * \brief An object representing an argument to a method
 */ 

typedef struct _IrmoMethodArg IrmoMethodArg;

/*!
 * \brief Variable types
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

/*!
 *---------------------------------------------------------------------
 *
 * IrmoWorld
 *
 *---------------------------------------------------------------------
 *
 * \addtogroup world
 * \{
 */

//! An Irmo World.

typedef struct _IrmoWorld IrmoWorld;

/*!
 * \brief A numerical object identifier
 *
 * All objects in a \ref IrmoWorld have a unique number assigned to
 * them. This can be used to refer to objects by their number. You can
 * search for an object by number using the 
 * \ref irmo_world_get_object_for_id function.
 */

typedef unsigned int IrmoObjectID;

//! \}


/*!
 *---------------------------------------------------------------------
 *
 * Method Callbacks
 *
 *---------------------------------------------------------------------
 *
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

/*!
 * \brief Callback function for methods
 *
 * When a method invocation is received from a client, a callback function
 * of this type is invoked.
 *
 * \param data      A \ref IrmoMethodData object holding data about the
 *                  invoked method.
 * \param user_data A pointer to user defined piece of memory specified in
 *                  the \ref irmo_world_method_watch call used to set
 *                  the callback.
 */

typedef void (*IrmoInvokeCallback)(IrmoMethodData *data, 
				   void *user_data);

//! \}

/*!
 *---------------------------------------------------------------------
 *
 * IrmoObject
 *
 *---------------------------------------------------------------------
 *
 * \addtogroup object
 * \{
 */

/*!
 * \brief A union structure that can hold an integer or a string pointer
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

/*!
 *---------------------------------------------------------------------
 *
 * IrmoSocket
 *
 *---------------------------------------------------------------------
 *
 * \addtogroup socket
 * \{
 */

/*!
 * \brief an IrmoSocket object.
 */

typedef struct _IrmoSocket IrmoSocket;

/*!
 * \brief Socket domain
 * 
 * When creating a new socket, it is neccessary to specify the type
 * of socket to be created, IPv4 or IPv6.
 */

typedef enum {
	IRMO_SOCKET_AUTO,
	IRMO_SOCKET_IPV4,
	IRMO_SOCKET_IPV6,
} IrmoSocketDomain;

//! \}

/*!
 *---------------------------------------------------------------------
 *
 * IrmoServer
 *
 *---------------------------------------------------------------------
 *
 * \addtogroup server
 * \{
 */

/*!
 * \brief An IrmoServer object
 */

typedef struct _IrmoServer IrmoServer;

//! \}

/*!
 *---------------------------------------------------------------------
 *
 * IrmoClient
 *
 *---------------------------------------------------------------------
 *
 * \addtogroup client
 * \{
 */

/*!
 * \brief An IrmoClient object
 *
 * This represents a client connected to a \ref IrmoServer server.
 */

typedef struct _IrmoClient IrmoClient;

/*! 
 * \brief Callback function for client actions.
 *
 * Functions of this type are used for client disconnect callbacks.
 */

typedef void (*IrmoClientCallback) (IrmoClient *client, void *user_data);

//! \}

/*!
 *---------------------------------------------------------------------
 *
 * IrmoConnection
 *
 *---------------------------------------------------------------------
 *
 * \addtogroup connection
 * \{
 */

/*!
 * \brief An IrmoConnection object 
 *
 * This represents a connection to a \ref IrmoServer server running
 * on a remote machine.
 *
 * This is exactly the same as an \ref IrmoClient. Internally the remote
 * server is seen as a client connected back to the local machine.
 */

typedef IrmoClient IrmoConnection;

//! \}

/*!
 *---------------------------------------------------------------------
 *
 * IrmoCallback
 *
 *---------------------------------------------------------------------
 *
 * \addtogroup callback
 * \{
 */

/*!
 * \brief Callback object
 * 
 * When a new callback watch is created, an IrmoCallback object is
 * returned representing the watch. The watch can be unset with
 * \ref irmo_callback_unset
 */

typedef struct _IrmoCallback IrmoCallback;

//! \}

#ifdef __cplusplus
}
#endif
	
#endif /* #ifndef IRMO_TYPES_H */

// $Log$
// Revision 1.1  2003/11/21 17:46:18  fraggle
// Restructure header files: move type definitions into "types.h"; move
// callback prototypes into their appropriate headers instead of
// callback.h; make headers C++-safe
//
