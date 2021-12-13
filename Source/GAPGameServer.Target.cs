// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class GAPGameServerTarget : TargetRules
{
	public GAPGameServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		ExtraModuleNames.Add("GAPGame");
	}
}
