#pragma once

#include <string>

#include <sdk/SexySDK.hpp>

class DuelMgr {
public:
    Sexy::ThunderballApp* mApp;
    Sexy::Board* mBoard;
    Sexy::Gun* mGun;
    Sexy::LogicMgr* mLogicMgr;
    Sexy::PlayerInfo* mPlayerInfo;
    bool mPlayerTwoTurn;
    std::string mLevel;
    bool mOpenedCharacterDialog;

    DuelMgr();
    ~DuelMgr();

    void Update();
    void LoadRandomLevel();
    int LoadLevel(const char* levelName);
};