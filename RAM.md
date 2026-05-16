### Analýza zbytečného vytváření objektů v projektu mobile-eggbert

Po prohledání celého zdrojového kódu (zejména `Decor.cpp`, `Game1.cpp`, `Pixmap.cpp`) jsem identifikoval tato problematická místa:

---

### 1. `ByeByeDraw` — kopírování objektů v cyklu (každý snímek!)

**Soubor:** `src/WindowsPhoneSpeedyBlupi/Decor.cpp`, řádek 9252

```cpp
// PROBLÉM: kopíruje každý ByeByeObject místo použití reference
for (ByeByeObject byeByeObject : byeByeObjects)
{
    TinyPoint tinyPoint;
    tinyPoint.X = m_drawBounds.Left + (int)byeByeObject.posX - posDecor.X;
    ...
}
```

`ByeByeObject` je třída se 9 členy (většinou `double` = 8 bytů každý) — celkem ~72 bytů. Tato smyčka se volá **každý snímek** (`Decor::Draw`) a zbytečně kopíruje každý ByeByeObject.

**Doporučení:**
```cpp
// Správně — const reference, žádná kopie
for (const ByeByeObject& byeByeObject : byeByeObjects)
```

---

### 2. `ByeByeAdd` — zbytečná dvojí kopie objektu

**Soubor:** `src/WindowsPhoneSpeedyBlupi/Decor.cpp`, řádky 9187–9206

```cpp
void Decor::ByeByeAdd(...)
{
    ByeByeObject byeByeObject;      // konstrukt 1
    byeByeObject.channel = channel;
    ...
    ByeByeObject byeByeObject2 = byeByeObject;  // kopie konstrukt 2 (zbytečná!)
    byeByeObject2.speedX = ...;
    byeByeObjects.push_back(byeByeObject2);      // kopie 3 (move by šlo použít)
}
```

Objekt se vytváří dvakrát a pak kopíruje do vektoru. Jde to zjednodušit na přímou inicializaci a `emplace_back`.

---

### 3. `ByeByeStep` — `erase` uprostřed vektoru (O(n) na smazání)

**Soubor:** `src/WindowsPhoneSpeedyBlupi/Decor.cpp`, řádek 9241

```cpp
while (num < byeByeObjects.size())
{
    ...
    if (byeByeObject.phase > 30.0)
    {
        // Toto posune všechny zbývající prvky o 1 doleva — O(n) per erase!
        byeByeObjects.erase(byeByeObjects.begin() + num);
    }
    ...
}
```

Při 100 ByeByeObjectech může každé `erase` přesunout až 99 prvků. Lepší alternativou je **erase-remove idiom** nebo označení objektů jako „mrtvé" a jejich hromadné smazání na konci.

---

### 4. `MoveObjectSort` — bubble sort s O(n²) kopírováním struktur

**Soubor:** `src/WindowsPhoneSpeedyBlupi/Decor.cpp`, řádky 9074–9108

```cpp
void Decor::MoveObjectSort()
{
    MoveObject dst;
    ...
    do {
        flag = false;
        for (int i = 0; i < num - 1; i++)
        {
            if (SortGetType(...) > SortGetType(...))
            {
                MoveObjectCopy(dst, m_moveObject[i]);       // kopie ~67 bytů
                MoveObjectCopy(m_moveObject[i], m_moveObject[i + 1]);
                MoveObjectCopy(m_moveObject[i + 1], dst);
                flag = true;
            }
        }
    } while (flag);
}
```

`MoveObjectCopy` manuálně kopíruje všechna pole struktury `MoveObject` (~67 bytů). Bubble sort s `MAXMOVEOBJECT = 200` objekty = v nejhorším případě až **~40 000 kopírování** (každá ~67 bytů).

`MoveObjectCopy` je navíc implementováno manuálně, přestože by šlo použít přímé přiřazení (`dst = src;`) nebo `std::swap`.

**Doporučení:**
```cpp
// Místo MoveObjectCopy — použít default přiřazení:
dst = m_moveObject[i];          // automatická kopie všech členů
m_moveObject[i] = m_moveObject[i + 1];
m_moveObject[i + 1] = dst;

// Nebo ještě lépe — použít std::sort s lambda:
std::sort(m_moveObject, m_moveObject + num,
    [](const MoveObject& a, const MoveObject& b) {
        return SortGetType(a.type) < SortGetType(b.type);
    });
```

---

### 5. Velká pole jako členné proměnné třídy `Decor`

**Soubor:** `include/WindowsPhoneSpeedyBlupi/Decor.hpp`

```cpp
Cellule m_decor[100][100]{};       // 10 000 × 4 byty = 40 KB
Cellule m_bigDecor[100][100]{};    // 10 000 × 4 byty = 40 KB
intcs m_balleTraj[1300]{};         // 1300 × 4 = 5.2 KB
intcs m_moveTraj[1300]{};          // 1300 × 4 = 5.2 KB
MoveObject m_moveObject[200];      // 200 × ~67 = ~13.4 KB
```

Tato pole jsou na zásobníku objektu `Decor` — celkem asi **~104 KB** jen pro tato pole. Jsou to statické alokace, ne dynamické, takže nevznikají opakovaně. Nicméně při inicializaci `InitDecor()` se vynulují všechny smyčkou (`for 100×100`).

---

### Shrnutí — přehled problémů

| Místo | Soubor | Typ problému | Závažnost |
|-------|--------|-------------|-----------|
| `ByeByeDraw` (ř. 9252) | `Decor.cpp` | Kopie objektu v každém snímku v cyklu | ⚠️ Střední |
| `ByeByeAdd` (ř. 9187–9206) | `Decor.cpp` | Zbytečná mezikopie objektu | 🔵 Nízká |
| `ByeByeStep` (ř. 9241) | `Decor.cpp` | `erase` uprostřed vektoru = O(n) posun | ⚠️ Střední |
| `MoveObjectSort` (ř. 9074–9108) | `Decor.cpp` | Bubble sort + manuální kopírování O(n²) | 🔴 Vysoká |
| `MoveObjectCopy` (ř. 23) | `Decor.cpp` | Manuální kopie místo default assign / `std::swap` | 🔵 Nízká |

Žádná nežádoucí heap alokace přes `new` v herním kódu nebyla nalezena — veškerá dynamická alokace (`make_shared<Pixmap>`, `make_shared<Sound>`, `make_unique<Random>`, `make_unique<SpriteBatch>`) jsou jednorázové inicializace na začátku, nikoliv opakované alokace.
