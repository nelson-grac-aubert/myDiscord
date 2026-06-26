#include "model_video.h"
#include <stdlib.h>
#include <string.h>

VideoState *video_state_create(void) {
    VideoState *vs = (VideoState *)calloc(1, sizeof(VideoState));
    if (!vs) return NULL;
    vs->capture = NULL;
    vs->camera_on = 0;
    vs->peer_count = 0;
    vs->mutex = CreateMutex(NULL, FALSE, NULL);
    return vs;
}

void video_state_destroy(VideoState *vs) {
    if (!vs) return;
    if (vs->capture) {
        vs->capture->release();
        delete vs->capture;
    }
    CloseHandle(vs->mutex);
    free(vs);
}

int video_state_set_local_frame(VideoState *vs, VideoFrame *frame) {
    WaitForSingleObject(vs->mutex, INFINITE);
    memcpy(&vs->local_frame, frame, sizeof(VideoFrame));
    ReleaseMutex(vs->mutex);
    return 0;
}

VideoFrame *video_state_get_local_frame(VideoState *vs) {
    return &vs->local_frame;  // appelant doit prendre le mutex
}

int video_state_set_peer_frame(VideoState *vs,
                                const char *peer_id,
                                VideoFrame *frame) {
    WaitForSingleObject(vs->mutex, INFINITE);
    for (int i = 0; i < MAX_PEERS; i++) {
        if (!vs->peers[i].active ||
            strcmp(vs->peers[i].peer_id, peer_id) == 0) {
            strncpy(vs->peers[i].peer_id, peer_id, 63);
            memcpy(&vs->peers[i].last_frame, frame, sizeof(VideoFrame));
            vs->peers[i].active = 1;
            if (i >= vs->peer_count) vs->peer_count = i + 1;
            ReleaseMutex(vs->mutex);
            return 0;
        }
    }
    ReleaseMutex(vs->mutex);
    return -1;
}