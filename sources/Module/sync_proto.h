/*
 *@author: Frederico Gonçalves
 */

#ifndef __SYNCH_PROTO_H__
#define __SYNCH_PROTO_H__

#ifdef CONFIG_SYNCH_ADHOC

#include <linux/usb.h>

#include "rt_config.h"
#include "rtmp.h" /*PRTMP_ADAPTER*/

extern void synch_out_data_packet(struct urb* bulk_out, PRTMP_ADAPTER pad);
#endif

#endif
