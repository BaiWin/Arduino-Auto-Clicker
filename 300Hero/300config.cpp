#include "300config.h"

const GameConfig config300Hero = {
    "300.exe",
    {
        {
            "zc",         // 模式名
            "Lobby",           // 初始状态
            {
                {
                    "Lobby", {
                        { BehaviorType::Hover, "hover", "resources/300hero/start.png", 0, 0, 0, 0, "SelectMode" }
                    }
                },
                {
                    "SelectMode", {
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/zc.png", 0, 0, 0, 0, "SelectHero", 3000, false, 0, false, 0.6 }
                    }
                },
                {
                    "SelectHero", {
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/rnd.png", 0, 0, 0, 0, "", 5000 },
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/go.png", 0, 0, 0, 0, "InGame" }
                    }
                },
                {
                    "InGame", {
                        { BehaviorType::RandomClick, "rightclick", "", 0, 0, 0, 0, "" },
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/return.png", 0, 0, 0, 0, "Lobby", 8000 }
                    }
                }
            }
        },
        {
            "tower",         // 模式名
            "InGame",           // 初始状态
            {
                {
                    "InGame", {
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/skillw.png", 0, 0, 100, 40, "", 1300 },
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/base.png", 0, 0, 560, -120, "", 1400 },
                    }
                }
            }
        },
        {
            "zcRoyale",         // 模式名
            "Event",           // 初始状态
            {
                {
                    "Event", {
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/zcRoyale.png", 0, 0, 0, 0, "Start"},
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/event.png", 0, 0, 0, 0, ""}
                    }
                },
                {
                    "Start", {
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/zcRoyaleStart.png", 0, 0, 0, 0, "SelectHero" }
                    }
                },
                {
                    "SelectHero", {
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/rnd.png", 0, 0, 0, -50, "", 5000 },
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/go.png", 0, 0, 0, 0, "InGame" }
                    }
                },
                {
                    "InGame", {
                        { BehaviorType::RandomClick, "rightclick", "", 0, 0, 0, 0, "" },
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/return.png", 0, 0, 0, -50, "Event", 8000 }
                    }
                },
            }
        },
        {
            "grail",
            "InMapStep1",
            {
                {
                    "InMapStep1", {
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/grid.png", 0, 0, 100, -100, "InMapStep2", 3000, false, 0, false, 0.8, 1},
                    }
                },
                {
                    "InMapStep2", {
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/attack.png", 0, 0, 0, 0, "InMapStep3", 3000, false, 0, false, 0.6, 1},
                    }
                },
                {
                    "InMapStep3", {
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/attackconfirm.png", 0, 0, 0, 0, "SelectHero", 3000, false, 0, false, 0.6, 1}
                    }
                },
                {
                    "SelectHero", {
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/rnd.png", 0, 0, 0, -50, ""},
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/go.png", 0, 0, 0, 0, "InGame" }
                    }
                },
                {
                    "InGame", {
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/menu.png", 0, 0, 0, 0, ""},
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/setting.png", 0, 0, 0, 0, "" },
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/leave.png", 0, 0, 0, 0, "" },
                        { BehaviorType::ImageClick, "leftclick", "resources/300hero/leaveconfirm.png", 0, 0, 0, 0, "InMap", 12000 },
                    }
                },
            }
        },

    }
};

//{ BehaviorType::KeyPressImage, "keypress", "resources/base.png", 0, 0, 150, -360, "", 3000, false, 0x45},
//{ BehaviorType::KeyPressImage, "keypress", "resources/base.png", 0, 0, 560, -120, "InGame", 3000, false, 0x45 }