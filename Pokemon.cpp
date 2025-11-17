#include <graphics.h>
#include<string>
#include<vector>
#include<time.h>

#include <iostream>
#include <fstream>
#include <stdexcept>

#pragma comment(lib,"MSIMG32.LIB")
#pragma comment(lib,"Winmm.lib")

bool is_game_started = false;
bool running = true;

std::string getSavePath() {
    char exePath[MAX_PATH];
    if (GetModuleFileNameA(NULL, exePath, MAX_PATH) > 0) {
        std::string exeDir = exePath;
        size_t pos = exeDir.find_last_of("\\");
        if (pos != std::string::npos) {
            return exeDir.substr(0, pos) + "\\pokemon_save.dat";
        }
    }
    return "pokemon_save.dat";
}

const std::string saveFile = "D:\\game_save.dat";

const int BUTTON_WIDTH = 192;
const int BUTTON_HEIGHT = 75;

// 尝试读取存档数据
bool readSaveFile(const std::string& filename, int& value1, int& value2) {
    std::ifstream inFile(filename);
    if (inFile) {
        // 成功打开文件，尝试读取数据
        if (inFile >> value1 >> value2) {
            return true; // 成功读取数据
        }
    }
    return false; // 文件不存在或读取失败
}

// 写入存档数据
bool writeSaveFile(const std::string& filename, int value1, int value2) {
    std::ofstream outFile(filename);
    if (!outFile) {
        return false; // 无法创建文件
    }
    outFile << value1 << "\n" << value2;
    return true; // 写入成功
}

int GOLD = 100;
int GetLevel = 0;

inline void putimage_alpha(int x, int y, IMAGE* images)
{
    int w = images->getwidth();
    int h = images->getheight();
    AlphaBlend(GetImageHDC(NULL), x, y, w, h,
        GetImageHDC(images), 0, 0, w, h, { AC_SRC_OVER,0,255, AC_SRC_ALPHA });
}

const int PLAYER_ANIM_NUM = 6;
IMAGE img_player_left;
IMAGE img_player_right;
IMAGE enemy[6];



class Animation
{
public:
    Animation(LPCTSTR path, int num, int interval)
    {
        interval_ms = interval;

        TCHAR path_file[256];
        for (size_t i = 0; i < num; i++)
        {
            _stprintf_s(path_file, path, i);

            IMAGE* frame = new IMAGE();
            loadimage(frame, path_file);
            frame_list.push_back(frame);
        }
    }

    ~Animation()
    {
        for (size_t i = 0; i < frame_list.size(); i++)
        {
            delete frame_list[i];
        }
    }


    void Play(int x, int y, int delta)
    {
        timer += delta;
        if (timer >= interval_ms)
        {
            idx_frame = (idx_frame + 1) % frame_list.size();
            timer = 0;
        }

        putimage_alpha(x, y, frame_list[idx_frame]);

    }



private:
    std::vector<IMAGE*> frame_list;
    int interval_ms = 0;//帧间隔
    int timer = 0;//动画计时器
    int idx_frame = 0;//动画帧索引
};

class Obstacle
{
public:

    int WIDTH = 100;
    int HEIGHT = 100;
    Animation* anim;

    POINT position = { 200,200 };

    Obstacle(bool isboss, int pos_x = 200, int pos_y = 200)
    {
        position.x = pos_x;
        position.y = pos_y;

        if (!isboss)
        {
            anim = new Animation(_T("images/obstacle.png"), 1, 45);
        }
        else if (isboss)
        {
            anim = new Animation(_T("images/obstacle_boss.png"), 1, 45);
        }
    }

    void Draw(int delta)
    {
        anim->Play(position.x, position.y, delta);
    }
};



class Player
{
public:
    Player(int choice = 1)
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

    ~Player()
    {
        delete anim_left;
        delete anim_right;
    }



    void ProcessEvent(const ExMessage& msg)
    {
        switch (msg.message)
        {
        case WM_KEYDOWN:
            switch (msg.vkcode)
            {
            case VK_UP:
                is_move_up = true;
                break;
            case VK_DOWN:
                is_move_down = true;
                break;
            case VK_LEFT:
                is_move_left = true;
                break;
            case VK_RIGHT:
                is_move_right = true;
                break;
            }
            break;

        case WM_KEYUP:
            switch (msg.vkcode)
            {
            case VK_UP:
                is_move_up = false;
                break;
            case VK_DOWN:
                is_move_down = false;
                break;
            case VK_LEFT:
                is_move_left = false;
                break;
            case VK_RIGHT:
                is_move_right = false;
                break;
            }
            break;
        }
    }

    const POINT& GetPosition() const
    {
        return player_pos;
    }


    void Draw(int delta)
    {
        int dir_x = is_move_right - is_move_left;
        static bool facing_left = false;
        if (dir_x < 0)
            facing_left = true;
        else if (dir_x > 0)
            facing_left = false;

        if (facing_left)
            anim_left->Play(player_pos.x, player_pos.y, delta);
        else
            anim_right->Play(player_pos.x, player_pos.y, delta);

    }

