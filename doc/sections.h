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


// Documentation for sections

/*!
 * @mainpage Irmo
 * @section intro Introduction
 *
 * Irmo is an abstract library designed for Multiplayer Computer Gaming.
 * 
 * @section genericsec Generic Basic interfaces
 *
 * These interfaces are generic interfaces that are used throughout
 * the library.
 *
 * @li @ref callback
 * @li @ref iterator
 * @li @ref error
 *
 * @section ifacesec Interface
 *
 * An interface defines a collection of classes and methods that form
 * a game world.
 *
 * @li @ref iface
 * @li @ref ifaceparser
 *
 * @section worldsec World
 * 
 * A game world consists of a collection of objects that represent
 * the state of the game.
 *
 * @li @ref world
 * @li @ref object
 * @li @ref method
 * 
 * @section netsec Networking
 *
 * The networking interface allows the state of game worlds to be 
 * reproduced over a network connection.  The world is shared through
 * a server, which is accessed by a number of clients.
 *
 * @li @ref server
 * @li @ref client
 * @li @ref connection
 * @li @ref module_ipv4
 *
 * @section netmodsec Network modules
 *
 * Network modules form the foundation of the networking interface,
 * abstracting the process of sending and receiving packets.
 * Data can be exchanged over a new type of connection by writing a 
 * new network module.
 *
 * @li @ref packet
 * @li @ref net_module
 */

/*!
 * @defgroup iterator Generic iterator interface
 */

/*!
 * @defgroup callback Generic callback interface
 */

/*!
 * @defgroup error Error Reporting
 */

/*!
 * @defgroup iface Irmo Interface
 */

/*!
 * @defgroup ifaceparser Interface Parser Library
 */

/*! 
 * @defgroup world Irmo World
 */

/*! 
 * @defgroup object Irmo Objects
 */

/*! 
 * @defgroup method Irmo Method Call Interface
 */

/*! 
 * @defgroup server Irmo Server
 */

/*! 
 * @defgroup client Irmo Client
 */

/*! 
 * @defgroup connection Irmo Client Interface
 */

/*!
 * @defgroup module_ipv4 IPv4 networking module
 */

/*!
 * @defgroup packet Network packet
 */

/*!
 * @defgroup net_module Network module
 */

