#include "../include/ui_video.h"
#include <stdio.h>

#define LOCAL_WIN  "Ma caméra"

void ui_video_render_local(cv::Mat frame) {
    cv::imshow(LOCAL_WIN, frame);
}

void ui_video_render_peer(const char *peer_id, cv::Mat frame) {
    char win_name[128];
    snprintf(win_name, sizeof(win_name), "Caméra - %s", peer_id);
    cv::imshow(win_name, frame);
}

void ui_video_update(void) {
    cv::waitKey(1);  // Nécessaire pour rafraîchir les fenêtres OpenCV
}

void ui_video_close_all(void) {
    cv::destroyAllWindows();
}