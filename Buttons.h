#pragma once
#include <graphics.h>
#include "Core.h"

// 基类按钮
class Button
{
public:
    Button(RECT rect, LPCTSTR path_idle, LPCTSTR path_hover, LPCTSTR path_pushed);
    virtual ~Button() = default;

    void Draw();
    void ProcessEvent(const ExMessage& msg);

protected:
    virtual void OnClick() = 0;

private:
    enum class Status { Idle = 0, Hovered, Pushed };

    RECT region;
    IMAGE img_idle;
    IMAGE img_hovered;
    IMAGE img_pushed;
    Status status = Status::Idle;

    bool CheckCursorHit(int x, int y);
};

// 子类按钮
class StartGameButton : public Button
{
public:
    using Button::Button;
protected:
    void OnClick() override;
};

class QuitGameButton : public Button
{
public:
    using Button::Button;
protected:
    void OnClick() override;
};

class StartBossButton : public Button
{
public:
    using Button::Button;
protected:
    void OnClick() override;
};

class ResetButton : public Button
{
public:
    using Button::Button;
protected:
    void OnClick() override;
};

class FreeButton : public Button
{
public:
    using Button::Button;
protected:
    void OnClick() override;
};

class BuildUpButton : public Button
{
public:
    using Button::Button;
protected:
    void OnClick() override;
};
