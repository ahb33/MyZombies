#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"

class AMyPlayerController;
class UWdiget;

namespace UIHelpers
{
    
    void SetUIInputMode(AMyPlayerController* PC, bool bUIOnly, UWdiget* FocusWidget = nullptr); // Sets UI-only input mode with optional widget focus

    
    void RestoreGameplayInput(AMyPlayerController* PC); // Restores gameplay-only input mode


    void ForceSafeNetUpdate(AActor* Actor); // Forces safe network update (dormancy + flush + force) if actor has authority. Sets UI-only input mode with optional widget focus

}