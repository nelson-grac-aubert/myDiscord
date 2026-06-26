#ifndef UI_VIDEO_H
#define UI_VIDEO_H

#include <opencv2/opencv.hpp>

void ui_video_render_local(cv::Mat frame);
void ui_video_render_peer(const char *peer_id, cv::Mat frame);
void ui_video_update(void);
void ui_video_close_all(void);

#endif