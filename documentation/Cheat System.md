## Cheat System

The `mobile-eggbert` project includes an internal cheat system composed of three main parts: input handling, UI triggers, and gameplay effects.

---

### Activation Points

* **Cheat menu unlock logic**
  Defined in `include/WindowsPhoneSpeedyBlupi/Game1.hpp` as the `cheatGeste` sequence (10 steps), and evaluated in
  `Game1::Update()` (`src/WindowsPhoneSpeedyBlupi/Game1.cpp`).

* **UI button handling (Cheat1–Cheat9)**
  Implemented in
  `Game1::CheatAction(Def::ButtonGlyph glyph)`.

* **Cheat execution logic**
  Handled by
  `Decor::CheatAction(Tables::CheatCodes cheat)` in `Decor.cpp`.

* **Button labels (single-letter hints)**
  Provided by
  `Decor::GetCheatTinyText(...)` → (`D, B, S, E, R, T, C, T, G`)

---

### Unlock Gesture

The cheat menu is unlocked by entering the following sequence:

```text
Cheat12, Cheat22, Cheat32, Cheat12, Cheat11,
Cheat21, Cheat22, Cheat21, Cheat31, Cheat32
```

Once the sequence is entered correctly, the cheat menu is enabled:

```cpp
showCheatMenu = true;
```

---

### UI Cheats (Cheat1–Cheat9)

The following actions are mapped in `Game1::CheatAction`:

| Button | Effect      | Description                    |
| ------ | ----------- | ------------------------------ |
| Cheat1 | OpenDoors   | Toggle door unlocking          |
| Cheat2 | SuperBlupi  | Toggle enhanced abilities      |
| Cheat3 | ShowSecret  | Toggle hidden elements display |
| Cheat4 | LayEgg      | Set lives to 9                 |
| Cheat5 | Reset       | Reset game data                |
| Cheat6 | TrialMode   | Toggle trial mode simulation   |
| Cheat7 | CleanAll    | Clear the map                  |
| Cheat8 | AllTreasure | Grant all treasures            |
| Cheat9 | EndGoal     | Instantly finish level         |

---

### Complete List of Cheat Codes

All available cheat codes are defined in:

`include/WindowsPhoneSpeedyBlupi/Tables.hpp` (`enum class CheatCodes`)

```
BuildOfficialMissions
OpenDoors
CleanAll
SuperBlupi
LayEgg
KillEgg
Skate
Copter
Jeep
AllTreasure
EndGoal
ShowSecret
RoundShield
Lollipop
Bombs
BirdLime
Tank
PowerCharge
Drink
Overcraft
Dynamite
WeelKeys
```

> **Note:** Only a subset of these cheats is exposed through the UI (Cheat1–Cheat9).
> However, all of them can be triggered programmatically via `Decor::CheatAction`.

### How to add a temporary hook during the start to enable cheats

```cpp
void Game1::StartMission(int mission)
{
    if (mission > 20 && mission % 10 > 1 && getIsTrialModeProperty())
    {
        SetPhase(Def::Phase::Trial);
        return;
    }
    this->mission = mission;
    if (this->mission != 1)
    {
        gameData.setLastWorldProperty(this->mission / 10);
    }
    decor.Read(0, this->mission, false);
    decor.LoadImages();
    decor.SetMission(this->mission);
    decor.SetNbVies(gameData.getNbViesProperty());
    decor.InitializeDoors(gameData);
    decor.AdaptDoors(false);
    decor.MainSwitchInitialize(gameData.getLastWorldProperty());
    decor.PlayPrepare(false);

    // TEMP CHEAT HOOK (remove later):
    for (Tables::CheatCodes c : {Tables::CheatCodes::OpenDoors, Tables::CheatCodes::SuperBlupi, Tables::CheatCodes::ShowSecret, Tables::CheatCodes::LayEgg, Tables::CheatCodes::AllTreasure, Tables::CheatCodes::RoundShield, Tables::CheatCodes::Bombs, Tables::CheatCodes::BirdLime, Tables::CheatCodes::Tank, Tables::CheatCodes::Dynamite, Tables::CheatCodes::WeelKeys})
        decor.CheatAction(c);

    decor.StartSound();
    inputPad.StartMission(this->mission);
}
```