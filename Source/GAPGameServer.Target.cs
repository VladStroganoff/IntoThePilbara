// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class GAPGameServerTarget : TargetRules
{
	public GAPGameServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;

        bUsesSteam = true;
        bUseLoggingInShipping = true;

        GlobalDefinitions.Add("UE4_PROJECT_STEAMPRODUCTNAME=\"Spacewar\"");
        GlobalDefinitions.Add("UE4_PROJECT_STEAMDESC=\"GAPGame\"");
        GlobalDefinitions.Add("UE4_PROJECT_STEAMGAMEDIR=\"Spacewar\"");
        GlobalDefinitions.Add("UE4_PROJECT_STEAMSHIPPINGID=480");

        ExtraModuleNames.Add("GAPGame");
	}
}
