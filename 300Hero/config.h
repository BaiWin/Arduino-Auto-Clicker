#pragma once
#include <string>
#include <vector>

enum class BehaviorType
{
    ImageClick,
    FreeClick,
    RandomClick,
    Hover,
    Drag,
    KeyPressImage,
    FreeKeyPress,
};

enum class ExecutionState
{
    GoNext,
    Loop,
};

// 一个行为
struct Action
{
    BehaviorType type;
    std::string actionName;   // 比如 "leftclick", "drag"
    std::string imagePath;    // 仅 ImageClick 使用
    int x = 0, y = 0;         // 用于 FreeClick / Drag, 目标位置
    int x2 = 0, y2 = 0;       // Drag 增量, Image偏移
    std::string nextState;    // 行为执行完成后跳转状态
    int delay = 3000;         // 执行完该行为后的延迟（毫秒）
    bool executed = false;    // 标记是否执行过
    int virtualKey = 0;       // KeyPressImage 使用的虚拟按键码
    bool useColorMatch = false;       // 是否启用色彩
    double threshold = 0.7;   // 匹配阈值
    int imageCaptureMethod = 0;
};

// 一个状态
struct State
{
    std::string name;
    std::vector<Action> actions;
};

struct GameModeConfig
{
    std::string modeName;              // 模式名，如 "Match", "Ranked"
    std::string initialState;
    std::vector<State> states;        // 该模式的状态机
};

struct GameConfig
{
    std::string exeName;
    std::vector<GameModeConfig> modes;  // 所有模式
};

extern GameModeConfig* currentMode;

extern GameConfig myGame;

extern std::string selectedModeName;

extern std::string currentState;
