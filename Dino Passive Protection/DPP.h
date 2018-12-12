#pragma once

#include <Logger/Logger.h>
#include <API/UE/Containers/FString.h>
#include "json.hpp"

namespace DinoPassiveProtection
{
	bool RequiresNotFollowing;
	bool RequiresPassiveFlee;
	bool RequiresIgnoreWhistle;
	bool RequiresNeutered;
	bool RequiresNoRider;
	bool RequiresNoInventory;
	bool RequiresNoNearbyEnemyStructures;
	float MinimumEnemyStructureDistanceInFoundations;
	float MinimumHealthPercentage;
	float MessageTextSize;
	float MessageDisplayDelay;
	FLinearColor MessageColor;
	FString PassiveProtectedDinoTakingDamageMessage;

	nlohmann::json config, TempConfig;
}



