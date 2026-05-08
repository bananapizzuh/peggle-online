#include "SteamMgr.h"
#include "Sync.h"

#include <sdk/SexySDK.hpp>

#include <steam/steam_api.h>

SteamMgr::SteamMgr() {
    mIsHost = false;
    mDuelMgr = nullptr;
    mLobbyID = 0;
    mOpponent = {};
}

SteamMgr::~SteamMgr() {
    if (mDuelMgr) delete mDuelMgr;
}

bool SteamMgr::Init() {
    if (!SteamAPI_Init()) { 
        std::printf("Steam must be running to play multiplayer!\n");
        return false;
    }

    if (!SteamMatchmaking()) {
        std::printf("Failed to initialize Steam Matchmaking!\n");
        return false;
    }

    return true;
}

static int gUpdateCounter = 0;

void SteamMgr::Update() {
    SteamAPI_RunCallbacks();
    HandleMessages();

    if (!mDuelMgr) return;

    mDuelMgr->Update();


    gUpdateCounter++;
    if (gUpdateCounter >= 100) {
        gUpdateCounter = 0;
    }

    if (gUpdateCounter % 2 == 0 && mDuelMgr->mBoard && !IsMyTurn()) {
        Sexy::Buffer buffer;
        buffer.WriteLong(static_cast<int>(MessageType::RequestGunState));

        if (!mOpponent.IsInvalid()) {
            SteamNetworkingMessages()->SendMessageToUser(mOpponent, buffer.GetDataPtr(), buffer.GetDataLen(), k_nSteamNetworkingSend_Reliable, 0);
        }
    }
}

bool SteamMgr::IsMyTurn() {
    if (!mDuelMgr) return false;
    return mDuelMgr->mPlayerTwoTurn != mIsHost;
}

void SteamMgr::HostDuel() {
    if (mIsHost) {
        std::printf("Already hosting a lobby!\n");
        return;
    }

    SteamMatchmaking()->CreateLobby(k_ELobbyTypeFriendsOnly, 2);
}

void SteamMgr::EndDuel() {
        if (mIsHost) {
            Sexy::Buffer buffer;
            buffer.WriteLong(static_cast<int>(MessageType::EndDuel));
            if (!mOpponent.IsInvalid()) {
                SteamNetworkingMessages()->SendMessageToUser(mOpponent, buffer.GetDataPtr(), buffer.GetDataLen(), k_nSteamNetworkingSend_Reliable, 0);
            }
        }

        if (mDuelMgr) {
            reinterpret_cast<void(__thiscall*)(Sexy::ThunderballApp*)>(0x0042D7A0)(mDuelMgr->mApp);
            if (!mIsHost) {
                std::string header = "Disconnected";
                std::string lines = "You have been disconnected from the game.";
                std::string footer = "OK";
                Sexy::ThunderballApp::DoDialogScroll(mDuelMgr->mApp, 100, false, header, lines, footer, 3);
            }
            mDuelMgr->mBoard = nullptr;
            delete mDuelMgr;
            mDuelMgr = nullptr;
        }

        if (mLobbyID != 0) {
            SteamMatchmaking()->LeaveLobby(mLobbyID);
            mLobbyID = 0;
        }
    
        mIsHost = false;
        mOpponent = {};

}

void SteamMgr::SendBoardState(SteamNetworkingIdentity& identityRemote) {
    if (!mDuelMgr || !mDuelMgr->mBoard) return;

    Sexy::Buffer buffer;
    buffer.WriteLong(static_cast<int>(MessageType::ReceiveBoardState));
    WriteBufferFrom(&buffer, mDuelMgr->mBoard);

    if (identityRemote.IsInvalid()) {
        std::printf("Cannot send board state: invalid remote identity!\n");
        return;
    }

    SteamNetworkingMessages()->SendMessageToUser(identityRemote, buffer.GetDataPtr(), buffer.GetDataLen(), k_nSteamNetworkingSend_Reliable, 0);
}

