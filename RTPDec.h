



#ifndef RTPSERVER_RTPDEC_H
#define RTPSERVER_RTPDEC_H

#include "Network.h"

#define FRAME_PAYLOAD_MAX     1280*1920


struct RTP_Header
{
	uint16_t csrc_count:4;
	uint16_t extension:1;
	uint16_t  padding:1;
	uint16_t  version:2;
	uint16_t  payloadtype:7;	
	uint16_t  marker:1;	

	uint16_t  seq;
	uint32_t  timestamp;
	uint32_t  ssrc;
};


typedef struct {
      //RTP packet = RTP header + buf
    uint8_t buf[FRAME_PAYLOAD_MAX];       // NAL header + NAL
    uint8_t *buf_ptr;  //current buffer

    int aggregation;   // 0: Single Unit, 1: Aggregation Unit
    int payload_type;  // 0, H.264/AVC; 1, HEVC/H.265
    uint32_t ssrc;
    uint32_t seq;
    uint32_t timestamp;
}H264MuxContext;

int initH264MuxContext(H264MuxContext *ctx);

/* send a H.264/HEVC video stream */
void rtpRecvH264HEVC(H264MuxContext *ctx, const uint8_t *buf, int size);

int deencapsulateFU(H264MuxContext *ctx,uint8_t* buf, int len);
void   addDataToEncodedFrame(H264MuxContext *ctx, uint8_t* buf, int insize, uint8_t header,int needStartCode);

#endif //RTPSERVER_RTPENC_H