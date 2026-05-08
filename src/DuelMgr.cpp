#include "DuelMgr.h"

DuelMgr::DuelMgr() {
    mApp = *reinterpret_cast<Sexy::ThunderballApp**>(0x00687394);

    if (mApp == nullptr) {
        std::printf("Error: ThunderballApp instance not found!\n");
        return;
    }

    // GameState
    *reinterpret_cast<int*>(reinterpret_cast<char*>(mApp)+0x760) = 3;

    Sexy::ThunderballApp::ShowBoard(mApp, true, true);

    mBoard = *reinterpret_cast<Sexy::Board**>(reinterpret_cast<char*>(mApp)+0x7B8);
    
    if (mBoard == nullptr) {
        std::printf("Error: Board instance not found!\n");
        return;
    }

    mGun = *reinterpret_cast<Sexy::Gun**>(reinterpret_cast<char*>(mBoard)+0x150);
    if (mGun == nullptr) {
        std::printf("Error: Gun instance not found!\n");
        return;
    }

    mLogicMgr = *reinterpret_cast<Sexy::LogicMgr**>(reinterpret_cast<char*>(mBoard)+0x154); 

    if (mLogicMgr == nullptr) {
        std::printf("Error: LogicMgr instance not found!\n");
        return;
    }

    mPlayerInfo = *reinterpret_cast<Sexy::PlayerInfo**>(reinterpret_cast<char*>(mApp)+0x878);
    if (mPlayerInfo == nullptr) {
        std::printf("Error: PlayerInfo instance not found!\n");
        return;
    }

    mPlayerTwoTurn = false;
    mLevel = "";
    mOpenedCharacterDialog = false;
}

// board is handled by the game
DuelMgr::~DuelMgr() {}

void DuelMgr::Update() {
    mPlayerTwoTurn = *reinterpret_cast<bool*>(reinterpret_cast<char*>(mLogicMgr)+0x128);
}

void DuelMgr::LoadRandomLevel() {
    int* theStage = reinterpret_cast<int*>(reinterpret_cast<char*>(mApp)+0x764);
    int* theLevel = reinterpret_cast<int*>(reinterpret_cast<char*>(mApp)+0x768);

    srand(time(0));
    *theStage = rand() % 11;
    *theLevel = rand() % 5;
    
    reinterpret_cast<void(__thiscall*)(Sexy::Board*)>(0x0042DA00)(mBoard);
}

int DuelMgr::LoadLevel(const char* levelName) {
    mLevel = levelName;

    reinterpret_cast<void(__thiscall*)(Sexy::Board*)>(0x0042DA00)(mBoard);

    int result = reinterpret_cast<int(__thiscall*)(Sexy::Board*, const std::string&)>(0x0042AAE0)(mBoard, mLevel);
    if (result == 0) return 0;

    return result;
}

