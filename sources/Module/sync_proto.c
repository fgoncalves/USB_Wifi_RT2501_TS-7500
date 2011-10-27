/*
 *@author: Frederico Gonçalves
 */

#ifdef CONFIG_SYNCH_ADHOC

#include <linux/kernel.h>
#include <linux/if_ether.h> /*ETH_P_IP*/
#include <linux/ip.h>
#include <linux/udp.h>

#include "sync_proto.h"
#include "rt73.h"
#include "mlme.h"
#include "filter_chains.h"
#include "miavita_packet.h"

#define SNAP 0xAA

#define udph_from_iph(ip)					\
  ((struct udphdr*) (((uint8_t*) (ip)) + ((ip)->ihl << 2))

#define application_payload_from_iph(ip)				\
  ((packet_t*) (((uint8_t*) (ip)) + ((ip)->ihl << 2) + sizeof(struct udphdr))

void dump_data_to_syslog(char* label, uint8_t* data, uint32_t len){
  uint32_t i;
  printk("START DUMP %s\n", label);
  for(i = 0; i < len; i++)
    printk("%02X ", data[i]);
  printk("END DUMP %s\n", label);
}

void synch_out_data_packet(struct urb* bulk_out, PRTMP_ADAPTER pad){
  if(ADHOC_ON(pad)){
    uint8_t* raw_packet = (uint8_t*) (bulk_out->transfer_buffer);
    uint32_t raw_packet_len = bulk_out->transfer_buffer_length;
    uint16_t pid;
    uint8_t* raw_llc_header;
    struct iphdr* ip;
    struct udphdr* udp; 
    packet_t* pdu;

    //jump ralink's control data
    HEADER_802_11* _80211 = (HEADER_802_11*) (raw_packet + sizeof(TXD_STRUC)); 

    //Check if logical link control header will have 8 bytes
    raw_llc_header = (uint8_t*) (((uint8_t*) _80211) + sizeof(HEADER_802_11));

    dump_data_to_syslog("LLC HEADER", raw_llc_header, 8);

    if(raw_llc_header[0] == (uint8_t) SNAP
       &&
       raw_llc_header[1] == (uint8_t) SNAP){

      printk("dest mac: %02X:%02X:%02X:%02X:%02X:%02X\n", _80211->Addr1[0], _80211->Addr1[1], _80211->Addr1[2], _80211->Addr1[3], _80211->Addr1[4], _80211->Addr1[5]);
      printk("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n", _80211->Addr2[0], _80211->Addr2[1], _80211->Addr2[2], _80211->Addr2[3], _80211->Addr2[4], _80211->Addr2[5]);

      memcpy(&pid, raw_llc_header + 6, sizeof(uint16_t)); //This is giving me pid = 8 instead of pid = 800. I don't suppose it's byte order, there's something else going on here.
      if(pid == (uint16_t) ETH_P_IP){
	printk("PACKET IS IP\n");
	ip = (struct iphdr*) (raw_llc_header + 8);
	udp = udph_from_iph(ip);
	pdu = application_payload_from_iph(ip);

	dump_data_to_syslog("IP HEADER", ip, ip->ihl << 2);
	dump_data_to_syslog("UDP HEADER", udph, ntohs(udp->len));
	dump_data_to_syslog("PDU HEADER", pdu, sizeof(packet_t));

	//TODO: match ip, match udp, time it!
	//Don't forget endianess and Checksums
      }else{
	printk("PACKET IS NOT IP since its type is %02X and eth type is %02X\n", pid, (uint16_t) ETH_P_IP);
      }

    }

  }else
    printk("%s:%d: Cannot synchronize outgoing packet. Board is not in adhoc.\n", __FILE__, __LINE__);
}

#endif
