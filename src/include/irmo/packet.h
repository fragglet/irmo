//
// Copyright (C) 2002-7 Simon Howard
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

#ifdef __cplusplus
extern "C" {
#endif

/*!
 * Safe packet handling code.  Packet buffers are read/written to
 * entirely through these safe routines that prevent buffer overflow.
 *
 * \addtogroup packet
 * \{
 */


#ifndef IRMO_PACKET_H
#define IRMO_PACKET_H

#include <irmo/types.h>

/*!
 * A packet.
 */

typedef struct _IrmoPacket IrmoPacket;

/*!
 * Allocate a new empty packet.
 *
 * \return                 Pointer to a new packet structure.
 */

IrmoPacket *irmo_packet_new(void);

/*!
 * Create a new packet, wrapping the provided data.  The packet can
 * be used to parse the data in the array provided.  No copy is 
 * made of the array.
 *
 * \param data       Pointer to the data buffer to parse.
 * \param data_len   Length of the data buffer.
 * \return           Pointer to a new IrmoPacket structure.
 */

IrmoPacket *irmo_packet_new_from(unsigned char *data, int data_len);

/*!
 * Free a packet.
 *
 * \param packet     The packet to free.
 */

void irmo_packet_free(IrmoPacket *packet);

/*!
 * Write a single byte (8-bit integer) to the packet.
 *
 * \param packet     The packet to write to.
 * \param i          Value to write.
 * \return           Non-zero if successful.
 */

int irmo_packet_writei8(IrmoPacket *packet, unsigned int i);

/*!
 * Write a 16-bit integer to the packet.
 *
 * \param packet     The packet to write to.
 * \param i          Value to write.
 * \return           Non-zero if successful.
 */

int irmo_packet_writei16(IrmoPacket *packet, unsigned int i);

/*!
 * Write a 32-bit integer to the packet.
 *
 * \param packet     The packet to write to.
 * \param i          Value to write.
 * \return           Non-zero if successful.
 */

int irmo_packet_writei32(IrmoPacket *packet, unsigned int i);

/*!
 * Write a NUL-terminated string to the packet.
 *
 * \param packet     The packet to write to.
 * \param s          Value to write.
 * \return           Non-zero if successful.
 */

int irmo_packet_writestring(IrmoPacket *packet, char *s);

/*!
 * Read a single byte (8-bit integer) from the packet.
 *
 * \param packet     The packet to read from.
 * \param i          Pointer to a variable to store the value read.
 * \return           Non-zero if successful.
 */

int irmo_packet_readi8(IrmoPacket *packet, unsigned int *i);

/*!
 * Read a 16-bit integer from the packet.
 *
 * \param packet     The packet to read from.
 * \param i          Pointer to a variable to store the value read.
 * \return           Non-zero if successful.
 */

int irmo_packet_readi16(IrmoPacket *packet, unsigned int *i);

/*!
 * Read a 32-bit integer from the packet.
 *
 * \param packet     The packet to read from.
 * \param i          Pointer to a variable to store the value read.
 * \return           Non-zero if successful.
 */

int irmo_packet_readi32(IrmoPacket *packet, unsigned int *i);

/*!
 * Read a NUL-terminated from the packet.
 * The pointer returned points within the packet buffer; it is therefore
 * valid until the packet is freed.
 *
 * \param packet     The packet to read from.
 * \return           A pointer to the string read, or NULL if unsuccessful.
 */

char *irmo_packet_readstring(IrmoPacket *packet);

/*!
 * Identical to \ref irmo_packet_read_value except that no value is stored
 * or returned; this function simply returns whether a valid value was read
 * successfully.
 *
 * \param packet     The packet to read from.
 * \param type       Type of value to read.
 * \return           Non-zero for success, or zero for failure.
 */

int irmo_packet_verify_value(IrmoPacket *packet, IrmoValueType type);

/*!
 * Read an \ref IrmoValue from a packet.
 *
 * \param packet     The packet to read from.
 * \param value      Pointer to an \ref IrmoValue structure to read the
 *                   result into.
 * \param type       Type of value to read.
 */

void irmo_packet_read_value(IrmoPacket *packet, IrmoValue *value, 
			    IrmoValueType type);

/*! 
 * Write an \ref IrmoValue to a packet.
 *
 * \param packet     The packet to write into.
 * \param value      Pointer to an \ref Irmovalue structure containing
 *                   the data to write.
 * \param type       Type of value to write.
 */

void irmo_packet_write_value(IrmoPacket *packet, IrmoValue *value, 
			     IrmoValueType type);

//! \}

#ifdef __cplusplus
}
#endif

#endif /* #ifndef IRMO_PACKET_H */

