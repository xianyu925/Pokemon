#include "Game.h"

#include <Windows.h>
#include <mmsystem.h>
#include <ctime>
#include <vector>
#include <iostream>

#pragma comment(lib,"Winmm.lib")

#include "Core.h"
#include "Animation.h"
#include "Entities.h"
#include "Buttons.h"
#include "UI.h"

Game::Game()
    : player(g_state.choice)
    , boss()
    , firstSceneInit(true)
    , firstLevelInit(true)
{
    InitGraphics();
    InitBGM();
	InitMenu();
	InitButtons();

    // 模式1环绕子弹初始数量
    bullet_list.resize(3);
}

Game::~Game()
{
	// uniqure_ptr 会自动释放资源
	EndBatchDraw();
	closegraph();
}

// =========================== 初始化相关 ===========================

void Game::InitGraphics()
{
    initgraph(WINDOW_WIDTH, WINDOW_HEIGHT);
    HWND hWnd = GetHWnd();
	SetWindowText(hWnd, L"POKEMON");
	BeginBatchDraw();
}

void Game::InitBGM()
{
    mciSendString(_T("open BGM/exaid_bgm.mp3 alias exaid_bgm"), NULL, 0, NULL);
    mciSendString(_T("open BGM/bgm.mp3 alias bgm"), NULL, 0, NULL);
    mciSendString(_T("open BGM/hit.wav alias hit"), NULL, 0, NULL);
}

void Game::InitMenu()
{
    loadimage(&img_menu, _T("images/menu.png"));
}

void Game::InitButtons()
{
	RECT region_btn_start_game, region_btn_quit_game, region_btn_boss_game;
	RECT region_btn_build_up, region_btn_reset, region_btn_free;

	region_btn_start_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_start_game.right = (WINDOW_WIDTH + BUTTON_WIDTH) / 2;
    region_btn_start_game.top = 430;
	region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

	region_btn_quit_game.left = (WINDOW_WIDTH - BUTTON_WIDTH) / 2;
	region_btn_quit_game.right = (WINDOW_WIDTH + BUTTON_WIDTH) / 2;
	region_btn_quit_game.top = 550;
	region_btn_quit_game.bottom = region_btn_quit_game.top + BUTTON_HEIGHT;

    region_btn_boss_game.left = 800;
	region_btn_boss_game.right = region_btn_boss_game.left + BUTTON_WIDTH;
	region_btn_boss_game.top = 550;
	region_btn_boss_game.bottom = region_btn_boss_game.top + BUTTON_HEIGHT;

    region_btn_build_up.left = 300;
	region_btn_build_up.right = region_btn_build_up.left + BUTTON_WIDTH;
	region_btn_build_up.top = 550;
	region_btn_build_up.bottom = region_btn_build_up.top + BUTTON_HEIGHT;

    region_btn_reset.left = 300;
    region_btn_reset.right = region_btn_reset.left + BUTTON_WIDTH;
    region_btn_reset.top = 430;
    region_btn_reset.bottom = region_btn_reset.top + BUTTON_HEIGHT;

    region_btn_free.left = 800;
    region_btn_free.right = region_btn_free.left + BUTTON_WIDTH;
    region_btn_free.top = 430;
    region_btn_free.bottom = region_btn_free.top + BUTTON_HEIGHT;

    buttons.push_back(std::make_unique<StartGameButton>(
        region_btn_start_game,
        _T("images/ui_start_idle.png"),
        _T("images/ui_start_hovered.png"),
        _T("images/ui_start_pushed.png")));

    buttons.push_back(std::make_unique<QuitGameButton>(
        region_btn_quit_game,
        _T("images/ui_quit_idle.png"),
        _T("images/ui_quit_hovered.png"),
        _T("images/ui_quit_pushed.png")));

    buttons.push_back(std::make_unique<StartBossButton>(
        region_btn_boss_game,
        _T("images/boss_id.png"),
        _T("images/boss_hovered.png"),
        _T("images/boss_pushed.png")));

    buttons.push_back(std::make_unique<BuildUpButton>(
        region_btn_build_up,
        _T("images/build_id.png"),
        _T("images/build_hovered.png"),
        _T("images/build_pushed.png")));

    buttons.push_back(std::make_unique<ResetButton>(
        region_btn_reset,
        _T("images/Reset_id.png"),
        _T("images/Reset_hovered.png"),
        _T("images/Reset_pushed.png")));

    buttons.push_back(std::make_unique<FreeButton>(
        region_btn_free,
        _T("images/free_id.png"),
        _T("images/free_hovered.png"),
        _T("images/free_pushed.png")));
}

