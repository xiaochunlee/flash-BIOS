/*
 * klbios.h -- definitions for the char module
 *
 * Copyright (C) 2001 Alessandro Rubini and Jonathan Corbet
 * Copyright (C) 2001 O'Reilly & Associates
 *
 * The source code in this file can be freely used, adapted,
 * and redistributed in source or binary form, so long as an
 * acknowledgment appears in derived source files.  The citation
 * should list that the code comes from the book "Linux Device
 * Drivers" by Alessandro Rubini and Jonathan Corbet, published
 * by O'Reilly & Associates.   No warranty is attached;
 * we cannot take responsibility for errors or fitness for use.
 *
 * $Id: klbios.h,v 1.15 2004/11/04 17:51:18 rubini Exp $
 */

#ifndef _KLBIOS_H_
#define _KLBIOS_H_

//#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/*
 * Macros to help debugging
 */

#undef PDEBUG             /* undef it, just in case */
#ifdef KLBIOS_DEBUG
#  ifdef __KERNEL__
     /* This one if debugging is on, and kernel space */
#    define PDEBUG(fmt, args...) printk( KERN_ERR "klbios: " fmt, ## args)
#  else
     /* This one for user space */
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#  endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

#ifndef KLBIOS_MAJOR
#define KLBIOS_MAJOR 0   /* dynamic major by default */
#endif

#ifndef KLBIOS_NR_DEVS
#define KLBIOS_NR_DEVS 1    /* klbios0 through klbios3 */
#endif


/*
 * The bare device is a variable-length region of memory.
 * Use a linked list of indirect blocks.
 *
 * "klbios_dev->data" points to an array of pointers, each
 * pointer refers to a memory area of KLBIOS_QUANTUM bytes.
 *
 * The array (quantum-set) is KLBIOS_QSET long.
 */
#ifndef KLBIOS_QUANTUM
#define KLBIOS_QUANTUM 4096
#endif

#ifndef KLBIOS_QSET
#define KLBIOS_QSET   3 
#endif


/*
 * Representation of klbios quantum sets.
 */
struct klbios_qset {
	void **data;
	struct klbios_qset *next;
};

struct klbios_dev {
	struct klbios_qset *data;  /* Pointer to first quantum set */
	int quantum;              /* the current quantum size */
	int qset;                 /* the current array size */
	unsigned long size;       /* amount of data stored here */
	unsigned int access_key;  /* used by klbiosuid and klbiospriv */
	struct semaphore sem;     /* mutual exclusion semaphore     */
	struct cdev cdev;	  /* Char device structure		*/
};

/*
 * Split minors in two parts
 */
#define TYPE(minor)	(((minor) >> 4) & 0xf)	/* high nibble */
#define NUM(minor)	((minor) & 0xf)		/* low  nibble */


/*
 * The different configurable parameters
 */
extern int klbios_major;     /* main.c */
extern int klbios_nr_devs;
extern int klbios_quantum;
extern int klbios_qset;



/*
 * Prototypes for shared functions
 */


int     klbios_trim(struct klbios_dev *dev);

ssize_t klbios_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos);
ssize_t klbios_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos);
loff_t  klbios_llseek(struct file *filp, loff_t off, int whence);




#endif /* _KLBIOS_H_ */
