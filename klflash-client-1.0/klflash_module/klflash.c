/*
 * main.c -- the bare klbios char module
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
 */

//#include <linux/config.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

#include <linux/kernel.h>	/* printk() */
#include <linux/slab.h>		/* kmalloc() */
#include <linux/fs.h>		/* everything... */
#include <linux/errno.h>	/* error codes */
#include <linux/types.h>	/* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h>	/* O_ACCMODE */
#include <linux/seq_file.h>
#include <linux/cdev.h>

#include <asm/system.h>		/* cli(), *_flags */
#include <asm/uaccess.h>	/* copy_*_user */
#include <asm/io.h>

#include "klflash.h"		/* local definitions */

/*
 * Our parameters which can be set at load time.
 */

int klbios_major =   KLBIOS_MAJOR;
int klbios_minor =   0;
int klbios_nr_devs = KLBIOS_NR_DEVS;	/* number of bare klbios devices */
int klbios_quantum = KLBIOS_QUANTUM;
int klbios_qset =    KLBIOS_QSET;
#if 0
module_param(klbios_major, int, S_IRUGO);
module_param(klbios_minor, int, S_IRUGO);
module_param(klbios_nr_devs, int, S_IRUGO);
module_param(klbios_quantum, int, S_IRUGO);
module_param(klbios_qset, int, S_IRUGO);
#endif

MODULE_AUTHOR("lixiaochun");
MODULE_LICENSE("Dual BSD/GPL");

struct klbios_dev *klbios_devices;	/* allocated in klbios_init_module */


/*
 * Empty out the klbios device; must be called with the device
 * semaphore held.
 */
int klbios_trim(struct klbios_dev *dev)
{
	struct klbios_qset *next, *dptr;
	int qset = dev->qset;   /* "dev" is not-null */
	int i;

	for (dptr = dev->data; dptr; dptr = next) { /* all the list items */
		if (dptr->data) {
			for (i = 0; i < qset; i++)
				kfree(dptr->data[i]);
			kfree(dptr->data);
			dptr->data = NULL;
		}
		next = dptr->next;
		kfree(dptr);
	}
	dev->size = 0;
	dev->quantum = klbios_quantum;
	dev->qset = klbios_qset;
	dev->data = NULL;
	return 0;
}
#ifdef KLBIOS_DEBUG /* use proc only if debugging */
/*
 * The proc filesystem: function to read and entry
 */

int klbios_read_procmem(char *buf, char **start, off_t offset,
                   int count, int *eof, void *data)
{
	int i, j, len = 0;
	int limit = count - 80; /* Don't print more than this */

	for (i = 0; i < klbios_nr_devs && len <= limit; i++) {
		struct klbios_dev *d = &klbios_devices[i];
		struct klbios_qset *qs = d->data;
		if (down_interruptible(&d->sem))
			return -ERESTARTSYS;
		len += sprintf(buf+len,"\nDevice %i: qset %i, q %i, sz %li\n",
				i, d->qset, d->quantum, d->size);
		for (; qs && len <= limit; qs = qs->next) { /* scan the list */
			len += sprintf(buf + len, "  item at %p, qset at %p\n",
					qs, qs->data);
			if (qs->data && !qs->next) /* dump only the last item */
				for (j = 0; j < d->qset; j++) {
					if (qs->data[j])
						len += sprintf(buf + len,
								"    % 4i: %8p\n",
								j, qs->data[j]);
				}
		}
		up(&klbios_devices[i].sem);
	}
	*eof = 1;
	return len;
}


/*
 * For now, the seq_file implementation will exist in parallel.  The
 * older read_procmem function should maybe go away, though.
 */

/*
 * Here are our sequence iteration methods.  Our "position" is
 * simply the device number.
 */
static void *klbios_seq_start(struct seq_file *s, loff_t *pos)
{
	if (*pos >= klbios_nr_devs)
		return NULL;   /* No more to read */
	return klbios_devices + *pos;
}

