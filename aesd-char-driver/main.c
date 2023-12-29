/**
 * @file aesdchar.c
 * @brief Functions and data related to the AESD char driver implementation
 *
 * Based on the implementation of the "scull" device driver, found in
 * Linux Device Drivers example code.
 *
 * @author Dan Walkes
 * @date 2019-10-22
 * @copyright Copyright (c) 2019
 *
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/printk.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/fs.h>           // file_operations
#include <linux/slab.h>		// kmalloc()

#include "aesd-circular-buffer.h"
#include "aesdchar.h"

int aesd_major =   0; // use dynamic major
int aesd_minor =   0;

MODULE_AUTHOR("part-of-the-crew");
MODULE_LICENSE("Dual BSD/GPL");

struct aesd_dev aesd_device;
cbuf_t cbuf;

int aesd_open(struct inode *inode, struct file *filp)
{
    struct aesd_dev *dev; /* device information */
    PDEBUG("open");
    /**
     * TODO: handle open
     */
    dev = container_of(inode->i_cdev, struct aesd_dev, cdev);
    filp->private_data = dev; /* for other methods */

    return 0;
}

int aesd_release(struct inode *inode, struct file *filp)
{
    PDEBUG("release");
    /**
     * TODO: handle release
     */
    //MOD_DEC_USE_COUNT;
    return 0;
}

ssize_t aesd_read(struct file *filp, char __user *buf, size_t count,
                loff_t *f_pos)
{
    aesd_dev_t * const dev = filp->private_data;
    const ssize_t kcount = dev->data->total_size;
    char * const kbuf = kmalloc(kcount, GFP_KERNEL);
    //int i;

    PDEBUG("read %zu bytes with offset %lld",count,*f_pos);
    /**
     * TODO: handle read
     */
    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    if (0 > aesd_circular_buffer_allread(dev->data, kbuf))
    {
       PDEBUG("sizes of data do not match");
       count = -ERESTARTSYS;
       goto zero;
    }

    if (*f_pos >= kcount)
    {
       count = 0;
       goto zero;
    }
    if (count >= kcount - *f_pos)
    {
      count = kcount - *f_pos;
      //if (copy_to_user(buf, kbuf + *f_pos, count))
      if (copy_to_user(buf, kbuf, count))
        PDEBUG("bad copy_to_user");
      //for (i = 0; i < count; i++)
      //  PDEBUG("%c", kbuf[i]);
      //PDEBUG("\nEND-count = %ld", count);
    }
    *f_pos += count;

    zero:
    kfree (kbuf);
    mutex_unlock(&dev->lock);

    return count;
}

ssize_t aesd_write(struct file *filp, const char __user *buf, size_t count,
                loff_t *f_pos)
{
    //ssize_t retval = -ENOMEM;
    aesd_dev_t *dev = filp->private_data;

    char *chunk_buf = NULL;
    char *kbuf = NULL;
    //
    PDEBUG("write %zu bytes with offset %lld",count,*f_pos);
    /*
     * TODO: handle write
    */

    if (mutex_lock_interruptible(&dev->lock))
        return -ERESTARTSYS;

    kbuf = kmalloc(count, GFP_KERNEL);
    if (copy_from_user(kbuf, buf, count))
      ;

//-----------------
    if (NULL != dev->data->chunk.buffptr || '\n' != kbuf[count - 1])
    {
      //PDEBUG("1write %zu bytes with offset %lld",count,*f_pos);

      chunk_buf = kmalloc(dev->data->chunk.size + count, GFP_KERNEL);
      if (NULL != dev->data->chunk.buffptr)
      {
        memcpy(chunk_buf, dev->data->chunk.buffptr, dev->data->chunk.size);
        kfree(dev->data->chunk.buffptr);
        dev->data->chunk.buffptr = NULL;
      }
      memcpy(chunk_buf + dev->data->chunk.size, kbuf, count);
      dev->data->chunk.size += count;
      dev->data->chunk.buffptr = chunk_buf;

    } else {
      //PDEBUG("2write %zu bytes with offset %lld",count,*f_pos);
      dev->data->chunk.size = count;
      dev->data->chunk.buffptr = kmalloc(count, GFP_KERNEL);
      memcpy(dev->data->chunk.buffptr, kbuf, count);
    }


    if ('\n' == kbuf[count - 1] )
    {
      aesd_circular_buffer_add_entry(dev->data, &dev->data->chunk);
      *f_pos += dev->data->chunk.size;
      //PDEBUG("write %zu bytes :%s",dev->data->chunk.size,dev->data->chunk.buffptr);
      dev->data->chunk.buffptr = NULL;
      dev->data->chunk.size = 0;
    }

    kfree(kbuf);
    kbuf = NULL;

    mutex_unlock(&dev->lock);
    return count;
}
//size = lseek(*filp, 0, iSEEK_END)
loff_t aesd_llseek(struct file *filp, loff_t off, int whence)
{
  aesd_dev_t *dev = filp->private_data;
  loff_t newpos;

  switch(whence) {
   case 0: /* SEEK_SET */
    newpos = off;
    break;

   case 1: /* SEEK_CUR */
    newpos = filp->f_pos + off;
    break;

   case 2: /* SEEK_END */
    newpos = dev->data->total_size + off;
    break;

   default: /* can't happen */
    return -EINVAL;
  }
  if (newpos<0) return -EINVAL;
  filp->f_pos = newpos;
  return newpos;
}

struct file_operations aesd_fops = {
    .owner =    THIS_MODULE,
    .llseek =   aesd_llseek,
    .read =     aesd_read,
    .write =    aesd_write,
    .open =     aesd_open,
    .release =  aesd_release,
};

static int aesd_setup_cdev(struct aesd_dev *dev)
{
    int err, devno = MKDEV(aesd_major, aesd_minor);

    cdev_init(&dev->cdev, &aesd_fops);
    dev->cdev.owner = THIS_MODULE;
    dev->cdev.ops = &aesd_fops;
    err = cdev_add (&dev->cdev, devno, 1);
    if (err) {
        printk(KERN_ERR "Error %d adding aesd cdev", err);
    }
    return err;
}



int aesd_init_module(void)
{
    dev_t dev = 0;
    int result;
    result = alloc_chrdev_region(&dev, aesd_minor, 1,
            "aesdchar");
    aesd_major = MAJOR(dev);
    if (result < 0) {
        printk(KERN_WARNING "Can't get major %d\n", aesd_major);
        return result;
    }
    memset(&aesd_device, 0, sizeof(struct aesd_dev));

    PDEBUG("Init\n");

    /**
     * TODO: initialize the AESD specific portion of the device
     */
    aesd_circular_buffer_init(&cbuf);
    aesd_device.data = &cbuf;
    //PDEBUG("init aesd_device.data->chunk.buffptr = %p", &aesd_device.data->chunk.buffptr);
    mutex_init(&aesd_device.lock);


/*
 * struct cdev {
        struct kobject kobj;
	struct module *owner;
	const struct file_operations *ops;
	struct list_head list;
	dev_t dev;
	unsigned int count;
} __randomize_layout;
*/
    result = aesd_setup_cdev(&aesd_device);

    if( result ) {
        unregister_chrdev_region(dev, 1);
    }
    return result;

}

void aesd_cleanup_module(void)
{
    dev_t devno = MKDEV(aesd_major, aesd_minor);

    cdev_del(&aesd_device.cdev);

    /**
     * TODO: cleanup AESD specific poritions here as necessary
     */

    mutex_destroy(&aesd_device.lock);



    unregister_chrdev_region(devno, 1);
}



module_init(aesd_init_module);
module_exit(aesd_cleanup_module);
