#ifndef IRMO_IF_SPEC_H
#define IRMO_IF_SPEC_H

/*!
 * \addtogroup if_spec
 * \{
 */

/*!
 * \brief An InterfaceSpec object
 * \ingroup if_spec
 */

typedef struct _InterfaceSpec InterfaceSpec;

/*!
 * \brief Create a new InterfaceSpec object
 *
 * A file is parsed with the interface described in a simple
 * C-style syntax.
 *
 * \param filename	The filename of the specification file
 * \return		A new InterfaceSpec object or NULL for failure
 */

InterfaceSpec *interface_spec_new(char *filename);

/*!
 * \brief Add a reference to an InterfaceSpec object
 *
 * Reference counting is implemented for InterfaceSpec objects. Every time 
 * a new reference is kept, call this to increment the reference count.
 * When a reference is removed, call \ref interface_spec_unref. The 
 * reference count starts at 1. When the reference count reaches 0, the
 * object is destroyed.
 *
 * \param spec		The object to reference
 */

void interface_spec_ref(InterfaceSpec *spec);

/*!
 * \brief Remove a reference to an InterfaceSpec object
 *
 * See \ref interface_spec_ref
 *
 * \param spec		The object to unreference
 */

void interface_spec_unref(InterfaceSpec *spec);

//! \}

#endif /* #ifndef IFSPEC_H */

// $Log: not supported by cvs2svn $
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
