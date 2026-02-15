

CC Gameplay Debugger Category

What it is
    A custom Gameplay Debugger category for quickly validating AI perception + combat gating in live gameplay without adding temporary HUD debug code.

Key features
    Range marker**: visual “attack/interaction range bubble” around the selected AI
    Vision marker**: segment from AI → player + label when AI can see the player
    Blackboard readout**: prints `PlayerWithinRange`, `CanSeePlayer`, `CanHearPlayer`
    Networking hint**: prints actor net role to confirm authority context while testing

    

Controls
- **R**: Toggle Range visualization
- **V**: Toggle Vision visualization
- **O**: Toggle AI-only filtering (hide non-AI actors)

How it works
    Attempts to read a radius from the selected actor (`AttackRange`, `InteractRange`, `InteractionRange`). If noneexist, falls back to a capsule-size-derived default.
    Computes **player distance** and highlights range state (green=in range, red=out of range).
    Uses both: Blackboard `CanSeePlayer` (behavior-tree view) and `AAIController::LineOfSightTo()` (engine LOS check)  to drive the vision marker.

Why this matters
    Reduces iteration time while tuning perception, ranges, and BT conditions.
    Makes replicated testing less guessy by surfacing role/state inline



CCDebug (Runtime Debug Utilities)

What it is
    A lightweight debug utilities module for Unreal projects that provides:
    runtime-togglable logging macros,
    scoped timing for quick perf probes,
    a network “flush” helper to speed up replication iteration,
    optional Visual Logger integration.

All functionality is disabled in Shipping builds.

Key features

Runtime toggle (no rebuild)
    Console variable: `cc.Debug`
    `cc.Debug 1` = enabled
    `cc.Debug 0` = disabled

Internally, `CCD_IsEnabled()` gates all output in non-shipping builds.

Logging macros
    `CCDBG(Actor, TEXT("Message %d"), Value)`
    Logs: `[Role][ActorName]: formatted message`
    `CCDBG_IF(Cond, Actor, TEXT("..."), ...)`
    Conditional logging

Example:
    cpp
CCDBG(this, TEXT("Health=%0.1f Ammo=%d"), Health, Ammo);
CCDBG_IF(bIsDead, this, TEXT("Player died"));


