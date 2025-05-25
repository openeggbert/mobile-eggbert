//
// Created by robertvokac on 3/27/25.
//

#ifndef GAME1_H
#define GAME1_H

#include "Microsoft/Xna/Framework/Game.h"

namespace WindowsPhoneSpeedyBlupi {
class Game1 : public Microsoft::Xna::Framework::Game {
public:
    Game1();
    virtual ~Game1();
    void LoadContent() override;
    void Update(float deltaTime) override;
    void Draw() override;
    DEF_PROP_AUTO(bool, IsTrialMode, false)
    DEF_PROP_AUTO(bool, IsRankingMode, false)

private:



protected:

};

}

#endif //GAME1_H
