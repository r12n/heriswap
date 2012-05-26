#include "UserInputGameStateManager.h"

#ifndef ANDROID
#include <GL/glew.h>
#include <GL/glfw.h>
#endif

#include "base/Log.h"
#include "base/TouchInputManager.h"
#include "base/EntityManager.h"

#include "systems/TransformationSystem.h"
#include "systems/RenderingSystem.h"
#include "systems/ADSRSystem.h"
#include "systems/ButtonSystem.h"
#include "systems/SoundSystem.h"

#include "GridSystem.h"
#include "Game.h"
#include "CombinationMark.h"

static void activateADSR(Entity e, float a, float s);
static void diffToGridCoords(const Vector2& c, int* i, int* j);


void UserInputGameStateManager::setAnimSpeed() {
	int difficulty = (theGridSystem.GridSize!=8)+1; //1 : normal, 2 : easy

	ADSR(eSwapper)->idleValue = 0;
	ADSR(eSwapper)->attackValue = 1.0;
	ADSR(eSwapper)->attackTiming = 0.07 * difficulty;
	ADSR(eSwapper)->decayTiming = 0;
	ADSR(eSwapper)->sustainValue = 1.0;
	ADSR(eSwapper)->releaseTiming = 0.07 * difficulty;
}

void UserInputGameStateManager::Setup() {
	eSwapper = theEntityManager.CreateEntity();
	ADD_COMPONENT(eSwapper, ADSR);

	ADD_COMPONENT(eSwapper, Sound);
	originI = originJ = -1;
    swapI = swapJ = 0;

	setAnimSpeed();
}

void UserInputGameStateManager::Enter() {
	LOGI("%s", __PRETTY_FUNCTION__);
	dragged = 0;
	ADSR(eSwapper)->active = false;
	ADSR(eSwapper)->activationTime = 0;
	originI = originJ = -1;
	dragged = 0;

	successMgr->timeUserInputloop = 0.f;
}

