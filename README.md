# Peggle Online
> [!WARNING]
> This mod only works on Peggle Deluxe 1.01 on steam!

This mod adds online duels through steam networking. This is a very early version that I'm unsure about maintaining, so expect bugs. Currently it only works on Peggle Deluxe 1.01, but it shouldn't theoretically be too hard to get working on nights. Steam is required in order to play multiplayer. 

## Instructions
Click the online button that now replaces the demo button on the main menu. The steam lobbies are open to join or are joinable via invite. In order to invite or join someone, you might have to enable or reinitialize the 3D acceleration setting to get steam to hook the overlay into peggle.

## Current State
Only the host can change characters and player names, and the peers desync after one level is played.

### Known Bugs:
 - The ball can disappear somewhere in the transfering of turns.
 - Desync is very easy to achieve (especially through fast-forward).
 - Character setting sometimes override choices.

## Installation
> [!NOTE]
> This mod requires the [Haggle Mod Loader](https://github.com/PeggleCommunity/haggle-mod-loader/releases/latest).

Either get the latest release from the [releases page](https://github.com/bananapizzuh/peggle-online/releases/latest), or build from scratch. Then put the dll in the mods folder of your respective game (usually in `C:\Program Files (x86)\Steam\steamapps\common\`).

## Building
- Clone the github reposity recursively: `git clone --recursive https://github.com/bananapizzuh/peggle-online`
  - Alternatively clone then initialize submodules: `git submodule update --init --recursive`
- Configure the build system with premake: `tools\premake5.exe vs2022` 
- Build with either Visual Studio or run `msbuild build\online-duel.sln` in a developer command prompt.