void Game::InitSceneIfNeeded()
{
    if (!firstSceneInit) return;

    if(!g_state.isBoss)
		loadimage(&img_background, _T("images/mainscene.png"));
    else
		loadimage(&img_background, _T("images/mainscene_boss.png"));

    obstacle_list.push_back(std::make_unique<Obstacle>(g_state.isBoss, 700, 360));
    obstacle_list.push_back(std::make_unique<Obstacle>(g_state.isBoss, 400, 200));
    obstacle_list.push_back(std::make_unique<Obstacle>(g_state.isBoss, 200, 100));
    obstacle_list.push_back(std::make_unique<Obstacle>(g_state.isBoss, 900, 600));
    obstacle_list.push_back(std::make_unique<Obstacle>(g_state.isBoss, 600, 600));

	firstSceneInit = false;
}

// ============================ 事件处理 ============================

void Game::HandleEvents()
{
    while (peekmessage(&msg))
    {
        if (g_state.isGameStarted)
        {
            player.ProcessEvent(msg);
        }
        else
        {
			for (auto& btn : buttons)
            {
                btn->ProcessEvent(msg);
            }
        }
    }
}

// ========== 辅助函数：敌人 & 子弹逻辑（集中在 Game.cpp） ==========

void Game::TryGenerateEnemy()
{
    const int INTERVAL = 100;
    static int counter = 0;
    if ((++counter) % INTERVAL == 0)
    {
        enemy_list.push_back(std::make_unique<Enemy>());
    }
}

void Game::UpdateBulletsOrbit()
{
    if (bullet_list.empty()) return;

    const double RADIAL_SPEED = 0.0045;
    const double TANGENT_SPEED = 0.0055;
    double radian_interval = 2 * 3.14159 / bullet_list.size();

    POINT player_position = player.GetPosition();
    double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);

    for (size_t i = 0; i < bullet_list.size(); ++i)
    {
        double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;
        bullet_list[i].position.x = player_position.x + Player::FRAME_WIDTH / 2 +
            static_cast<int>(radius * cos(radian));
        bullet_list[i].position.y = player_position.y + Player::FRAME_HEIGHT / 2 +
            static_cast<int>(radius * sin(radian));
    }
}

void Game::UpdateBulletsLine()
{
    for (auto it = bullet_line_list.begin(); it != bullet_line_list.end(); )
    {
        if (it->position.x < 0 || it->position.x >= WINDOW_WIDTH ||
            it->position.y < 0 || it->position.y >= WINDOW_HEIGHT)
        {
            it = bullet_line_list.erase(it);
        }
        else
        {
            it->position.x += static_cast<int>(8 * cos(it->angle));
            it->position.y += static_cast<int>(8 * sin(it->angle));
            ++it;
        }
    }

    if (g_state.timerLine >= 40)
    {
        int count = 4 + player.GetLevel() / 2;
        for (int i = 0; i < count; ++i)
            bullet_line_list.emplace_back(count, i, player.GetPosition());
        g_state.timerLine = 0;
    }
    else
        ++g_state.timerLine;
}

void Game::UpdateBulletsLineBoss()
{
    // 移动现有 Boss 子弹
    for (auto it = bullet_line_boss_list.begin(); it != bullet_line_boss_list.end(); )
    {
        if (it->position.x < 0 || it->position.x >= WINDOW_WIDTH ||
            it->position.y < 0 || it->position.y >= WINDOW_HEIGHT)
        {
            it = bullet_line_boss_list.erase(it);
        }
        else
        {
            it->position.x += static_cast<int>(8 * cos(it->angle));
            it->position.y += static_cast<int>(8 * sin(it->angle));
            ++it;
        }
    }

    // 0.5s 生成一波 Boss 子弹
    if (g_state.timerLineBoss >= 40)
    {
        int boss_bullet_count = rand() % 4 + 1;
        for (int i = 0; i < boss_bullet_count; ++i)
            bullet_line_boss_list.emplace_back(boss_bullet_count, i, boss.GetPosition());
        g_state.timerLineBoss = 0;
    }
    else
        ++g_state.timerLineBoss;
}

