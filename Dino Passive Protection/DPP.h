#pragma once

#include <Logger/Logger.h>
#include <API/UE/Containers/FString.h>
#include "json.hpp"

namespace DinoPassiveProtection
{

	#pragma region Variables
	//Variable for console logging for debugging

	bool EnableConsoleDebugging;

	//Protection Parameters from config
	bool RequiresPassive;
	bool RequiresPassiveFlee;
	bool RequiresNoRider;
	bool RequiresNoInventory;
	bool RequireNoSaddle;
	bool RequiresNotFollowing;
	bool RequiresIgnoreWhistle;
	bool RequiresNeutered;
	float MinimumHealthPercentage;
	bool RequiresNoNearbyEnemyStructures;
	float MinimumEnemyStructureDistanceInFoundations;
	bool RequiresNotTurretMode;
	bool ProtectBabyDino;
	
	//Message settings from config
	float MessageTextSize;
	float MessageDisplayDelay;
	FLinearColor MessageColor;
	FString PassiveProtectedDinoMessage;
	FString UnprotectedDinoMessage;
	FString NotTribeSameMessage;
	FString NotADinoMessage;
	FString DPPChatCommandPrefix;
	FString DPPInvalidCommand;
	FString NotADinoOrStructureMessage;

	//Missing Protection hint messages
	FString DinoNotPassiveMessage;
	FString DinoNotPassiveFleeMessage;
	FString DinoHasRiderMessage;
	FString DinoHasInventoryeMessage;
	FString DinoHasSaddleMessage;
	FString DinoFollowingMessage;
	FString DinoNotIgnoringWhistlesMessage;
	FString DinoNotNeuteredMessage;
	FString DinoBelowMinHealthMessage;
	FString DinoNearEnemyStructureMessage;
	FString DinoInTurretModeMessage;
	FString DinoIsBlacklistedMessage;

	//FString array for printing why protection fails
	std::vector<FString> MissingProtectionHintMessages;

	//Vectors for tracking dino blacklist and structure whitelist
	std::vector<std::string> DinoBlacklist;
	std::vector<std::string> StructureWhitelist;

	//JSON vars
	nlohmann::json config, TempConfig;

	#pragma endregion Variables

	//Takes an Object and prints the blueprint path if it's a structure or dino
	//Credit to Michidu: Structure Limit v1.1
	//Does not reflect "True" Blueprint paths: Spawn blueprints and plugin blueprints may differ wildly
	//Use the "!dpp getpath" command for correct blueprint for black and white lists
	FString GetBlueprint(UObjectBase* object)
	{

		if (object != nullptr && object->ClassField() != nullptr)
		{
			FString path_name;
			object->ClassField()->GetDefaultObject(true)->GetFullName(&path_name, nullptr);

			if (int find_index = 0; path_name.FindChar(' ', find_index))
			{
				path_name = "Blueprint'" + path_name.Mid(find_index + 1,
					path_name.Len() - (find_index + (path_name.EndsWith(
						"_C", ESearchCase::
						CaseSensitive)
						? 3
						: 1))) + "'";

				return path_name.Replace(L"Default__", L"", ESearchCase::CaseSensitive);
			}
		}
		return FString("");
	}

