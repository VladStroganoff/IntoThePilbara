// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class GAPGameClientTarget : TargetRules
{
	public GAPGameClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;

        bUsesSteam = true;
        bUseLoggingInShipping = true;

        GlobalDefinitions.Add("UE4_PROJECT_STEAMGAMEDIR=\"Spacewar\"");
        GlobalDefinitions.Add("UE4_PROJECT_STEAMSHIPPINGID=480");

        ExtraModuleNames.Add("GAPGame");
	}
}