bool Game::CheckBossHitPlayer() const
{
    POINT pos = player.GetPosition();
    for (const auto& b : bullet_line_boss_list)
    {
        bool x = b.position.x >= pos.x &&
            b.position.x <= pos.x + Player::FRAME_WIDTH;
        bool y = b.position.y >= pos.y &&
            b.position.y <= pos.y + Player::FRAME_HEIGHT;
        if (x && y) return true;
    }
    return false;
}

// ============================== UpdateLogic ==============================

void Game::UpdateLogic()
{
    if (!g_state.isGameStarted) return;

    // 读档等级 → 只在第一次进游戏时加成
    if (firstLevelInit)
    {
        for (int i = 0; i < g_state.getLevel; ++i)
            player.level_up();
        firstLevelInit = false;
    }

    // 场景初始化（背景 + 障碍物）
    InitSceneIfNeeded();

    // Boss 移动
    if (g_state.isBoss)
        boss.Move();

    // 玩家移动
    player.Move(obstacle_list);

    // 玩家子弹逻辑
    if (g_state.choice == 1)
    {
        int targetCount = 3 + player.GetLevel() / 2;
        while (static_cast<int>(bullet_list.size()) < targetCount)
            bullet_list.emplace_back();
        UpdateBulletsOrbit();
    }
    else if (g_state.choice == 2)
    {
        UpdateBulletsLine();
    }

    // Boss 子弹
    if (g_state.isBoss)
    {
        UpdateBulletsLineBoss();
    }

    // Boss 子弹打到玩家
    if (g_state.isBoss)
    {
        if (CheckBossHitPlayer())
        {
            player.Hurt();
            if (!player.CheckAlive())
            {
                TCHAR text[128];
                _stprintf_s(text, _T("Your kill：%d!  我一定会回来的！就算续命也要通关！"), g_state.score);
                MessageBox(GetHWnd(), text, _T("Game Over"), MB_OK);

                g_state.gold += g_state.score;
                WriteSaveFile(g_saveFile, g_state.gold, g_state.getLevel);
                g_state.running = false;
                return;
            }
        }
    }

    // 敌人生成 + 移动
    TryGenerateEnemy();
    for (auto& e : enemy_list)
        e->Move(player);

    // 敌人与玩家碰撞
    for (auto& e : enemy_list)
    {
        if (e->CheckPlayerCollision(player))
        {
            player.Hurt();
            if (!player.CheckAlive())
            {
                TCHAR text[128];
                _stprintf_s(text, _T("Your kill：%d!  氪金也不能复活哈哈哈"), g_state.score);
                MessageBox(GetHWnd(), text, _T("Game Over"), MB_OK);

                g_state.gold += g_state.score;
                WriteSaveFile(g_saveFile, g_state.gold, g_state.getLevel);
                g_state.running = false;
                return;
            }
        }
    }

    // 玩家子弹打 Boss
    if (g_state.isBoss)
    {
        if (g_state.choice == 1)
        {
            for (const Bullet& b : bullet_list)
            {
                if (boss.CheckBulletCollision(b))
                {
                    if (!boss.wudi)
                        mciSendString(_T("play hit from 0"), NULL, 0, NULL);
                    boss.Hurt();
                }
            }
        }
        else if (g_state.choice == 2)
        {
            for (const Bullet_line& b : bullet_line_list)
            {
                if (boss.CheckBulletLineCollision(b))
                {
                    if (!boss.wudi)
                        mciSendString(_T("play hit from 0"), NULL, 0, NULL);
                    boss.Hurt();
                }
            }
        }
    }

    // Boss 死亡
    if (g_state.isBoss && !boss.CheckAlive())
    {
        g_state.score += 25;
        TCHAR text[128];
        if (g_state.score >= 125)
            _stprintf_s(text, _T("Perfect! GAME CLEAR!"));
        else
            _stprintf_s(text, _T("Great! Game Clear!"));

        MessageBox(GetHWnd(), text, _T("Game Over"), MB_OK);

        g_state.gold += g_state.score;
        WriteSaveFile(g_saveFile, g_state.gold, g_state.getLevel);
        g_state.running = false;
        return;
    }

    // 玩家子弹打普通敌人
    for (auto& e : enemy_list)
    {
        if (g_state.choice == 1)
        {
            for (const Bullet& b : bullet_list)
            {
                if (e->CheckBulletCollision(b))
                {
                    mciSendString(_T("play hit from 0"), NULL, 0, NULL);
                    e->Hurt();
                    g_state.score++;
                }
            }
        }
        else if (g_state.choice == 2)
        {
            for (const Bullet_line& b : bullet_line_list)
            {
                if (e->CheckBulletLineCollision(b))
                {
                    mciSendString(_T("play hit from 0"), NULL, 0, NULL);
                    e->Hurt();
                    g_state.score++;
                }
            }
        }
    }

    // 玩家捡原石
    for (auto& d : drop_list)
    {
        if (d->CheckPlayerCollision(player))
        {
            d->pick_up();
            int chance = rand() % 3 + 1;
            if (chance == 1)
                player.level_up();
        }
    }

    // 移除死亡敌人并生成掉落
    for (size_t i = 0; i < enemy_list.size(); )
    {
        auto& e = enemy_list[i];
        if (!e->CheckAlive())
        {
            int chance = rand() % 3 + 1;
            if (chance == 1)
                drop_list.push_back(std::make_unique<dropping>(e->GetPosition()));

            std::swap(enemy_list[i], enemy_list.back());
            enemy_list.pop_back();   // unique_ptr 自动 delete
        }
        else
            ++i;
    }

    // 移除已拾取的原石
    for (size_t i = 0; i < drop_list.size(); )
    {
        auto& d = drop_list[i];
        if (!d->CheckAlive())
        {
            std::swap(drop_list[i], drop_list.back());
            drop_list.pop_back();
        }
        else
            ++i;
    }
}

