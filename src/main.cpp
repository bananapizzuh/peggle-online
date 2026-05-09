#include <MinHook.h>
#include <Windows.h>
#include <cstring>
#include <sdk/SexySDK.hpp>
#include <utils/memory.hpp>

#include <steam/steam_api.h>

#include "SteamMgr.h"

static SteamMgr* steamMgr = nullptr;

static void(__fastcall* Sexy__ThunderballApp__LostFocus_)(Sexy::ThunderballApp*, char*);
void __fastcall Sexy__ThunderballApp__LostFocus(Sexy::ThunderballApp* this_, char* edx) {
    return;
}

static void (__fastcall* Sexy__MainMenu__ButtonDepress_)(Sexy::MainMenu* this_, char* edx, int button);
void __fastcall Sexy__MainMenu__ButtonDepress(Sexy::MainMenu* this_, char* edx, int button) {
    if (button != 6) return Sexy__MainMenu__ButtonDepress_(this_, edx, button);

    if (steamMgr) {
        steamMgr->HostDuel();
    }
}

static void (__fastcall* Sexy__LogicMgr__UpdateGun_)(Sexy::LogicMgr* this_, char* edx);
void __fastcall Sexy__LogicMgr__UpdateGun(Sexy::LogicMgr* this_, char* edx) {
    if (steamMgr && steamMgr->mDuelMgr) {
        if (steamMgr->IsMyTurn()) { 
            Sexy__LogicMgr__UpdateGun_(this_, edx);
        }
    } else {
        Sexy__LogicMgr__UpdateGun_(this_, edx);
    }
}

static void (__fastcall* Sexy__Board__MouseDown_)(Sexy::Board* this_, char* edx, int a1, int a2, int a3);
void __fastcall Sexy__Board__MouseDown(Sexy::Board* this_, char* edx, int a1, int a2, int a3) {
    if (steamMgr && steamMgr->mDuelMgr) {
        if (steamMgr->IsMyTurn()) { 
            Sexy__Board__MouseDown_(this_, edx, a1, a2, a3);
        }
    } else {
        Sexy__Board__MouseDown_(this_, edx, a1, a2, a3);
    }
}

static void (__fastcall* Sexy__Board__Board_dtor_)(Sexy::Board* this_, char* edx);
void __fastcall Sexy__Board__Board_dtor(Sexy::Board* this_, char* edx) {
    if (steamMgr) {
        steamMgr->EndDuel();
    }
    Sexy__Board__Board_dtor_(this_, edx);
}

static void (__fastcall* Sexy__EndLevelDialog__ButtonDepress_)(void* this_, char* edx, int button);
void __fastcall Sexy__EndLevelDialog__ButtonDepress(void* this_, char* edx, int theId) {
    if (steamMgr) {
        if (theId == 1000) {
            steamMgr->EndDuel();
        } else if (theId == 1001) {
            if (!steamMgr->mIsHost) {
                std::string header = "Not Host";
                std::string lines = "You are unable to do this action. Only the host can choose to play again.";
                std::string footer = "OK";
                Sexy::ThunderballApp::DoDialogScroll(steamMgr->mDuelMgr->mApp, 101, true, header, lines, footer, 3);
            } else {
                reinterpret_cast<void(__thiscall*)(Sexy::Board*)>(0x0042DA00)(steamMgr->mDuelMgr->mBoard);
                steamMgr->SendBoardState(steamMgr->mOpponent);
                Sexy::Buffer buffer;
                buffer.WriteLong(static_cast<int>(SteamMgr::MessageType::PlayAgain));
                if (!steamMgr->mOpponent.IsInvalid()) {
                    SteamNetworkingMessages()->SendMessageToUser(steamMgr->mOpponent, buffer.GetDataPtr(), buffer.GetDataLen(), k_nSteamNetworkingSend_Reliable, 0);
                }
            }
        } else if (theId == 0) {
            if (!steamMgr->mIsHost) {
                std::string header = "Not Host";
                std::string lines = "You are unable to do this action. Only the host can choose to play again.";
                std::string footer = "OK";
                Sexy::ThunderballApp::DoDialogScroll(steamMgr->mDuelMgr->mApp, 101, true, header, lines, footer, 3);
            } else {
                steamMgr->mDuelMgr->LoadRandomLevel();
                Sexy::Buffer buffer;
                buffer.WriteLong(static_cast<int>(SteamMgr::MessageType::PlayAgain));
                if (!steamMgr->mOpponent.IsInvalid()) {
                    SteamNetworkingMessages()->SendMessageToUser(steamMgr->mOpponent, buffer.GetDataPtr(), buffer.GetDataLen(), k_nSteamNetworkingSend_Reliable, 0);
                }
                
            }
        }
    } else {
        Sexy__EndLevelDialog__ButtonDepress_(this_, edx, theId);
    }
}

