#pragma once

#include <steam/steam_api.h>

#include "DuelMgr.h"

class SteamMgr {
    public:
        enum class MessageType : int {
            RequestBoardState,
            ReceiveBoardState,
            RequestGunState,
            ReceiveGunState,
            EndDuel,
            PlayAgain,
            UpdateName,
            UpdateChar
        };

        DuelMgr* mDuelMgr;
        bool mIsHost;
        uint64 mLobbyID;
        SteamNetworkingIdentity mOpponent;


        SteamMgr();
        ~SteamMgr();

        bool Init();
        void Update();

        void HostDuel();
        void EndDuel();
        bool IsMyTurn();

        void SendBoardState(SteamNetworkingIdentity& identityRemote);
        void HandleMessages();

    private:
        STEAM_CALLBACK(SteamMgr, OnLobbyCreated, LobbyCreated_t);
        STEAM_CALLBACK(SteamMgr, OnLobbyEntered, LobbyEnter_t);
        STEAM_CALLBACK(SteamMgr, GameLobbyJoinRequested, GameLobbyJoinRequested_t);
        STEAM_CALLBACK(SteamMgr, OnSessionRequest, SteamNetworkingMessagesSessionRequest_t);
        STEAM_CALLBACK(SteamMgr, OnSessionFailed, SteamNetworkingMessagesSessionFailed_t);
};