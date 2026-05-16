### Analýza projektu mobile-eggbert: Kandidáti na nové enumy místo magických konstant

Prošel jsem celý zdrojový kód projektu a identifikoval následující místa, kde by bylo vhodné vytvořit nové (nebo využít existující) enumy namísto magických čísel či volných `intcs` konstant.

---

### 1. `BlupiAction` — nejdůležitější kandidát

**Problém:** Pole `m_blupiAction` (typ `intcs`) v `Decor.hpp` a `Decor.cpp` se přiřazují přímo magická čísla (cca 256× v Decor.cpp). V `Def.hpp` sice existují pojmenované konstanty `ACTION_*`, ale **nikde se nepoužívají** — kód místo nich píše čísla přímo.

**Příklady magických přiřazení v `Decor.cpp`:**
```cpp
m_blupiAction = 1;   // ACTION_STOP
m_blupiAction = 2;   // ACTION_MARCH
m_blupiAction = 4;   // ACTION_JUMP
m_blupiAction = 5;   // ACTION_AIR
m_blupiAction = 13;  // ACTION_WIN
m_blupiAction = 29;  // ACTION_POP
m_blupiAction = 36;  // ACTION_JUMPAIE
m_blupiAction = 59;  // ACTION_TURNAIR
m_blupiAction = 61;  // ACTION_STOPJUMP
```

**Řešení:** Vytvořit `enum class BlupiAction` (analogicky jako `ObjectType`, `SoundChannel`) s hodnotami pojmenovanými dle existujících konstant v `Def.hpp`:

```cpp
// include/WindowsPhoneSpeedyBlupi/enums/BlupiAction.hpp
enum class BlupiAction : SharpRuntime::ushortcs
{
    Stop          = 1,   // ACTION_STOP
    March         = 2,   // ACTION_MARCH
    Turn          = 3,   // ACTION_TURN
    Jump          = 4,   // ACTION_JUMP
    Air           = 5,   // ACTION_AIR
    Down          = 6,   // ACTION_DOWN
    Up            = 7,   // ACTION_UP
    Vertigo       = 8,   // ACTION_VERTIGO
    Recede        = 9,   // ACTION_RECEDE
    Advance       = 10,  // ACTION_ADVANCE
    Win           = 13,  // ACTION_WIN
    // ... atd. až po
    PutDynamite   = 87   // ACTION_PUTDYNAMITE
};
```

Konstanty `ACTION_*` v `Def.hpp` by pak mohly být odstraněny nebo označeny jako deprecated.

---

### 2. `Direction` — směr pohybu Blupiho

**Problém:** Pole `m_blupiDir` (typ `intcs`) nabývá hodnot `1` a `2`, které odpovídají konstantám `DIR_LEFT` a `DIR_RIGHT` z `Def.hpp`. Kód na 63 místech píše přímo čísla:

```cpp
m_blupiDir = 1;  // DIR_LEFT
m_blupiDir = 2;  // DIR_RIGHT
if (m_blupiDir == 1) ...
if (m_blupiDir == 2) ...
```

**Řešení:**
```cpp
// include/WindowsPhoneSpeedyBlupi/enums/Direction.hpp
enum class Direction : SharpRuntime::ushortcs
{
    Left  = 1,   // DIR_LEFT
    Right = 2    // DIR_RIGHT
};
```

---

### 3. `SecretPower` — speciální moc Blupiho

**Problém:** Pole `m_blupiSec` (typ `intcs`) nabývá hodnot 0–4, které mají pojmenované konstanty v `Def.hpp`:

```cpp
m_blupiSec = 0;  // žádná moc
m_blupiSec = 1;  // SEC_SHIELD
m_blupiSec = 2;  // SEC_POWER
m_blupiSec = 3;  // SEC_CLOUD
m_blupiSec = 4;  // SEC_HIDE
```

**Řešení:**
```cpp
// include/WindowsPhoneSpeedyBlupi/enums/SecretPower.hpp
enum class SecretPower : SharpRuntime::ushortcs
{
    None   = 0,
    Shield = 1,   // SEC_SHIELD
    Power  = 2,   // SEC_POWER
    Cloud  = 3,   // SEC_CLOUD
    Hide   = 4    // SEC_HIDE
};
```

---