static void *klbios_seq_next(struct seq_file *s, void *v, loff_t *pos)
{
	(*pos)++;
	if (*pos >= klbios_nr_devs)
		return NULL;
	return klbios_devices + *pos;
}

static void klbios_seq_stop(struct seq_file *s, void *v)
{
	/* Actually, there's nothing to do here */
}

static int klbios_seq_show(struct seq_file *s, void *v)
{
	struct klbios_dev *dev = (struct klbios_dev *) v;
	struct klbios_qset *d;
	int i;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	seq_printf(s, "\nDevice %i: qset %i, q %i, sz %li\n",
			(int) (dev - klbios_devices), dev->qset,
			dev->quantum, dev->size);
	for (d = dev->data; d; d = d->next) { /* scan the list */
		seq_printf(s, "   item at %p, qset at %p\n", d, d->data);
		
		seq_printf(s, "PY:item at %p, qset at %p\n", virt_to_phys(d), virt_to_phys(d->data));
		if (d->data && !d->next) /* dump only the last item */
			for (i = 0; i < dev->qset; i++) {
				if (d->data[i]) {
					seq_printf(s, "    % 4i: %8p\n",
							i, d->data[i]);
					seq_printf(s, "PY  % 4i: %8p\n",
							i, virt_to_phys(d->data[i]));
							}
			}
	}
	up(&dev->sem);
	return 0;
}
	
/*
 * Tie the sequence operators up.
 */
static struct seq_operations klbios_seq_ops = {
	.start = klbios_seq_start,
	.next  = klbios_seq_next,
	.stop  = klbios_seq_stop,
	.show  = klbios_seq_show
};

/*
 * Now to implement the /proc file we need only make an open
 * method which sets up the sequence operators.
 */
static int klbios_proc_open(struct inode *inode, struct file *file)
{
	return seq_open(file, &klbios_seq_ops);
}

/*
 * Create a set of file operations for our proc file.
 */
static struct file_operations klbios_proc_ops = {
	.owner   = THIS_MODULE,
	.open    = klbios_proc_open,
	.read    = seq_read,
	.llseek  = seq_lseek,
	.release = seq_release
};
	

/*
 * Actually create (and remove) the /proc file(s).
 */

static void klbios_create_proc(void)
{
	struct proc_dir_entry *entry;
	create_proc_read_entry("klbiosmem", 0 /* default mode */,
			NULL /* parent dir */, klbios_read_procmem,
			NULL /* client data */);
	entry = create_proc_entry("klbiosseq", 0, NULL);
	if (entry)
		entry->proc_fops = &klbios_proc_ops;
}

static void klbios_remove_proc(void)
{
	/* no problem if it was not registered */
	remove_proc_entry("klbiosmem", NULL /* parent dir */);
	remove_proc_entry("klbiosseq", NULL);
}


#endif /* KLBIOS_DEBUG */





/*
 * Open and close
 */

int klbios_open(struct inode *inode, struct file *filp)
{
	struct klbios_dev *dev; /* device information */

	dev = container_of(inode->i_cdev, struct klbios_dev, cdev);
	filp->private_data = dev; /* for other methods */

	/* now trim to 0 the length of the device if open was write-only */
	if ( (filp->f_flags & O_ACCMODE) == O_WRONLY) {
		if (down_interruptible(&dev->sem))
			return -ERESTARTSYS;
		klbios_trim(dev); /* ignore errors */
		up(&dev->sem);
	}
	return 0;          /* success */
}

int klbios_release(struct inode *inode, struct file *filp)
{
	return 0;
}
/*
 * Follow the list
 */
struct klbios_qset *klbios_follow(struct klbios_dev *dev, int n)
{
	struct klbios_qset *qs = dev->data;

        /* Allocate first qset explicitly if need be */
	if (! qs) {
		qs = dev->data = kmalloc(sizeof(struct klbios_qset), GFP_KERNEL);
		if (qs == NULL)
			return NULL;  /* Never mind */
		memset(qs, 0, sizeof(struct klbios_qset));
	}

