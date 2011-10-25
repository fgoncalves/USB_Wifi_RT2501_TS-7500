/*
 *@author: Frederico Gonçalves
 */

#ifdef CONFIG_SYNCH_ADHOC

#include <linux/slab.h>
#include "filter_chains.h"

void register_filter(uint8_t proto, uint32_t dst_addr, uint32_t src_addr, uint16_t dst_port, uint16_t src_port){
  filter* f = kmalloc(sizeof(filter), GFP_ATOMIC);
  if(!f){
    printk(KERN_EMERG "%s:%d: kmalloc failed.\n", __FILE__, __LINE__);
    return;
  }

  f->proto = proto;
  f->dst_addr = dst_addr;
  f->src_addr = src_addr;
  f->dst_port = dst_port;
  f->src_port = src_port;

  chains[nfilters++] = f;
}

void unregister_filter(uint8_t index){
  if(!chains[index])
    return;

  kfree(chains[index]);

  nfilters--;
  memmove(&(chains[index]), &(chains[index + 1]), nfilters - index);
}

#endif
