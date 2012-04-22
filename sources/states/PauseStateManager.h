#pragma once

#include "base/EntityManager.h"

#include "systems/TransformationSystem.h"
#include "systems/TextRenderingSystem.h"
#include "systems/ContainerSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/SoundSystem.h"

#include "GameStateManager.h"
#include "Game.h"
#include "DepthLayer.h"

class PauseStateManager : public GameStateManager {
	public:

	PauseStateManager();
	~PauseStateManager();
	void Setup();
	void Enter();
	GameState Update(float dt);
	void Exit();

	private:
		Entity eRestart, eAbort;
		Entity bRestart, bAbort;
};
