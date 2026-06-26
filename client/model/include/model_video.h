#ifndef MODEL_VIDEO_H
#define MODEL_VIDEO_H

#include <opencv2/opencv.hpp>
#include <winsock2.h>
#include <windows.h>
#include <stdint.h>

#define MAX_PEERS 8
#define FRAME_MAX_SIZE (1024 * 1024)  // 1MB max par frame compressée

typedef struct {
    uint32_t size;
    uint8_t  data[FRAME_MAX_SIZE];
} VideoFrame;

typedef struct {
    char       peer_id[64];
    VideoFrame last_frame;
    int        active;
} PeerVideo;

typedef struct {
    // Capture locale
    cv::VideoCapture *capture;
    VideoFrame        local_frame;
    int               camera_on;

    // Frames reçues des pairs
    PeerVideo peers[MAX_PEERS];
    int       peer_count;

    // Thread safety
    HANDLE    mutex;
    HANDLE    capture_thread;
    HANDLE    recv_thread;
} VideoState;

VideoState *video_state_create(void);
void        video_state_destroy(VideoState *vs);
int         video_state_set_local_frame(VideoState *vs, VideoFrame *frame);
VideoFrame *video_state_get_local_frame(VideoState *vs);
int         video_state_set_peer_frame(VideoState *vs,
                                        const char *peer_id,
                                        VideoFrame *frame);
VideoFrame *video_state_get_peer_frame(VideoState *vs, const char *peer_id);

#endif