GameState UserInputGameStateManager::Update(float dt) {
	successMgr->timeUserInputloop += dt;
	successMgr->sWhatToDo(theTouchInputManager.wasTouched() && theTouchInputManager.isTouched(), dt);

	// drag/drop of cell
	if (!theTouchInputManager.wasTouched() &&
		theTouchInputManager.isTouched()) {
		// don't start new drag while the previous one isn't finished
		if (!dragged) {
			// start drag: find nearest cell
			const Vector2& pos = theTouchInputManager.getTouchLastPosition();
			int i, j;
			for( i=0; i<theGridSystem.GridSize && !dragged; i++) {
				for(j=0; j<theGridSystem.GridSize; j++) {
					Entity e = theGridSystem.GetOnPos(i,j);
						if(e && ButtonSystem::inside(
						pos,
						TRANSFORM(e)->worldPosition,
						TRANSFORM(e)->size)) {
						dragged = e;
						break;
					}
				}
			}
			if (dragged) {
				i--;
				originI = i;
				originJ = j;

				activateADSR(dragged, 1.3, 1.3);

				// active neighboors
				activateADSR(theGridSystem.GetOnPos(i+1,j), 1.1, 1.1);
				activateADSR(theGridSystem.GetOnPos(i,j+1), 1.1, 1.1);
				activateADSR(theGridSystem.GetOnPos(i-1,j), 1.1, 1.1);
				activateADSR(theGridSystem.GetOnPos(i,j-1), 1.1, 1.1);
			}
		}
	} else if (theTouchInputManager.wasTouched() && dragged && ADSR(dragged)->active) {
		if (theTouchInputManager.isTouched()) {
			// continue drag
			Vector2 diff = theTouchInputManager.getTouchLastPosition()
				- Game::GridCoordsToPosition(originI, originJ, theGridSystem.GridSize);

			if (diff.Length() > 1) {
				int i=0,j=0;
				diffToGridCoords(diff, &i, &j);

				if (theGridSystem.IsValidGridPosition(originI + i, originJ + j)) {
					if ((swapI == i && swapJ == j) ||
						(swapI == 0 && swapJ == 0)) {
						ADSR(eSwapper)->active = true;
						swapI = i;
						swapJ = j;

						Entity a = dragged, e = theGridSystem.GetOnPos(originI+swapI, originJ+swapJ);
						if (e && a) {
							GRID(a)->i = originI + swapI;
							GRID(a)->j = originJ + swapJ;
							GRID(e)->i = originI;
							GRID(e)->j = originJ;
						}
						std::vector<Combinais> combinaisons = theGridSystem.LookForCombination(false,false);
						if (e && a) {
							GRID(a)->i = originI;
							GRID(a)->j = originJ;
							GRID(e)->i = originI+swapI;
							GRID(e)->j =  originJ+swapJ;
						}
						if (!combinaisons.empty())
						{
							for ( std::vector<Combinais>::reverse_iterator it = combinaisons.rbegin(); it != combinaisons.rend(); ++it ) {
								for ( std::vector<Vector2>::reverse_iterator itV = (it->points).rbegin(); itV != (it->points).rend(); ++itV )
								{
                                    Entity cell;
                                    if (itV->X == originI && itV->Y == originJ) {
                                        cell = e;
                                    } else if (itV->X == (originI + swapI) && itV->Y == (originJ + swapJ)) {
                                        cell = a;
                                    } else {
                                        cell = theGridSystem.GetOnPos(itV->X, itV->Y);
                                    }
                                    if (cell) {
                                        inCombinationCells.push_back(cell);
                                        CombinationMark::markCellInCombination(cell);
                                    }
								}
							}
                        }
					} else {
                        for (int k=0; k<inCombinationCells.size(); k++) {
                            CombinationMark::clearCellInCombination(inCombinationCells[k]);
                        }
                        inCombinationCells.clear();
						if (ADSR(eSwapper)->activationTime > 0) {
							ADSR(eSwapper)->active = false;
						} else {
							ADSR(eSwapper)->active = true;
							swapI = i;
							swapJ = j;
						}
					}
				} else {
					ADSR(eSwapper)->active = false;
                    for (int k=0; k<inCombinationCells.size(); k++) {
                        CombinationMark::clearCellInCombination(inCombinationCells[k]);
                    }
                    inCombinationCells.clear();
				}
			} else {
				ADSR(eSwapper)->active = false;
                for (int k=0; k<inCombinationCells.size(); k++) {
                    CombinationMark::clearCellInCombination(inCombinationCells[k]);
                }
                inCombinationCells.clear();
			}
		} else {
			LOGI("release");

			// release drag
			ADSR(theGridSystem.GetOnPos(originI,originJ))->active = false;
			if (theGridSystem.IsValidGridPosition(originI+1, originJ))
				ADSR(theGridSystem.GetOnPos(originI+1,originJ))->active = false;
			if (theGridSystem.IsValidGridPosition(originI, originJ+1))
				ADSR(theGridSystem.GetOnPos(originI,originJ+1))->active = false;
			if (theGridSystem.IsValidGridPosition(originI-1,originJ))
				ADSR(theGridSystem.GetOnPos(originI-1,originJ))->active = false;
			if (theGridSystem.IsValidGridPosition(originI,originJ-1))
				ADSR(theGridSystem.GetOnPos(originI,originJ-1))->active = false;
			ADSR(eSwapper)->active = false;

			/* must swap ? */
			if (ADSR(eSwapper)->value >= 0.99) {
				Entity e2 = theGridSystem.GetOnPos(originI+ swapI,originJ+ swapJ);
				GRID(e2)->i = originI;
				GRID(e2)->j = originJ;
				GRID(e2)->checkedH = false;
				GRID(e2)->checkedV = false;

				Entity e1 = dragged ;
				GRID(e1)->i = originI + swapI;
				GRID(e1)->j = originJ + swapJ;
				GRID(e1)->checkedH = false;
				GRID(e1)->checkedV = false;

				std::vector<Combinais> combinaisons = theGridSystem.LookForCombination(false,true);
				if (
			#ifndef ANDROID
				glfwGetMouseButton(GLFW_MOUSE_BUTTON_2) != GLFW_PRESS &&
			#endif
				combinaisons.empty()) {
					// revert swap
					GRID(e1)->i = originI;
					GRID(e1)->j = originJ;
					GRID(e2)->i = originI + swapI;
					GRID(e2)->j = originJ + swapJ;

					SOUND(eSwapper)->sound = theSoundSystem.loadSoundFile("audio/son_descend.ogg");
					return UserInput;
				} else {
					// validate position
					TRANSFORM(e1)->position = Game::GridCoordsToPosition(GRID(e1)->i, GRID(e1)->j,theGridSystem.GridSize);
					TRANSFORM(e2)->position = Game::GridCoordsToPosition(GRID(e2)->i, GRID(e2)->j,theGridSystem.GridSize);

					originI = originJ = -1;
					return Delete;
				}
			}
			for (int k=0; k<inCombinationCells.size(); k++) {
            	CombinationMark::clearCellInCombination(inCombinationCells[k]);
            }
            inCombinationCells.clear();
		}
	} else {
		ADSR(eSwapper)->active = false;
		if (dragged) ADSR(dragged)->active = false;
	}

	if (dragged) {
		// reset 'dragged' cell only if both anim are ended
		if (!ADSR(dragged)->active && !ADSR(eSwapper)->active) {
			if (ADSR(eSwapper)->activationTime <= 0 && ADSR(dragged)->activationTime <= 0) {
				dragged = 0;
			}
		}
	}
	return UserInput;
}