void SteamMgr::HandleMessages() {
    SteamNetworkingMessage_t* pIncomingMsg = nullptr;
    int numMsgs = SteamNetworkingMessages()->ReceiveMessagesOnChannel(0, &pIncomingMsg, 1);
    if (numMsgs > 0) {
        if (mOpponent.IsInvalid()) {
            mOpponent = pIncomingMsg->m_identityPeer;
            std::printf("Set opponent SteamID to: %llu\n", mOpponent.GetSteamID64());
        }

        Sexy::Buffer buffer;
        buffer.SetData((uchar*)pIncomingMsg->m_pData, pIncomingMsg->m_cbSize);
        int messageType = buffer.ReadLong();

        switch (messageType) {
            case static_cast<int>(MessageType::RequestBoardState):
                buffer.Clear();
                buffer.WriteLong(static_cast<int>(MessageType::ReceiveBoardState));
                WriteBufferFrom(&buffer, mDuelMgr->mBoard);
                SteamNetworkingMessages()->SendMessageToUser(pIncomingMsg->m_identityPeer, buffer.GetDataPtr(), buffer.GetDataLen(), k_nSteamNetworkingSend_Reliable, 0);
                break;

            case static_cast<int>(MessageType::ReceiveBoardState):
                 if (mDuelMgr && mDuelMgr->mBoard) {
                    WriteBufferTo(&buffer, mDuelMgr->mBoard);

                    if (!mDuelMgr->mOpenedCharacterDialog && !mIsHost) {
                        reinterpret_cast<void(__thiscall*)(Sexy::Board*, bool)>(0x004022f0)(mDuelMgr->mBoard, true);
                        mDuelMgr->mOpenedCharacterDialog = true;
                    }
                }
                break;

            case static_cast<int>(MessageType::RequestGunState):
                if (mDuelMgr && mDuelMgr->mGun) {
                    buffer.Clear();
                    buffer.WriteLong(static_cast<int>(MessageType::ReceiveGunState));
                    WriteBufferFrom(&buffer, mDuelMgr->mGun);
                    SteamNetworkingMessages()->SendMessageToUser(pIncomingMsg->m_identityPeer, buffer.GetDataPtr(), buffer.GetDataLen(), k_nSteamNetworkingSend_Reliable, 0);
                }
                break;

            case static_cast<int>(MessageType::ReceiveGunState):
                if (mDuelMgr && mDuelMgr->mGun) {
                    WriteBufferTo(&buffer, mDuelMgr->mGun);
                }
                break;

            case static_cast<int>(MessageType::EndDuel):
                EndDuel();
                break;

            case static_cast<int>(MessageType::PlayAgain):
                if (mDuelMgr && mDuelMgr->mBoard) {
                    reinterpret_cast<void(__thiscall*)(Sexy::ThunderballApp*, int, bool, bool)>(0x0052e350)(mDuelMgr->mApp, 12, true, true);
                    reinterpret_cast<void(__thiscall*)(Sexy::ThunderballApp*, int, bool, bool)>(0x0052e350)(mDuelMgr->mApp, 101, true, true);

                    reinterpret_cast<void(__thiscall*)(Sexy::Board*)>(0x0042DA00)(mDuelMgr->mBoard);

                    buffer.Clear();
                    buffer.WriteLong(static_cast<int>(MessageType::RequestBoardState));
                    if (!mOpponent.IsInvalid()) {
                        SteamNetworkingMessages()->SendMessageToUser(mOpponent, buffer.GetDataPtr(), buffer.GetDataLen(), k_nSteamNetworkingSend_Reliable, 0);
                    }
                }
                break;

            case static_cast<int>(MessageType::UpdateName):
                    if (mDuelMgr && mIsHost) {
                        std::printf("Received player name from opponent.\n");

                        std::string aString;
	                    int aLen = buffer.ReadShort();
	                    for (int i = 0; i < aLen; i++)
		                    aString += (char) buffer.ReadByte();

                        std::printf("Opponent name: %s\n", aString.c_str());
                        *reinterpret_cast<std::string*>(mDuelMgr->mPlayerInfo + 0xb4) = aString;
                    }
                break;
                
            case static_cast<int>(MessageType::UpdateChar):
                    if (mDuelMgr) {
                        std::printf("Received character choice from opponent.\n");
                        int p1CharIndex = buffer.ReadLong();
                        int p2CharIndex = buffer.ReadLong();
                        std::printf("Opponent character indices: %d, %d\n", p1CharIndex, p2CharIndex);
                        *reinterpret_cast<int*>(mDuelMgr->mPlayerInfo + 0x40) = p1CharIndex;
                        *reinterpret_cast<int*>(mDuelMgr->mPlayerInfo + 0x44) = p2CharIndex;
                        reinterpret_cast<void(__thiscall*)(Sexy::Board*, bool)>(0x0040f400)(mDuelMgr->mBoard, true);
                    }
                break;

            default:
                std::printf("Received unknown message type %d from SteamID: %llu\n", messageType, pIncomingMsg->m_identityPeer.GetSteamID64());
                break;
        }

        pIncomingMsg->Release();
    }
}

