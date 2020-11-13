
#ifndef _DRV_I2C_H_
#define _DRV_I2C_H_

#ifndef uint8
typedef unsigned char __u8;
#endif

#ifndef uint16
typedef unsigned short __u16;
#endif

#ifndef uint32
typedef unsigned int __u32;
#endif

#ifndef __s32
typedef signed int __s32;
#endif

/* SMBus transaction types (size parameter in the above functions)
   Note: these no longer correspond to the (arbitrary) PIIX4 internal codes! */
#define I2C_SMBUS_QUICK		            0
#define I2C_SMBUS_BYTE		            1
#define I2C_SMBUS_BYTE_DATA	            2
#define I2C_SMBUS_WORD_DATA	            3
#define I2C_SMBUS_PROC_CALL	            4
#define I2C_SMBUS_BLOCK_DATA	        5
#define I2C_SMBUS_I2C_BLOCK_BROKEN      6
#define I2C_SMBUS_BLOCK_PROC_CALL       7		/* SMBus 2.0 */
#define I2C_SMBUS_I2C_BLOCK_DATA        8


int i2c_read_data(int i2c_bus, int mode, int dev_id, int offset);

int i2c_read_buffer(int i2c_bus, int mode, int dev_id, int offset, __u8 *data_p, int len);

int i2c_write_buffer(int i2c_bus, int mode, int dev_id, int offset, __u8 *data_p, int len);


#endif