// ================================ Render =================================

void Game::Render()
{
    cleardevice();

    if (g_state.isGameStarted)
    {
        putimage(0, 0, &img_background);

        for (auto& o : obstacle_list)
            o->Draw(1000 / 144);

        player.Draw(1000 / 144);

        if (g_state.choice == 1)
            for (const Bullet& b : bullet_list)
                b.Draw();

        if (g_state.choice == 2)
            for (const Bullet_line& b : bullet_line_list)
                b.Draw();

        if (g_state.isBoss)
            for (const Bullet_line_boss& b : bullet_line_boss_list)
                b.Draw();

        if (g_state.isBoss)
            boss.Draw(1000 / 144);

        for (auto& e : enemy_list)
            e->Draw(1000 / 144);

        for (auto& d : drop_list)
            d->Draw(1000 / 144);

        DrawPlayerScore(g_state.score);
        DrawPlayerLevel(player.GetLevel());
        DrawPlayerHP(player.GetHP());
        if (g_state.isBoss)
            DrawBossHP(boss.GetHP());

        player.UpdataWudi();
        player.DrawHurt();
        if (g_state.isBoss)
        {
            boss.UpdataWudi();
            boss.DrawHurt();
        }
    }
    else
    {
        putimage(0, 0, &img_menu);

        for (auto& btn : buttons)
        {
			btn->Draw();
        }

        DrawGold(g_state.gold);
        DrawGetLevel(g_state.getLevel);
    }

    FlushBatchDraw();
}

// ================================ 主循环 =================================

int Game::Run()
{
    while (g_state.running)
    {
        ULONGLONG start_time = GetTickCount();

        HandleEvents();
        UpdateLogic();
        Render();

        ULONGLONG end_time = GetTickCount();
        ULONGLONG delta_time = end_time - start_time;
        if (delta_time < 1000 / 80)
            Sleep(static_cast<DWORD>(1000 / 80 - delta_time));
    }

    return 0;
}

// ================================ 对外接口 ================================

int RunGame()
{
    srand(static_cast<unsigned>(time(NULL)));

    // 初始化本局全局状态
    g_state.choice = rand() % 2 + 1;   // 1 或 2
    g_state.score = 0;
    g_state.timerLine = 80;
    g_state.timerLineBoss = 80;

    Game game;
    return game.Run();
}