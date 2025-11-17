#include "Entities.h"
#include "Core.h"
#include <algorithm>
#include <cmath>

// ===== Obstacle =====
Obstacle::Obstacle(bool isBoss, int pos_x, int pos_y) : anim(nullptr)
{
    position.x = pos_x;
    position.y = pos_y;
    if (!isBoss)
        anim = new Animation(_T("images/obstacle.png"), 1, 45);
    else
        anim = new Animation(_T("images/obstacle_boss.png"), 1, 45);
}

Obstacle::~Obstacle()
{
    delete anim;
}

void Obstacle::Draw(int delta)
{
    anim->Play(position.x, position.y, delta);
}

// ===== Player =====
Player::Player(int choice) : anim_left(nullptr), anim_right(nullptr)
{
    if (choice == 1)
    {
        anim_left = new Animation(_T("images/player_left.png"), 6, 45);
        anim_right = new Animation(_T("images/player_right.png"), 6, 45);
    }
    else if (choice == 2)
    {
        anim_left = new Animation(_T("images/player_two_left.png"), 6, 45);
        anim_right = new Animation(_T("images/player_two_right.png"), 6, 45);
    }
}

Player::~Player()
{
    delete anim_left;
    delete anim_right;
}

void Player::ProcessEvent(const ExMessage& msg)
{
    switch (msg.message)
    {
    case WM_KEYDOWN:
        switch (msg.vkcode)
        {
        case VK_UP:    is_move_up = true; break;
        case VK_DOWN:  is_move_down = true; break;
        case VK_LEFT:  is_move_left = true; break;
        case VK_RIGHT: is_move_right = true; break;
        }
        break;
    case WM_KEYUP:
        switch (msg.vkcode)
        {
        case VK_UP:    is_move_up = false; break;
        case VK_DOWN:  is_move_down = false; break;
        case VK_LEFT:  is_move_left = false; break;
        case VK_RIGHT: is_move_right = false; break;
        }
        break;
    }
}

const POINT& Player::GetPosition() const
{
    return player_pos;
}

void Player::Draw(int delta)
{
    int dir_x = is_move_right - is_move_left;
    static bool facing_left = false;

    if (dir_x < 0)      facing_left = true;
    else if (dir_x > 0) facing_left = false;

    if (facing_left)
        anim_left->Play(player_pos.x, player_pos.y, delta);
    else
        anim_right->Play(player_pos.x, player_pos.y, delta);
}

int Player::CheckMove(const Obstacle& obstacle, POINT new_pos)
{
    RECT player_rect{
        new_pos.x,
        new_pos.y,
        new_pos.x + FRAME_WIDTH,
        new_pos.y + FRAME_HEIGHT
    };
    RECT obstacle_rect{
        obstacle.position.x,
        obstacle.position.y,
        obstacle.position.x + obstacle.WIDTH,
        obstacle.position.y + obstacle.HEIGHT
    };

    if (player_rect.right <= obstacle_rect.left ||
        player_rect.left >= obstacle_rect.right ||
        player_rect.bottom <= obstacle_rect.top ||
        player_rect.top >= obstacle_rect.bottom)
    {
        return 3; // 无碰撞
    }

    int overlap_left = max(player_rect.left, obstacle_rect.left);
    int overlap_right = min(player_rect.right, obstacle_rect.right);
    int overlap_top = max(player_rect.top, obstacle_rect.top);
    int overlap_bottom = min(player_rect.bottom, obstacle_rect.bottom);

    int overlap_width = overlap_right - overlap_left;
    int overlap_height = overlap_bottom - overlap_top;

    if (overlap_width > 0 && overlap_height > 0)
    {
        if (overlap_height > overlap_width)
            return 1; // 只能 Y 方向移动
        else
            return 2; // 只能 X 方向移动
    }
    return 0;
}

void Player::Move(std::vector<std::unique_ptr<Obstacle>>& obstacles)
{
    int dir_x = is_move_right - is_move_left;
    int dir_y = is_move_down - is_move_up;
    double len_dir = std::sqrt(dir_x * dir_x + dir_y * dir_y);

    if (len_dir != 0)
    {
        POINT new_pos = player_pos;
        double nx = dir_x / len_dir;
        double ny = dir_y / len_dir;
        new_pos.x += static_cast<int>(SPEED * nx);
        new_pos.y += static_cast<int>(SPEED * ny);

        bool can_move_x = true;
        bool can_move_y = true;

        for (auto& obs : obstacles)
        {
            int result = CheckMove(*obs, new_pos);
            if (result == 0) {
                can_move_x = false;
                can_move_y = false;
                break;
            }
            else if (result == 1) {
                can_move_x = false;
            }
            else if (result == 2) {
                can_move_y = false;
            }
        }

        if (can_move_x && can_move_y)
            player_pos = new_pos;
        else if (can_move_x)
            player_pos.x = new_pos.x;
        else if (can_move_y)
            player_pos.y = new_pos.y;
    }

    if (player_pos.x < 0) player_pos.x = 0;
    if (player_pos.y < 0) player_pos.y = 0;
    if (player_pos.x + FRAME_WIDTH > WINDOW_WIDTH)
        player_pos.x = WINDOW_WIDTH - FRAME_WIDTH;
    if (player_pos.y + FRAME_HEIGHT > WINDOW_HEIGHT)
        player_pos.y = WINDOW_HEIGHT - FRAME_HEIGHT;
}

