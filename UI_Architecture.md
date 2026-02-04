UI Architecture Overview
Core Principles

The UI system is designed to be modular, event-driven, and multiplayer-safe.
All game logic and state live outside widgets; widgets are strictly presentation and input forwarding.

Ownership & Responsibilities

GameMode (Server-only)
Owns rules and win/lose conditions.

GameState / PlayerState (Replicated)
Own replicated match data (round number, match phase, player stats).

PlayerController (Per-client, authoritative over UI)

    Owns all UI decisions and input modes

    Creates and manages widgets

    Responds to replicated state via delegates

    Handles pause behavior (local-only in multiplayer)

Widgets (UUserWidget)

    Presentation-only

    Bind to buttons and forward intent to PlayerController

    Never pause the game, change input modes, or modify game state directly

Menu Management

A centralized MenuUIManager, owned by MyPlayerController, handles:

    Widget creation and caching

    Menu navigation (show/hide)

    Layering and z-order

    Consistent input/focus behavior

This avoids widget-to-widget dependencies and keeps menus scalable.

Input Handling

Input is applied via a single switch point in PlayerController:

    Gameplay

    Lobby

    Pause

Pause input is local-only:

    Solo: pauses the world

    Multiplayer: pauses UI only

Pause Menu Behavior

    Opened via PlayerController (Escape key)

    Resume / Quit / Restart actions are routed back to PlayerController

    Restart is disabled in multiplayer

    Widgets do not know whether the game is solo or multiplayer

Debugging Support

A custom Gameplay Debugger category provides runtime visibility into:

    Actor networking roles

    Movement state

    Gameplay tags

    Health/state variables

This supports rapid validation of UI and gameplay state without modifying UI code.

Scope Note

Advanced systems (leaderboards, post-game summaries) are intentionally excluded to focus on production-ready UI architecture and gameplay flow, not feature breadth.