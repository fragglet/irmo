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

#ifndef IRMO_ERROR_H
#define IRMO_ERROR_H

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Some functions report errors. All error messages are echoed to the 
 * standard error output (stderr) and the error message can be retrieved
 * by the program with \ref irmo_error_get.
 *
 * \addtogroup error
 * \{
 */

/*!
 * \brief Get an error message 
 *
 * Returns the string containing the last reported error message
 * by the library.
 *
 * \return A constant string containing the error
 *
 */

char *irmo_error_get(void);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_ERROR_H */

// $Log$
// Revision 1.4  2003/11/21 18:10:18  fraggle
// Fix up doxygen documentation; move section documentation into headers
// from 'sections.doxygen'
//
// Revision 1.3  2003/11/21 17:46:18  fraggle
// Restructure header files: move type definitions into "types.h"; move
// callback prototypes into their appropriate headers instead of
// callback.h; make headers C++-safe
//
// Revision 1.2  2003/08/13 18:32:40  fraggle
// Convert () in function definitions to the more explicit (void)
//
// Revision 1.1  2003/07/24 01:32:08  fraggle
// Add error reporting modules
//