static char* (__fastcall* Sexy__CharacterDialog__CharacterDialog_)(void* this_, char* edx, Sexy::Board* board);
char* __fastcall Sexy__CharacterDialog__CharacterDialog(void* this_, char* edx, Sexy::Board* board) {
    if (steamMgr && steamMgr->mDuelMgr) {
        *reinterpret_cast<bool*>(reinterpret_cast<char*>(steamMgr->mDuelMgr->mLogicMgr)+0x124) = false;
        char* result = Sexy__CharacterDialog__CharacterDialog_(this_, edx, board);
        *reinterpret_cast<bool*>(reinterpret_cast<char*>(steamMgr->mDuelMgr->mLogicMgr)+0x124) = true;
        return result;
    }

    return Sexy__CharacterDialog__CharacterDialog_(this_, edx, board);
}

static void (__fastcall* Sexy__CharacterDialog__SaveDetails_)(void* this_, char* edx);
void __fastcall Sexy__CharacterDialog__SaveDetails(void* this_, char* edx) {
    if (steamMgr && steamMgr->mDuelMgr) {
        int oldP1CharIndex = *reinterpret_cast<int*>(steamMgr->mDuelMgr->mPlayerInfo + 0x40);
        int oldP2CharIndex = *reinterpret_cast<int*>(steamMgr->mDuelMgr->mPlayerInfo + 0x44);

        Sexy__CharacterDialog__SaveDetails_(this_, edx);

        int p1CharIndex, p2CharIndex;
        if (steamMgr->mIsHost) {
            p1CharIndex = *reinterpret_cast<int*>(steamMgr->mDuelMgr->mPlayerInfo + 0x40);
            p2CharIndex = oldP2CharIndex;
        } else {
            p1CharIndex = oldP1CharIndex;
            p2CharIndex = *reinterpret_cast<int*>(steamMgr->mDuelMgr->mPlayerInfo + 0x40);
        }

        *reinterpret_cast<int*>(steamMgr->mDuelMgr->mPlayerInfo + 0x40) = p1CharIndex;
        *reinterpret_cast<int*>(steamMgr->mDuelMgr->mPlayerInfo + 0x44) = p2CharIndex;

        Sexy::Buffer buffer;
        buffer.WriteLong(static_cast<int>(SteamMgr::MessageType::UpdateChar));
        buffer.WriteLong(p1CharIndex);
        buffer.WriteLong(p2CharIndex);
        if (!steamMgr->mOpponent.IsInvalid()) {
            SteamNetworkingMessages()->SendMessageToUser(steamMgr->mOpponent, buffer.GetDataPtr(), buffer.GetDataLen(), k_nSteamNetworkingSend_Reliable, 0);
        }
    } else {
        Sexy__CharacterDialog__SaveDetails_(this_, edx);
    }
}

void init() {
    set(0x4a7cce + 0x1, "Online");
    set(0x4a7cc3 + 0x1, (char)0x06);

    steamMgr = new SteamMgr();
    if (!steamMgr->Init()) {
      return;
    }

    MH_Initialize();

    Sexy::callbacks::on(Sexy::callbacks::type::main_loop, []() {
        if (steamMgr) {
            steamMgr->Update();
        }
    });

    Sexy::callbacks::after_begin_shot([](Sexy::LogicMgr* logic_mgr, bool doGetReplayPoint) {
        std::printf("after_begin_shot callback triggered. IsMyTurn: %d\n", steamMgr ? steamMgr->IsMyTurn() : -1);
        if (steamMgr && steamMgr->IsMyTurn()) {
            steamMgr->SendBoardState(steamMgr->mOpponent);
        }
    });

    MH_CreateHook((void*)0x00408470, Sexy__ThunderballApp__LostFocus, (void**)&Sexy__ThunderballApp__LostFocus_);
    MH_CreateHook((void*)0x004afd20, Sexy__MainMenu__ButtonDepress, (void**)&Sexy__MainMenu__ButtonDepress_);
    MH_CreateHook((void*)0x00402810, Sexy__Board__MouseDown, (void**)&Sexy__Board__MouseDown_);
    MH_CreateHook((void*)0x0046d630, Sexy__LogicMgr__UpdateGun, (void**)&Sexy__LogicMgr__UpdateGun_);
    MH_CreateHook((void*)0x004299a0, Sexy__Board__Board_dtor, (void**)&Sexy__Board__Board_dtor_);
    MH_CreateHook((void*)0x0049dc20, Sexy__EndLevelDialog__ButtonDepress, (void**)&Sexy__EndLevelDialog__ButtonDepress_);
    MH_CreateHook((void*)0x004b2990, Sexy__CharacterDialog__CharacterDialog, (void**)&Sexy__CharacterDialog__CharacterDialog_);
    MH_CreateHook((void*)0x004a0f90, Sexy__CharacterDialog__SaveDetails, (void**)&Sexy__CharacterDialog__SaveDetails_);

    MH_EnableHook(MH_ALL_HOOKS);
}

DWORD WINAPI OnAttachImpl(LPVOID lpParameter) {
  init();
  return 0;
}

DWORD WINAPI OnAttach(LPVOID lpParameter) {
  __try {
    return OnAttachImpl(lpParameter);
  } __except (0) {
    FreeLibraryAndExitThread((HMODULE)lpParameter, 0xDECEA5ED);
  }

  return 0;
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
  switch (dwReason) {
  case DLL_PROCESS_ATTACH:
    DisableThreadLibraryCalls(hModule);
    CreateThread(nullptr, 0, OnAttach, hModule, 0, nullptr);
    return true;
  }

  return false;
}