/*
 *@author: Frederico Gonçalves
 */

#ifdef CONFIG_SYNCH_ADHOC

#include <linux/kernel.h>
#include <linux/if_ether.h> /*ETH_P_IP*/

#include "sync_proto.h"
#include "rt73.h"
#include "mlme.h"
#include "filter_chains.h"

#define SNAP 0xAA

void synch_out_data_packet(struct urb* bulk_out, PRTMP_ADAPTER pad){
  if(ADHOC_ON(pad)){
    uint8_t* raw_packet = (uint8_t*) (bulk_out->transfer_buffer);
    uint32_t raw_packet_len = bulk_out->transfer_buffer_length;
    uint16_t pid;
    uint8_t* raw_llc_header;

    //jump ralink's control data
    HEADER_802_11* _80211 = (HEADER_802_11*) (raw_packet + sizeof(TXD_STRUC)); 

    //Check if logical link control header will have 8 bytes
    raw_llc_header = (uint8_t*) (((uint8_t*) _80211) + sizeof(HEADER_802_11));
    if(raw_llc_header[0] == (uint8_t) SNAP
       &&
       raw_llc_header[1] == (uint8_t) SNAP){

      printk("dest mac: %02X:%02X:%02X:%02X:%02X:%02X\n", _80211->Addr1[0], _80211->Addr1[1], _80211->Addr1[2], _80211->Addr1[3], _80211->Addr1[4], _80211->Addr1[5]);
      printk("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n", _80211->Addr2[0], _80211->Addr2[1], _80211->Addr2[2], _80211->Addr2[3], _80211->Addr2[4], _80211->Addr2[5]);


      memcpy(&pid, raw_llc_header + 6, sizeof(uint16_t));
      if(pid == (uint16_t) ETH_P_IP){
	printk("PACKET IS IP\n");
      }else{
	printk("PACKET IS NOT IP since its type is %02X and eth type is %02X\n", pid, (uint16_t) ETH_P_IP);
      }

    }

  }else
    printk("%s:%d: Cannot synchronize outgoing packet. Board is not in adhoc.\n", __FILE__, __LINE__);
}

#endif
