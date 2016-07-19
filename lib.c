#include "lib.h"
#include "libavcodec/avcodec.h"  
#include "libavdevice/avdevice.h" 


#pragma warning(disable: 4996)

struct VideoCaptureContext
{
    int RunState;
};

struct VideoCaptureContext* AllocVideoCaptureContext()
{
    struct VideoCaptureContext* pContext = (struct VideoCaptureContext*)malloc(sizeof(struct VideoCaptureContext));
    pContext->RunState = 0;
    return pContext;
}


int StartCaptureVideo(struct VideoCaptureContext* pVideoCapContext, const char* DeviceName, UsrVideoCaptureNotifyFunc UsrFunc, void* pUsr)
{
    AVFormatContext *pFormatCtx;
    int             i, videoindex;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    char            AVFormatUri[512] = { 0 };

    if (pVideoCapContext == NULL)
    {
        printf("VideoCaptureContext is NULL.\n");
        return -1;
    }

    if (UsrFunc == NULL)
    {
        printf("UsrFunc is NULL\n");
        return -1;
    }

    av_register_all();

    pFormatCtx = avformat_alloc_context();

    avdevice_register_all();

    AVDictionary* options = NULL;
    av_dict_set(&options, "video_size", "1920x2160", 0);
    av_dict_set(&options, "rtbufsize", "200M", 0);

    AVInputFormat *ifmt = av_find_input_format("dshow");
    pFormatCtx->video_codec_id = AV_CODEC_ID_H264;

    sprintf(AVFormatUri, "video=%s", DeviceName);

    if (avformat_open_input(&pFormatCtx, AVFormatUri, ifmt, &options) != 0) {
        printf("Couldn't open input stream.\n");
        return -1;
    }
    pFormatCtx->flags |= AVFMT_FLAG_NOBUFFER;
    pFormatCtx->probesize = 4096;

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        printf("Can't find stream information.\n");
        return -1;
    }

    videoindex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++)
        if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoindex = i;
            break;
        }
    if (videoindex == -1)
    {
        printf("Can't find a video stream.\n");
        return -1;
    }
    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        printf("Codec not found.\n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        printf("Can not open codec.\n");
        return -1;
    }
    AVFrame *pFrame;
    pFrame = av_frame_alloc();
 
    int ret, got_picture;

    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));

    pVideoCapContext->RunState = 1;

    while(pVideoCapContext->RunState) {
        //------------------------------  
        if (av_read_frame(pFormatCtx, packet) >= 0) {

            ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
            if (ret < 0) {
                printf("Decode Error.\n");
                continue;
            }
            if (got_picture) {
                UsrFunc(pFrame, pUsr);
            }
            av_free_packet(packet);
        }

    }

    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    printf("Video capture thread quit...");
    return 0;
}

int GetCaptureVideoSize(struct VideoCaptureContext* Context, int* height, int* width)
{
    *height = 2160;
    *width = 1920;
    return 0;
}

int StopCaptureVideo(struct VideoCaptureContext* Context)
{
    if (Context == NULL)
    {
        printf("VideoCaptureContext is NULL.\n");
        return -1;
    }
    Context->RunState = 0;

    return 0;
}

int FreeVideoCaptureContext(struct VideoCaptureContext** Context)
{
    if (Context || *Context)
    {
        printf("VideoCaptureContext is NULL.\n");
        return -1;
    }

    if ((*Context)->RunState != 0)
    {
        printf("VideoCaptureContext thread is running.\n");
        return -1;
    }

    free(*Context);

    *Context = NULL;

    return 0;
}

//------------------------------------------------------------------------------------

struct AudioCaptureContext
{
    int RunState;
};

struct AudioCaptureContext* AllocAudioCaptureContext()
{
    struct AudioCaptureContext* pContext = (struct AudioCaptureContext*)malloc(sizeof(struct AudioCaptureContext));
    pContext->RunState = 0;
    return pContext;
}

int StartCaptureAudio(struct AudioCaptureContext* pAudioCapContext, const char* DeviceName, UsrAudioCaptureNotifyFunc UsrFunc, void* pUsr)
{
    AVFormatContext *pFormatCtx;
    int             videoindex;
    AVCodecContext  *pCodecCtx;
    AVCodec         *pCodec;
    char            AVFormatUri[512] = { 0 };

    if (UsrFunc == NULL)
    {
        printf("UsrFunc is NULL\n");
        return -1;
    }

    av_register_all();
    pFormatCtx = avformat_alloc_context();

    avdevice_register_all();

    AVDictionary* options = NULL;
    av_dict_set(&options, "rtbufsize", "40M", 0);

    AVInputFormat *ifmt = av_find_input_format("dshow");

    sprintf(AVFormatUri, "audio=%s", DeviceName);

    if (avformat_open_input(&pFormatCtx, AVFormatUri, ifmt, NULL) != 0) {
        printf("Couldn't open input stream.\n");
        return -1;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        printf("Couldn't find stream information.\n");
        return -1;
    }
    videoindex = 0;

    pCodecCtx = pFormatCtx->streams[videoindex]->codec;
    pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
    if (pCodec == NULL)
    {
        printf("Codec not found.\n");
        return -1;
    }
    if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0)
    {
        printf("Could not open codec.\n");
        return -1;
    }

    AVPacket *packet = (AVPacket *)av_malloc(sizeof(AVPacket));
 
    pAudioCapContext->RunState = 1;

    while(pAudioCapContext->RunState) {
        if (av_read_frame(pFormatCtx, packet) >= 0) {

            UsrFunc(packet, pUsr);
          
            av_free_packet(packet);
        }
    }

    avcodec_close(pCodecCtx);
    avformat_close_input(&pFormatCtx);

    return 0;
}

int StopCaptureAudio(struct AudioCaptureContext* Context)
{
    if (Context == NULL)
    {
        printf("VideoCaptureContext is NULL.\n");
        return -1;
    }
    Context->RunState = 0;

    return 0;
}

int FreeAudioCaptureContext(struct AudioCaptureContext** Context)
{
    if (Context || *Context)
    {
        printf("VideoCaptureContext is NULL.\n");
        return -1;
    }

    if ((*Context)->RunState != 0)
    {
        printf("VideoCaptureContext thread is running.\n");
        return -1;
    }

    free(*Context);

    *Context = NULL;

    return 0;
}


//FILE* fp = fopen("ttttt11.h264", "ab+");

//             if (0)
//             {
//                 if (packet->data[4] == 0x67)isIFrameBegin = 1;
//                 if (!isIFrameBegin)
//                 {
//                     continue;
//                 }
//                 printf("size=%d, %x,%x,%x,%x,%x\n", packet->size, packet->data[0], packet->data[1],
//                     packet->data[2], packet->data[3], packet->data[4]);
//                 //fwrite(packet->data, packet->size, 1, fp);
//                 //fflush(fp);
//                 fsize += packet->size;
//                 av_free_packet(packet);
//                 continue;
//             }
