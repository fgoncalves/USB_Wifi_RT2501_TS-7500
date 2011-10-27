/*
 *@author: Frederico Gonçalves
 */

#ifdef CONFIG_SYNCH_ADHOC

#include <linux/slab.h>
#include <linux/ip.h>
#include <linux/udp.h>
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

/*
 * Try to match registered filters against an udp packet
 */
static uint8_t match_filter_against_udp_ports(struct udphdr* udp, filter* f){
  if(f->dst_port != 0 && udp->dest == f->dst_port)
    return 1;
  if(f->src_port != 0 && udp->source == f->src_port)
    return 1;
  return 0;
}

/*
 * Try to match registered filters against an ip packet
 */
static uint8_t match_filter_against_ip_ips(struct iphdr* ip, filter* f){
  if(f->dst_addr != 0 && ip->daddr == f->dst_addr)
    return 1;
  if(f->src_addr != 0 && ip->saddr == f->src_addr)
    return 1;
  return 0;
}

/*
 * Match filters against ip packet
 */
uint8_t match_filter(struct iphdr* ip, filter* f){
  return ( match_filter_against_ip_ips(ip, f) | match_filter_against_udp_ports((struct udphdr*) (((uint8_t*) ip) + (ip->ihl << 2)), f) );
}

#endif
