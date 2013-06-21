/*
	This file is part of Heriswap.

	@author Soupe au Caillou - Pierre-Eric Pelloux-Prayer
	@author Soupe au Caillou - Gautier Pelloux-Prayer

	Heriswap is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, version 3.

	Heriswap is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Heriswap.  If not, see <http://www.gnu.org/licenses/>.
*/
#pragma once

#include <string>
#include <map>

#include <base/EntityManager.h>

#include "systems/SoundSystem.h"
#include "systems/MusicSystem.h"

#include "modes/GameModeManager.h"

#include "Jukebox.h"
class Game;
class ScoreStorage;
class PlayerNameInputUI;
class GameStateManager;
class SuccessAPI;
class NameInputAPI;
class AdAPI;
class CommunicationAPI;

class PrivateData {
    public:
        PrivateData(HeriswapGame* game, GameContext* context, SuccessManager* successMgr);
        ~PrivateData();
        void Setup();
     // GameState state, stateBeforePause, newState;
     bool stateBeforePauseNeedEnter;
     Entity sky, decord2nd, decord1er;

     std::map<GameMode, GameModeManager*> mode2Manager;
     SuccessManager* successMgr;

     StorageAPI* storageAPI;

     GameMode mode;

     Entity cursor;

     Entity soundButton, socialGamNet;
     Entity musicMenu[2], musicStress1[2], musicStress2[2], music[8];
     Entity menu;
     struct {
	     Entity masterTrack;
	     Entity secondaryTracks[3];
	     Entity accessoryTrack;
	     Entity stressTrack;
     } inGameMusic;
     Jukebox jukebox;
     int scoreboardRankInSight;
     std::vector<int> bestScores;

    // animations timing
    struct {
        float deletion;
        float swap;
        float fall;
        float haveToAddLeavesInGrid;
        float replaceGrid;
    } timing;
};
