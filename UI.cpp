#include "UI.h"
#include <graphics.h>

void DrawPlayerScore(int score)
{
    TCHAR text[64];
    _stprintf_s(text, _T("你的得分为：%d"), score);
    setbkmode(TRANSPARENT);
    settextcolor(RGB(255, 85, 185));
    outtextxy(10, 30, text);
}

void DrawPlayerLevel(int level)
{
    TCHAR text[64];
    _stprintf_s(text, _T("你的等级为：%d"), level);
    setbkmode(TRANSPARENT);
    settextcolor(RGB(255, 85, 185));
    outtextxy(10, 50, text);
}

void DrawPlayerHP(int hp)
{
    settextcolor(RED);
    setbkmode(TRANSPARENT);
    outtextxy(10, 10, _T("你的HP为："));

    setfillcolor(RED);

    const int radius = 10;
    const int spacing = 5;
    const int startX = 120;
    const int centerY = 12;

    for (int i = 0; i < hp; ++i) {
        int centerX = startX + i * (2 * radius + spacing);
        solidcircle(centerX, centerY, radius);
    }
}

void DrawBossHP(int hp)
{
    TCHAR text[64];
    _stprintf_s(text, _T("Boss剩余血量：%d"), hp);
    setbkmode(TRANSPARENT);
    settextcolor(BLACK);
    outtextxy(1140, 10, text);
}

void DrawGetLevel(int getLevel)
{
    TCHAR text[64];
    _stprintf_s(text, _T("角色的初始等级为：%d"), getLevel + 1);
    setbkmode(TRANSPARENT);
    settextcolor(BLUE);
    outtextxy(10, 30, text);
}

void DrawGold(int gold)
{
    TCHAR text[64];
    _stprintf_s(text, _T("您拥有的创世结晶数为：%d"), gold);
    setbkmode(TRANSPARENT);
    settextcolor(BLUE);
    outtextxy(10, 10, text);
}
