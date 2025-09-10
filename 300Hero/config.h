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

// һ����Ϊ
struct Action
{
    BehaviorType type;
    std::string actionName;   // ���� "leftclick", "drag"
    std::string imagePath;    // �� ImageClick ʹ��
    int x = 0, y = 0;         // ���� FreeClick / Drag, Ŀ��λ��
    int x2 = 0, y2 = 0;       // Drag ����, Imageƫ��
    std::string nextState;    // ��Ϊִ����ɺ���ת״̬
    int delay = 3000;         // ִ�������Ϊ����ӳ٣����룩
    bool executed = false;    // ����Ƿ�ִ�й�
    int virtualKey = 0;       // KeyPressImage ʹ�õ����ⰴ����
    bool useColorMatch = false;       // �Ƿ�����ɫ��
    double threshold = 0.7;   // ƥ����ֵ
    int imageCaptureMethod = 0;
};

// һ��״̬
struct State
{
    std::string name;
    std::vector<Action> actions;
};

struct GameModeConfig
{
    std::string modeName;              // ģʽ������ "Match", "Ranked"
    std::string initialState;
    std::vector<State> states;        // ��ģʽ��״̬��
};

struct GameConfig
{
    std::string exeName;
    std::vector<GameModeConfig> modes;  // ����ģʽ
};

extern GameModeConfig* currentMode;

extern GameConfig myGame;

extern std::string selectedModeName;

extern std::string currentState;