    // 改进的CheckMove函数
    int CheckMove(const Obstacle& obstacle, POINT new_pos)
    {
        // 计算玩家和障碍物的矩形边界
        RECT player_rect = {
            new_pos.x,
            new_pos.y,
            new_pos.x + FRAME_WIDTH,
            new_pos.y + FRAME_HEIGHT
        };

        RECT obstacle_rect = {
            obstacle.position.x,
            obstacle.position.y,
            obstacle.position.x + obstacle.WIDTH,
            obstacle.position.y + obstacle.HEIGHT
        };

        // 检测是否完全无碰撞
        if (player_rect.right <= obstacle_rect.left ||
            player_rect.left >= obstacle_rect.right ||
            player_rect.bottom <= obstacle_rect.top ||
            player_rect.top >= obstacle_rect.bottom)
        {
            return 3; // 无碰撞
        }

        // 计算重叠区域
        int overlap_left = max(player_rect.left, obstacle_rect.left);
        int overlap_right = min(player_rect.right, obstacle_rect.right);
        int overlap_top = max(player_rect.top, obstacle_rect.top);
        int overlap_bottom = min(player_rect.bottom, obstacle_rect.bottom);

        int overlap_width = overlap_right - overlap_left;
        int overlap_height = overlap_bottom - overlap_top;

        // 判断碰撞类型
        if (overlap_width > 0 && overlap_height > 0)
        {
            // 水平方向碰撞更严重（玩家左右侧碰撞）
            if (overlap_height > overlap_width)
            {
                return 1; // 只能Y方向移动
            }
            // 垂直方向碰撞更严重（玩家上下侧碰撞）
            else
            {
                return 2; // 只能X方向移动
            }
        }

        return 0; // 完全阻挡
    }

    // 改进的Move函数
    void Move(std::vector<Obstacle*>& obstacles)
    {
        int dir_x = is_move_right - is_move_left;
        int dir_y = is_move_down - is_move_up;
        double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);

        if (len_dir != 0)
        {
            POINT new_pos = player_pos;
            double normalized_x = dir_x / len_dir;
            double normalized_y = dir_y / len_dir;
            new_pos.x += (int)(SPEED * normalized_x);
            new_pos.y += (int)(SPEED * normalized_y);

            // 初始化移动能力
            bool can_move_x = true;
            bool can_move_y = true;

            // 检查所有障碍物
            for (Obstacle* obs : obstacles)
            {
                int result = CheckMove(*obs, new_pos);

                // 根据碰撞结果更新移动能力
                if (result == 0) {
                    can_move_x = false;
                    can_move_y = false;
                    break; // 完全阻挡，无需检查其他障碍物
                }
                else if (result == 1) {
                    can_move_x = false;
                }
                else if (result == 2) {
                    can_move_y = false;
                }
            }

            // 应用移动
            if (can_move_x && can_move_y) {
                player_pos = new_pos;
            }
            else if (can_move_x) {
                player_pos.x = new_pos.x;
            }
            else if (can_move_y) {
                player_pos.y = new_pos.y;
            }
        }

        // 边界检查（保持不变）
        if (player_pos.x < 0) player_pos.x = 0;
        if (player_pos.y < 0) player_pos.y = 0;
        if (player_pos.x + FRAME_WIDTH > 1280) player_pos.x = 1280 - FRAME_WIDTH;
        if (player_pos.y + FRAME_HEIGHT > 720) player_pos.y = 720 - FRAME_HEIGHT;
    }

    void Hurt()
    {
        if (!wudi)
        {
            HP--;
            wudi = true;
        }
        if (HP <= 0)
        {
            alive = false;
        }
    }

    bool CheckAlive()
    {
        return alive;
    }

    int GetLevel()
    {
        return level;
    }

    void level_up()
    {
        level++;
        if (HP <= 4)
            HP++;
    }

    int GetHP()
    {
        return HP;
    }

    void UpdataWudi()
    {
        if (wudi)
        {
            wudi_count++;
        }
        if (wudi_count >= 80)
        {
            wudi = false;
            wudi_count = 0;
        }
    }

    void DrawHurt()
    {
        //TODO
        if (wudi)
        {
            static TCHAR text[64];
            _stprintf_s(text, _T("Hurt!"));

            setbkmode(TRANSPARENT);
            settextcolor(RGB(255, 0, 0));
            outtextxy(player_pos.x + 25, player_pos.y - 20, text);
        }
    }



public:

    const int SPEED = 5;
    const int FRAME_WIDTH = 80;
    const int FRAME_HEIGHT = 80;

private:
    Animation* anim_left;
    Animation* anim_right;
    POINT player_pos = { 500,500 };
    bool is_move_up = false;
    bool is_move_down = false;
    bool is_move_left = false;
    bool is_move_right = false;

    int level = 1;
    int HP = 2;
    int alive = true;

    bool wudi = false;
    int wudi_count = 0;
};

class Bullet
{
public:
    POINT position = { 0,0 };

public:
    Bullet() = default;
    ~Bullet() = default;

    void Draw() const
    {
        /*setlinecolor(RGB(255, 155, 50));//橙红
        setfillcolor(RGB(200, 75, 10));
        fillcircle(position.x, position.y, RADIUS);*/
        setlinecolor(RGB(0, 178, 238));    // 深天蓝色边框
        setfillcolor(RGB(135, 206, 235));  // 标准天蓝色填充
        fillcircle(position.x, position.y, RADIUS);
    }

private:
    const int RADIUS = 8;

};


