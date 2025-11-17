#pragma once
#include <graphics.h>
#include <vector>

// 带 alpha 通道的绘制
void putimage_alpha(int x, int y, IMAGE* img);

class Animation
{
public:
    Animation(LPCTSTR path, int num, int interval_ms);
    ~Animation();

    void Play(int x, int y, int delta);

private:
    std::vector<IMAGE*> frame_list;
    int interval_ms = 0;
    int timer = 0;
    int idx_frame = 0;
};
