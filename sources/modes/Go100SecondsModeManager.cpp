/*
    This file is part of Heriswap.

    @author Soupe au Caillou - Jordane Pelloux-Prayer
    @author Soupe au Caillou - Gautier Pelloux-Prayer
    @author Soupe au Caillou - Pierre-Eric Pelloux-Prayer

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


#include "Go100SecondsModeManager.h"

#include "DepthLayer.h"
#include "CombinationMark.h"

#include "systems/HeriswapGridSystem.h"

#include "base/PlacementHelper.h"
#include "base/EntityManager.h"

#include "systems/AnimationSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/PhysicsSystem.h"
#include "systems/TextSystem.h"
#include "systems/TransformationSystem.h"

#include "util/Random.h"

#include <glm/glm.hpp>

#include <iomanip>
#include <sstream>


Go100SecondsGameModeManager::Go100SecondsGameModeManager(HeriswapGame* game, SuccessManager* successMgr, StorageAPI* sAPI) : GameModeManager(game, successMgr, sAPI) {
}

Go100SecondsGameModeManager::~Go100SecondsGameModeManager() {
}

void Go100SecondsGameModeManager::Setup() {
	GameModeManager::Setup();

	validBranchPos.clear();
	#include "PositionFeuilles.h"
	for (int i=0; i<8*6; i++) {
		glm::vec2 v(PlacementHelper::GimpXToScreen(pos[3*i]), PlacementHelper::GimpYToScreen(pos[3*i+1]));
		Render truc = {v, glm::radians<float>(pos[3*i+2])};
		validBranchPos.push_back(truc);
	}

	for (int i = 0; i < 100;  i++) {
		squallLeaves.push_back(GameModeManager::createAndAddLeave(0, glm::vec2(0, 0), Random::Float(0.f, 7.f)));
		Entity e = squallLeaves[i];

		ADD_COMPONENT(e, Physics);
		PHYSICS(e)->mass = Random::Float(1.f, 10.f);
		PHYSICS(e)->gravity = glm::vec2(0 ,0*-10.f);

		RENDERING(e)->show = false;
	}
}

void Go100SecondsGameModeManager::initPosition() {
	limit = 100;
	pts.clear();
	pts.push_back(glm::vec2(0.f, 0.f));

	pts.push_back(glm::vec2(limit, 1.f));//need limit leaves to end game
}
#define RATE 1
void Go100SecondsGameModeManager::Enter() {
	time = 0;
	limit = 100;
	points = 0;
	squallGo = false;
	squallDuration = 0.f;
	bonus = Random::Int(0, theHeriswapGridSystem.Types-1);

	initPosition();

	generateLeaves(0, 8);

	for (unsigned int i = 0; i < squallLeaves.size();  i++)
		RENDERING(squallLeaves[i])->texture = theRenderingSystem.loadTextureFile(HeriswapGame::cellTypeToTextureNameAndRotation(bonus, &TRANSFORM(squallLeaves[i])->rotation));

	TEXT(uiHelper.scoreProgress)->flags |= TextComponent::IsANumberBit;

	GameModeManager::Enter();
}

void Go100SecondsGameModeManager::squall() {
	squallGo = true;
	float minX = PlacementHelper::ScreenSize.x/2.f + 1;
	float maxX = PlacementHelper::ScreenSize.x/2.f + 3;

	float minY = PlacementHelper::GimpYToScreen(1000);
	float maxY = -PlacementHelper::ScreenSize.y / 2.f;
	for (unsigned int i = 0; i < squallLeaves.size();  i++) {

		Entity  e = squallLeaves[i];
		TRANSFORM(e)->position = glm::vec2(Random::Float(minX, maxX), Random::Float(minY, maxY));
		TRANSFORM(e)->size = HeriswapGame::CellSize(8, 0) * HeriswapGame::CellContentScale() * Random::Float(0.35f,1.2f);
		RENDERING(e)->texture = theRenderingSystem.loadTextureFile(HeriswapGame::cellTypeToTextureNameAndRotation(bonus, &TRANSFORM(e)->rotation));
		RENDERING(e)->show = true;

		Force force;
		force.vector = glm::vec2(-45, 0);
		force.point =  glm::vec2( 0, Random::Float(TRANSFORM(e)->size.y/48.f, TRANSFORM(e)->size.y/2.f));

		std::pair<Force, float> f (force, 1.f);
		PHYSICS(e)->forces.push_back(f);
		PHYSICS(e)->mass = Random::Float(1.f, 10.f); // go update (dumb PhysicsSystem!)
	}
	//herisson's Y. He'll change his bonus behind this leaf
	TRANSFORM(squallLeaves[0])->position = glm::vec2(Random::Float(minX, maxX), PlacementHelper::GimpYToScreen(1028));
	TRANSFORM(squallLeaves[0])->size = HeriswapGame::CellSize(8, 0) * HeriswapGame::CellContentScale();
}

void Go100SecondsGameModeManager::Exit() {
	GameModeManager::Exit();
}

void Go100SecondsGameModeManager::GameUpdate(float dt, Scene::Enum state) {
	//update UI (pause button, etc)
	if (HeriswapGame::pausableState(state))
	    uiHelper.update(dt);

	if (!HeriswapGame::inGameState(state))
	    return;

	time+=dt;
	//squall is running...
	if (squallGo) {
		squallDuration += dt;
		//if the central leaf is next to the herisson, change his bonus
		if (TRANSFORM(squallLeaves[0])->position.x <= TRANSFORM(herisson)->position.x) {
			// LoadHerissonTexture(bonus+1);
            char tmp[32];
            snprintf(tmp, 32, "herisson_%d", bonus + 1);
			ANIMATION(herisson)->name = Murmur::RuntimeHash(tmp);
			// RENDERING(herisson)->texture = theRenderingSystem.loadTextureFile(c->anim[0]);
		}
		//make the tree leaves grow ...
		for (unsigned int i = 0; i < branchLeaves.size(); i++) {
			TRANSFORM(branchLeaves[i].e)->size =
				HeriswapGame::CellSize(8, bonus) * HeriswapGame::CellContentScale() * glm::min(squallDuration, 1.f);
		}
		//check if every leaves has gone...
		bool ended = true;
		for (unsigned int i = 0 ; i < squallLeaves.size() ; i++) {
			if (TRANSFORM(squallLeaves[i])->position.x >= -PlacementHelper::ScreenSize.x * 0.5 - TRANSFORM(squallLeaves[i])->size.x)
				ended = false;
		}
		//YES : stop the squall, keep playing !
		if (ended) {
			squallGo = false;
			squallDuration = 0.f;
			for (unsigned int i = 0; i < squallLeaves.size(); i++) {
				RENDERING(squallLeaves[i])->show = false;
				PHYSICS(squallLeaves[i])->linearVelocity = glm::vec2(0.f);
				PHYSICS(squallLeaves[i])->angularVelocity = 0.f;
				PHYSICS(squallLeaves[i])->mass = 0.f; // stop update (dumb PhysicsSystem!)
			}


		}
	} else {
		//oh noes, no longer leaf on tree ! Give me new one
		if (branchLeaves.size() == 0) {
			//Ok, but first u'll have a new bonus
			bonus = Random::Int(0, theHeriswapGridSystem.Types-1);
			//And leaves aren't magic, they need to grow ... be patient.
			generateLeaves(0, 8);
			for (unsigned int i = 0; i < branchLeaves.size(); i++)
				TRANSFORM(branchLeaves[i].e)->size = glm::vec2(0.f);
			//The squall will make them grow
			squall();
		}
	}

}

float Go100SecondsGameModeManager::GameProgressPercent() {
	return glm::min(1.0f, (float)time/limit);
}

void Go100SecondsGameModeManager::UiUpdate(float dt) {
	//Score
	{
	std::stringstream a;
	a.precision(0);
	//~not enable currently
	//~a << rank << ". ";
	a << std::fixed << points;
	TEXT(uiHelper.scoreProgress)->text = a.str();
	}

	updateHerisson(dt, time, 0);

#if SAC_DEBUG
	if (_debug) {
		for(int i=0; i<8; i++) {
			std::stringstream text;
			text << countBranchLeavesOfType(i);
			TEXT(debugEntities[2*i+1])->text = text.str();
			TEXT(debugEntities[2*i+1])->show = true;
			TEXT(debugEntities[2*i+1])->color = Color(0.2f, 0.2f, 0.2f);
		}
	}
#endif
}

void Go100SecondsGameModeManager::ScoreCalc(int nb, unsigned int type) {
	Difficulty diff = theHeriswapGridSystem.sizeToDifficulty();

	float score = 10 * nb * nb * nb * nb;

	switch (diff) {
		case DifficultyEasy: break;
		case DifficultyMedium: score *= 2; break;
		case DifficultyHard: score *= 4; break;
		default:
			LOGE("Unhandled difficulty: '" << diff << "'");
	}

	if (type == bonus) {
		score *= 2;
		deleteLeaves(~0u, 2*nb);
	} else {
		deleteLeaves(~0u, nb);
	}
	points += score;

}

void Go100SecondsGameModeManager::TogglePauseDisplay(bool paused) {
	GameModeManager::TogglePauseDisplay(paused);
}

void Go100SecondsGameModeManager::WillScore(int count, int, std::vector<BranchLeaf>& LeavesToDelete) {
    int nb = glm::min((int)branchLeaves.size(), count);
    for (unsigned int i=0; nb>0 && i<branchLeaves.size(); i++) {
		CombinationMark::markCellInCombination(branchLeaves[i].e);
        LeavesToDelete.push_back(branchLeaves[i]);
        nb--;
    }
}

int Go100SecondsGameModeManager::saveInternalState(uint8_t** out) {
    uint8_t* tmp;
    int parent = GameModeManager::saveInternalState(&tmp);
    uint8_t* ptr = *out = new uint8_t[parent];
    MEMPCPY(uint8_t*, ptr, tmp, parent);


    delete[] tmp;
    return (parent);
}

const uint8_t* Go100SecondsGameModeManager::restoreInternalState(const uint8_t* in, int size) {
    in = GameModeManager::restoreInternalState(in, size);

    initPosition();

    TRANSFORM(herisson)->position.x = GameModeManager::position(time);

    return in;
}
