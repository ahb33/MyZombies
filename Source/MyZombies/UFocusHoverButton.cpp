// Fill out your copyright notice in the Description page of Project Settings.


#include "UFocusHoverButton.h"


// void UFocusHoverButton::SynchronizeProperties()
// {
// 	Super::SynchronizeProperties();

// 	// Cache designer-edited style as "default".
// 	bDefaultCached = false;
// 	CacheDefaultStyleIfNeeded();

// 	ApplyFocusVisual(HasKeyboardFocus() || HasAnyUserFocus());
// }

// void UFocusHoverButton::OnAddedToFocusPath(const FFocusEvent& InFocusEvent)
// {
// 	Super::OnAddedToFocusPath(InFocusEvent);

// 	// Critical: keep keyboard focus synced with navigation focus (fixes Enter picking the wrong button).
// 	SetKeyboardFocus();

// 	ApplyFocusVisual(true);
// }

// void UFocusHoverButton::OnRemovedFromFocusPath(const FFocusEvent& InFocusEvent)
// {
// 	Super::OnRemovedFromFocusPath(InFocusEvent);
// 	ApplyFocusVisual(false);
// }

// void UFocusHoverButton::CacheDefaultStyleIfNeeded()
// {
// 	if (bDefaultCached)
// 	{
// 		return;
// 	}

// 	DefaultStyle = WidgetStyle;
// 	bDefaultCached = true;
// }

// void UFocusHoverButton::ApplyFocusVisual(bool bFocused)
// {
// 	if (!bUseHoverStyleWhenFocused)
// 	{
// 		return;
// 	}

// 	CacheDefaultStyleIfNeeded();

// 	if (!bFocused)
// 	{
// 		SetStyle(DefaultStyle);
// 		return;
// 	}

// 	FButtonStyle FocusedStyle = DefaultStyle;

// 	// Render "focused" as if it's hovered.
// 	FocusedStyle.SetNormal(DefaultStyle.Hovered);
// 	FocusedStyle.SetHovered(DefaultStyle.Hovered);

// 	SetStyle(FocusedStyle);
// }