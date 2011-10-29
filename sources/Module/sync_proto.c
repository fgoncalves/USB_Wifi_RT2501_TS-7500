/*
 *@author: Frederico Gonçalves
 */

#ifdef CONFIG_SYNCH_ADHOC

#include <linux/kernel.h>
#include <linux/if_ether.h> /*ETH_P_IP*/
#include <linux/ip.h>
#include <linux/in.h>
#include <linux/udp.h>
#include <linux/byteorder/generic.h>

#include "sync_proto.h"
#include "rt73.h"
#include "mlme.h"
#include "filter_chains.h"
#include "miavita_packet.h"

#define SNAP 0xAA
#define SEC_2_NSEC 1000000000L
#define USEC_2_NSEC 1000L

#define udph_from_iph(ip)					\
  ((struct udphdr*) (((uint8_t*) (ip)) + ((ip)->ihl << 2)))

#define application_payload_from_iph(ip)				\
  ((packet_t*) (((uint8_t*) (ip)) + ((ip)->ihl << 2) + sizeof(struct udphdr)))

int64_t get_kernel_current_time(void) {
  struct timeval t;
  memset(&t, 0, sizeof(struct timeval));
  do_gettimeofday(&t);
  return ((int64_t) t.tv_sec) * SEC_2_NSEC + ((int64_t) t.tv_usec)
    * USEC_2_NSEC;
}

void dump_data_to_syslog(char* label, uint8_t* data, uint32_t len){
  uint32_t i;
  printk("START DUMP %s\n", label);
  for(i = 0; i < len; i++){
    if(i != 0 && i % 7 == 0)
      printk("\n");
    printk("%02X ", data[i]);
  }
  printk("\nEND DUMP %s\n", label);
}

void synch_out_data_packet(struct urb* bulk_out, PRTMP_ADAPTER pad){
  if(ADHOC_ON(pad)){
    uint8_t* raw_packet = (uint8_t*) (bulk_out->transfer_buffer);
    uint32_t raw_packet_len = bulk_out->transfer_buffer_length, filter_it;
    uint16_t pid;
    uint8_t* raw_llc_header;
    struct iphdr* ip;
    struct udphdr* udp; 
    packet_t* pdu;
    int64_t incomming_ts;

    //jump ralink's control data
    HEADER_802_11* _80211 = (HEADER_802_11*) (raw_packet + sizeof(TXD_STRUC)); 

    //Check if logical link control header will have 8 bytes
    raw_llc_header = (uint8_t*) (((uint8_t*) _80211) + sizeof(HEADER_802_11));

    dump_data_to_syslog("LLC HEADER", raw_llc_header, 8);

    if(raw_llc_header[0] == (uint8_t) SNAP
       &&
       raw_llc_header[1] == (uint8_t) SNAP){

      #ifdef DBG
      printk("dest mac: %02X:%02X:%02X:%02X:%02X:%02X\n", _80211->Addr1[0], _80211->Addr1[1], _80211->Addr1[2], _80211->Addr1[3], _80211->Addr1[4], _80211->Addr1[5]);
      printk("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n", _80211->Addr2[0], _80211->Addr2[1], _80211->Addr2[2], _80211->Addr2[3], _80211->Addr2[4], _80211->Addr2[5]);
      #endif

      memcpy(&pid, raw_llc_header + 6, sizeof(uint16_t)); 
      pid = ntohs(pid);
      if(pid == (uint16_t) ETH_P_IP){
	ip = (struct iphdr*) (raw_llc_header + 8);
	//udp = udph_from_iph(ip);
	udp = (struct udphdr*) (((char*) ip) + (ip->ihl << 2));
	pdu = application_payload_from_iph(ip);
	
	#ifdef DBG
	dump_data_to_syslog("IP HEADER", (uint8_t*) ip, ip->ihl << 2);
	#endif

	if(ip->protocol == IPPROTO_UDP){
	  #ifdef DBG
	  dump_data_to_syslog("UDP HEADER", (uint8_t*) udp, ntohs(udp->len));
	  dump_data_to_syslog("PDU HEADER", (uint8_t*) pdu, sizeof(packet_t));
	  #endif

	  for(filter_it = 0; filter_it < MAX_FILTERS; filter_it++)
	    if(match_filter(ip, chains[filter_it])){
	      incomming_ts = be64_to_cpu(pdu->timestamp);	      
	      pdu->timestamp = get_kernel_current_time() - incoming_ts;
	      pdu->timestamp = cpu_to_be64(pdu->timestamp);
	      printk("%s:%d: TODO: replace this with proper checksumming.", __FILE__, __LINE__);
	      break;
	    }
	}
      }
    }

  }else
    printk("%s:%d: Cannot synchronize outgoing packet. Board is not in adhoc.\n", __FILE__, __LINE__);
}

#endif
