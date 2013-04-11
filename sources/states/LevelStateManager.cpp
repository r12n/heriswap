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
#include "LevelStateManager.h"

#include <sstream>

#include <glm/glm.hpp>

#include <base/PlacementHelper.h>

#include "systems/ParticuleSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/TransformationSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/MusicSystem.h"
#include "systems/MorphingSystem.h"
#include "systems/ADSRSystem.h"

#include "modes/GameModeManager.h"
#include "TwitchSystem.h"
#include "DepthLayer.h"
#include "CombinationMark.h"
#include "GridSystem.h"

#include <algorithm>

void LevelStateManager::Setup() {
	eGrid = theEntityManager.CreateEntity();
	ADD_COMPONENT(eGrid, ADSR);

	ADSR(eGrid)->idleValue = 0;
	ADSR(eGrid)->attackValue = 1;
	ADSR(eGrid)->attackTiming = 3;
	ADSR(eGrid)->decayTiming = 0.;
	ADSR(eGrid)->sustainValue = 1;
	ADSR(eGrid)->releaseTiming = 0;
	ADSR(eGrid)->active = false;

	eBigLevel = theEntityManager.CreateEntity();
	ADD_COMPONENT(eBigLevel, Transformation);
	ADD_COMPONENT(eBigLevel, TextRendering);
	TRANSFORM(eBigLevel)->position = glm::vec2(0.f, (float)PlacementHelper::GimpYToScreen(846));
	TEXT_RENDERING(eBigLevel)->color = Color(1, 1, 1);
	TEXT_RENDERING(eBigLevel)->fontName = "gdtypo";
	TEXT_RENDERING(eBigLevel)->charHeight = PlacementHelper::GimpHeightToScreen(350);
	TRANSFORM(eBigLevel)->position = glm::vec2(0.f, (float)PlacementHelper::GimpYToScreen(846));
	TEXT_RENDERING(eBigLevel)->positioning = TextRenderingComponent::CENTER;
	ADD_COMPONENT(eBigLevel, Music);
    ADD_COMPONENT(eBigLevel, Morphing);
	MUSIC(eBigLevel)->control = MusicControl::Stop;

	eSnowEmitter = theEntityManager.CreateEntity();
	ADD_COMPONENT(eSnowEmitter, Transformation);
	TRANSFORM(eSnowEmitter)->size = glm::vec2((float)PlacementHelper::ScreenWidth, 0.5f);
	TransformationSystem::setPosition(TRANSFORM(eSnowEmitter), 
									  glm::vec2(0.f, (float)PlacementHelper::GimpYToScreen(0)), 
									  TransformationSystem::S);
	TRANSFORM(eSnowEmitter)->z = DL_Snow;
	ADD_COMPONENT(eSnowEmitter, Particule);
	PARTICULE(eSnowEmitter)->emissionRate = 0;
	PARTICULE(eSnowEmitter)->texture = theRenderingSystem.loadTextureFile("snow_flake0");
	PARTICULE(eSnowEmitter)->lifetime = Interval<float>(5.0f, 6.5f);
	PARTICULE(eSnowEmitter)->initialColor = Interval<Color> (Color(1.0, 1.0, 1.0, 1.0), Color(1.0, 1.0, 1.0, 1.0));
	PARTICULE(eSnowEmitter)->finalColor  = Interval<Color> (Color(1.0, 1.0, 1.0, 1.0), Color(1.0, 1.0, 1.0, 1.0));
	PARTICULE(eSnowEmitter)->initialSize = Interval<float>(PlacementHelper::GimpWidthToScreen(30), PlacementHelper::GimpWidthToScreen(40));
	PARTICULE(eSnowEmitter)->finalSize = Interval<float>(PlacementHelper::GimpWidthToScreen(30), PlacementHelper::GimpWidthToScreen(40));
	PARTICULE(eSnowEmitter)->forceDirection = Interval<float>(0, 0);
	PARTICULE(eSnowEmitter)->forceAmplitude  = Interval<float>(0, 0);
	PARTICULE(eSnowEmitter)->moment  = Interval<float>(-3, 3);
	PARTICULE(eSnowEmitter)->mass = 1;

	eSnowBranch = theEntityManager.CreateEntity();
	ADD_COMPONENT(eSnowBranch, Transformation);
	TRANSFORM(eSnowBranch)->size = glm::vec2((float)PlacementHelper::GimpWidthToScreen(800),
											 (float)PlacementHelper::GimpHeightToScreen(218));
	TransformationSystem::setPosition(TRANSFORM(eSnowBranch), 
									  glm::vec2((float)(-PlacementHelper::ScreenWidth*0.5), 
									  			(float)PlacementHelper::GimpYToScreen(0)), 
									  TransformationSystem::NW);
	TRANSFORM(eSnowBranch)->z = DL_SnowBackground;
	ADD_COMPONENT(eSnowBranch, Rendering);
	RENDERING(eSnowBranch)->texture = theRenderingSystem.loadTextureFile("snow_branch");

	eSnowGround = theEntityManager.CreateEntity();
	ADD_COMPONENT(eSnowGround, Transformation);
	TRANSFORM(eSnowGround)->size = glm::vec2((float)PlacementHelper::ScreenWidth, 
											 (float)PlacementHelper::GimpHeightToScreen(300));
	TransformationSystem::setPosition(TRANSFORM(eSnowGround), 
									  glm::vec2(0.f, (float)PlacementHelper::GimpYToScreen(1280)), 
									  TransformationSystem::S);
	TRANSFORM(eSnowGround)->z = DL_SnowBackground;
	ADD_COMPONENT(eSnowGround, Rendering);
	RENDERING(eSnowGround)->texture = theRenderingSystem.loadTextureFile("snow_ground");
}