void UserInputGameStateManager::BackgroundUpdate(float dt) {
	for(int i=0; i<theGridSystem.GridSize; i++) {
		for(int j=0; j<theGridSystem.GridSize; j++) {
			Entity e = theGridSystem.GetOnPos(i,j);
			if (e) {
				TRANSFORM(e)->size = ADSR(e)->value;
			}
		}
	}

	if (ADSR(eSwapper)->activationTime >= 0 && originI >= 0 && originJ >= 0) {

		Vector2 pos1 = Game::GridCoordsToPosition(originI, originJ, theGridSystem.GridSize);
		Vector2 pos2 = Game::GridCoordsToPosition(originI + swapI, originJ + swapJ, theGridSystem.GridSize);

		Vector2 interp1 = MathUtil::Lerp(pos1, pos2, ADSR(eSwapper)->value);
		Vector2 interp2 = MathUtil::Lerp(pos2, pos1, ADSR(eSwapper)->value);

		Entity e1 = theGridSystem.GetOnPos(originI,originJ);
		Entity e2 = theGridSystem.GetOnPos(originI + swapI,originJ + swapJ);

		if (e1)
			TRANSFORM(e1)->position = interp1;
		if (e2)
			TRANSFORM(e2)->position = interp2;
	}
}

void UserInputGameStateManager::Exit() {
	LOGI("%s", __PRETTY_FUNCTION__);
    inCombinationCells.clear();

    successMgr->sLuckyLuke();
    successMgr->sWhatToDo(false, 0.f);
}

static void activateADSR(Entity e, float a, float s) {
	if (!e)
		return;
	float size = TRANSFORM(e)->size.X;
	ADSRComponent* ac = ADSR(e);
	ac->idleValue = size;
	ac->attackValue = size * a;
	ac->attackTiming = 0.1;
	ac->decayTiming = 0.0;
	ac->sustainValue = size * s;
	ac->releaseTiming = 0.08;
	ac->active = true;
}

void diffToGridCoords(const Vector2& c, int* i, int* j) {
	*i = *j = 0;
	if (MathUtil::Abs(c.X) > MathUtil::Abs(c.Y)) {
		*i = (c.X < 0) ? -1 : 1;
	} else {
		*j = (c.Y < 0) ? -1 : 1;
	}
}
