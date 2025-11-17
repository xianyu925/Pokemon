#include "Core.h"
#include <Windows.h>
#include <fstream>

// 全局变量定义
GameState g_state;
std::string g_saveFile;

// 获取存档路径（沿用你原来的逻辑）
std::string GetSavePath()
{
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) > 0) {
        std::string exeDir = exePath;
        size_t pos = exeDir.find_last_of('\\');
        if (pos != std::string::npos) {
            return exeDir.substr(0, pos) + "\\pokemon_save.dat";
        }
    }
    return "pokemon_save.dat";
}

bool ReadSaveFile(const std::string& filename, int& value1, int& value2)
{
    std::ifstream inFile(filename);
    if (inFile) {
        if (inFile >> value1 >> value2) {
            return true;
        }
    }
    return false;
}

bool WriteSaveFile(const std::string& filename, int value1, int value2)
{
    std::ofstream outFile(filename);
    if (!outFile) {
        return false;
    }
    outFile << value1 << "\n" << value2;
    return true;
}
