#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformTime.h"
#include "HAL/IConsoleManager.h"


CCDEBUG_API DECLARE_LOG_CATEGORY_EXTERN(LogCCDebug, Log, All);


// Runtime toggle: cc.Debug 0/1
FORCEINLINE bool CCD_IsEnabled()
{
#if !UE_BUILD_SHIPPING
	if (IConsoleVariable* CVar = IConsoleManager::Get().FindConsoleVariable(TEXT("cc.Debug")))
		return CVar->GetInt() != 0;
	return true;
#else
	return false;
#endif
}


FORCEINLINE const TCHAR* CCD_RoleStr(const AActor* A)
{
	if (!A) return TEXT("None");
	switch (A->GetLocalRole())
	{
	case ROLE_Authority:       return TEXT("Authority");
	case ROLE_AutonomousProxy: return TEXT("AutoProxy");
	case ROLE_SimulatedProxy:  return TEXT("SimProxy");
	default:                   return TEXT("Unknown");
	}
}

struct FCCDScopeTimer
{
	AActor* Actor; const TCHAR* Label; double Start;
	explicit FCCDScopeTimer(AActor* InActor, const TCHAR* InLabel)
		: Actor(InActor), Label(InLabel), Start(FPlatformTime::Seconds()) {}
	~FCCDScopeTimer()
	{
#if !UE_BUILD_SHIPPING
		if (!CCD_IsEnabled()) return;
		const double Ms = (FPlatformTime::Seconds() - Start) * 1000.0;
		UE_LOG(LogCCDebug, Display, TEXT("[SCOPE][%s][%s] %.2f ms"), // formats the floating-point duration with two decimal places in ms
		*GetNameSafe(Actor), Label ? Label : TEXT(""), Ms);
#endif
	}
};


#define CCD_JOIN_INNER(a,b) a##b
#define CCD_JOIN(a,b)       CCD_JOIN_INNER(a,b)

#if !UE_BUILD_SHIPPING
  #define CCDBG(ACTOR, FMT, ...)                                                      \
  do{ if(CCD_IsEnabled()){                                                            \
        UE_LOG(LogCCDebug, Display, TEXT("[%s] %s: %s"),                              \
               CCD_RoleStr(ACTOR), *GetNameSafe(ACTOR),                               \
               *FString::Printf(FMT, ##__VA_ARGS__));                                 \
  } } while(0)
  #define CCDBG_IF(COND, ACTOR, FMT, ...)  do{ if ((COND)) CCDBG(ACTOR, FMT, ##__VA_ARGS__); }while(0)
  #define CCDBG_SCOPE(ACTOR, LABEL_LIT)    FCCDScopeTimer CCD_JOIN(_ccdscope_,__LINE__)(ACTOR, TEXT(LABEL_LIT))
  #define CCDBG_NETFLUSH(ACTOR)            do{ if(ACTOR){ (ACTOR)->FlushNetDormancy(); (ACTOR)->ForceNetUpdate(); CCDBG(ACTOR, TEXT("NET FLUSHED")); } }while(0)
  // Optional Visual Logger:
  // #include "VisualLogger/VisualLogger.h"
  // #define CCD_VLOG(ACTOR, FMT, ...) UE_VLOG(ACTOR, LogCCDebug, Log, FMT, ##__VA_ARGS__)
#else
  #define CCDBG(ACTOR, FMT, ...)          do{}while(0)
  #define CCDBG_IF(COND, ACTOR, FMT, ...) do{}while(0)
  #define CCDBG_SCOPE(ACTOR, LABEL)       do{}while(0)
  #define CCDBG_NETFLUSH(ACTOR)           do{}while(0)
  #define CCD_VLOG(ACTOR, FMT, ...)       do{}while(0)
#endif
