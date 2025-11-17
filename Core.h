#pragma once
#include <string>

struct GameState
{
	// ===== 全局游戏状态 =====
	bool isGameStarted = false;
	bool running = true;
	bool isBoss = false;

	// ===== 存档相关 =====
	int gold = 0;
	int getLevel = 0;

	// ===== 游戏元素 =====
	int score = 0;
	int choice = 1; // 1: 子弹环绕  2: 子弹直线
	int timerLine = 80;
	int timerLineBoss = 80;
};

extern GameState g_state;
extern std::string g_saveFile;

// ===== 常量 =====
constexpr int WINDOW_WIDTH = 1280;
constexpr int WINDOW_HEIGHT = 720;
constexpr int BUTTON_WIDTH = 192;
constexpr int BUTTON_HEIGHT = 75;

// ===== API =====
std::string GetSavePath();
bool ReadSaveFile(const std::string& filename, int& value1, int& value2);
bool WriteSaveFile(const std::string& filename, int value1, int value2);
