#include "controller_video.h"
#include "../view/ui_video.h"
#include "../network/network_video.h"
#include <stdlib.h>
#include <stdio.h>

VideoController *video_controller_create(VideoState *state, SOCKET sock) {
    VideoController *vc = (VideoController *)calloc(1, sizeof(VideoController));
    if (!vc) return NULL;
    vc->state = state;
    vc->server_sock = sock;
    vc->running = 0;
    return vc;
}

void video_controller_destroy(VideoController *vc) {
    if (!vc) return;
    vc->running = 0;
    free(vc);
}

int video_controller_start_camera(VideoController *vc, int device_index) {
    vc->state->capture = new cv::VideoCapture(device_index);
    if (!vc->state->capture->isOpened()) {
        fprintf(stderr, "[VIDEO] Impossible d'ouvrir la caméra %d\n",
                device_index);
        return -1;
    }
    vc->state->capture->set(cv::CAP_PROP_FRAME_WIDTH,  640);
    vc->state->capture->set(cv::CAP_PROP_FRAME_HEIGHT, 480);
    vc->state->capture->set(cv::CAP_PROP_FPS, 30);
    vc->state->camera_on = 1;
    vc->running = 1;

    vc->state->capture_thread =
        CreateThread(NULL, 0, capture_loop, vc, 0, NULL);
    vc->state->recv_thread =
        CreateThread(NULL, 0, recv_loop, vc, 0, NULL);
    return 0;
}

void video_controller_stop_camera(VideoController *vc) {
    vc->running = 0;
    vc->state->camera_on = 0;
    WaitForSingleObject(vc->state->capture_thread, 2000);
    WaitForSingleObject(vc->state->recv_thread, 2000);
}

// Thread : capture + envoi
DWORD WINAPI capture_loop(LPVOID arg) {
    VideoController *vc = (VideoController *)arg;
    cv::Mat frame;
    std::vector<uint8_t> buf;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 70};

    while (vc->running) {
        *vc->state->capture >> frame;
        if (frame.empty()) continue;

        // Compression JPEG
        cv::imencode(".jpg", frame, buf, params);

        VideoFrame vf;
        vf.size = (uint32_t)buf.size();
        memcpy(vf.data, buf.data(), vf.size);

        // Mise à jour modèle (frame locale)
        video_state_set_local_frame(vc->state, &vf);

        // Envoi au serveur
        network_send_frame(vc->server_sock, &vf);

        // Affichage local
        ui_video_render_local(frame);

        cv::waitKey(1);
    }
    return 0;
}

// Thread : réception des pairs
DWORD WINAPI recv_loop(LPVOID arg) {
    VideoController *vc = (VideoController *)arg;

    while (vc->running) {
        char peer_id[64];
        VideoFrame vf;

        if (network_recv_frame(vc->server_sock, peer_id, &vf) == 0) {
            video_controller_on_frame_recv(vc, peer_id, vf.data, vf.size);
        }
        Sleep(1);
    }
    return 0;
}

void video_controller_on_frame_recv(VideoController *vc,
                                     const char *peer_id,
                                     uint8_t *data,
                                     uint32_t size) {
    // Décompression
    std::vector<uint8_t> buf(data, data + size);
    cv::Mat frame = cv::imdecode(buf, cv::IMREAD_COLOR);
    if (frame.empty()) return;

    // Mise à jour modèle
    VideoFrame vf;
    vf.size = size;
    memcpy(vf.data, data, size);
    video_state_set_peer_frame(vc->state, peer_id, &vf);

    // Affichage
    ui_video_render_peer(peer_id, frame);
}