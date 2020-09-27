////

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "Utils.h"
#include "RTPEnc.h"
#include "Network.h"

#include"RTPDec.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <netinet/in.h>

UDPContext udpContext = {
        .dstIp = "192.168.23.40",   // destination ip
        .dstPort = 1234         // destination port
};
UDPContext udpContext1= {
        .dstIp = "192.168.23.40",   // destination ip
        .dstPort = 5678         // destination port
};

H264MuxContext  gh264MuxContext;


int * thread_recvd(void * arg)
{
    int sock_fd;
    char rcv_buff[2048];
    struct sockaddr_in client_addr;
    struct sockaddr_in server_addr;
    int client_len;
    int rcv_num = -1;

    H264MuxContext * ph264MuxContext = NULL;

	pthread_t newthid;
	newthid = pthread_self();
	printf("this is a new thread, thread ID = %d\n", newthid);

    if ((sock_fd = socket(AF_INET, SOCK_DGRAM,0)) < 0)
    {
        perror("socket create error\n");
        exit(1);
    }
    memset(&server_addr,0,sizeof(struct sockaddr_in));

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(udpContext1.dstPort);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    client_len = sizeof(struct sockaddr_in);

    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr_in)) < 0)
    {
        perror("bind error.\n");
        exit(1);
    }
    // if(NULL == ph264MuxContext){

    //     printf("rrrrrrrrr____size:%d\r\n",sizeof(H264MuxContext));
    //     ph264MuxContext =(H264MuxContext*) (sizeof(H264MuxContext));
    // }

     initH264MuxContext(&gh264MuxContext);

     printf("rrrrrrrrr\r\n");


    while (1)
    {
        rcv_num= recvfrom(sock_fd, rcv_buff, sizeof(rcv_buff), 0, (struct sockaddr*)&client_addr, &client_len);
        if (rcv_num>0)
        {
            rcv_buff[rcv_num] = '\0';
            printf("%s %u says: len:%d \n",inet_ntoa(client_addr.sin_addr),ntohs(client_addr.sin_port),rcv_num);
            rtpRecvH264HEVC(&gh264MuxContext,rcv_buff,rcv_num);
        }
        else
        {
            perror("recv error\n");
            break;
        }
    }
    close(sock_fd);
    if(NULL == ph264MuxContext){
        free(ph264MuxContext);
    }
    return 0;
}
int main() {

    int len = 0;
    int res;
    uint8_t *stream = NULL;
    const char *fileName = "./test.h264";

    pthread_t thid;
    RTPMuxContext rtpMuxContext;

    //recv--thread
    if(pthread_create(&thid, NULL, (void *)thread_recvd, NULL) != 0) {
		printf("thread creation failed\n");
		exit(1);
    }


    res = readFile(&stream, &len, fileName);
    if (res){
        printf("readFile error.\n");
        return -1;
    }

    // create udp socket
    res = udpInit(&udpContext);
    if (res){
        printf("udpInit error.\n");
        return -1;
    }
     res = udpInit(&udpContext1);
    if (res){
        printf("udpInit error.\n");
        return -1;
    }

    initRTPMuxContext(&rtpMuxContext);
    rtpSendH264HEVC(&rtpMuxContext, &udpContext,&udpContext1, stream, len);

    free(stream);

    return 0;
}