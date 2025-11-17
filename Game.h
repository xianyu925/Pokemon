// Game.h
#pragma once

#include <vector>
#include <memory>
#include <graphics.h>

#include "Core.h"
#include "Entities.h"
#include "Buttons.h"

// 游戏主类：负责主循环、更新、渲染等
class Game
{
public:
    Game();        // 不再传 choice，使用全局 g_choice
    ~Game();

    int Run();

private:
    // 初始化
    void InitGraphics();
    void InitBGM();
    void InitMenu();
    void InitButtons();
    void InitSceneIfNeeded();

    // 每帧逻辑
    void HandleEvents();
    void UpdateLogic();
    void Render();

    // 子弹/敌人逻辑
    void TryGenerateEnemy();
    void UpdateBulletsOrbit();       // 玩家模式1：环绕子弹
    void UpdateBulletsLine();        // 玩家模式2：射线子弹
    void UpdateBulletsLineBoss();    // Boss 子弹
    bool CheckBossHitPlayer() const; // Boss 子弹是否打到玩家

private:
    // 资源
    IMAGE img_menu;
    IMAGE img_background;

    // 核心对象
    Player    player;
    EnemyBoss boss;

    // 场景实体（用智能指针管理生命周期）
    std::vector<std::unique_ptr<Obstacle>> obstacle_list;
    std::vector<std::unique_ptr<Enemy>> enemy_list;
    std::vector<std::unique_ptr<dropping>> drop_list;

    // 子弹（值类型，不需要指针）
    std::vector<Bullet> bullet_list;
    std::vector<Bullet_line> bullet_line_list;
    std::vector<Bullet_line_boss> bullet_line_boss_list;

    // 所有按钮统一放在一个容器里（智能指针）
	std::vector<std::unique_ptr<Button>> buttons;

    // 事件
    ExMessage msg;

    // 标志位
    bool firstSceneInit;  // 场景初始化（障碍、背景）
    bool firstLevelInit;  // 读档后给角色加等级只执行一次
};

// 对外提供接口，main.cpp 调用这个
int RunGame();
