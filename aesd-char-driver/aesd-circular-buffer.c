﻿/**
 * @file aesd-circular-buffer.c
 * @brief Functions and data related to a circular buffer imlementation
 *
 * @author Dan Walkes
 * @date 2020-03-01
 * @copyright Copyright (c) 2020
 *
 */

#ifdef __KERNEL__
  #include <linux/string.h>
  #include <linux/module.h>
  #include <linux/init.h>
  #include <linux/printk.h>
  #include <linux/types.h>
  #include <linux/cdev.h>
  #include <linux/fs.h> // file_operations
  #include <linux/slab.h>		// kmalloc()
#else
  #include <string.h>
  #include <stdio.h>
#endif

#include "aesd-circular-buffer.h"
#include "aesdchar.h"


int aesd_circular_buffer_set_write_off(struct aesd_circular_buffer *buffer, int write_cmd, int write_cmd_offset)
{
  if (write_cmd >= AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
    return -1;

  buffer->write_cmd = write_cmd;
  buffer->write_cmd_offset = write_cmd_offset;
  buffer->buf_offset = 0;
  return 0;
}


/**
 * @param buffer the buffer to search for corresponding offset.  Any necessary locking must be performed by caller.
 * @param char_offset the position to search for in the buffer list, describing the zero referenced
 *      character index if all buffer strings were concatenated end to end
 * @param entry_offset_byte_rtn is a pointer specifying a location to store the byte of the returned aesd_buffer_entry
 *      buffptr member corresponding to char_offset.  This value is only set when a matching char_offset is found
 *      in aesd_buffer.
 * @return the struct aesd_buffer_entry structure representing the position described by char_offset, or
 * NULL if this position is not available in the buffer (not enough data is written).
 */
struct aesd_buffer_entry *aesd_circular_buffer_find_entry_offset_for_fpos(struct aesd_circular_buffer *buffer,
            size_t char_offset, size_t *entry_offset_byte_rtn )
{
  uint8_t offs = buffer->in_offs;
  if (char_offset >= buffer->total_size)
    return NULL;

  while (char_offset >= buffer->entry[offs].size)
  {
    char_offset -= buffer->entry[offs].size;
    offs++;
    if (offs >= AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
      offs = 0;
  }
  *entry_offset_byte_rtn = char_offset;
  return (struct aesd_buffer_entry *)buffer->entry + offs;
}

//--------------------------------------------------------------------------------------------------

ssize_t aesd_circular_buffer_allread ( struct aesd_circular_buffer *buffer, char * const buf)
{
  uint8_t offs = buffer->out_offs;
  int offset = 0;
  ssize_t size_exp = 0;

/*
  offs += buffer->write_cmd;
  if (offs >= AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
    offs -= AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED;

  //buffer->write_cmd_offset
*/
   //offset = buffer->write_cmd_offset;
  do {

    memcpy (buf + size_exp, buffer->entry[offs].buffptr, buffer->entry[offs].size);
    size_exp += buffer->entry[offs].size;

    if(++offset <= buffer->write_cmd)
    {
      PDEBUG("offset = %u, write_cmd = %u, buf_offset = %u", offset, buffer->write_cmd, buffer->buf_offset);
      if (0 != buffer->write_cmd)
        buffer->buf_offset += buffer->entry[offs].size;
      //offset++;

    }
    offs++;
    if (offs >= AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
      offs = 0;
  } while (buffer->in_offs != offs);

  buffer->buf_offset += buffer->write_cmd_offset;

  //PDEBUG("buffer->total_size = %zu, size_exp = %zu", buffer->total_size, size_exp);

  //if (buffer->total_size != size_exp)
  //  return -1;
  //return buffer->total_size;
  return size_exp;
}

/**
* Adds entry @param add_entry to @param buffer in the location specified in buffer->in_offs.
* If the buffer was already full, overwrites the oldest entry and advances buffer->out_offs to the
* new start location.
* Any necessary locking must be handled by the caller
* Any memory referenced in @param add_entry must be allocated by and/or must have a lifetime managed by the caller.
*/
void aesd_circular_buffer_add_entry(struct aesd_circular_buffer *buffer, const struct aesd_buffer_entry *add_entry)
{
  //clean previous
  if (NULL != buffer->entry[buffer->in_offs].buffptr)
  {
    kfree (buffer->entry[buffer->in_offs].buffptr);
    buffer->entry[buffer->in_offs].buffptr = NULL;
  }
  buffer->total_size-= buffer->entry[buffer->in_offs].size;
  memcpy (&buffer->entry[buffer->in_offs], add_entry, sizeof (struct aesd_buffer_entry));
  buffer->total_size+= buffer->entry[buffer->in_offs].size;

  buffer->in_offs++;

  if (buffer->in_offs >= AESDCHAR_MAX_WRITE_OPERATIONS_SUPPORTED)
  {
    buffer->in_offs = 0;
    PDEBUG ("overwrap\n");
  }

  if (1 == buffer->full)
  {
    buffer->out_offs = buffer->in_offs;
  }


  if (buffer->in_offs == buffer->out_offs)
  {
    buffer->full = 1;
    //PDEBUG ("buffer->full = %d\n", buffer->full);
  }
}

/**
* Initializes the circular buffer described by @param buffer to an empty struct
*/
void aesd_circular_buffer_init(struct aesd_circular_buffer *buffer)
{
    memset(buffer,0,sizeof(struct aesd_circular_buffer));
}