	//TODO: Needs ability to ignore environment damage like falling through the map and lava
	//Checks the dinos protection status
	int CheckDinoProtection(APrimalDinoCharacter* dino)
	{
		//_this != NULL
		if (dino)
		{
			if (DinoPassiveProtection::EnableConsoleDebugging)
			{
				Log::GetLog()->warn("## Beginning of debug ##");
				Log::GetLog()->warn("#####################################################################");
			}

			//Dino is a tribe dino
			if (dino->TargetingTeamField() > 10000)
			{
				//var for dino name for debugging (and one day Logging)
				FString DinoName = "";

				//variable declarations for potential protection parameters
				bool isPassiveAggressive = false;
				bool isPassiveFlee = false;
				bool hasNoRider = false;
				bool hasNoInventory = true;
				bool hasNoSaddle = true;
				bool isNotFollowing = true;
				bool isIgnoringWhistles = false;
				bool isNeutered = false;
				bool isHealthAboveMin = false;
				bool isNotNearEnemyStructures = true;
				bool isNotInTurretMode = false;
				bool isBaby = false;
				bool isBlacklisted = false;

				//Check to see if name is needed
				if (DinoPassiveProtection::DinoBlacklist.size() > 0 || DinoPassiveProtection::EnableConsoleDebugging)
				{
					//get Dino Name
					dino->GetDinoDescriptiveName(&DinoName);
					int32 Index = 0;
					if (DinoName.FindLastChar('-', Index))
					{
						DinoName = DinoName.RightChop(Index + 2);
						if (DinoName.FindLastChar('(', Index)) DinoName.RemoveAt(Index);
						if (DinoName.FindLastChar(')', Index)) DinoName.RemoveAt(Index);
					}
				}
				
				//Check if passive check is needed
				if (DinoPassiveProtection::RequiresPassive || DinoPassiveProtection::EnableConsoleDebugging)
				{
					//0 is passive or passive flee (Both considered "passive")
					if (dino->TamedAggressionLevelField() == 0)
					{
						isPassiveAggressive = true;
					}
				}

				//check if passive flee check is needed
				if (DinoPassiveProtection::RequiresPassiveFlee || DinoPassiveProtection::EnableConsoleDebugging)
				{
					//get passive flee status
					isPassiveFlee = dino->bPassiveFlee()();
				}
				
				//check if has rider check is needed
				if (DinoPassiveProtection::RequiresNoRider || DinoPassiveProtection::EnableConsoleDebugging)
				{
					//get rider status
					hasNoRider = !(dino->bHasRider()());
				}
				
				//check if inventory check is needed
				if (DinoPassiveProtection::RequiresNoInventory || DinoPassiveProtection::EnableConsoleDebugging) 
				{
					//Checks inventory for any items
					UPrimalInventoryComponent* inventory = dino->MyInventoryComponentField();

					if (inventory != nullptr)
					{
						int item_count = 0;

						// Count items
						TArray<UPrimalItem*> items = inventory->InventoryItemsField();

						for (UPrimalItem* item : items)
						{
							if (item->ClassField() != nullptr)
							{
								item_count += item->GetItemQuantity();

								//if at least one item in inventory, set hasNoInventory to false and break
								if (item_count >= 1)
								{
									hasNoInventory = false;
									break;
								}
							}
						}
					}
				}

				//check if saddle check is needed
				if (DinoPassiveProtection::RequireNoSaddle || DinoPassiveProtection::EnableConsoleDebugging)
				{
					UPrimalInventoryComponent* inventory = dino->MyInventoryComponentField();
					
					if (inventory != nullptr)
					{
						TArray<UPrimalItem*> equppedItems = inventory->EquippedItemsField();
						for (UPrimalItem* item : equppedItems)
						{
							if (item->ClassField() != nullptr)
							{
								FString itemName;
								item->NameField().ToString(&itemName);
								if (itemName.Contains("Saddle"))
								{
									hasNoSaddle = false;
								}
								if (DinoPassiveProtection::EnableConsoleDebugging)
								{
									Log::GetLog()->warn("Equipped Item: {}", itemName.ToString());
								}
							}
						}
					}
				}

				//check if not following check is needed
				if (DinoPassiveProtection::RequiresNotFollowing || DinoPassiveProtection::EnableConsoleDebugging)
				{
					//check if dino is following something
					if (dino->GetTamedFollowTarget())
					{
						isNotFollowing = false;
					}
				}
				
				//check if ignore whistle check is needed
				if (DinoPassiveProtection::RequiresIgnoreWhistle || DinoPassiveProtection::EnableConsoleDebugging)
				{
					//get ingoring whistle status
					isIgnoringWhistles = dino->bIgnoreAllWhistles()();
				}
				
				//check if neuter check is needed
				if (DinoPassiveProtection::RequiresNeutered || DinoPassiveProtection::EnableConsoleDebugging)
				{
					//get neutered status
					isNeutered = dino->bNeutered()();
				}

				//check to see if minimum health check is needed
				if (DinoPassiveProtection::MinimumHealthPercentage > 0 || DinoPassiveProtection::EnableConsoleDebugging)
				{
					//var to get dino stats
					UPrimalCharacterStatusComponent* charStatus = dino->GetCharacterStatusComponent();

					//Check if health is above threshold percentage
					float* currentHealth = charStatus->CurrentStatusValuesField()();
					float* maxHealth = charStatus->MaxStatusValuesField()();

					if (*currentHealth > (*maxHealth * (DinoPassiveProtection::MinimumHealthPercentage / 100.f)))
					{
						isHealthAboveMin = true;
					}
				}
				
				//check if baby check is needed
				if (DinoPassiveProtection::ProtectBabyDino || DinoPassiveProtection::EnableConsoleDebugging)
				{
					//get info if it's a baby
					isBaby = dino->bIsBaby()();
					if (DinoPassiveProtection::ProtectBabyDino && isBaby)
					{
						isHealthAboveMin = true;
					}
				}
				
				//check if turret check is needed
				if (DinoPassiveProtection::RequiresNotTurretMode || DinoPassiveProtection::EnableConsoleDebugging)
				{
					//get Turret mode status
					isNotInTurretMode = !(dino->bIsInTurretMode()());
				}

				//OPTIMIZE?
				//Check if Structure check is needed
				if ((DinoPassiveProtection::RequiresNoNearbyEnemyStructures && DinoPassiveProtection::StructureWhitelist.size() > 0) || DinoPassiveProtection::EnableConsoleDebugging)
				{
					bool  previousProtectionRequirementsMet = true;

					//Structure Whitelist check is very lag inducing. 
					//The below check is being introduced in an attempt to reduce it in most cases
					//Check if all previous conditions are met before running structure check

					//build config array for comparing against
					bool configConditionsForStructureCheck[] =
					{
						DinoPassiveProtection::RequiresPassive,
						DinoPassiveProtection::RequiresPassiveFlee,
						DinoPassiveProtection::RequiresNoRider,
						DinoPassiveProtection::RequiresNoInventory,
						DinoPassiveProtection::RequireNoSaddle,
						DinoPassiveProtection::RequiresNotFollowing,
						DinoPassiveProtection::RequiresIgnoreWhistle,
						DinoPassiveProtection::RequiresNeutered,
						(DinoPassiveProtection::MinimumHealthPercentage > 0),
						DinoPassiveProtection::RequiresNotTurretMode
					};

					//build array of aquired dino parameters for comparing
					bool dinoActualsForStructureCheck[] =
					{
						isPassiveAggressive,
						isPassiveFlee,
						hasNoRider,
						hasNoInventory,
						hasNoSaddle,
						isNotFollowing,
						isIgnoringWhistles,
						isNeutered,
						isHealthAboveMin,
						isNotInTurretMode
					};

					//Compare config values to dino values to decide if dino is protectected or not
					//if configCondition is true, then dinoActual has to be true as well or else check fails 
					//if configCondition is false, then it does not matter what dinoActual is
					for (int i = 0; i < sizeof(configConditionsForStructureCheck); ++i)
					{

						//Dino not eligible for blacklist check
						if (configConditionsForStructureCheck[i] && !dinoActualsForStructureCheck[i])
						{
							previousProtectionRequirementsMet = false;
							break;
						}
					}

					//dino eligible for blacklist check
					if (previousProtectionRequirementsMet || DinoPassiveProtection::EnableConsoleDebugging)
					{
						//TODO: Potentially force a check to be required near owned structures
						//check for enemy structures nearby	
						UWorld* world = ArkApi::GetApiUtils().GetWorld();
						TArray<AActor*> new_actors;
						TArray<AActor*> actors_ignore;
						TArray<TEnumAsByte<enum EObjectTypeQuery>> types;

						UKismetSystemLibrary::SphereOverlapActors_NEW(world, dino->RootComponentField()->RelativeLocationField(),
							static_cast<float>((DinoPassiveProtection::MinimumEnemyStructureDistanceInFoundations * 300)),
							&types, APrimalStructure::GetPrivateStaticClass(), &actors_ignore, &new_actors);

						for (const auto& actor : new_actors)
						{
							APrimalStructure* structure = static_cast<APrimalStructure*>(actor);

							if (structure->TargetingTeamField() != dino->TargetingTeamField())
							{
								FString stuctPath;
								stuctPath = GetBlueprint(structure);

								//log line to check structure path
								if (DinoPassiveProtection::EnableConsoleDebugging)
								{
									Log::GetLog()->warn("=================================================================================");
									Log::GetLog()->warn("stuctPath:            {}", stuctPath.ToString());
									Log::GetLog()->warn("Foundations from dino {}", (FVector::Distance(dino->RootComponentField()->RelativeLocationField(), actor->RootComponentField()->RelativeLocationField())) / 300);
									Log::GetLog()->warn("=================================================================================");
								}

								//If Whitelist BP path matches structure BP path
								if (std::count(DinoPassiveProtection::StructureWhitelist.begin(), DinoPassiveProtection::StructureWhitelist.end(), stuctPath.ToString()))
								{
									isNotNearEnemyStructures = false;

									//log line to confirm enemy structure found
									if (DinoPassiveProtection::EnableConsoleDebugging)
									{
										Log::GetLog()->warn("=================================================================================");
										Log::GetLog()->warn("Whitelisted structure found:");
										Log::GetLog()->warn("stuctPath:            {}", stuctPath.ToString());
										Log::GetLog()->warn("Foundations from dino {}", (FVector::Distance(dino->RootComponentField()->RelativeLocationField(), actor->RootComponentField()->RelativeLocationField())) / 300);
										Log::GetLog()->warn("=================================================================================");
									}
									break;
								}
							}
						}
					}
				}

				//OPTIMIZE?
				//Check if dino Blacklist check is needed
				if (DinoPassiveProtection::DinoBlacklist.size() > 0 || DinoPassiveProtection::EnableConsoleDebugging)
				{
					bool  previousProtectionRequirementsMet = true;

					//Black list check is very lag inducing The blow check is being introduced in an attempt to reduce it in most cases
					//check if all previous conditions are met before running blacklist check
					//build config array for comparing against
					bool configConditionsForBlacklistCheck[] =
					{
						DinoPassiveProtection::RequiresPassive,
						DinoPassiveProtection::RequiresPassiveFlee,
						DinoPassiveProtection::RequiresNoRider,
						DinoPassiveProtection::RequiresNoInventory,
						DinoPassiveProtection::RequireNoSaddle,
						DinoPassiveProtection::RequiresNotFollowing,
						DinoPassiveProtection::RequiresIgnoreWhistle,
						DinoPassiveProtection::RequiresNeutered,
						(DinoPassiveProtection::MinimumHealthPercentage > 0),
						DinoPassiveProtection::RequiresNoNearbyEnemyStructures,
						DinoPassiveProtection::RequiresNotTurretMode
					};

					//build array of aquired dino parameters for comparing
					bool dinoActualsForBlacklistCheck[] =
					{
						isPassiveAggressive,
						isPassiveFlee,
						hasNoRider,
						hasNoInventory,
						hasNoSaddle,
						isNotFollowing,
						isIgnoringWhistles,
						isNeutered,
						isHealthAboveMin,
						isNotNearEnemyStructures,
						isNotInTurretMode
					};

					//Compare config values to dino values to decide if dino is protectected or not
					//if configCondition is true, then dinoActual has to be true as well or else check fails 
					//if configCondition is false, then it does not matter what dinoActual is
					for (int i = 0; i < sizeof(configConditionsForBlacklistCheck); ++i)
					{

						//Dino not eligible for blacklist check
						if (configConditionsForBlacklistCheck[i] && !dinoActualsForBlacklistCheck[i])
						{
							previousProtectionRequirementsMet = false;
							break;
						}
					}

					//dino eligible for blacklist check
					if (previousProtectionRequirementsMet)
					{
						//Check dino to see if it blacklisted in the config

						FString dinoGetBlueprint;
						dinoGetBlueprint = GetBlueprint(dino);

						if (std::count(DinoPassiveProtection::DinoBlacklist.begin(), DinoPassiveProtection::DinoBlacklist.end(), dinoGetBlueprint.ToString()))
						{
							isBlacklisted = true;
						}

						//log line to check dino path
						if (DinoPassiveProtection::EnableConsoleDebugging)
						{
							Log::GetLog()->warn("dinoGetBlueprint: {}", dinoGetBlueprint.ToString());
							Log::GetLog()->warn("Blacklisted dino: {}", isBlacklisted);
							Log::GetLog()->warn("-------------------------------------------------");
						}
					}
				}

				#pragma region Additional Logging
				//Logging for Debugging
				if (DinoPassiveProtection::EnableConsoleDebugging)
				{
					Log::GetLog()->warn("Dino name:                        {}", DinoName.ToString());
					Log::GetLog()->warn("Dino is Passive:                  {}", isPassiveAggressive);
					Log::GetLog()->warn("Dino is Passive flee:             {}", isPassiveFlee);
					Log::GetLog()->warn("Dino has No rider:                {}", hasNoRider);
					Log::GetLog()->warn("Dino has No inventory:            {}", hasNoInventory);
					Log::GetLog()->warn("Dino has No saddle:               {}", hasNoSaddle);
					Log::GetLog()->warn("Dino isn't Following a target:    {}", isNotFollowing);
					Log::GetLog()->warn("Dino is Ignoring whistle:         {}", isIgnoringWhistles);
					Log::GetLog()->warn("Dino is Neutered:                 {}", isNeutered);
					Log::GetLog()->warn("Dino is Above min health:         {}", isHealthAboveMin);
					Log::GetLog()->warn("Dino isn't Near enemy Structures: {}", isNotNearEnemyStructures);
					Log::GetLog()->warn("Dino isn't in Turret mode:        {}", isNotInTurretMode);
					Log::GetLog()->warn("Dino is a Baby:                   {}", isBaby);
				}
				#pragma endregion Additional Logging

				//build config array for comparing against
				bool configConditions[] =
				{
					DinoPassiveProtection::RequiresPassive,
					DinoPassiveProtection::RequiresPassiveFlee,
					DinoPassiveProtection::RequiresNoRider,
					DinoPassiveProtection::RequiresNoInventory,
					DinoPassiveProtection::RequireNoSaddle,
					DinoPassiveProtection::RequiresNotFollowing,
					DinoPassiveProtection::RequiresIgnoreWhistle,
					DinoPassiveProtection::RequiresNeutered,
					(DinoPassiveProtection::MinimumHealthPercentage > 0),
					DinoPassiveProtection::RequiresNoNearbyEnemyStructures,
					DinoPassiveProtection::RequiresNotTurretMode,
					(DinoPassiveProtection::DinoBlacklist.size() > 0)
				};

				//build array of aquired dino parameters for comparing
				bool dinoActuals[] =
				{
					isPassiveAggressive,
					isPassiveFlee,
					hasNoRider,
					hasNoInventory,
					hasNoSaddle,
					isNotFollowing,
					isIgnoringWhistles,
					isNeutered,
					isHealthAboveMin,
					isNotNearEnemyStructures,
					isNotInTurretMode,
					!isBlacklisted
				};

				//Compare config values to dino values to decide if dino is protectected or not
				//if configCondition is true, then dinoActual has to be true as well or else check fails 
				//if configCondition is false, then it does not matter what dinoActual is
				for (int i = 0; i < sizeof(configConditions); ++i)
				{

					if (configConditions[i] && !dinoActuals[i])
					{
						if (DinoPassiveProtection::EnableConsoleDebugging)
						{
							Log::GetLog()->warn("Dino {} is not Protected", DinoName.ToString());
							Log::GetLog()->warn("Reason: {}", DinoPassiveProtection::MissingProtectionHintMessages[i].ToString());
							Log::GetLog()->warn("#####################################################################");
							Log::GetLog()->warn("## End of debug ##");
						}
						//dino not protected, returning index for missing protection hint
						return i;
					}
				}

				//logging for when dino is protected
				if (DinoPassiveProtection::EnableConsoleDebugging)
				{
					Log::GetLog()->warn("Dino {} is Protected", DinoName.ToString());
					Log::GetLog()->warn("#####################################################################");
					Log::GetLog()->warn("## End of debug ##");
				}
				// dinoe is protected
				return -1;
			}
		}
		//not a dino
		return 999;
	}
}