class Bullet_line
{
public:
    POINT position = { 0,0 };
    int bullet_count = 3;
    double radian_interval = 2 * 3.14159 / bullet_count;//弧度间隔
    double angle;

public:
    Bullet_line(int count, int idx, POINT pos)
    {
        bullet_count = count;
        radian_interval = 2 * 3.14159 / bullet_count;
        angle = radian_interval * idx;
        position.x = pos.x + 40;
        position.y = pos.y + 40;
    }
    Bullet_line() = default;
    ~Bullet_line() = default;

    void Draw() const
    {
        setlinecolor(RGB(0, 0, 0));  //黑色  
        setfillcolor(RGB(0, 0, 0));
        fillcircle(position.x, position.y, RADIUS);
    }

    int GetCount()
    {
        return bullet_count;
    }

private:
    static const int RADIUS = 8;
};

class Bullet_line_boss
{
public:
    POINT position = { 0,0 };
    int bullet_count = 3;
    double radian_interval = 2 * 3.14159 / bullet_count;//弧度间隔
    double angle;

public:
    Bullet_line_boss(int count, int idx, POINT pos)
    {
        bullet_count = count;
        radian_interval = 2 * 3.14159 / bullet_count;
        angle = radian_interval * idx;
        position.x = pos.x + 40;
        position.y = pos.y + 40;
    }
    Bullet_line_boss() = default;
    ~Bullet_line_boss() = default;

    void Draw() const
    {
        setlinecolor(RGB(0, 128, 0));  //深绿色  
        setfillcolor(RGB(0, 128, 0));
        fillcircle(position.x, position.y, RADIUS);
    }

    int GetCount()
    {
        return bullet_count;
    }

private:
    static const int RADIUS = 8;
};


class Enemy
{
public:
    Enemy()
    {
        anim = new Animation(_T("images/enemy_%d.png"), 6, 45);

        enum class SpawnEdge
        {
            Up = 0,
            Down,
            Left,
            Right
        };

        SpawnEdge edge = (SpawnEdge)(rand() % 4);
        switch (edge)
        {
        case SpawnEdge::Up:
            position.x = rand() % 1280;
            position.y = -FRAME_HEIGHT;
            break;
        case SpawnEdge::Down:
            position.x = rand() % 1280;
            position.y = 720;
            break;
        case SpawnEdge::Left:
            position.x = -FRAME_WIDTH;
            position.y = rand() % 720;
            break;
        case SpawnEdge::Right:
            position.x = 1280;
            position.y = rand() % 720;
            break;
        default:
            break;
        }
    }

    void Hurt()
    {
        alive = false;
    }

    bool CheckAlive()
    {
        return alive;
    }

    bool CheckBulletCollision(const Bullet& bullet)
    {
        bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + FRAME_WIDTH;
        bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + FRAME_HEIGHT;
        return is_overlap_x && is_overlap_y;
    }

    bool CheckBulletLineCollision(const Bullet_line& bullet)
    {
        bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + FRAME_WIDTH;
        bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + FRAME_HEIGHT;
        return is_overlap_x && is_overlap_y;
    }

    bool CheckPlayerCollision(const Player& player)
    {
        POINT check_position = { position.x + FRAME_WIDTH / 3,position.y + FRAME_HEIGHT / 3 };//这里锁定的是敌人（1/3，1/3）的位置
        POINT player_position = player.GetPosition();

        bool is_overlap_x = check_position.x >= player_position.x && check_position.x <= player_position.x + 80;
        bool is_overlap_y = check_position.y >= player_position.y && check_position.y <= player_position.y + 80;
        return is_overlap_x && is_overlap_y;
    }


    void Move(const Player& player)
    {
        const POINT& player_pos = player.GetPosition();
        int dir_x = player_pos.x - position.x;
        int dir_y = player_pos.y - position.y;
        double len_dir = sqrt(dir_x * dir_x + dir_y * dir_y);
        if (len_dir != 0)
        {
            double normalized_x = dir_x / len_dir;
            double normalized_y = dir_y / len_dir;
            position.x += (int)(SPEED * normalized_x);
            position.y += (int)(SPEED * normalized_y);
        }
    }


    void Draw(int delta)
    {
        anim->Play(position.x, position.y, delta);
    }

    const POINT& GetPosition() const
    {
        return position;
    }


    ~Enemy()
    {
        delete anim;
    }

public:

    const int SPEED = 3;
    const int FRAME_WIDTH = 50;
    const int FRAME_HEIGHT = 50;

private:
    Animation* anim;

    POINT position = { 0,0 };

    bool alive = true;
};

class EnemyBoss
{
private:
    int HP = 35;
    POINT position = { 0,0 };
    bool alive = true;

public:
    //int HP=35;
    int SPEED = 5;


    //POINT position = { 0,0 };
    //bool alive = true;

    int WIDTH = 100;
    int HEIGHT = 100;

    bool wudi = false;
    int wudi_count = 0;

    bool down = true;
    bool right = false;
    bool up = false;
    bool left = false;

    Animation* anim;

    EnemyBoss()
    {
        anim = new Animation(_T("images/enemy_boss.png"), 1, 45);
    }

    ~EnemyBoss()
    {
        delete anim;
    }

