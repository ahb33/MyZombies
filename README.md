"# MyZombies" 


Overview
    Multiplayer Unreal Engine project focused on UI system architecture,networked gameplay flow, and debug friendly workflows. This portfolio slice highlights a complete Zombies mode supported by a modular UMG menu stack, HUD updates, and multiplayer session UX.

Tech
    Unreal Engine (C++ + Blueprints/UMG)
    Client/server networking (authority + replication boundaries)
    Online sessions (create/find/join flow)
    Gameplay Debugger (custom category)

UI Systems Demonstrated
    Centralized UI management : `UMenuUIManager` owned by `AMyPlayerController` (single source of truth for menu stack + transitions)
    Menu stack navigation : show / push / pop across Main → Mode Select → Multiplayer → Create/Join
    Async UX states : session search disables inputs + blocks navigation, shows “searching” state, then renders results
    Input mode switching : UI only vs gameplay modes controlled by PlayerController (no widget logic leaks)
    Keyboard/controller focus : menus expose `GetDefaultFocusWidget()`; focus applied post `AddToViewport` via `SetUserFocus/SetKeyboardFocus`
    In game HUD + overlays : health/ammo updates, pause menu, death/game over screens
    Round intro presentation : round splash + audio cue, plus persistent round HUD

Networking / Replication Highlights
    Clear server authority boundaries (damage/death, match flow)
    Replicated gameplay state drives UI (no per frame UI polling where avoidable)
    Multiplayer ready UI behavior (client leave vs server shutdown flow)

Debugging / Tooling
    Custom Gameplay Debugger category for runtime inspection (network + actor state)
    Built to support fast iteration and verification of replicated behavior

Architecture Decisions
    GameMode: server only rules
    GameState: replicated match state (rounds/phases)
    PlayerController: owns UI decisions + input modes
    Widgets: presentation only (no gameplay authority)

Animation-driven melee damage (server-authoritative)
    Implemented a custom UAnimNotifyState (“MeleeWindow”) that opens a hit window during the attack montage and performs a socket-based sphere sweep each tick to apply damage on the server only, with per-swing hit tracking to prevent double-hits.


How to Run (Quick)
    Open the project and play `MainMenu_Level`
    Multiplayer flow: Mode Select → Multiplayer → Find/Join (or Create Session) → Lobby → Ready → Start

Demo Checklist (What to Look For)
    Menu stack navigation + focus behavior
    Async session search UX (disabled buttons + blocker + status text)
    Join session → lobby ready flow
    In game HUD updates (health/ammo) + pause/death/game over
    Round splash + persistent round HUD

Scope Note
    A Deathmatch mode is planned but intentionally excluded from this slice. The focus is a polished Zombies loop and supporting UI/tools rather than splitting effort across unfinished modes.

Please find my demo reels links below :

Google Drive : https://drive.google.com/drive/folders/1zFbdSLvvZmIGkbZsb9blHg2UJH909r82
Youtube channel : https://www.youtube.com/channel/UCBSKX107MpsEqtzOctA4nog 