void SteamMgr::OnLobbyCreated(LobbyCreated_t* pCallback) {
    if (pCallback->m_eResult != k_EResultOK) {
        std::printf("Failed to create lobby!\n");
        return;
    }

    mIsHost = true;
    mDuelMgr = new DuelMgr();

    std::printf("Lobby created successfully! Lobby ID: %llu\n", pCallback->m_ulSteamIDLobby);
    mLobbyID = pCallback->m_ulSteamIDLobby;

    mDuelMgr->LoadRandomLevel();
}

void SteamMgr::OnLobbyEntered(LobbyEnter_t* pCallback) {
    if (mIsHost) return;

    if (pCallback->m_EChatRoomEnterResponse != k_EChatRoomEnterResponseSuccess) {
        std::printf("Failed to enter lobby!\n");
        return;
    }

    std::printf("Entered lobby successfully! Lobby ID: %llu\n", pCallback->m_ulSteamIDLobby);
    mLobbyID = pCallback->m_ulSteamIDLobby;

    mDuelMgr = new DuelMgr();

    std::printf("Requesting board state from host...\n");

    mOpponent.SetSteamID(SteamMatchmaking()->GetLobbyOwner(mLobbyID));
    std::printf("Host SteamID: %llu\n", mOpponent.GetSteamID64());

    if (mOpponent.IsInvalid()) {
        std::printf("Failed to get host SteamID\n");
        return;
    }

    Sexy::Buffer buffer;
    buffer.WriteLong(static_cast<int>(MessageType::RequestBoardState));

    SteamNetworkingMessages()->SendMessageToUser(mOpponent, buffer.GetDataPtr(), buffer.GetDataLen(), k_nSteamNetworkingSend_Reliable, 0);
    
    buffer.Clear();
    buffer.WriteLong(static_cast<int>(MessageType::UpdateName));
    std::string playerName = *reinterpret_cast<std::string*>(mDuelMgr->mPlayerInfo + 0x4);
    
    buffer.WriteString(playerName);
    SteamNetworkingMessages()->SendMessageToUser(mOpponent, buffer.GetDataPtr(), buffer.GetDataLen(), k_nSteamNetworkingSend_Reliable, 0);
}

void SteamMgr::OnSessionRequest(SteamNetworkingMessagesSessionRequest_t* pCallback) {
    std::printf("Received session request from SteamID: %llu\n", pCallback->m_identityRemote.GetSteamID64());
    if (!SteamNetworkingMessages()->AcceptSessionWithUser(pCallback->m_identityRemote)) {
        std::printf("Failed to accept session with user %llu\n", pCallback->m_identityRemote.GetSteamID64());
    } else {
        std::printf("Accepted session with user %llu\n", pCallback->m_identityRemote.GetSteamID64());
    }
}

void SteamMgr::OnSessionFailed(SteamNetworkingMessagesSessionFailed_t* pCallback) {
    std::printf("Session failed with SteamID: %llu, state: %d\n", pCallback->m_info.m_identityRemote.GetSteamID64(), pCallback->m_info.m_eState);
}

void SteamMgr::GameLobbyJoinRequested(GameLobbyJoinRequested_t* pCallback) {
    std::printf("Received game lobby join request from SteamID: %llu\n", pCallback->m_steamIDFriend);

    SteamMatchmaking()->JoinLobby(pCallback->m_steamIDLobby);
}