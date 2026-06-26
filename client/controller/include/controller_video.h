#ifndef CONTROLLER_VIDEO_H
#define CONTROLLER_VIDEO_H

#include "../model/model_video.h"

typedef struct {
    VideoState *state;
    SOCKET      server_sock;
    int         running;
} VideoController;

VideoController *video_controller_create(VideoState *state, SOCKET sock);
void             video_controller_destroy(VideoController *vc);
int              video_controller_start_camera(VideoController *vc,
                                                int device_index);
void             video_controller_stop_camera(VideoController *vc);
void             video_controller_on_frame_recv(VideoController *vc,
                                                 const char *peer_id,
                                                 uint8_t *data,
                                                 uint32_t size);

// Threads internes
DWORD WINAPI capture_loop(LPVOID arg);
DWORD WINAPI recv_loop(LPVOID arg);

#endif