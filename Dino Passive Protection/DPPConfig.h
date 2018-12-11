#pragma once
#include <fstream>
#include <Permissions.h>


inline void InitConfig()
{
	std::ifstream file(ArkApi::Tools::GetCurrentDir() + "/ArkApi/Plugins/DinoPassiveProtection/config.json");

	if (!file.is_open())
	{
		Log::GetLog()->warn("Config File not found!");
		return;
	}

	file >> DinoPassiveProtection::config;
	file.close();

	DinoPassiveProtection::RequiresNotFollowing = DinoPassiveProtection::config["General"]["RequiresNotFollowing"];
	DinoPassiveProtection::RequiresPassiveFlee = DinoPassiveProtection::config["General"]["RequiresPassiveFlee"];
	DinoPassiveProtection::RequiresIgnoreWhistle = DinoPassiveProtection::config["General"]["RequiresIgnoreWhistle"];
	DinoPassiveProtection::RequiresNeutered = DinoPassiveProtection::config["General"]["RequiresNeutered"];
	DinoPassiveProtection::RequiresNoRider = DinoPassiveProtection::config["General"]["RequiresNoRider"];
	DinoPassiveProtection::RequiresNoInventory = DinoPassiveProtection::config["General"]["RequiresNoInventory"];
	DinoPassiveProtection::RequiresNoNearbyEnemyStructures = DinoPassiveProtection::config["General"]["RequiresNoNearbyEnemyStructures"];
	DinoPassiveProtection::MinimumEnemyStructureDistance = DinoPassiveProtection::config["General"]["MinimumEnemyStructureDistance"];
	DinoPassiveProtection::MinimumHealthPercentage = DinoPassiveProtection::config["General"]["MinimumHealthPercentage"];
	DinoPassiveProtection::MessageTextSize = DinoPassiveProtection::config["General"]["MessageTextSize"];
	DinoPassiveProtection::MessageDisplayDelay = DinoPassiveProtection::config["General"]["MessageDisplayDelay"];
	DinoPassiveProtection::TempConfig = DinoPassiveProtection::config["General"]["MessageColor"];
	DinoPassiveProtection::MessageColor = FLinearColor(DinoPassiveProtection::TempConfig[0], DinoPassiveProtection::TempConfig[1], DinoPassiveProtection::TempConfig[2], DinoPassiveProtection::TempConfig[3]);
	DinoPassiveProtection::PassiveProtectedDinoTakingDamageMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["PassiveProtectedDinoTakingDamageMessage"]).c_str());
	}