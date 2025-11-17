#pragma once
#include <vector>
#include <memory>
#include <Windows.h>
#include "Animation.h"
#include "EntityBase.h" // <- 新增：基类放到单独文件

class Player;
class Bullet;

// ========== Obstacle ==========
class Obstacle
{
public:
    Obstacle(bool isBoss, int pos_x, int pos_y);
    ~Obstacle();

    void Draw(int delta);
    POINT position{ 0, 0 };
    int WIDTH = 50;
    int HEIGHT = 50;

private:
    Animation* anim = nullptr;
};

// ========== Player ==========
class Player
{
public:
    Player(int choice);
    ~Player();

    void ProcessEvent(const ExMessage& msg);
    void Move(std::vector<std::unique_ptr<Obstacle>>& obstacles);
    void Draw(int delta);

    void Hurt();
    bool CheckAlive() const;
    void UpdataWudi();
    void DrawHurt();

    const POINT& GetPosition() const;
    int CheckMove(const Obstacle& obstacle, POINT new_pos);

    int GetLevel() const;
    void level_up();
    int GetHP() const;

    static constexpr int SPEED = 5;
    static constexpr int FRAME_WIDTH = 50;
    static constexpr int FRAME_HEIGHT = 50;

private:
    Animation* anim_left = nullptr;
    Animation* anim_right = nullptr;
    POINT player_pos{ 0, 0 };
    bool is_move_up = false;
    bool is_move_down = false;
    bool is_move_left = false;
    bool is_move_right = false;
    int HP = 5;
    int level = 1;
    bool alive = true;
    bool wudi = false;
    int wudi_count = 0;
};

// ========== Bullet ==========
class Bullet
{
public:
    POINT position{ 0, 0 };
    void Draw() const;

private:
    static constexpr int RADIUS = 8;
};

// 具体武器：Bullet_line（仍在 Entities.h 中声明，继承 Weapon）
class Bullet_line : public Weapon
{
public:
    POINT position{ 0, 0 };
    int bullet_count = 3;
    double radian_interval = 0.0;
    double angle = 0.0;

    Bullet_line(int count, int idx, POINT pos);
    Bullet_line() = default;

    void Draw() const override;
    int  GetCount() const override { return bullet_count; }
    POINT GetPosition() const override { return position; }

    void Update() override;
    bool IsOutOfBounds() const override;

private:
    static const int RADIUS = 8;
    static constexpr double SPEED = 8.0;
};

class Bullet_line_boss : public Weapon
{
public:
    POINT position{ 0, 0 };
    int bullet_count = 3;
    double radian_interval = 0.0;
    double angle = 0.0;

    Bullet_line_boss(int count, int idx, POINT pos);
    Bullet_line_boss() = default;

    void Draw() const override;
    int  GetCount() const override { return bullet_count; }
    POINT GetPosition() const override { return position; }

    void Update() override;
    bool IsOutOfBounds() const override;

private:
    static const int RADIUS = 8;
    static constexpr double SPEED = 8.0;
};

// ========== Enemy (继承 EnemyBase) ==========
class Enemy : public EnemyBase
{
public:
    Enemy();
    ~Enemy();

    void Move(const Player& player) override;
    void Draw(int delta) override;

    void Hurt() override;
    bool CheckAlive() const override;

    bool CheckBulletCollision(const Bullet& bullet) const override;
    bool CheckBulletLineCollision(const Weapon& weapon) const override;

    bool CheckPlayerCollision(const Player& player) const override;
};

// ========== EnemyBoss ==========
class EnemyBoss : public EnemyBase
{
public:
    EnemyBoss();
    ~EnemyBoss();

    void Move(const Player& player) override { Move(); } // 兼容旧签名
    void Move(); // 原来无参
    void Draw(int delta) override;

    bool CheckBulletCollision(const Bullet& bullet) const override;
    bool CheckBulletLineCollision(const Weapon& weapon) const override;

    void Hurt() override;
    bool CheckAlive() const override;

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
    bool alive_local = true;
};

// ========== dropping ==========
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