    void Move()
    {
        if (down)position.y += SPEED;
        if (right)position.x += SPEED;
        if (up)position.y -= SPEED;
        if (left)position.x -= SPEED;

        if (position.x < 0) { position.x = 0; left = false; down = true; }
        if (position.y < 0) { position.y = 0; up = false; left = true; }
        if (position.x + WIDTH > 1280) { position.x = 1280 - WIDTH; right = false; up = true; }
        if (position.y + HEIGHT > 720) { position.y = 720 - HEIGHT; down = false; right = true; }

    }

    void Draw(int delta)
    {
        anim->Play(position.x, position.y, delta);
    }

    bool CheckBulletCollision(const Bullet& bullet)
    {
        bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + WIDTH;
        bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + HEIGHT;
        return is_overlap_x && is_overlap_y;
    }

    bool CheckBulletLineCollision(const Bullet_line& bullet)
    {
        bool is_overlap_x = bullet.position.x >= position.x && bullet.position.x <= position.x + WIDTH;
        bool is_overlap_y = bullet.position.y >= position.y && bullet.position.y <= position.y + HEIGHT;
        return is_overlap_x && is_overlap_y;
    }

    void Hurt()
    {
        if (!wudi)
        {
            HP--;
            wudi = true;
        }
        if (HP <= 0)
        {
            alive = false;
        }
    }

    bool CheckAlive()
    {
        return alive;
    }

    void UpdataWudi()
    {
        if (wudi)
        {
            wudi_count++;
        }
        if (wudi_count >= 80)
        {
            wudi = false;
            wudi_count = 0;
        }
    }

    void DrawHurt()
    {
        //TODO
        if (wudi)
        {
            static TCHAR text[64];
            _stprintf_s(text, _T("HIT!"));

            setbkmode(TRANSPARENT);
            settextcolor(RGB(255, 0, 0));
            outtextxy(position.x + 45, position.y - 20, text);
        }
    }

    int GetHP()
    {
        return HP;
    }

    const  POINT GetPosition() const
    {
        return position;
    }

};


void TryGenerarteEnemy(std::vector<Enemy*>& enemy_list)
{
    const int INTERVAL = 100;
    static int counter = 0;
    if ((++counter) % INTERVAL == 0)
    {
        enemy_list.push_back(new Enemy());
    }
}

void UpdataBullets(std::vector<Bullet>& bullet_list, const Player& player)
{
    const double RADIAL_SPEED = 0.0045;//径向波动速度
    const double TANGENT_SPEED = 0.0055;//切向波动速度
    double radian_interval = 2 * 3.14159 / bullet_list.size();//弧度间隔

    POINT player_position = player.GetPosition();
    double radius = 100 + 25 * sin(GetTickCount() * RADIAL_SPEED);

    for (size_t i = 0; i < bullet_list.size(); i++)
    {
        double radian = GetTickCount() * TANGENT_SPEED + radian_interval * i;
        bullet_list[i].position.x = player_position.x + player.FRAME_WIDTH / 2 + (int)(radius * cos(radian));//换成sin就是逆时针
        bullet_list[i].position.y = player_position.y + player.FRAME_HEIGHT / 2 + (int)(radius * sin(radian));
    }
}

void UpdateBullets_line(std::vector<Bullet_line>& bullet_list, const Player& player, int& timer, int count)
{

    for (auto it = bullet_list.begin(); it != bullet_list.end(); )
    {
        if (it->position.x < 0 || it->position.x >= 1280
            || it->position.y < 0 || it->position.y >= 720)
        {
            // 删除越界子弹，并自动更新迭代器
            it = bullet_list.erase(it);
        }
        else
        {
            // 保留未越界的子弹，继续处理其他逻辑（如移动子弹）
            // 例如：it->UpdatePosition();
            it->position.x += (int)(8 * cos(it->angle));
            it->position.y += (int)(8 * sin(it->angle));
            ++it;
        }
    }

    //0.5s生成一次
    if (timer >= 40)
    {
        for (int i = 0; i < count; i++)
            bullet_list.push_back(Bullet_line(count, i, player.GetPosition()));
        timer = 0;
    }
    else { timer++; }
}

void UpdateBullets_line_boss(std::vector<Bullet_line_boss>& bullet_list, const EnemyBoss& Boss, int& timer, int count)
{

    for (auto it = bullet_list.begin(); it != bullet_list.end(); )
    {
        if (it->position.x < 0 || it->position.x >= 1280
            || it->position.y < 0 || it->position.y >= 720)
        {
            // 删除越界子弹，并自动更新迭代器
            it = bullet_list.erase(it);
        }
        else
        {
            // 保留未越界的子弹，继续处理其他逻辑（如移动子弹）
            // 例如：it->UpdatePosition();
            it->position.x += (int)(8 * cos(it->angle));
            it->position.y += (int)(8 * sin(it->angle));
            ++it;
        }
    }

    //0.5s生成一次
    if (timer >= 40)
    {
        for (int i = 0; i < count; i++)
            bullet_list.push_back(Bullet_line_boss(count, i, Boss.GetPosition()));
        timer = 0;
    }
    else { timer++; }
}

void DrawPlayerScore(int score)
{
    static TCHAR text[64];
    _stprintf_s(text, _T("你的得分为：%d"), score);

    setbkmode(TRANSPARENT);
    settextcolor(RGB(255, 85, 185));
    outtextxy(10, 30, text);
}

