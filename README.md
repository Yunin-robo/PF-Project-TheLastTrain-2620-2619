# THE LAST TRAIN

CS-1002 Programming Fundamentals (AI & DS), Semester Project

**Author(s):** Fasih ur Rehman (26i-2620), Maryam Murtaza (26i-2619)
**Release tag:** `v1.0.2620.2619`

## About

THE LAST TRAIN is a deterministic, text-based rescue game written in C++ (no classes). The player moves through a 5-floor, 20-room building and must:

- rescue the injured passenger (needs the Medical Kit),
- secure the cat (needs the Cat Carrier),
- find clue digits 4, 2 and 7, then unlock the cabinet with code `427` to get the Train Key,
- install the Fuse at the Control Panel and collect the Lift Pass to restore Lift 2,
- prepare and board the train in F5-R2,

all before Energy or Time reaches 0. Maximum score is 520.

## Project Structure

```
include/    header files (config.h, state.h, level.h, simulator.h, physics.h,
            switches.h, logger.h, util.h, viewer_iface.h)
src/core/   source files (main_core.cpp, level.cpp, simulator.cpp, physics.cpp,
            switches.cpp, logger.cpp, util.cpp, viewer_iface_core.cpp)
data/       building.txt, initial-state.txt, test command files
out/        savegame.txt, game_summary.txt (sample completed run)
docs/       Project_Diagram.jpg, Flowchart.png, ConsoleUI.png
```

## Build

Requires g++ with C++17. From the project root:

```bash
mkdir -p out
g++ -std=c++17 -Wall -Wextra -Iinclude src/core/*.cpp -o last_train
```

The build produces no warnings.

## Run

Always run from the project root, because the program uses relative paths (`data/building.txt`, `out/...`).

```bash
./last_train
```

1. The building blueprint is shown, then the game asks for a starting floor and room (for example `1 1`).
2. Type commands at the `>` prompt.

| `move F R` | Move to floor F, room R |
| `inspect` | Search the room (reveals contents, discovers clue digits) |
| `collect X` | Pick up item with symbol X |
| `use X` | Use an inventory item (Medical Kit, Cat Carrier, Fuse) |
| `unlock CODE` | Unlock the cabinet (in F5-R1) |
| `board` | Prepare and board the train (in F5-R2) |
| `map`, `status`, `objectives` | Display views (no cost) |
| `connections` | To check the connections |
| `save` / `load` | Save or load `out/savegame.txt` |
| `building` | To load the whole building Map |
| `quit` | End the mission (writes a defeat summary) |

Invalid or rejected actions cost no Energy or Time.

### Costs

| Action | Energy | Time |
|---|---|---|
| Door, locked door, stairs | -2 | -1 |
| Window | -3 | -2 |
| Lift 1 / Lift 2 | -4 | -3 |
| Inspect, pickup, use | -1 | -1 |

Energy Pack +25 Energy, Emergency Toolkit +15 Energy, Observation Token +15 Time (all capped at the maximum).


### Commands

| Command | Action |
|---|---|

## Verifying Determinism

The simulation uses no randomness and no clock. Identical command input gives identical output.

```bash
./last_train < data/commands.txt > out/run1.txt
./last_train < data/commands.txt > out/run2.txt
diff out/run1.txt out/run2.txt && echo "DETERMINISTIC"
```

`diff` prints nothing and the message `DETERMINISTIC` appears when both runs are identical. The saved state can be compared the same way:

```bash
./last_train < data/commands.txt
cp out/savegame.txt out/save_a.txt
./last_train < data/commands.txt
diff out/save_a.txt out/savegame.txt && echo "SAVE IDENTICAL"
```

## Output Files

- `out/savegame.txt`: player position, resources, inventory, emergency trips, objective flags and the contents of every room.
- `out/game_summary.txt`: written on victory or defeat with the result, final score, remaining Energy and Time, and an objectives checklist.

## Design Notes and Assumptions

- Data is stored in primitive arrays: `horizontalConn[5][3]`, `roomEntities[5][4][5]`, `entityCounts[5][4]`, and `objectives[16]`. Stairs and lifts are handled by logic in `isDirectlyConnected()`.
- Lift 1 connects F1 and F2 between any of R2/R3 on one floor and R2/R3 on the other. *(Assumption: confirm with instructor.)*
- Lift 2 connects F2-R4, F4-R4 and F5-R4. It needs the repaired panel and the Lift Pass, or one of 3 emergency trips from the Emergency Access Card.
- Clue digits are discovered with `inspect` and are not collectable.
- The Train Key is tracked as an objective flag, obtained by unlocking the cabinet.
- Boarding requires the Train Key, the rescued passenger and the secured cat. *(Assumption: spec does not state this.)*
- `MAX_TIME` is 100, as in `config.h` and `initial-state.txt`.
- `building.txt` shows one Energy Pack in F1-R4, so `main` adds the second pack for new games.
- `loadGameStateFromFile` validates all values and applies them only if the whole file is valid.
- All file reads and writes check for failure, and bad user input is rejected without crashing.

## Memory Safety

All arrays are fixed-size with bounds checks before every index. File handles are closed after use.