	/* Then follow the list */
	while (n--) {
		if (!qs->next) {
			qs->next = kmalloc(sizeof(struct klbios_qset), GFP_KERNEL);
			if (qs->next == NULL)
				return NULL;  /* Never mind */
			memset(qs->next, 0, sizeof(struct klbios_qset));
		}
		qs = qs->next;
		continue;
	}
	return qs;
}

/*
 * Data management: read and write
 */

ssize_t klbios_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct klbios_dev *dev = filp->private_data; 
	struct klbios_qset *dptr;	/* the first listitem */
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset; /* how many bytes in the listitem */
	int item, s_pos, q_pos, rest;
	ssize_t retval = 0;

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	if (*f_pos >= dev->size)
		goto out;
	if (*f_pos + count > dev->size)
		count = dev->size - *f_pos;

	/* find listitem, qset index, and offset in the quantum */
	item = (long)*f_pos / itemsize;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

	/* follow the list up to the right position (defined elsewhere) */
	dptr = klbios_follow(dev, item);

	if (dptr == NULL || !dptr->data || ! dptr->data[s_pos])
		goto out; /* don't fill holes */

	/* read only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_to_user(buf, dptr->data[s_pos] + q_pos, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

  out:
	up(&dev->sem);
	return retval;
}

ssize_t klbios_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
	struct klbios_dev *dev = filp->private_data;
	struct klbios_qset *dptr = NULL;
	int quantum = dev->quantum, qset = dev->qset;
	int itemsize = quantum * qset;
	int item, s_pos, q_pos, rest;
	ssize_t retval = -ENOMEM; /* value used in "goto out" statements */

	if (down_interruptible(&dev->sem))
		return -ERESTARTSYS;
	/* find listitem, qset index and offset in the quantum */
	item = (long)*f_pos / itemsize;
	if (item >= 1)
		goto out;
	rest = (long)*f_pos % itemsize;
	s_pos = rest / quantum; q_pos = rest % quantum;

	/* follow the list up to the right position */
	dptr = klbios_follow(dev, item);
	if (dptr == NULL)
		goto out;
	if (!dptr->data) {
		dptr->data = kmalloc(qset * sizeof(char *), GFP_KERNEL);
		if (!dptr->data)
			goto out;
		memset(dptr->data, 0, qset * sizeof(char *));
		PDEBUG("alloc %d * %d byte(char *data[%d]) for dptr->data\n",qset, sizeof(char *),qset);
	}
	if (!dptr->data[s_pos]) {
		dptr->data[s_pos] = kmalloc(quantum, GFP_KERNEL);
		PDEBUG("alloc %d byte(quantum) for dptr->data[%d][%d]\n",quantum, item, s_pos);
		if (!dptr->data[s_pos])
			goto out;
	}
	/* write only up to the end of this quantum */
	if (count > quantum - q_pos)
		count = quantum - q_pos;

	if (copy_from_user(dptr->data[s_pos]+q_pos, buf, count)) {
		retval = -EFAULT;
		goto out;
	}
	*f_pos += count;
	retval = count;

        /* update the size */
	if (dev->size < *f_pos)
		dev->size = *f_pos;
#if 1 
	if (dev->size >= 0x1000*3) {
		*(unsigned long *)(dptr->data[2] + 0) = virt_to_phys(dptr->data[2]);  
		*(unsigned long *)(dptr->data[2] + 4) = virt_to_phys(dptr->data[1]);  
		*(unsigned long *)(dptr->data[2] + 8) = virt_to_phys(dptr->data[0]);  
	}
#endif		

  out:
	up(&dev->sem);
	return retval;
}



/*
 * The "extended" operations -- only seek
 */

