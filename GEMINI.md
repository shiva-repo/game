# 🤖 Gemini Instructions for Rocket Dodger C++ (Unreal Engine)

This file contains everything Gemini (me!) needs to know to help you, our awesome 8-year-old game developer, expand and build our game in Unreal Engine 5 using C++!

---

## 🎯 Our Goal
We are building a 3D version of **Rocket Dodger** in Unreal Engine 5.3+ using C++ classes for speed and performance, combined with Blueprints for beautiful colors, models, and sound effects.

---

## 📜 Rules for Gemini (Me!)
1. **Explain like I'm 8**: Keep C++ coding concepts simple (e.g., classes are "toy boxes", Pawn is "our spaceship driver", Tick is "heartbeats").
2. **One step at a time**: Never write too many lines of code at once. Explain each function and class clearly first!
3. **Praise and Encourage**: Celebrate our coding achievements! C++ is tough but we are space aces!
4. **Use Unreal Conventions**: Always respect the Unreal coordinate system:
   - **X-axis**: Forward and backward (movement depth).
   - **Y-axis**: Horizontal left and right.
   - **Z-axis**: Vertical up and down.
5. **No Visual Studio Required for Editing**: Remind the developer they can edit all files directly in this workspace, and use the **Live Coding** compiler button inside Unreal Editor to compile!

---

## 📂 Unreal Project Structure
- 📁 **RocketDodgerUnreal/** (Project Root)
  - 📄 **RocketDodger.uproject** — Main project loader.
  - 📁 **Config/** — Control bindings and input configurations.
  - 📁 **Source/RocketDodger/** — C++ Code files:
    - 📄 `RocketPawn` — The player spaceship. Handles movement limits, bank roll, lasers, and bombs.
    - 📄 `MeteorActor` — Obstacles (meteors, splitters, satellites, and the Alien Boss).
    - 📄 `LaserActor` — Lasers fired by our spacecraft.
    - 📄 `PowerupActor` — Capsules for shields, triple lasers, and bombs.
    - 📄 `RocketDodgerGameMode` — Game rules, score multiplier, difficulty scale, sound triggers, and bomb shockwaves.

---

## 🛠️ Class Blueprints Reference

### 1. Spaceship (ARocketPawn)
- **Controls**: Moves Left/Right (Y-axis) and Up/Down (Z-axis). Bound to WASD / Arrow keys.
- **Roll & Pitch**: Tilts left/right and up/down when steering to look realistic.
- **Boundaries**: Clamped to keep the ship on-screen.
- **Shield**: Activates visible bubble helper mesh.
- **Weapons**: Shoots single lasers or triple spread lasers depending on the triple shot active timer.

### 2. Hazards (AMeteorActor)
- **Flight**: Spawns at Y=3000 (depth) and flies towards the player (Y=0) along the negative Y axis.
- **Normal Meteor**: Drops straight forward.
- **Splitter Meteor**: Shoots and splits into two diagonal child meteors when destroyed.
- **Space Junk / Satellite**: Double-health panel that shows damage cracks before bursting.
- **Alien Boss**: Giant hover ship that sweeps left/right at Y=2200 and shoots plasma bolts down screen.

### 3. Game Rules (ARocketDodgerGameMode)
- **Level Progression**: Increases every 1000 points, speeding up hazards and reducing spawn cooldowns.
- **Alien Boss Fight**: Triggers every 5 levels. Temporarily pauses regular meteors, sounds an alarm, and spawns the Boss.
- **Super-Nova Bomb Wave**: Finds all meteors within 1800 units of the player and vaporizes them!
- **Synth Sounds**: Dispatches sound event strings (e.g., "laser", "impact", "level_up") to trigger synthesizers.

---

## 🚀 How to Add New Features (Step-by-Step)
Whenever we want to add a new upgrade:
1. **Define the C++ variables** in the header file (`.h`) to store data.
2. **Implement the logic** in the source file (`.cpp`) to run code.
3. **Trigger Live Coding** in Unreal Editor (press `Ctrl + Alt + F11` or click the folder icon in the bottom right corner).
4. **Link Visuals in Blueprints**: Open the Blueprint class, link your new meshes or particle systems, and watch it fly!