void LevelStateManager::Enter() {
	LOGI("'" << __PRETTY_FUNCTION__ << "'");
	Color blue = Color(164.0/255.0, 164.0/255, 164.0/255);

	levelState = Start;
	std::stringstream a;
	a << currentLevel;
	TEXT_RENDERING(eBigLevel)->text = a.str();
	TEXT_RENDERING(eBigLevel)->show = true;
	TEXT_RENDERING(eBigLevel)->charHeight = PlacementHelper::GimpHeightToScreen(300);
	TRANSFORM(eBigLevel)->position = glm::vec2(0.f, (float)PlacementHelper::GimpYToScreen(846));
	TRANSFORM(eBigLevel)->z = DL_Score;

	TEXT_RENDERING(eBigLevel)->color = blue;

	MUSIC(eBigLevel)->control = MusicControl::Play;
	MORPHING(eBigLevel)->timing = 1;
	MORPHING(eBigLevel)->elements.push_back(new TypedMorphElement<float> (&TEXT_RENDERING(eBigLevel)->color.a, 0, 1));
	MORPHING(eBigLevel)->elements.push_back(new TypedMorphElement<float> (&TEXT_RENDERING(smallLevel)->color.a, 1, 0));
	MORPHING(eBigLevel)->elements.push_back(new TypedMorphElement<float> (&RENDERING(eSnowGround)->color.a, 0, 1));
	MORPHING(eBigLevel)->elements.push_back(new TypedMorphElement<float> (&RENDERING(eSnowBranch)->color.a, 0, 2));
	MORPHING(eBigLevel)->elements.push_back(new TypedMorphElement<float> (&RENDERING(modeMgr->herisson)->color.a, 1, 0));
	MORPHING(eBigLevel)->active = true;

	PARTICULE(eSnowEmitter)->emissionRate = 50;
	RENDERING(eSnowBranch)->show = true;
	RENDERING(eSnowGround)->show = true;

	duration = 0;

	// desaturate everyone except the branch, mute, pause and text elements
	TextureRef branch = theRenderingSystem.loadTextureFile("branche");
	TextureRef pause = theRenderingSystem.loadTextureFile("pause");
	TextureRef sound1 = theRenderingSystem.loadTextureFile("sound_on");
	TextureRef sound2 = theRenderingSystem.loadTextureFile("sound_off");
	std::vector<Entity> text = theTextRenderingSystem.RetrieveAllEntityWithComponent();
	std::vector<Entity> entities = theRenderingSystem.RetrieveAllEntityWithComponent();
	for (unsigned int i=0; i<entities.size(); i++) {
		TransformationComponent* tc = TRANSFORM(entities[i]);
		if (tc->parent <= 0 || std::find(text.begin(), text.end(), tc->parent) == text.end()) {
			RenderingComponent* rc = RENDERING(entities[i]);
			if (rc->texture == branch || rc->texture == pause || rc->texture == sound1 || rc->texture == sound2) {
				continue;
			}
			rc->effectRef = theRenderingSystem.effectLibrary.load("desaturate.fs");
		}
	}

	entities = theGridSystem.RetrieveAllEntityWithComponent();
	for (unsigned int i=0; i<entities.size(); i++) {
		CombinationMark::markCellInCombination(entities[i]);
	}
}

