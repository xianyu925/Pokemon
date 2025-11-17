#include "EntityBase.h"

// EnemyBase 通用实现（放在单独 cpp 文件中以减少 Entities.cpp 责任）
EnemyBase::~EnemyBase()
{
    delete anim;
}

bool EnemyBase::CheckBulletLineCollision(const Weapon& weapon) const
{
    POINT wp = weapon.GetPosition();
    bool is_overlap_x = wp.x >= position.x &&
        wp.x <= position.x + FRAME_WIDTH;
    bool is_overlap_y = wp.y >= position.y &&
        wp.y <= position.y + FRAME_HEIGHT;
    return is_overlap_x && is_overlap_y;
}