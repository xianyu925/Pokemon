#pragma once
#include <Windows.h>
#include "Animation.h"

class Player;
class Bullet;

// Weapon 抽象基类
class Weapon
{
public:
    virtual ~Weapon() = default;
    virtual void Draw() const = 0;
    virtual POINT GetPosition() const = 0;
    virtual int GetCount() const { return 1; }

    // 每帧更新、越界判断（由具体子类实现）
    virtual void Update() = 0;
    virtual bool IsOutOfBounds() const = 0;
};

// Enemy 抽象基类
class EnemyBase
{
public:
    virtual ~EnemyBase();

    virtual void Move(const Player& player) = 0;
    virtual void Draw(int delta) = 0;
    virtual void Hurt() = 0;
    virtual bool CheckAlive() const = 0;

    virtual bool CheckBulletCollision(const Bullet& bullet) const = 0;
    virtual bool CheckBulletLineCollision(const Weapon& weapon) const;

    // 敌人与玩家的碰撞（默认 false，具体敌人可覆盖）
    virtual bool CheckPlayerCollision(const Player& player) const { return false; }

    const POINT& GetPosition() const { return position; }

protected:
    Animation* anim = nullptr;
    POINT position{ 0, 0 };
    bool  alive = true;

    static constexpr int SPEED = 3;
    static constexpr int FRAME_WIDTH = 50;
    static constexpr int FRAME_HEIGHT = 50;
};
