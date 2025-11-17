#include "Core.h"
#include "Game.h"
#include <iostream>

int main()
{
    g_saveFile = GetSavePath();
    std::cout << "存档位置: " << g_saveFile << "\n";

    if (ReadSaveFile(g_saveFile, g_state.gold, g_state.getLevel))
    {
        std::cout << "存档加载成功!\n";
        std::cout << "GOLD: " << g_state.gold << ", Level: " << g_state.getLevel << "\n";
    }
    else
    {
        std::cout << "未找到存档，创建新存档...\n";
        g_state.gold = 100;
        g_state.getLevel = 0;
        if (!WriteSaveFile(g_saveFile, g_state.gold, g_state.getLevel))
        {
            std::cerr << "错误: 无法创建存档文件\n";
            return -1;
        }
    }

    return RunGame();
}
