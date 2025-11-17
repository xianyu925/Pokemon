#include "Animation.h"
#include <Windows.h>

#pragma comment(lib,"MSIMG32.LIB")

void putimage_alpha(int x, int y, IMAGE* img)
{
    int w = img->getwidth();
    int h = img->getheight();

    BLENDFUNCTION bf;
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = 255;
	bf.AlphaFormat = AC_SRC_ALPHA;

    AlphaBlend(GetImageHDC(NULL), x, y, w, h,
        GetImageHDC(img), 0, 0, w, h, bf);
}

Animation::Animation(LPCTSTR path, int num, int interval)
{
    interval_ms = interval;
    TCHAR path_file[256];
    for (int i = 0; i < num; ++i)
    {
        _stprintf_s(path_file, path, i);
        IMAGE* frame = new IMAGE();
        loadimage(frame, path_file);
        frame_list.push_back(frame);
    }
}

Animation::~Animation()
{
    for (auto f : frame_list) {
        delete f;
    }
}

void Animation::Play(int x, int y, int delta)
{
    timer += delta;
    if (timer >= interval_ms) {
        idx_frame = (idx_frame + 1) % frame_list.size();
        timer = 0;
    }
    putimage_alpha(x, y, frame_list[idx_frame]);
}
