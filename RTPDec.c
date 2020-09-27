#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "Utils.h"

#include"RTPDec.h"


#define H264_NAL_TYPE_NON_IDR_SLICE 1
#define H264_NAL_TYPE_DP_A_SLICE 2
#define H264_NAL_TYPE_DP_B_SLICE 3
#define H264_NAL_TYPE_DP_C_SLICE 0x4
#define H264_NAL_TYPE_IDR_SLICE 0x5
#define H264_NAL_TYPE_SEI 0x6
#define H264_NAL_TYPE_SEQ_PARAM 0x7
#define H264_NAL_TYPE_PIC_PARAM 0x8
#define H264_NAL_TYPE_ACCESS_UNIT 0x9
#define H264_NAL_TYPE_END_OF_SEQ 0xa
#define H264_NAL_TYPE_END_OF_STREAM 0xb
#define H264_NAL_TYPE_FILLER_DATA 0xc
#define H264_NAL_TYPE_SEQ_EXTENSION 0xd

FILE * p264File = NULL;

int initH264MuxContext(H264MuxContext *ctx)
{

    //printf("TTTTTTTTTTTTTTTTTTTTT %p\r\n",ctx);
    ctx->seq = 0;
    //printf("TTTTTT33333TTTTTTT\r\n");

    ctx->timestamp = 0;
    ctx->ssrc = 0x12345678; // random number
    ctx->aggregation = 0;   // use Aggregation Unit
    ctx->buf_ptr = ctx->buf;
    ctx->payload_type = 0;  // 0, H.264/AVC; 1, HEVC/H.265
    return 0;
}


void   addDataToEncodedFrame(H264MuxContext *ctx, uint8_t* buf, int insize, uint8_t header,int needStartCode)
{
    if(needStartCode ==1 ){
        printf("IN addDataToEncodedFrame :%d\r\n",header&0x1f);
        Load32(ctx->buf_ptr,0x00000001);//验证
        ctx->buf_ptr += 4;

        Load8(ctx->buf_ptr,header) ;
        //Load8(ctx->buf_ptr,header & 0x1f) ;
        ctx->buf_ptr +=1;
    }
    memcpy(ctx->buf_ptr,buf,insize);
    ctx->buf_ptr += insize;
}

void rtpRecvH264HEVC(H264MuxContext *ctx, const uint8_t *buf, int size)
{
    struct RTP_Header * pRTPHeader = (struct RTP_Header*)buf;

    uint8_t curNALType = *((uint8_t*)buf+12) & 0x1f;
    int  boGetFrame  = 0;
    int  boStatFrame = 0;

    if (curNALType >= H264_NAL_TYPE_NON_IDR_SLICE &&
      curNALType <= H264_NAL_TYPE_FILLER_DATA)
    {
        printf("======siggle unit  len:%d , curNALType:%d\r\n",size,curNALType);

        addDataToEncodedFrame(ctx,buf+12+1,size -13,buf[12],1);

        boGetFrame = 1;

        //找到开头的帧数据
        // boStatFrame  =1;
        // Load32(ctx->buf_ptr,0x00000001);
        // ctx->buf_ptr+=4;
        // memcpy(ctx->buf_ptr,buf+1,size-1);

    } 
    else if (curNALType == 24) 
    {
        printf("======stap unit  len:%d \r\n",size);

        int  curSTAPLen = size - 12 -1;
        uint8_t* curSTAP  = buf +13;

        while (curSTAPLen > 0){
            uint32_t len = (curSTAP[0] << 8) | curSTAP[1];
            curSTAP +=2;
            printf("stap unit  len:%d \r\n",len);

            addDataToEncodedFrame(ctx,curSTAP+1,len - 1,*curSTAP,1);
            curSTAP += len;

            if ((len + 2) > curSTAPLen){
                printf("error  may be \r\n");
            }else{
                curSTAPLen -= (len + 2);
            }
        }
        boGetFrame = 1;

        //failed
    }
    else if (curNALType == 28) 
    {
        printf("======FU unit  len:%d \r\n",size);

        //Load8 ( ctx->buf_ptr, buf[12] & 0xE0 | buf[13] &0x1F);
        //ctx->buf_ptr += 1;
        //memcpy(ctx->buf_ptr,buf+2,size-2);


        // get result end  

        boGetFrame =  deencapsulateFU(ctx,buf+12,size -12);

        // Fragmentation Units
        //return DeencapsulateFU (frame, flags);
    }

    printf("NALTYPE:%d, PRT_Header pt:%d,mark:%d,\r\n",curNALType,pRTPHeader->payloadtype,pRTPHeader->marker);

    if(1 == boGetFrame ){
        if(NULL == p264File){
            p264File = fopen("tmp.h264","wb+");
        }
        if(p264File){
            fwrite(ctx->buf,(int)(ctx->buf_ptr - ctx->buf),1,p264File);
        }

        initH264MuxContext(ctx);

    }


    //get nal  and save  file
}

int deencapsulateFU(H264MuxContext *ctx,uint8_t* buf, int len) 
{
    int real_end =0;
    uint8_t* curFUPtr = buf;
    uint32_t curFULen = len; 
    uint8_t header;

    if ((curFUPtr[1] & 0x80) && !(curFUPtr[1] & 0x40))
    {
        //start packet
        header = (curFUPtr[0] & 0xe0) | (curFUPtr[1] & 0x1f);
        addDataToEncodedFrame(ctx,buf+2,len -2,header,1);
    }   else if (!(curFUPtr[1] & 0x80) && !(curFUPtr[1] & 0x40)){
        
        addDataToEncodedFrame(ctx,buf+2,len -2,  0, 0);

    }  else if (!(curFUPtr[1] & 0x80) && (curFUPtr[1] & 0x40)){
        addDataToEncodedFrame(ctx,buf+2,len -2,  0, 0);
        real_end =1;
    } else if ((curFUPtr[1] & 0x80) && (curFUPtr[1] & 0x40)){
        printf("error is not well \r\n");
    }

    return real_end;
}