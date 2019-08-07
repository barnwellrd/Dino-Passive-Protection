#pragma once
#include <fstream>
#include <ARKPermissions.h>


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

	DinoPassiveProtection::EnableConsoleDebugging = DinoPassiveProtection::config["General"]["EnableConsoleDebugging"];

	//Get Protection Parameters
	DinoPassiveProtection::RequiresPassive = DinoPassiveProtection::config["General"]["RequiresPassive"];
	DinoPassiveProtection::RequiresPassiveFlee = DinoPassiveProtection::config["General"]["RequiresPassiveFlee"];
	DinoPassiveProtection::RequiresNoRider = DinoPassiveProtection::config["General"]["RequiresNoRider"];
	DinoPassiveProtection::RequiresNoInventory = DinoPassiveProtection::config["General"]["RequiresNoInventory"];
	DinoPassiveProtection::RequireNoSaddle = DinoPassiveProtection::config["General"]["RequireNoSaddle"];
	DinoPassiveProtection::RequiresNotFollowing = DinoPassiveProtection::config["General"]["RequiresNotFollowing"];
	DinoPassiveProtection::RequiresIgnoreWhistle = DinoPassiveProtection::config["General"]["RequiresIgnoreWhistle"];
	DinoPassiveProtection::RequiresNeutered = DinoPassiveProtection::config["General"]["RequiresNeutered"];
	DinoPassiveProtection::MinimumHealthPercentage = DinoPassiveProtection::config["General"]["MinimumHealthPercentage"];
	DinoPassiveProtection::RequiresNoNearbyEnemyStructures = DinoPassiveProtection::config["General"]["RequiresNoNearbyEnemyStructures"];
	DinoPassiveProtection::MinimumEnemyStructureDistanceInFoundations = DinoPassiveProtection::config["General"]["MinimumEnemyStructureDistanceInFoundations"];
	DinoPassiveProtection::RequiresNotTurretMode = DinoPassiveProtection::config["General"]["RequiresNotTurretMode"];
	DinoPassiveProtection::ProtectBabyDino = DinoPassiveProtection::config["General"]["ProtectBabyDino"];

	//Get Message display settings
	DinoPassiveProtection::MessageTextSize = DinoPassiveProtection::config["General"]["MessageTextSize"];
	DinoPassiveProtection::MessageDisplayDelay = DinoPassiveProtection::config["General"]["MessageDisplayDelay"];
	DinoPassiveProtection::TempConfig = DinoPassiveProtection::config["General"]["MessageColor"];
	DinoPassiveProtection::MessageColor = FLinearColor(DinoPassiveProtection::TempConfig[0], DinoPassiveProtection::TempConfig[1], DinoPassiveProtection::TempConfig[2], DinoPassiveProtection::TempConfig[3]);
	
	//Get plugin mconfig messages
	DinoPassiveProtection::PassiveProtectedDinoMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["PassiveProtectedDinoMessage"]).c_str());
	DinoPassiveProtection::UnprotectedDinoMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["UnprotectedDinoMessage"]).c_str());
	DinoPassiveProtection::NotTribeSameMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["NotTribeSameMessage"]).c_str());
	DinoPassiveProtection::NotADinoMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["NotADinoMessage"]).c_str());
	DinoPassiveProtection::DPPChatCommandPrefix = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DPPChatCommandPrefix"]).c_str());
	DinoPassiveProtection::DPPInvalidCommand = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DPPInvalidCommand"]).c_str());
	DinoPassiveProtection::NotADinoOrStructureMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["NotADinoOrStructureMessage"]).c_str());

	//Get dino protection hint messages
	DinoPassiveProtection::DinoNotPassiveMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DinoNotPassiveMessage"]).c_str());
	DinoPassiveProtection::DinoNotPassiveFleeMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DinoNotPassiveFleeMessage"]).c_str());
	DinoPassiveProtection::DinoHasRiderMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DinoHasRiderMessage"]).c_str());
	DinoPassiveProtection::DinoHasInventoryeMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DinoHasInventoryeMessage"]).c_str());
	DinoPassiveProtection::DinoHasSaddleMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DinoHasSaddleMessage"]).c_str());
	DinoPassiveProtection::DinoFollowingMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DinoFollowingMessage"]).c_str());
	DinoPassiveProtection::DinoNotIgnoringWhistlesMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DinoNotIgnoringWhistlesMessage"]).c_str());
	DinoPassiveProtection::DinoNotNeuteredMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DinoNotNeuteredMessage"]).c_str());
	DinoPassiveProtection::DinoBelowMinHealthMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DinoBelowMinHealthMessage"]).c_str());
	DinoPassiveProtection::DinoNearEnemyStructureMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DinoNearEnemyStructureMessage"]).c_str());
	DinoPassiveProtection::DinoInTurretModeMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DinoInTurretModeMessage"]).c_str());
	DinoPassiveProtection::DinoIsBlacklistedMessage = FString(ArkApi::Tools::Utf8Decode(DinoPassiveProtection::config["General"]["DinoIsBlacklistedMessage"]).c_str());



	DinoPassiveProtection::MissingProtectionHintMessages =
	{
		DinoPassiveProtection::DinoNotPassiveMessage,
		DinoPassiveProtection::DinoNotPassiveFleeMessage,
		DinoPassiveProtection::DinoHasRiderMessage,
		DinoPassiveProtection::DinoHasInventoryeMessage,
		DinoPassiveProtection::DinoHasSaddleMessage,
		DinoPassiveProtection::DinoFollowingMessage,
		DinoPassiveProtection::DinoNotIgnoringWhistlesMessage,
		DinoPassiveProtection::DinoNotNeuteredMessage,
		DinoPassiveProtection::DinoBelowMinHealthMessage,
		DinoPassiveProtection::DinoNearEnemyStructureMessage,
		DinoPassiveProtection::DinoInTurretModeMessage,
		DinoPassiveProtection::DinoIsBlacklistedMessage
	};

	//Clear vector so that config reload is clean
	DinoPassiveProtection::DinoBlacklist.clear();

	//Load Blacklisted dinos from config
	DinoPassiveProtection::TempConfig = DinoPassiveProtection::config["General"]["DinoBlacklist"];

	for (nlohmann::json x : DinoPassiveProtection::TempConfig)
	{
		DinoPassiveProtection::DinoBlacklist.push_back(FString(ArkApi::Tools::Utf8Decode(x).c_str()));
	}

	//Clear vector so that config reload is clean
	DinoPassiveProtection::StructureWhitelist.clear();

	//Load Whitelisted structures from config
	DinoPassiveProtection::TempConfig = DinoPassiveProtection::config["General"]["StructureWhitelist"];

	for (nlohmann::json x : DinoPassiveProtection::TempConfig)
	{
		DinoPassiveProtection::StructureWhitelist.push_back(FString(ArkApi::Tools::Utf8Decode(x).c_str()));
	}
}