void DrawPlayerLevel(int level)
{
    static TCHAR text[64];
    _stprintf_s(text, _T("你的等级为：%d"), level);

    setbkmode(TRANSPARENT);
    settextcolor(RGB(255, 85, 185));
    outtextxy(10, 50, text);

}

void DrawPlayerHP(int hp)
{
    // 设置文字颜色和背景模式
    settextcolor(RED);
    setbkmode(TRANSPARENT);

    // 绘制文字
    outtextxy(10, 10, _T("你的HP为："));

    // 设置填充颜色为红色
    setfillcolor(RED);

    // 圆形参数
    const int radius = 10;          // 圆半径
    const int spacing = 5;          // 圆之间的间距
    const int startX = 120;         // 第一个圆心的X坐标（文字后开始）
    const int centerY = 12;         // 所有圆心Y坐标（垂直居中）

    // 绘制HP对应的红色圆形
    for (int i = 0; i < hp; ++i) {
        int centerX = startX + i * (2 * radius + spacing);
        solidcircle(centerX, centerY, radius);
    }
}

void DrawBossHP(int hp)
{
    static TCHAR text[64];
    _stprintf_s(text, _T("Boss剩余血量：%d"), hp);

    setbkmode(TRANSPARENT);
    settextcolor(BLACK);
    outtextxy(1140, 10, text);
}

void DrawGetLevel(int& getlevel)
{
    static TCHAR text[64];
    _stprintf_s(text, _T("角色的初始等级为：%d"), getlevel + 1);

    setbkmode(TRANSPARENT);
    settextcolor(BLUE);
    outtextxy(10, 30, text);
}

void DrawGold(int& gold)
{
    static TCHAR text[64];
    _stprintf_s(text, _T("您拥有的创世结晶数为：%d"), gold);

    setbkmode(TRANSPARENT);
    settextcolor(BLUE);
    outtextxy(10, 10, text);
}

class Button
{
public:
    Button(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
    {
        region = rect;

        loadimage(&img_idle, path_img_idle);
        loadimage(&img_hovered, path_img_hovered);
        loadimage(&img_pushed, path_img_pushed);
    }

    ~Button() = default;


    void Draw()
    {
        switch (status)
        {
        case Status::Idle:
            putimage(region.left, region.top, &img_idle);
            break;
        case Status::Hovered:
            putimage(region.left, region.top, &img_hovered);
            break;
        case Status::Pushed:
            putimage(region.left, region.top, &img_pushed);
            break;
        }
    }

    void ProcessEvent(const ExMessage& msg)
    {
        switch (msg.message)
        {
        case WM_MOUSEMOVE:
            if (status == Status::Idle && CheckCursorHit(msg.x, msg.y))
                status = Status::Hovered;
            else if (status == Status::Hovered && !CheckCursorHit(msg.x, msg.y))
                status = Status::Idle;
            break;
        case WM_LBUTTONDOWN:
            if (CheckCursorHit(msg.x, msg.y))
                status = Status::Pushed;
            break;
        case WM_LBUTTONUP:
            if (CheckCursorHit(msg.x, msg.y))
                OnClick();
            status = Status::Idle;
            break;
        default:
            break;

        }
    }


private:

    virtual void OnClick() = 0;

    enum class Status
    {
        Idle = 0,
        Hovered,
        Pushed
    };

    RECT region;
    IMAGE img_idle;
    IMAGE img_hovered;
    IMAGE img_pushed;
    Status status = Status::Idle;

    bool CheckCursorHit(int x, int y)
    {
        return x >= region.left && x <= region.right && y >= region.top && y <= region.bottom;
    }

};
bool is_boss = false;
class StartGameButton : public Button
{
public:
    StartGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
        :Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {
    }
    ~StartGameButton() = default;

protected:
    void OnClick()
    {
        is_game_started = true;
        is_boss = false;
        mciSendString(_T("play bgm repeat from 0"), NULL, 0, NULL);
    }
};

class QuitGameButton : public Button
{
public:
    QuitGameButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
        :Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {
    }
    ~QuitGameButton() = default;

protected:
    void OnClick()
    {
        running = false;
    }
};


class StartBossButton :public Button
{
public:
    StartBossButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
        :Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {
    }
    ~StartBossButton() = default;

protected:
    void OnClick()
    {
        is_game_started = true;
        is_boss = true;
        mciSendString(_T("play exaid_bgm repeat from 0"), NULL, 0, NULL);
    }
};

class ResetButton :public Button
{
public:
    ResetButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
        :Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {
    }
    ~ResetButton() = default;

protected:
    void OnClick()
    {
        mciSendString(_T("play hit from 0"), NULL, 0, NULL);
        GOLD = 100;
        GetLevel = 0;
        if (writeSaveFile(saveFile, GOLD, GetLevel)) {
            std::cout << "游戏进度已重置!\n";
        }
        else {
            std::cerr << "错误: 无法重置游戏进度\n";

        }

        static TCHAR text[128];
        _stprintf_s(text, _T("游戏进度已重置!看我一命通关！"));
        MessageBox(GetHWnd(), text, _T("世上有后悔药吗？也许吧"), MB_OK);

    }
};

class FreeButton :public Button
{
public:
    FreeButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
        :Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {
    }
    ~FreeButton() = default;

protected:
    void OnClick()
    {
        static TCHAR text[128];
        _stprintf_s(text, _T("都说不要碰我啦\\(/ _ \\)/！"));
        MessageBox(GetHWnd(), text, _T("想休息\\(/ _ \\)/"), MB_OK);

    }
};

class BuildUpButton :public Button
{
public:
    BuildUpButton(RECT rect, LPCTSTR path_img_idle, LPCTSTR path_img_hovered, LPCTSTR path_img_pushed)
        :Button(rect, path_img_idle, path_img_hovered, path_img_pushed) {
    }
    ~BuildUpButton() = default;

protected:
    void OnClick()
    {
        if (GOLD >= 50)
        {
            mciSendString(_T("play hit from 0"), NULL, 0, NULL);
            GetLevel += 1;
            GOLD -= 50;
        }
        else
        {
            static TCHAR text[128];
            _stprintf_s(text, _T("提醒：您的创世结晶数量不足，升级失败"));
            MessageBox(GetHWnd(), text, _T("Sorry"), MB_OK);
        }

        if (writeSaveFile(saveFile, GOLD, GetLevel)) {
            std::cout << "游戏进度已保存!\n";
        }
        else {
            std::cerr << "错误: 无法保存游戏进度\n";

        }
    }
};

class dropping
{
public:
    Animation* anim;
    bool alive = true;
    POINT position = { 0,0 };

