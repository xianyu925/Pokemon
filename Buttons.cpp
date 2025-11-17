#include "Buttons.h"
#include "Core.h"
#include <mmsystem.h>
#include <Windows.h>
#include <iostream>

#pragma comment(lib,"Winmm.lib")

Button::Button(RECT rect, LPCTSTR path_idle, LPCTSTR path_hover, LPCTSTR path_pushed)
{
    region = rect;
    loadimage(&img_idle, path_idle);
    loadimage(&img_hovered, path_hover);
    loadimage(&img_pushed, path_pushed);
}

bool Button::CheckCursorHit(int x, int y)
{
    return x >= region.left && x <= region.right &&
        y >= region.top && y <= region.bottom;
}

void Button::Draw()
{
    switch (status)
    {
    case Status::Idle:
        putimage(region.left, region.top, &img_idle);
        break;
    case Status::Hovered:
        putimage(region.left, region.top, &img_hovered);
        break;
    case Status::Pushed:
        putimage(region.left, region.top, &img_pushed);
        break;
    }
}

void Button::ProcessEvent(const ExMessage& msg)
{
    switch (msg.message)
    {
    case WM_MOUSEMOVE:
        if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
            status = Status::Hovered;
        else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
            status = Status::Idle;
        break;
    case WM_LBUTTONDOWN:
        if (CheckCursorHit(msg.x, msg.y))
            status = Status::Pushed;
        break;
    case WM_LBUTTONUP:
        if (CheckCursorHit(msg.x, msg.y))
            OnClick();
        status = Status::Idle;
        break;
    default:
        break;
    }
}

// 子类 OnClick 实现
void StartGameButton::OnClick()
{
    g_state.isGameStarted = true;
    g_state.isBoss = false;
    mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);
}

void QuitGameButton::OnClick()
{
    g_state.running = false;
}

void StartBossButton::OnClick()
{
    g_state.isGameStarted = true;
    g_state.isBoss = true;
    mciSendString(_T("play exaid_bgm repeat from 0"), NULL, 0, NULL);
}

void ResetButton::OnClick()
{
    mciSendString(_T("play hit from 0"), NULL, 0, NULL);
    g_state.gold = 100;
    g_state.getLevel = 0;

    if (WriteSaveFile(g_saveFile, g_state.gold, g_state.getLevel))
        std::cout << "游戏进度已重置!\n";
    else
        std::cerr << "错误: 无法重置游戏进度\n";

    TCHAR text[128];
    _stprintf_s(text, _T("游戏进度已重置!看我一命通关！"));
    MessageBox(GetHWnd(), text, _T("世上有后悔药吗？也许吧"), MB_OK);
}

void FreeButton::OnClick()
{
    TCHAR text[128];
    _stprintf_s(text, _T("都说不要碰我啦\\(/ _ \\)/！"));
    MessageBox(GetHWnd(), text, _T("想休息\\(/ _ \\)/"), MB_OK);
}

void BuildUpButton::OnClick()
{
    if (g_state.gold >= 50)
    {
        mciSendString(_T("play hit from 0"), NULL, 0, NULL);
        g_state.getLevel += 1;
        g_state.gold -= 50;
    }
    else
    {
        TCHAR text[128];
        _stprintf_s(text, _T("提醒：您的创世结晶数量不足，升级失败"));
        MessageBox(GetHWnd(), text, _T("Sorry"), MB_OK);
    }

    if (WriteSaveFile(g_saveFile, g_state.gold, g_state.getLevel))
        std::cout << "游戏进度已保存!\n";
    else
        std::cerr << "错误: 无法保存游戏进度\n";
}
