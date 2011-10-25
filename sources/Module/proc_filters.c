#ifdef CONFIG_SYNCH_ADHOC

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include "proc_filters.h"

#define PROC_FILE_NAME "synch_filters"

struct proc_dir_entry *proc_file_entry;

typedef struct {
	//indicate if the framework has changed and this buffer should be rebuilt.
	uint8_t dirty;
	uint32_t buffer_size;
	uint32_t buffer_offset;
	char* file_buffer;
} proc_file_contents_cache;

proc_file_contents_cache proc_contents;

/*
 *If the buffer is too short for the contents of filters,
 *this function will realloc it to its double.
 */
static void realloc_proc_contents(void) {
  char* old_buff = proc_contents.file_buffer;
  proc_contents.file_buffer = kmalloc(proc_contents.buffer_size << 1,
				      GFP_ATOMIC);
  if (!proc_contents.file_buffer) {
    printk("%s in %s:%d: kmalloc failed. Unnable to create proc entry.\n", __FILE__, __FUNCTION__, __LINE__);
    return;
  }
  proc_contents.buffer_size <<= 1;
  proc_file_entry->size = proc_contents.buffer_size;
  kfree(old_buff);
  memset(proc_contents.file_buffer, 0, proc_contents.buffer_size);
}

//Append the given buffer to the proc entry cache.
static void append_to_buffer(char* buff_to_append, uint32_t len) {
  memcpy(proc_contents.file_buffer + proc_contents.buffer_offset, buff_to_append, len);
  proc_contents.buffer_offset += len;
}

static void fill_proc_contents(void){
  //TODO: Fill buffer with contents from filters
}

static void parse_request(char* buff, unsigned long len){
  //TODO: Register filters
}

static int procfile_read(char *buffer, char **buffer_location, off_t offset, int buffer_length, int *eof, void *data) {
  int how_many_can_we_cpy;

  if (proc_contents.dirty)
    fill_proc_contents();

  how_many_can_we_cpy = (proc_contents.buffer_offset + 1 - offset > buffer_length) ? 
    buffer_length : 
    proc_contents.buffer_offset + 1 - offset;

  if (how_many_can_we_cpy == 0) {
    *eof = 1;
    return 0;
  }

  memcpy(buffer, proc_contents.file_buffer + offset, how_many_can_we_cpy);
  *buffer_location = buffer;
  return how_many_can_we_cpy;
}

#define MAX_REGISTRY_LINE 1024

/*
 *This proc entry accepts writing lines of the form 'PROTO;SRC_IP;DST_IP;SRC_PORT;DST_PORT' all in big endian format.
 */
static int procfile_write(struct file *file, const char *buffer, unsigned long count, void *data) {
  char internal_buffer[MAX_REGISTRY_LINE] = { 0 };

  if (copy_from_user(internal_buffer, buffer, count)) {
    return -EFAULT;
  }

  parse_request(internal_buffer, count);
  return count;
}

static void create_proc_file(void) {
  proc_file_entry = create_proc_entry(PROC_FILE_NAME, 0644, NULL);

  if (proc_file_entry == NULL) {
    printk (KERN_ALERT "Error: Could not initialize /proc/%s\n", PROC_FILE_NAME);
    return;
  }

  proc_file_entry->read_proc = procfile_read;
  proc_file_entry->write_proc = procfile_write;
  proc_file_entry->mode = S_IFREG | S_IRUGO | S_IWUSR | S_IWGRP | S_IWOTH;
  proc_file_entry->uid = 0;
  proc_file_entry->gid = 0;
  proc_file_entry->size = proc_contents.buffer_size;

  memset(&proc_contents, 0, sizeof(proc_file_contents_cache));
  proc_contents.dirty = 1;
  proc_contents.file_buffer = kmalloc(1024, GFP_ATOMIC);
  if (!proc_contents.file_buffer) {
    printk("%s in %s:%d: kmalloc failed. Unnable to create proc entry.\n",  __FILE__, __FUNCTION__, __LINE__);
    return;
  }
  memset(proc_contents.file_buffer, 0, 1024);
}

/*
 * This is called by the filters to notify this proc entry that the cache needs to be
 * rebuilt.
 * */
void mess_proc_entry(void) {
  proc_contents.dirty = 1;
}

void initialize_proc_filters(void){
  create_proc_file();
}

void teardown_proc_filters(void){
  kfree(proc_contents.file_buffer);
  remove_proc_entry(PROC_FILE_NAME, NULL);
}

#endif
