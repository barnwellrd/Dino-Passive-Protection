#pragma once

#include <Logger/Logger.h>
#include <API/UE/Containers/FString.h>
#include "json.hpp"

namespace DinoPassiveProtection
{
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

	//FString array for printing why protection fails
	std::vector<FString> MissingProtectionHintMessages;

	//Vectors for tracking dino blacklist and structure whitelist
	std::vector<FString> DinoBlacklist;
	std::vector<FString> StructureWhitelist;

	//JSON vars
	nlohmann::json config, TempConfig;

	
	

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
			//Check dino to see if it blacklisted in the config
			bool isBlacklisted = false;

			for (FString x : DinoPassiveProtection::DinoBlacklist)
			{
				FString dinoGetBlueprint;
				dinoGetBlueprint = GetBlueprint(dino);
				
				//log line to check dino path
				if (DinoPassiveProtection::EnableConsoleDebugging)
				{
					Log::GetLog()->warn("Blacklist Path:   {}", x.ToString());
					Log::GetLog()->warn("dinoGetBlueprint: {}", dinoGetBlueprint.ToString());

				}

				if (x.Compare(dinoGetBlueprint) == 0)
				{
					isBlacklisted = true;

					if (DinoPassiveProtection::EnableConsoleDebugging)
					{
						Log::GetLog()->warn("Blacklisted dino: {}", isBlacklisted);
						Log::GetLog()->warn("-------------------------------------------------");
					}
					break;
				}

				if (DinoPassiveProtection::EnableConsoleDebugging)
				{
					Log::GetLog()->warn("Blacklisted dino: {}", isBlacklisted);
					Log::GetLog()->warn("-------------------------------------------------");
				}
			}
			
