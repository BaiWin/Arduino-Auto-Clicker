#include "hearthstoneconfig.h"

const GameConfig configHearthStone = {
    "Hearthstone.exe",
    {
        {
            "rank",         // 模式名
            "FindMatch",           // 初始状态
            {
                {
                    "FindMatch", {
                        { BehaviorType::ImageClick, "leftclick", "resources/hearthstone/findmatch.png", 0, 0, 0, 0, "SelectCard", 20000 }
                    }
                },

                {
                    "SelectCard", {
                        { BehaviorType::ImageClick, "leftclick", "resources/hearthstone/selectcard.png", 0, 0, 0, 0, "InGame", 8000 }
                    }
                },
                {
                    "InGame", {                  // "" = do nothing
                        { BehaviorType::ImageClick, "leftclick", "resources/hearthstone/nextgame.png", 0, 0, 0, -500, "EndCheck"},
                        { BehaviorType::ImageClick, "", "resources/hearthstone/turn1.png", 0, 0, -200, 0, "Turn1", 500},
                    }

                },
                {
                    "Turn1", {
                        { BehaviorType::Drag, "drag", "", 1280, 1380, 0, -980, ""},
                        { BehaviorType::FreeClick, "rightclick", "", 1280, 1200, 0, 0, "", 1000},
                        { BehaviorType::FreeClick, "leftclick", "", 1480, 1200, 0, 0, "", 1000},
                        { BehaviorType::ImageClick, "leftclick", "resources/hearthstone/turn1.png", 0, 0, 0, 0, "InGame", 10000},
                    }

                },
                {
                    "EndCheck", {
                        { BehaviorType::ImageClick, "leftclick", "resources/hearthstone/nextgame.png", 0, 0, 0, -500, "FindMatch", 8000},
                    }

                },
                /*{
                    "Turn2", {
                        { BehaviorType::ImageClick, "leftclick", "resources/hearthstone/nextgame.png", 0, 0, 0, -500, "FindMatch" },
                        { BehaviorType::ImageClick, "", "resources/hearthstone/turn1.png", 0, 0, -200, 0, "InGame" }
                    }

                },*/
            }
        },
    }
};