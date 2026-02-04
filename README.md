"# MyZombies" 

Project Overview
    This project is a multiplayer Unreal Engine game built to explore UI system architecture, gameplay flow, and debugging workflows in a networked environment. The primary focus of this portfolio slice is a complete Zombies game mode, supported by modular UI systems and custom developer tools.
Key Systems Demonstrated
•	Multiplayer client/server gameplay (authority, replication boundaries)
•	Centralized UI management via MenuUIManager owned by PlayerController
•	Input profile switching (Gameplay, Lobby, Pause) without widget logic leaks
•	Event-driven UI updates using delegates (no per-frame UI ticking)
•	Custom Gameplay Debugger category for runtime inspection
Design Decisions
•	GameMode owns rules (server-only)
•	GameState owns replicated match state (rounds, phases)
•	PlayerController owns all UI decisions and input modes
•	Widgets are presentation-only, never owning game logic
Scope Note
    A Deathmatch mode is planned but intentionally excluded from this slice. I chose to polish a complete Zombies gameplay loop and supporting tools rather than spread effort across unfinished modes.

