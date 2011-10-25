/*
 *@author: Frederico Gonçalves
 */

#ifndef __SYNCH_PROTO_H__
#define __SYNCH_PROTO_H__

#ifdef CONFIG_SYNCH_ADHOC

#include <linux/kernel.h>

typedef struct{
  uint8_t proto;
  uint32_t dst_addr;
  uint32_t src_addr;
  uint16_t dst_port;
  uint16_t src_port;
}filter;

#define MAX_FILTERS 256

filter* chains[MAX_FILTERS] = {0};
uint8_t nfilters = 0;

extern void register_filter(uint8_t proto, uint32_t dst_addr, uint32_t src_addr, uint16_t dst_port, uint16_t src_port);
extern void unregister_filter(uint8_t index);

#endif

#endif
