#pragma once

#include "GameModeManager.h"
#include "DepthLayer.h"

class ScoreAttackGameModeManager : public GameModeManager {
	public:
		ScoreAttackGameModeManager();
		~ScoreAttackGameModeManager();
		void Setup();
		bool Update(float dt);

		void UpdateUI(float dt);
		void HideUI(bool toHide);

		void LevelUp();
		//permet de savoir si on a change de niveau
		bool LeveledUp();

		void ScoreCalc(int nb, int type);
		void Reset();
		std::string finalScore();

	private:
		class HUDManagerData;

		int level, obj[50], remain[8], bonus;
		bool isReadyToStart, levelUp;
		HUDManagerData* datas;
};
