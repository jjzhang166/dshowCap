#pragma once

#include "libavformat/avformat.h"  



//----------------------------------------------------------------------------------------------
typedef int(*UsrVideoCaptureNotifyFunc)(AVFrame *pFrame, void* pUsr);
struct VideoCaptureContext;

struct VideoCaptureContext* AllocVideoCaptureContext();
int StartCaptureVideo(struct VideoCaptureContext* Context, const char* DeviceName, UsrVideoCaptureNotifyFunc UsrFunc, void* pUsr);
int StopCaptureVideo(struct VideoCaptureContext* Context);
int FreeVideoCaptureContext(struct VideoCaptureContext** Context);
int GetCaptureVideoSize(struct VideoCaptureContext* Context, int* height, int* width);
//----------------------------------------------------------------------------------------------

typedef int(*UsrAudioCaptureNotifyFunc)(AVPacket *pFrame, void* pUsr);
struct AudioCaptureContext;

struct AudioCaptureContext* AllocAudioCaptureContext();
int StartCaptureAudio(struct AudioCaptureContext* Context, const char* DeviceName, UsrAudioCaptureNotifyFunc UsrFunc, void* pUsr);
int StopCaptureAudio(struct AudioCaptureContext* Context);
int FreeAudioCaptureContext(struct AudioCaptureContext** Context);
