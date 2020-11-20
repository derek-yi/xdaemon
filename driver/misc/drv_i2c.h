
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

/* /dev/i2c-X ioctl commands.  The ioctl's parameter is always an
 * unsigned long, except for:
 *	- I2C_FUNCS, takes pointer to an unsigned long
 *	- I2C_RDWR, takes pointer to struct i2c_rdwr_ioctl_data
 *	- I2C_SMBUS, takes pointer to struct i2c_smbus_ioctl_data
 */
#define I2C_RETRIES	        0x0701	/* number of times a device address should
				                        be polled when not acknowledging */
#define I2C_TIMEOUT	        0x0702	/* set timeout in units of 10 ms */

/* NOTE: Slave address is 7 or 10 bits, but 10-bit addresses
 * are NOT supported! (due to code brokenness)
 */
#define I2C_SLAVE	        0x0703	/* Use this slave address */
#define I2C_SLAVE_FORCE	    0x0706	/* Use this slave address, even if it
				                        is already in use by a driver! */
#define I2C_TENBIT	        0x0704	/* 0 for 7 bit addrs, != 0 for 10 bit */

#define I2C_FUNCS	        0x0705	/* Get the adapter functionality mask */

#define I2C_RDWR	        0x0707	/* Combined R/W transfer (one STOP only) */

#define I2C_PEC		        0x0708	/* != 0 to use PEC with SMBus */
#define I2C_SMBUS	        0x0720	/* SMBus transfer */


/* smbus_access read or write markers */
#define I2C_SMBUS_READ	    1
#define I2C_SMBUS_WRITE	    0

#define I2C_FUNC_I2C			        0x00000001
#define I2C_FUNC_10BIT_ADDR		        0x00000002
#define I2C_FUNC_PROTOCOL_MANGLING	    0x00000004 /* I2C_M_{REV_DIR_ADDR,NOSTART,..} */
#define I2C_FUNC_SMBUS_PEC		        0x00000008
#define I2C_FUNC_SMBUS_BLOCK_PROC_CALL	0x00008000 /* SMBus 2.0 */
#define I2C_FUNC_SMBUS_QUICK		    0x00010000
#define I2C_FUNC_SMBUS_READ_BYTE	    0x00020000
#define I2C_FUNC_SMBUS_WRITE_BYTE	    0x00040000
#define I2C_FUNC_SMBUS_READ_BYTE_DATA	0x00080000
#define I2C_FUNC_SMBUS_WRITE_BYTE_DATA	0x00100000
#define I2C_FUNC_SMBUS_READ_WORD_DATA	0x00200000
#define I2C_FUNC_SMBUS_WRITE_WORD_DATA	0x00400000
#define I2C_FUNC_SMBUS_PROC_CALL	    0x00800000
#define I2C_FUNC_SMBUS_READ_BLOCK_DATA	0x01000000
#define I2C_FUNC_SMBUS_WRITE_BLOCK_DATA 0x02000000
#define I2C_FUNC_SMBUS_READ_I2C_BLOCK	0x04000000 /* I2C-like block xfer  */
#define I2C_FUNC_SMBUS_WRITE_I2C_BLOCK	0x08000000 /* w/ 1-byte reg. addr. */

#define I2C_FUNC_SMBUS_BYTE             (I2C_FUNC_SMBUS_READ_BYTE | \
                                            I2C_FUNC_SMBUS_WRITE_BYTE)
#define I2C_FUNC_SMBUS_BYTE_DATA        (I2C_FUNC_SMBUS_READ_BYTE_DATA | \
                                            I2C_FUNC_SMBUS_WRITE_BYTE_DATA)
#define I2C_FUNC_SMBUS_WORD_DATA        (I2C_FUNC_SMBUS_READ_WORD_DATA | \
                                            I2C_FUNC_SMBUS_WRITE_WORD_DATA)
#define I2C_FUNC_SMBUS_BLOCK_DATA       (I2C_FUNC_SMBUS_READ_BLOCK_DATA | \
                                            I2C_FUNC_SMBUS_WRITE_BLOCK_DATA)
#define I2C_FUNC_SMBUS_I2C_BLOCK        (I2C_FUNC_SMBUS_READ_I2C_BLOCK | \
                                            I2C_FUNC_SMBUS_WRITE_I2C_BLOCK)

/* Old name, for compatibility */
#define I2C_FUNC_SMBUS_HWPEC_CALC	    I2C_FUNC_SMBUS_PEC

struct i2c_msg {
	__u16 addr;	/* slave address			*/
	unsigned short flags;
#define I2C_M_TEN	        0x10	/* we have a ten bit chip address	*/
#define I2C_M_RD	        0x01
#define I2C_M_NOSTART	    0x4000
#define I2C_M_REV_DIR_ADDR	0x2000
#define I2C_M_IGNORE_NAK	0x1000
#define I2C_M_NO_RD_ACK		0x0800
	short len;		/* msg length				*/
	char *buf;		/* pointer to msg data			*/
};

/*
 * Data for SMBus Messages
 */
#define I2C_SMBUS_BLOCK_MAX	32	/* As specified in SMBus standard */
#define I2C_SMBUS_I2C_BLOCK_MAX	32	/* Not specified but we use same structure */
union i2c_smbus_data {
	__u8 byte;
	__u16 word;
	__u8 block[I2C_SMBUS_BLOCK_MAX + 2]; /* block[0] is used for length */
	                                            /* and one more for PEC */
};

#if 0
/* This is the structure as used in the I2C_SMBUS ioctl call */
struct i2c_smbus_ioctl_data {
	__u8 read_write;
	__u8 command;
	__u32 size;
	union i2c_smbus_data *data;
};
#endif

int i2c_read_data(int i2c_bus, int mode, int dev_id, int offset);

int i2c_read_buffer(int i2c_bus, int mode, int dev_id, int offset, __u8 *data_p, int len);

int i2c_write_buffer(int i2c_bus, int mode, int dev_id, int offset, __u8 *data_p, int len);


#endif

