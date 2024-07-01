#ifndef __CRC_H__
#define __CRC_H__

//! computes the CRC of a buffer
/*!
This function computes a 32 bit CRC of a region in memory.

param data		The address of the buffer.
param size		Size in bytes of the buffer.
param seed		Optional seed.

return The CRC of the memory.
*/
unsigned long crc(const void* data, int size, unsigned long seed);

#endif
