#pragma once
#include <memory>
#include <cmath>
#include "Core.h"
#include "Animation.h"

// ========== 障碍物 ==========
class Obstacle
{
public:
    Obstacle(bool isBoss, int pos_x = 200, int pos_y = 200);
    ~Obstacle();

    void Draw(int delta);

    POINT position{ 200, 200 };
    int WIDTH = 100;
    int HEIGHT = 100;

private:
    Animation* anim = nullptr;
};

// 前置声明
//class Obstacle;
//class Player;

// ========== 玩家 ==========
class Player
{
public:
    Player(int choice = 1);
    ~Player();

    void ProcessEvent(const ExMessage& msg);
    void Draw(int delta);

    const POINT& GetPosition() const;
    void Move(std::vector<std::unique_ptr<Obstacle>>& obstacles);

    void Hurt();
    bool CheckAlive() const;

    int  GetLevel() const;
    void level_up();

    int  GetHP() const;

    void UpdataWudi();
    void DrawHurt();

    static constexpr int SPEED = 5;
    static constexpr int FRAME_WIDTH = 80;
    static constexpr int FRAME_HEIGHT = 80;

private:
    int CheckMove(const Obstacle& obstacle, POINT new_pos);

private:
    Animation* anim_left = nullptr;
    Animation* anim_right = nullptr;

    POINT player_pos{ 500, 500 };
    bool is_move_up = false;
    bool is_move_down = false;
    bool is_move_left = false;
    bool is_move_right = false;

    int  level = 1;
    int  HP = 2;
    bool alive = true;

    bool wudi = false;
    int  wudi_count = 0;
};

// ========== 子弹 ==========
class Bullet
{
public:
    POINT position{ 0, 0 };
    void Draw() const;
private:
    static constexpr int RADIUS = 8;
};

class Bullet_line
{
public:
    POINT position{ 0, 0 };
    int bullet_count = 3;
    double radian_interval = 0.0;
    double angle = 0.0;

    Bullet_line(int count, int idx, POINT pos);
    Bullet_line() = default;

    void Draw() const;
    int  GetCount() const { return bullet_count; }

private:
    static const int RADIUS = 8;
};

class Bullet_line_boss
{
public:
    POINT position{ 0, 0 };
    int bullet_count = 3;
    double radian_interval = 0.0;
    double angle = 0.0;

    Bullet_line_boss(int count, int idx, POINT pos);
    Bullet_line_boss() = default;

    void Draw() const;
    int  GetCount() const { return bullet_count; }

private:
    static const int RADIUS = 8;
};

// ========== 敌人 ==========
class Enemy
{
public:
    Enemy();
    ~Enemy();

    void Move(const Player& player);
    void Draw(int delta);

    void Hurt();
    bool CheckAlive() const;

    bool CheckBulletCollision(const Bullet& bullet) const;
    bool CheckBulletLineCollision(const Bullet_line& bullet) const;
    bool CheckPlayerCollision(const Player& player) const;

    const POINT& GetPosition() const { return position; }

    static constexpr int SPEED = 3;
    static constexpr int FRAME_WIDTH = 50;
    static constexpr int FRAME_HEIGHT = 50;

private:
    Animation* anim = nullptr;
    POINT position{ 0, 0 };
    bool  alive = true;
};

// ========== Boss ==========
class EnemyBoss
{
public:
    EnemyBoss();
    ~EnemyBoss();

    void Move();
    void Draw(int delta);

    bool CheckBulletCollision(const Bullet& bullet) const;
    bool CheckBulletLineCollision(const Bullet_line& bullet) const;

    void Hurt();
    bool CheckAlive() const;

    void UpdataWudi();
    void DrawHurt();

    int  GetHP() const { return HP; }
    POINT GetPosition() const { return position; }

    int SPEED = 5;
    int WIDTH = 100;
    int HEIGHT = 100;

    bool wudi = false;
    int  wudi_count = 0;

    bool down = true;
    bool right = false;
    bool up = false;
    bool left = false;

private:
    int  HP = 35;
    bool alive = true;
    POINT position{ 0, 0 };
    Animation* anim = nullptr;
};

// ========== 掉落物 ==========
class dropping
{
public:
    dropping(POINT pos);
    ~dropping();

    void Draw(int delta);
    POINT GetPosition() const { return position; }

    bool CheckPlayerCollision(const Player& player) const;
    void pick_up() { alive = false; }
    bool CheckAlive() const { return alive; }

private:
    Animation* anim = nullptr;
    bool alive = true;
    POINT position{ 0, 0 };
};