			//If dino is not blacklisted
			if (!isBlacklisted)
			{
				
				//Dino is a tribe dino
				if (dino->TargetingTeamField() > 10000)
				{
					//var to get dino stats
					UPrimalCharacterStatusComponent* charStatus = dino->GetCharacterStatusComponent();

					//var for dino name for debugging (and one day Logging)
					FString DinoName;

					//variable declarations for potential protection parameters
					bool isPassiveAggressive;
					bool isPassiveFlee;
					bool hasNoRider;
					bool hasNoInventory;
					bool hasNoSaddle;
					bool isNotFollowing;
					bool isIgnoringWhistles;
					bool isNeutered;
					bool isHealthAboveMin;
					bool isNotNearEnemyStructures;
					bool isNotInTurretMode;
					bool isBaby;

					//get Dino Name
					dino->GetDinoDescriptiveName(&DinoName);
					int32 Index = 0;
					if (DinoName.FindLastChar('-', Index))
					{
						DinoName = DinoName.RightChop(Index + 2);
						if (DinoName.FindLastChar('(', Index)) DinoName.RemoveAt(Index);
						if (DinoName.FindLastChar(')', Index)) DinoName.RemoveAt(Index);
					}

					//0 is passive or passive flee (Both considered "passive")
					if (dino->TamedAggressionLevelField() == 0)
					{
						isPassiveAggressive = true;
					}
					else
					{
						isPassiveAggressive = false;
					}

					//get passive flee status
					isPassiveFlee = dino->bPassiveFlee()();

					//get rider status
					hasNoRider = !(dino->bHasRider()());

					//TODO: Add Check for saddles and costumes
					//Checks inventory for any items
					//preset to true in case no items found
					hasNoInventory = true;

					UPrimalInventoryComponent* inventory = dino->MyInventoryComponentField();

					if (inventory != nullptr)
					{

						if (DinoPassiveProtection::RequiresNoInventory)
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
										FString itemName;
										item->NameField().ToString(&itemName);
										break;
									}
								}
							}
						}

						hasNoSaddle = true;

						if (DinoPassiveProtection::RequireNoSaddle)
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

					//check if dino is following something
					if (dino->GetTamedFollowTarget())
					{
						isNotFollowing = false;
					}
					else
					{
						isNotFollowing = true;
					}

					//get ingoring whistle status
					isIgnoringWhistles = dino->bIgnoreAllWhistles()();

					//get neutered status
					isNeutered = dino->bNeutered()();

					//Check if health is above threshold percentage
					float* currentHealth = charStatus->CurrentStatusValuesField()();
					float* maxHealth = charStatus->MaxStatusValuesField()();

					if (*currentHealth > (*maxHealth * (DinoPassiveProtection::MinimumHealthPercentage / 100.f)))
					{
						isHealthAboveMin = true;
					}
					else
					{
						isHealthAboveMin = false;
					}

					//get info if it's a baby
					isBaby = dino->bIsBaby()();

					if (DinoPassiveProtection::ProtectBabyDino && isBaby)
					{
						isHealthAboveMin = true;
					}

					//TODO: Potentially force a check top be required near owned structures
					//check for enemy structures nearby	
					UWorld* world = ArkApi::GetApiUtils().GetWorld();
					TArray<AActor*> new_actors;
					TArray<AActor*> actors_ignore;
					TArray<TEnumAsByte<enum EObjectTypeQuery>> types;

					UKismetSystemLibrary::SphereOverlapActors_NEW(world, dino->RootComponentField()->RelativeLocationField(),
						static_cast<float>((DinoPassiveProtection::MinimumEnemyStructureDistanceInFoundations * 300)),
						&types, APrimalStructure::GetPrivateStaticClass(), &actors_ignore, &new_actors);

					//Preset to not near in case no structures found
					isNotNearEnemyStructures = true;

					for (const auto& actor : new_actors)
					{
						APrimalStructure* structure = static_cast<APrimalStructure*>(actor);

						if (structure->TargetingTeamField() != dino->TargetingTeamField())
						{
							for (FString x : DinoPassiveProtection::StructureWhitelist)
							{
								FString stuctPath;
								stuctPath = GetBlueprint(structure);

								//log line to check dino path
								if (DinoPassiveProtection::EnableConsoleDebugging)
								{
									Log::GetLog()->warn("=================================================================================");
									Log::GetLog()->warn("stuctPath:            {}", stuctPath.ToString());
									Log::GetLog()->warn("---------------------------------------------------------------------------------");
									Log::GetLog()->warn("WhiteListed Path:     {}", x.ToString());
									Log::GetLog()->warn("Foundations from dino {}", (FVector::Distance(dino->RootComponentField()->RelativeLocationField(), actor->RootComponentField()->RelativeLocationField())) / 300);
									Log::GetLog()->warn("=================================================================================");
								}

								//If Whitelist BP path matches structure BP path
								if (x.Compare(stuctPath) == 0)
								{
									isNotNearEnemyStructures = false;
									break;
								}
							}
						}
					}

					//get Turret mode status
					isNotInTurretMode = !(dino->bIsInTurretMode()());

					#pragma region COMMENTED CODE FOR LOGGING/DEBUGGING PURPOSES
					// TODO: Add logging for why not protected for easier troubleshooting
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

					#pragma endregion Comment out for release builds

					//build config array for comparing against
					bool configConditions[] =
					{
						true,	//Always requires passive = 0
						DinoPassiveProtection::RequiresPassiveFlee,
						DinoPassiveProtection::RequiresNoRider,
						DinoPassiveProtection::RequiresNoInventory,
						DinoPassiveProtection::RequireNoSaddle,
						DinoPassiveProtection::RequiresNotFollowing,
						DinoPassiveProtection::RequiresIgnoreWhistle,
						DinoPassiveProtection::RequiresNeutered,
						true,	//always require min health percentage
						DinoPassiveProtection::RequiresNoNearbyEnemyStructures,
						DinoPassiveProtection::RequiresNotTurretMode
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
						isNotInTurretMode
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
		}
		//not a dino
		return 999;
	}
}
