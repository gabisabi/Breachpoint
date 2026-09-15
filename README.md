# 🎮 Breachpoint

**Multiplayer VR FPS for Meta Quest** — Built with Unreal Engine 5.8

## About

Breachpoint is a Fortnite-inspired multiplayer first-person shooter designed for Meta Quest VR headsets (also playable on PC). Fast-paced arena combat with multiple game modes, weapon progression, and VR motion controller support.

## Game Modes

- **Free For All** — Every player for themselves. 20 kills to win.
- **Team Deathmatch** — Two teams, first to the score limit wins.
- **Gun Game** — Kill enemies to upgrade through a weapon ladder.
- **Horde** — Survive endless waves of increasingly tough enemies.

## Features

- 🕶️ **VR Support** — Full motion controller input, two-handed weapon stabilization, teleport & smooth locomotion
- 🏠 **Lobby System** — Fortnite-style lobby with map & game mode selection
- 🏃 **Advanced Movement** — Sprint, slide, tackle, crouch
- 🔫 **Weapon System** — Multiple weapons, recoil, reload, projectile physics
- 🤖 **AI Enemies** — StateTree-driven NPCs with perception and combat behavior
- 🎯 **Dynamic HUD** — Crosshair spread, kill feed, health bar, ammo counter, weapon inventory
- 👥 **Multiplayer** — Team-based scoring with bot fill

## Tech Stack

- **Engine**: Unreal Engine 5.8
- **Language**: C++ with Blueprint subclasses
- **VR SDK**: OpenXR + MetaXR
- **AI**: StateTree + AI Perception
- **Input**: Enhanced Input System

## Project Structure

```
Source/Breachpoint/
├── Core files (base character, controller, camera, game mode)
├── Lobby/          — Lobby game mode, UI, map/mode selection
├── VR/             — VR character, motion controllers, VR player controller
├── Variant_Horror/ — Horror game mode (sprint/stamina, flashlight)
└── Variant_Shooter/
    ├── AI/         — NPC controller, StateTree tasks, spawners
    ├── Modes/      — FFA, TDM, Gun Game, Horde + HUD
    ├── Movement/   — Custom movement component (sprint/slide/tackle)
    ├── UI/         — Player HUD, crosshair, kill feed, bullet counter
    └── Weapons/    — Weapon base, projectiles, pickups, reload
```

## Getting Started

1. Clone the repo:
   ```
   git clone https://github.com/gabisabi/Breachpoint.git
   ```
2. Open `Breachpoint.uproject` in Unreal Engine 5.8
3. Compile (Build → Build Solution or Ctrl+B)
4. Hit Play!

## Contributing

This is a collaborative project. After making changes:
```bash
git add -A
git commit -m "describe your changes"
git push
```

To get the latest changes:
```bash
git pull
```

## Team

- **Gabi** — Creative lead, game design
- **Kagon** — Level design, assets, co-developer

---

*Built with ❤️ and way too much C++*
