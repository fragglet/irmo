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

#ifndef IRMO_IF_SPEC_H
#define IRMO_IF_SPEC_H

/*!
 * \addtogroup if_spec
 * \{
 */

/*!
 * \brief Variable types
 */

typedef enum {
	IRMO_TYPE_INT8,
	IRMO_TYPE_INT16,
	IRMO_TYPE_INT32,
	IRMO_TYPE_STRING,
} IrmoVarType;

/*!
 * \brief An IrmoInterfaceSpec object
 * \ingroup if_spec
 */

typedef struct _IrmoInterfaceSpec IrmoInterfaceSpec;

/*!
 * \brief Create a new IrmoInterfaceSpec object
 *
 * A file is parsed with the interface described in a simple
 * C-style syntax.
 *
 * \param filename	The filename of the specification file
 * \return		A new IrmoInterfaceSpec object or NULL for failure
 */

IrmoInterfaceSpec *irmo_interface_spec_new(char *filename);

/*!
 * \brief Add a reference to an IrmoInterfaceSpec object
 *
 * Reference counting is implemented for IrmoInterfaceSpec objects. Every time 
 * a new reference is kept, call this to increment the reference count.
 * When a reference is removed, call \ref irmo_interface_spec_unref. The 
 * reference count starts at 1. When the reference count reaches 0, the
 * object is destroyed.
 *
 * \param spec		The object to reference
 */

void irmo_interface_spec_ref(IrmoInterfaceSpec *spec);

/*!
 * \brief Remove a reference to an IrmoInterfaceSpec object
 *
 * See \ref irmo_interface_spec_ref
 *
 * \param spec		The object to unreference
 */

void irmo_interface_spec_unref(IrmoInterfaceSpec *spec);

//! \}

#endif /* #ifndef IFSPEC_H */

// $Log$
// Revision 1.2  2003/08/21 14:21:25  fraggle
// TypeSpec => IrmoVarType.  TYPE_* => IRMO_TYPE_*.  Make IrmoVarType publicly
// accessible.
//
// Revision 1.1.1.1  2003/06/09 21:33:25  fraggle
// Initial sourceforge import
//
// Revision 1.4  2003/06/09 21:06:55  sdh300
// Add CVS Id tag and copyright/license notices
//
// Revision 1.3  2003/03/07 12:17:22  sdh300
// Add irmo_ prefix to public function names (namespacing)
//
// Revision 1.2  2003/03/06 19:33:53  sdh300
// Rename InterfaceSpec to IrmoInterfaceSpec for API consistency
//
// Revision 1.1  2003/02/23 00:00:06  sdh300
// Split off public parts of headers into seperate files in the 'public'
// directory (objects now totally opaque)
//
// Revision 1.5  2002/11/17 22:01:58  sdh300
// InterfaceSpec documentation
//
// Revision 1.4  2002/11/13 13:56:24  sdh300
// add some documentation
//
// Revision 1.3  2002/10/29 15:58:26  sdh300
// add index entry for ClassSpec, use "index" instead of n for index
// member name
//
// Revision 1.2  2002/10/21 15:32:34  sdh300
// variable value setting
//
// Revision 1.1.1.1  2002/10/19 18:53:23  sdh300
// initial import
//