void Player::Hurt()
{
    if (!wudi)
    {
        HP--;
        wudi = true;
    }
    if (HP <= 0)
        alive = false;
}

bool Player::CheckAlive() const
{
    return alive;
}

int Player::GetLevel() const
{
    return level;
}

void Player::level_up()
{
    level++;
    if (HP <= 4) HP++;
}

int Player::GetHP() const
{
    return HP;
}

void Player::UpdataWudi()
{
    if (wudi)
        wudi_count++;
    if (wudi_count >= 80)
    {
        wudi = false;
        wudi_count = 0;
    }
}

void Player::DrawHurt()
{
    if (wudi)
    {
        TCHAR text[64];
        _stprintf_s(text, _T("Hurt!"));
        setbkmode(TRANSPARENT);
        settextcolor(RGB(255, 0, 0));
        outtextxy(player_pos.x + 25, player_pos.y - 20, text);
    }
}

// ===== Bullet =====
void Bullet::Draw() const
{
    setlinecolor(RGB(0, 178, 238));
    setfillcolor(RGB(135, 206, 235));
    fillcircle(position.x, position.y, RADIUS);
}

// ===== Weapon implementations (Bullet_line / Bullet_line_boss) =====
Bullet_line::Bullet_line(int count, int idx, POINT pos)
{
    bullet_count = count;
    radian_interval = 2 * 3.14159 / bullet_count;
    angle = radian_interval * idx;
    position.x = pos.x + 40;
    position.y = pos.y + 40;
}

void Bullet_line::Draw() const
{
    setlinecolor(RGB(0, 0, 0));
    setfillcolor(RGB(0, 0, 0));
    fillcircle(position.x, position.y, RADIUS);
}

void Bullet_line::Update()
{
    position.x += static_cast<int>(SPEED * cos(angle));
    position.y += static_cast<int>(SPEED * sin(angle));
}

bool Bullet_line::IsOutOfBounds() const
{
    return position.x < 0 || position.x >= WINDOW_WIDTH ||
           position.y < 0 || position.y >= WINDOW_HEIGHT;
}

Bullet_line_boss::Bullet_line_boss(int count, int idx, POINT pos)
{
    bullet_count = count;
    radian_interval = 2 * 3.14159 / bullet_count;
    angle = radian_interval * idx;
    position.x = pos.x + 40;
    position.y = pos.y + 40;
}

void Bullet_line_boss::Draw() const
{
    setlinecolor(RGB(0, 128, 0));
    setfillcolor(RGB(0, 128, 0));
    fillcircle(position.x, position.y, RADIUS);
}

void Bullet_line_boss::Update()
{
    position.x += static_cast<int>(SPEED * cos(angle));
    position.y += static_cast<int>(SPEED * sin(angle));
}

bool Bullet_line_boss::IsOutOfBounds() const
{
    return position.x < 0 || position.x >= WINDOW_WIDTH ||
           position.y < 0 || position.y >= WINDOW_HEIGHT;
}

// ===== Enemy (具体实现) =====
Enemy::Enemy()
{
    anim = new Animation(_T("images/enemy_%d.png"), 6, 45);

    enum class SpawnEdge { Up = 0, Down, Left, Right };
    SpawnEdge edge = static_cast<SpawnEdge>(rand() % 4);

    switch (edge)
    {
    case SpawnEdge::Up:
        position.x = rand() % WINDOW_WIDTH;
        position.y = -FRAME_HEIGHT;
        break;
    case SpawnEdge::Down:
        position.x = rand() % WINDOW_WIDTH;
        position.y = WINDOW_HEIGHT;
        break;
    case SpawnEdge::Left:
        position.x = -FRAME_WIDTH;
        position.y = rand() % WINDOW_HEIGHT;
        break;
    case SpawnEdge::Right:
        position.x = WINDOW_WIDTH;
        position.y = rand() % WINDOW_HEIGHT;
        break;
    }
}

Enemy::~Enemy()
{
    // EnemyBase 的析构负责删除 anim
}

void Enemy::Move(const Player& player)
{
    const POINT& player_pos = player.GetPosition();
    int dir_x = player_pos.x - position.x;
    int dir_y = player_pos.y - position.y;
    double len_dir = std::sqrt(dir_x * dir_x + dir_y * dir_y);
    if (len_dir != 0)
    {
        double nx = dir_x / len_dir;
        double ny = dir_y / len_dir;
        position.x += static_cast<int>(SPEED * nx);
        position.y += static_cast<int>(SPEED * ny);
    }
}