    dropping(POINT pos)
    {
        anim = new Animation(_T("images/yuanshi.png"), 6, 45);
        position = pos;
    }

    ~dropping()
    {
        delete anim;
    }

    void Draw(int delta)
    {
        anim->Play(position.x, position.y, delta);
    }

    POINT GetPosition()
    {
        return position;
    }

    bool CheckPlayerCollision(const Player& player)
    {
        POINT check_position = { position.x + 25,position.y + 25 };//锁定原石中心位置（原石大小50，50）
        POINT player_position = player.GetPosition();

        bool is_overlap_x = check_position.x >= player_position.x && check_position.x <= player_position.x + 80;
        bool is_overlap_y = check_position.y >= player_position.y && check_position.y <= player_position.y + 80;
        return is_overlap_x && is_overlap_y;
    }

    void pick_up()
    {
        alive = false;
    }

    bool CheckAlive()
    {
        return alive;
    }

};

bool CheckBossPlayer(std::vector<Bullet_line_boss>& bullet_list, Player& player)
{
    POINT position = player.GetPosition();
    bool result = false;
    for (int i = 0; i < bullet_list.size(); i++)
    {
        bool is_overlap_x = bullet_list[i].position.x >= position.x && bullet_list[i].position.x <= position.x + player.FRAME_WIDTH;
        bool is_overlap_y = bullet_list[i].position.y >= position.y && bullet_list[i].position.y <= position.y + player.FRAME_HEIGHT;
        if (is_overlap_x && is_overlap_y) { result = true; break; }
    }
    return result;

}

bool flag = true;//用于保证boss战中图片资源只加载一次
bool level_flag = true;//用于保证角色等级只在每次开始游戏时升级一次

int main()
{
    const std::string saveFile = getSavePath();
    std::cout << "存档位置: " << saveFile << "\n";

    if (readSaveFile(saveFile, GOLD, GetLevel))
    {
        std::cout << "存档加载成功!\n";
        std::cout << "值1: " << GOLD << ", 值2: " << GetLevel << "\n";
    }
    else
    {
        std::cout << "未找到存档，创建新存档...\n";
        GOLD = 100;
        GetLevel = 0;
        if (writeSaveFile(saveFile, GOLD, GetLevel))
        {
            std::cout << "新存档创建成功!\n";
            std::cout << "默认值1: " << GOLD << ", 默认值2: " << GetLevel << "\n";
        }
        else
        {
            std::cerr << "错误: 无法创建存档文件\n";
            return -1;
        }
    }

    srand((unsigned)time(NULL));

    initgraph(1280, 720);
    // 获得窗口句柄
    HWND hWnd = GetHWnd();
    // 使用 Windows API 修改窗口名称
    SetWindowText(hWnd, L"POKEMON");

    mciSendString(_T("open BGM/exaid_bgm.mp3 alias exaid_bgm"), NULL, 0, NULL);
    mciSendString(_T("open BGM/bgm.mp3 alias bgm"), NULL, 0, NULL);
    mciSendString(_T("open BGM/hit.wav alias hit"), NULL, 0, NULL);



    srand(time(0));
    int choice = rand() % 2 + 1;

    int score = 0;
    Player player(choice);
    ExMessage msg;
    IMAGE img_menu;
    IMAGE img_background;

    EnemyBoss Boss;

    std::vector<Obstacle*>obstacle;
    std::vector<Enemy*>enemy_list;
    std::vector<dropping*>drop_list;
    std::vector<Bullet>bullet_list(3);
    std::vector<Bullet_line>bullet_line_list;
    std::vector<Bullet_line_boss>bullet_line_boss_list;


    RECT region_btn_start_game, region_btn_quit_game, region_btn_boss_game, region_btn_build_up, region_btn_reset, region_btn_free;
    region_btn_start_game.left = (1280 - BUTTON_WIDTH) / 2;
    region_btn_start_game.right = (1280 + BUTTON_WIDTH) / 2;
    region_btn_start_game.top = 430;
    region_btn_start_game.bottom = region_btn_start_game.top + BUTTON_HEIGHT;

    region_btn_quit_game.left = (1280 - BUTTON_WIDTH) / 2;
    region_btn_quit_game.right = (1280 + BUTTON_WIDTH) / 2;
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

    StartGameButton btn_start_game = StartGameButton(region_btn_start_game,
        _T("images/ui_start_idle.png"), _T("images/ui_start_hovered.png"), _T("images/ui_start_pushed.png"));
    QuitGameButton btn_quit_game = QuitGameButton(region_btn_quit_game,
        _T("images/ui_quit_idle.png"), _T("images/ui_quit_hovered.png"), _T("images/ui_quit_pushed.png"));
    StartBossButton btn_boss_game = StartBossButton(region_btn_boss_game,
        _T("images/boss_id.png"), _T("images/boss_hovered.png"), _T("images/boss_pushed.png"));
    BuildUpButton btn_buid_up = BuildUpButton(region_btn_build_up,
        _T("images/build_id.png"), _T("images/build_hovered.png"), _T("images/build_pushed.png"));
    ResetButton btn_reset = ResetButton(region_btn_reset,
        _T("images/Reset_id.png"), _T("images/Reset_hovered.png"), _T("images/Reset_pushed.png"));
    FreeButton btn_free = FreeButton(region_btn_free,
        _T("images/free_id.png"), _T("images/free_hovered.png"), _T("images/free_pushed.png"));

    loadimage(&img_menu, _T("images/menu.png"));
    /*
if(!is_boss)
loadimage(&img_background, _T("images/mainscene.png"));
if(is_boss)
loadimage(&img_background, _T("images/mainscene_boss.png"));
*/

    BeginBatchDraw();


    while (running)
    {


        DWORD start_time = GetTickCount();

        while (peekmessage(&msg))
        {
            if (is_game_started)
                player.ProcessEvent(msg);
            else
            {
                btn_start_game.ProcessEvent(msg);
                btn_quit_game.ProcessEvent(msg);
                btn_boss_game.ProcessEvent(msg);
                btn_buid_up.ProcessEvent(msg);
                btn_reset.ProcessEvent(msg);
                btn_free.ProcessEvent(msg);
            }

        }

        if (is_game_started)
        {
            if (level_flag)
            {
                for (int i = 0; i < GetLevel; i++)
                {
                    player.level_up();
                }
                level_flag = false;
            }

            if (flag)
            {
                if (!is_boss)
                    loadimage(&img_background, _T("images/mainscene.png"));
                if (is_boss)
                    loadimage(&img_background, _T("images/mainscene_boss.png"));

                obstacle.push_back(new Obstacle(is_boss, 700, 360));
                obstacle.push_back(new Obstacle(is_boss, 400, 200));
                obstacle.push_back(new Obstacle(is_boss, 200, 100));
                obstacle.push_back(new Obstacle(is_boss, 900, 600));
                obstacle.push_back(new Obstacle(is_boss, 600, 600));

                flag = false;
            }

            if (is_boss)Boss.Move();

            player.Move(obstacle);
            if (choice == 1)
            {
                if (bullet_list.size() < 3 + player.GetLevel() / 2)
                {
                    for (int i = 0; i < bullet_list.size() - player.GetLevel() / 2 - 3; i++)
                    {
                        bullet_list.push_back(Bullet());
                    }
                }
                UpdataBullets(bullet_list, player);
            }

            static int timer = 80;
            static int timer_boss = 80;
            if (choice == 2)
                UpdateBullets_line(bullet_line_list, player, timer, 4 + player.GetLevel() / 2);
            if (is_boss)
            {
                srand(time(0));
                int boss_bullet_count = rand() % 4 + 1;
                UpdateBullets_line_boss(bullet_line_boss_list, Boss, timer_boss, boss_bullet_count);
            }

            if (is_boss)
            {
                if (CheckBossPlayer(bullet_line_boss_list, player))
                {
                    player.Hurt();
                    if (!player.CheckAlive())
                    {
                        static TCHAR text[128];
                        _stprintf_s(text, _T("Your kill：%d!  我一定会回来的！就算续命也要通关！"), score);
                        MessageBox(GetHWnd(), text, _T("Game Over"), MB_OK);

                        GOLD += score;
                        if (writeSaveFile(saveFile, GOLD, GetLevel)) {
                            std::cout << "游戏进度已保存!\n";
                        }
                        else {
                            std::cerr << "错误: 无法保存游戏进度\n";

                        }

                        running = false;
                        break;
                    }
                }
            }

            TryGenerarteEnemy(enemy_list);
            for (Enemy* enemy : enemy_list)
                enemy->Move(player);

            //检测与玩家的碰撞
            for (Enemy* enemy : enemy_list)
            {
                if (enemy->CheckPlayerCollision(player))
                {
                    player.Hurt();
                    if (!player.CheckAlive())
                    {
                        static TCHAR text[128];
                        _stprintf_s(text, _T("Your kill：%d!  氪金也不能复活哈哈哈"), score);
                        MessageBox(GetHWnd(), text, _T("Game Over"), MB_OK);

                        GOLD += score;
                        if (writeSaveFile(saveFile, GOLD, GetLevel)) {
                            std::cout << "游戏进度已保存!\n";
                        }
                        else {
                            std::cerr << "错误: 无法保存游戏进度\n";

                        }

                        running = false;
                        break;
                    }
                }
            }

            if (is_boss)
            {
                if (choice == 1)
                {
                    for (const Bullet& bullet : bullet_list)
                    {
                        if (Boss.CheckBulletCollision(bullet))
                        {
                            if (!Boss.wudi)
                            {
                                mciSendString(_T("play hit from 0"), NULL, 0, NULL);
                            }
                            Boss.Hurt();

                        }
                    }
                }
                else if (choice == 2)
                {
                    for (const Bullet_line& bullet : bullet_line_list)
                    {
                        if (Boss.CheckBulletLineCollision(bullet))
                        {
                            if (!Boss.wudi)
                            {
                                mciSendString(_T("play hit from 0"), NULL, 0, NULL);
                            }
                            Boss.Hurt();

                        }
                    }
                }
            }

            if (is_boss && !Boss.CheckAlive())
            {
                score += 25;
                static TCHAR text[128];
                if (score >= 125)
                {
                    _stprintf_s(text, _T("Perfect! GAME CLEAR!"));
                }
                else
                {
                    _stprintf_s(text, _T("Great! Game Clear!"));
                }
                MessageBox(GetHWnd(), text, _T("Game Over"), MB_OK);

                GOLD += score;
                if (writeSaveFile(saveFile, GOLD, GetLevel)) {
                    std::cout << "游戏进度已保存!\n";
                }
                else {
                    std::cerr << "错误: 无法保存游戏进度\n";

                }

                running = false;
                break;
            }

            for (Enemy* enemy : enemy_list)
            {
                if (choice == 1)
                {
                    for (const Bullet& bullet : bullet_list)
                    {
                        if (enemy->CheckBulletCollision(bullet))
                        {
                            mciSendString(_T("play hit from 0"), NULL, 0, NULL);
                            enemy->Hurt();
                            score++;
                        }
                    }
                }
                else if (choice == 2)
                {
                    for (const Bullet_line& bullet : bullet_line_list)
                    {
                        if (enemy->CheckBulletLineCollision(bullet))
                        {
                            mciSendString(_T("play hit from 0"), NULL, 0, NULL);
                            enemy->Hurt();
                            score++;
                        }
                    }
                }
            }

            for (dropping* drop : drop_list)
            {
                if (drop->CheckPlayerCollision(player))
                {
                    drop->pick_up();
                    srand(time(0));
                    int chance = rand() % 3 + 1;
                    if (chance == 1)player.level_up();
                }
            }

            //移除阵亡敌人
            for (size_t i = 0; i < enemy_list.size(); i++)
            {
                Enemy* enemy = enemy_list[i];//应该可以直接用enemy_list[i]判断，回头试试；试过了，果然行
                if (!enemy->CheckAlive())
                {
                    srand(time(0));
                    int chance = rand() % 3 + 1;
                    if (chance == 1) { drop_list.push_back(new dropping(enemy->GetPosition())); }
                    std::swap(enemy_list[i], enemy_list.back());
                    enemy_list.pop_back();
                    delete enemy;
                }
            }

            //移除拾取原石
            for (size_t i = 0; i < drop_list.size(); i++)
            {
                dropping* drop = drop_list[i];
                if (!drop->CheckAlive())
                {
                    std::swap(drop_list[i], drop_list.back());
                    drop_list.pop_back();
                    delete drop;
                }
            }

        }
        cleardevice();
        if (is_game_started)
        {
            putimage(0, 0, &img_background);

            for (int i = 0; i < obstacle.size(); i++)
            {
                obstacle[i]->Draw(1000 / 144);
            }

            player.Draw(1000 / 144);

            if (choice == 1)
            {
                for (const Bullet& bullet : bullet_list)
                    bullet.Draw();
            }

            if (choice == 2)
            {
                for (const Bullet_line& bullet : bullet_line_list)
                    bullet.Draw();
            }

            if (is_boss)
            {
                for (const Bullet_line_boss& bullet : bullet_line_boss_list)
                    bullet.Draw();
            }

            if (is_boss)
            {
                Boss.Draw(1000 / 144);
            }

            for (Enemy* enemy : enemy_list)
                enemy->Draw(1000 / 144);
            for (dropping* drop : drop_list)
                drop->Draw(1000 / 144);


            DrawPlayerScore(score);
            DrawPlayerLevel(player.GetLevel());
            DrawPlayerHP(player.GetHP());
            if (is_boss)DrawBossHP(Boss.GetHP());

            player.UpdataWudi();
            player.DrawHurt();
            if (is_boss)
            {
                Boss.UpdataWudi();
                Boss.DrawHurt();
            }
        }
        else
        {
            putimage(0, 0, &img_menu);

            btn_start_game.Draw();
            btn_quit_game.Draw();
            btn_boss_game.Draw();
            btn_buid_up.Draw();
            btn_reset.Draw();
            btn_free.Draw();

            DrawGold(GOLD);
            DrawGetLevel(GetLevel);
        }

        FlushBatchDraw();

        DWORD end_time = GetTickCount();
        DWORD delta_time = end_time - start_time;
        if (delta_time < 1000 / 80)
        {
            Sleep(1000 / 80 - delta_time);
        }

    }
    EndBatchDraw();

    return 0;

}