loff_t klbios_llseek(struct file *filp, loff_t off, int whence)
{
	struct klbios_dev *dev = filp->private_data;
	loff_t newpos;

	switch(whence) {
	  case 0: /* SEEK_SET */
		newpos = off;
		break;

	  case 1: /* SEEK_CUR */
		newpos = filp->f_pos + off;
		break;

	  case 2: /* SEEK_END */
		newpos = dev->size + off;
		break;

	  default: /* can't happen */
		return -EINVAL;
	}
	if (newpos < 0) return -EINVAL;
	filp->f_pos = newpos;
	return newpos;
}



struct file_operations klbios_fops = {
	.owner =    THIS_MODULE,
	.llseek =   klbios_llseek,
	.read =     klbios_read,
	.write =    klbios_write,
	.open =     klbios_open,
	.release =  klbios_release,
};

/*
 * Finally, the module stuff
 */

/*
 * The cleanup function is used to handle initialization failures as well.
 * Thefore, it must be careful to work correctly even if some of the items
 * have not been initialized
 */
void klbios_cleanup_module(void)
{
	int i;
	dev_t devno = MKDEV(klbios_major, klbios_minor);

	/* Get rid of our char dev entries */
	if (klbios_devices) {
		for (i = 0; i < klbios_nr_devs; i++) {
			klbios_trim(klbios_devices + i);
			cdev_del(&klbios_devices[i].cdev);
		}
		kfree(klbios_devices);
	}

#ifdef KLBIOS_DEBUG /* use proc only if debugging */
	klbios_remove_proc();
#endif

	/* cleanup_module is never called if registering failed */
	unregister_chrdev_region(devno, klbios_nr_devs);


}


/*
 * Set up the char_dev structure for this device.
 */
static void klbios_setup_cdev(struct klbios_dev *dev, int index)
{
	int err, devno = MKDEV(klbios_major, klbios_minor + index);
    
	cdev_init(&dev->cdev, &klbios_fops);
	dev->cdev.owner = THIS_MODULE;
	dev->cdev.ops = &klbios_fops;
	err = cdev_add (&dev->cdev, devno, 1);
	/* Fail gracefully if need be */
	if (err)
		printk(KERN_NOTICE "Error %d adding klbios%d", err, index);
}


int klbios_init_module(void)
{
	int result, i;
	dev_t dev = 0;

/*
 * Get a range of minor numbers to work with, asking for a dynamic
 * major unless directed otherwise at load time.
 */
	if (klbios_major) {
		dev = MKDEV(klbios_major, klbios_minor);
		result = register_chrdev_region(dev, klbios_nr_devs, "klflash");
	} else {
		result = alloc_chrdev_region(&dev, klbios_minor, klbios_nr_devs,
				"klflash");
		klbios_major = MAJOR(dev);
	}
	if (result < 0) {
		printk(KERN_WARNING "klbios: can't get major %d\n", klbios_major);
		return result;
	}

        /* 
	 * allocate the devices -- we can't have them static, as the number
	 * can be specified at load time
	 */
	klbios_devices = kmalloc(klbios_nr_devs * sizeof(struct klbios_dev), GFP_KERNEL);
	if (!klbios_devices) {
		result = -ENOMEM;
		goto fail;  /* Make this more graceful */
	}
	memset(klbios_devices, 0, klbios_nr_devs * sizeof(struct klbios_dev));

        /* Initialize each device. */
	for (i = 0; i < klbios_nr_devs; i++) {
		klbios_devices[i].quantum = klbios_quantum;
		klbios_devices[i].qset = klbios_qset;
		init_MUTEX(&klbios_devices[i].sem);
		klbios_setup_cdev(&klbios_devices[i], i);
	}

        /* At this point call the init function for any friend device */
//	dev = MKDEV(klbios_major, klbios_minor + klbios_nr_devs);
//	dev += klbios_p_init(dev);
//	dev += klbios_access_init(dev);

#ifdef KLBIOS_DEBUG /* only when debugging */
	klbios_create_proc();
#endif

	return 0; /* succeed */

  fail:
	klbios_cleanup_module();
	return result;
}

module_init(klbios_init_module);
module_exit(klbios_cleanup_module);
