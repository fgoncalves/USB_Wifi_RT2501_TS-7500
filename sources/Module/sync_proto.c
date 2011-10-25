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

    //jump ralink's control data
    HEADER_802_11* _80211 = (HEADER_802_11*) (raw_packet + sizeof(TXD_STRUC)); 

    //Check if logical link control header will have 8 bytes
    if(*(((uint8_t*) _80211) + sizeof(HEADER_802_11)) == (uint8_t) SNAP
       &&
       *(((uint8_t*) _80211) + sizeof(HEADER_802_11) + 1) == (uint8_t) SNAP){

      printk("dest mac: %02X:%02X:%02X:%02X:%02X:%02X\n", _80211->Addr1[0], _80211->Addr1[1], _80211->Addr1[2], _80211->Addr1[3], _80211->Addr1[4], _80211->Addr1[5]);
      printk("source mac: %02X:%02X:%02X:%02X:%02X:%02X\n", _80211->Addr2[0], _80211->Addr2[1], _80211->Addr2[2], _80211->Addr2[3], _80211->Addr2[4], _80211->Addr2[5]);


      pid = *((uint16_t*) (((uint8_t*) _80211) + sizeof(HEADER_802_11) + 6));
      if(pid == (uint16_t) ETH_P_IP){
	printk("PACKET IS IP\n");
      }

    }

  }else
    printk("%s:%d: Cannot synchronize outgoing packet. Board is not in adhoc.\n", __FILE__, __LINE__);
}

#endif