GameState LevelStateManager::Update(float dt) {
	//set grid alpha to 0
	if (levelState == Start && duration > 0.15) {
		ADSR(eGrid)->active = true;
		levelState = GridHided;
	}

	float alpha = 1 - ADSR(eGrid)->value;
	std::vector<Entity> entities = theGridSystem.RetrieveAllEntityWithComponent();
	for (std::vector<Entity>::iterator it = entities.begin(); it != entities.end(); ++it ) {
		RENDERING(*it)->color.a = alpha;
		TWITCH(*it)->speed = alpha * 9;
	}

	//start music at 0.5 s
	if (levelState == GridHided && duration > 0.5) {
		levelState = MusicStarted;
		if (MUSIC(eBigLevel)->music == InvalidMusicRef && !theMusicSystem.isMuted())
			MUSIC(eBigLevel)->music = theMusicSystem.loadMusicFile("audio/level_up.ogg");
	}
	//move big score + hedgehog
	//generate new leaves
	if (levelState == MusicStarted && duration > 6) {
		levelState = BigScoreBeganToMove;
		MorphingComponent* mc = MORPHING(eBigLevel);
		for (unsigned int i=0; i<mc->elements.size(); i++) {
			delete mc->elements[i];
		}
		mc->elements.clear();
		// move big score to small score
		//Color blue = Color(164.0/255.0, 164.0/255, 164.0/255);
		mc->elements.push_back(new TypedMorphElement<float> (&TEXT_RENDERING(eBigLevel)->charHeight, TEXT_RENDERING(eBigLevel)->charHeight, TEXT_RENDERING(smallLevel)->charHeight));
		// mc->elements.push_back(new TypedMorphElement<Color> (&TEXT_RENDERING(eBigLevel)->color, blue, Color(1,1,1,1)));
		mc->elements.push_back(new TypedMorphElement<glm::vec2> (&TRANSFORM(eBigLevel)->position, TRANSFORM(eBigLevel)->position, TRANSFORM(smallLevel)->position));
		mc->active = true;
		mc->activationTime = 0;
		mc->timing = 0.5;

		PARTICULE(eSnowEmitter)->emissionRate = 0;
		//on modifie le herisson
		TRANSFORM(modeMgr->herisson)->position.x = modeMgr->position(modeMgr->time);
		RENDERING(modeMgr->herisson)->color.a = 1;
		RENDERING(modeMgr->herisson)->effectRef = DefaultEffectRef;
		//on genere les nouvelles feuilles
		modeMgr->generateLeaves(0, theGridSystem.Types);
		for (unsigned int i=0; i<modeMgr->branchLeaves.size(); i++) {
			TRANSFORM(modeMgr->branchLeaves[i].e)->size =glm::vec2(0.f);
		}
	}
	if (levelState == BigScoreBeganToMove || levelState == BigScoreMoving) {
		levelState = BigScoreMoving;
		//if leaves created, make them grow !
		for (unsigned int i=0; i<modeMgr->branchLeaves.size(); i++) {
			TRANSFORM(modeMgr->branchLeaves[i].e)->size = 
                HeriswapGame::CellSize(8, modeMgr->branchLeaves[i].type) * HeriswapGame::CellContentScale() * glm::min((duration-6) / 4.f, 1.f);
		}
		RENDERING(eSnowBranch)->color.a = 1-(duration-6)/(10-6);
		RENDERING(eSnowGround)->color.a = 1-(duration-6)/(10-6.f);
	}

	duration += dt;

	//level animation ended - back to game
	if (levelState == BigScoreMoving && duration > 10) {
		if (currentLevel == 10 && theGridSystem.sizeToDifficulty() != DifficultyHard) {
			return ElitePopup;
		}
		return Spawn;
	}

	return LevelChanged;
}

void LevelStateManager::Exit() {
	theGridSystem.DeleteAll();
	ADSR(eGrid)->active = false;
	feuilles.clear();
	LOGI("'" << __PRETTY_FUNCTION__ << "'");
	PARTICULE(eSnowEmitter)->emissionRate = 0;
	RENDERING(eSnowBranch)->show = false;
	RENDERING(eSnowGround)->show = false;

	MorphingComponent* mc = MORPHING(eBigLevel);
	for (unsigned int i=0; i<mc->elements.size(); i++) {
		delete mc->elements[i];
	}
	for (unsigned int i=0; i<modeMgr->branchLeaves.size(); i++) {
		TRANSFORM(modeMgr->branchLeaves[i].e)->size = 
            HeriswapGame::CellSize(8, modeMgr->branchLeaves[i].type) * HeriswapGame::CellContentScale();
	}
	mc->elements.clear();
	// hide big level
	TEXT_RENDERING(eBigLevel)->show = false;
	// show small level
	TEXT_RENDERING(smallLevel)->text = TEXT_RENDERING(eBigLevel)->text;
	TEXT_RENDERING(smallLevel)->color.a = 1;
	RENDERING(modeMgr->herisson)->color.a = 1;

	std::vector<Entity> ent = theRenderingSystem.RetrieveAllEntityWithComponent();
	for (unsigned int i=0; i<ent.size(); i++) {
		RENDERING(ent[i])->effectRef = DefaultEffectRef;
	}
}
