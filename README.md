# RtpH264Split

#### 介绍
```
本篇通过rtp传输H264实现简单直播功能
```
#### 安装教程
```
1. make
```
#### 使用说明
```
1. 服务端执行：./RTPParser
2. 通过VLC打开：play.sdp
```
#### 实现难点
```
1.如何通过RTP承载H264数据

参考博客：

https://www.jianshu.com/p/efc5ef2113da
https://www.jianshu.com/p/5aa012b76951

参考技术文档：

 https://tools.ietf.org/html/rfc6184

参考实现ffmpeg
libavformat
 --> rtpenc_h264_hevc.c
  --> ff_rtp_send_h264_hevc()
```     