### 4. `KeyPressFlags` — bitové příznaky kláves

**Problém:** `m_keyPress` (typ `intcs`) je bitové pole s příznaky. V `Def.hpp` jsou konstanty `KEY_JUMP=1`, `KEY_FIRE=2`, `KEY_DOWN=4`. V kódu se používají přímá čísla:

```cpp
if ((m_keyPress & 1) == 0)           // KEY_JUMP bit
if (((unsigned int)m_keyPress & 2u) != 0)  // KEY_FIRE bit
if ((m_keyPress & -3) == 0)          // kombinace flagů
```

**Řešení:** Enum s bitovými hodnotami (nebo `enum class` s `|` operátorem):
```cpp
// include/WindowsPhoneSpeedyBlupi/enums/KeyPressFlags.hpp
enum class KeyPressFlags : SharpRuntime::intcs
{
    None  = 0,
    Jump  = 1,   // KEY_JUMP
    Fire  = 2,   // KEY_FIRE
    Down  = 4    // KEY_DOWN
};
// + operátory & a |
```

---

### 5. `ContinueMission` — stav pokračování mise (Game1.cpp)

**Problém:** Proměnná `continueMission` v `Game1.cpp` nabývá hodnot 0, 1, 2 bez jakéhokoliv pojmenování:

```cpp
continueMission = 1;   // spustit pokračování
continueMission = 2;   // pokračování aktivní
continueMission = 0;   // žádné pokračování
if (continueMission != 0) ...
if (continueMission == 2) ...
```

**Řešení:**
```cpp
enum class ContinueMission : int
{
    None    = 0,
    Pending = 1,
    Active  = 2
};
```

---

### 6. Redundantní `CH*` konstanty v `Def.hpp`

**Problém:** V `Def.hpp` existují konstanty `CHOBJECT=1`, `CHBLUPI=2`, `CHDECOR=3`, ..., `CHGEAR=17` — tyto jsou **přesně duplikáty** hodnot v již existujícím `PixmapChannel` enumu. Ve zdrojovém kódu se používá `PixmapChannel`, ale konstanty v `Def.hpp` zůstávají jako mrtvý kód.

**Doporučení:** Odstranit `CH*` konstanty z `Def.hpp`, protože jsou plně nahrazeny `PixmapChannel` enumem.

---

### Přehled — co udělat kde

| Nový enum | Soubor | Postižená pole | Počet výskytů |
|-----------|--------|----------------|---------------|
| `BlupiAction` | `enums/BlupiAction.hpp` | `m_blupiAction` | ~256 v Decor.cpp |
| `Direction` | `enums/Direction.hpp` | `m_blupiDir` | ~63 v Decor.cpp |
| `SecretPower` | `enums/SecretPower.hpp` | `m_blupiSec` | ~6 v Decor.cpp |
| `KeyPressFlags` | `enums/KeyPressFlags.hpp` | `m_keyPress` | ~20 v Decor.cpp |
| `ContinueMission` | `enums/ContinueMission.hpp` | `continueMission` | ~5 v Game1.cpp |

### Vzor implementace (analogicky k existujícím enumům)

Každý nový enum by měl mít pomocné funkce stejně jako `ObjectType`, `SoundChannel` a `PixmapChannel`:

```cpp
static constexpr auto ToRaw(BlupiAction action) -> SharpRuntime::ushortcs
{
    return static_cast<SharpRuntime::ushortcs>(action);
}

static constexpr auto ToBlupiAction(const int value) -> BlupiAction
{
    return static_cast<BlupiAction>(
        static_cast<SharpRuntime::ushortcs>(value)
    );
}
```

### Prioritizace

1. **`BlupiAction`** — nejvyšší dopad (256 výskytů), všechny hodnoty jsou pojmenované v `Def.hpp`
2. **`Direction`** — jednoduchý (jen 2 hodnoty, 63 výskytů)
3. **`SecretPower`** — jednoduchý (4 hodnoty, pojmenované v `Def.hpp`)
4. **`KeyPressFlags`** — složitější (bitové operace vyžadují přetížení `&` a `|` operátorů)
5. **`ContinueMission`** — lokální (jen Game1.cpp)
6. **Odstranění `CH*` z `Def.hpp`** — čistý refaktoring bez nového enumu