void Enemy::Draw(int delta)
{
    anim->Play(position.x, position.y, delta);
}

void Enemy::Hurt()
{
    alive = false;
}

bool Enemy::CheckAlive() const
{
    return alive;
}

bool Enemy::CheckBulletCollision(const Bullet& bullet) const
{
    bool is_overlap_x = bullet.position.x >= position.x &&
        bullet.position.x <= position.x + FRAME_WIDTH;
    bool is_overlap_y = bullet.position.y >= position.y &&
        bullet.position.y <= position.y + FRAME_HEIGHT;
    return is_overlap_x && is_overlap_y;
}

bool Enemy::CheckBulletLineCollision(const Weapon& weapon) const
{
    return EnemyBase::CheckBulletLineCollision(weapon);
}

bool Enemy::CheckPlayerCollision(const Player& player) const
{
    POINT check_position{ position.x + FRAME_WIDTH / 3,
                           position.y + FRAME_HEIGHT / 3 };
    POINT player_position = player.GetPosition();

    bool is_overlap_x = check_position.x >= player_position.x &&
        check_position.x <= player_position.x + Player::FRAME_WIDTH;
    bool is_overlap_y = check_position.y >= player_position.y &&
        check_position.y <= player_position.y + Player::FRAME_HEIGHT;
    return is_overlap_x && is_overlap_y;
}

// ===== EnemyBoss =====
EnemyBoss::EnemyBoss()
{
    anim = new Animation(_T("images/enemy_boss.png"), 1, 45);
}

EnemyBoss::~EnemyBoss()
{
    // EnemyBase 的析构负责删除 anim
}

void EnemyBoss::Move()
{
    if (down)  position.y += SPEED;
    if (right) position.x += SPEED;
    if (up)    position.y -= SPEED;
    if (left)  position.x -= SPEED;

    if (position.x < 0) {
        position.x = 0; left = false; down = true;
    }
    if (position.y < 0) {
        position.y = 0; up = false; left = true;
    }
    if (position.x + WIDTH > WINDOW_WIDTH) {
        position.x = WINDOW_WIDTH - WIDTH; right = false; up = true;
    }
    if (position.y + HEIGHT > WINDOW_HEIGHT) {
        position.y = WINDOW_HEIGHT - HEIGHT; down = false; right = true;
    }
}

void EnemyBoss::Draw(int delta)
{
    anim->Play(position.x, position.y, delta);
}

bool EnemyBoss::CheckBulletCollision(const Bullet& bullet) const
{
    bool is_overlap_x = bullet.position.x >= position.x &&
        bullet.position.x <= position.x + WIDTH;
    bool is_overlap_y = bullet.position.y >= position.y &&
        bullet.position.y <= position.y + HEIGHT;
    return is_overlap_x && is_overlap_y;
}

// 覆盖 Weapon 版本，使用 boss 自己的宽高
bool EnemyBoss::CheckBulletLineCollision(const Weapon& weapon) const
{
    POINT wp = weapon.GetPosition();
    bool is_overlap_x = wp.x >= position.x &&
        wp.x <= position.x + WIDTH;
    bool is_overlap_y = wp.y >= position.y &&
        wp.y <= position.y + HEIGHT;
    return is_overlap_x && is_overlap_y;
}

void EnemyBoss::Hurt()
{
    if (!wudi)
    {
        HP--;
        wudi = true;
    }
    if (HP <= 0)
        alive = false;
}

bool EnemyBoss::CheckAlive() const
{
    return alive;
}

void EnemyBoss::UpdataWudi()
{
    if (wudi)
        wudi_count++;
    if (wudi_count >= 80)
    {
        wudi = false;
        wudi_count = 0;
    }
}

void EnemyBoss::DrawHurt()
{
    if (wudi)
    {
        TCHAR text[64];
        _stprintf_s(text, _T("HIT!"));
        setbkmode(TRANSPARENT);
        settextcolor(RGB(255, 0, 0));
        outtextxy(position.x + 45, position.y - 20, text);
    }
}

// ===== dropping =====
dropping::dropping(POINT pos)
{
    anim = new Animation(_T("images/yuanshi.png"), 6, 45);
    position = pos;
}

dropping::~dropping()
{
    delete anim;
}

void dropping::Draw(int delta)
{
    anim->Play(position.x, position.y, delta);
}

bool dropping::CheckPlayerCollision(const Player& player) const
{
    POINT check_position{ position.x + 25, position.y + 25 };
    POINT player_position = player.GetPosition();

    bool is_overlap_x = check_position.x >= player_position.x &&
        check_position.x <= player_position.x + Player::FRAME_WIDTH;
    bool is_overlap_y = check_position.y >= player_position.y &&
        check_position.y <= player_position.y + Player::FRAME_HEIGHT;
    return is_overlap_x && is_overlap_y;
}
