#pragma once

#include <Logger/Logger.h>
#include <API/UE/Containers/FString.h>
#include "json.hpp"

namespace DinoPassiveProtection
{
	bool EnableConsoleDebugging;

	//Protection Parameters from config
	bool RequiresPassive;
	bool RequiresPassiveFlee;
	bool RequiresNoRider;
	bool RequiresNoInventory;
	//bool RequiresNotFollowing;
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

	nlohmann::json config, TempConfig;

	bool CheckDinoProtection(APrimalDinoCharacter* dino)
	{
		//_this != NULL
		if (dino)
		{
			//NEEDS ability to ignore environment damage like falling through the map and lava
			//Dino is a tribe dino
			if (dino->TargetingTeamField() > 10000)
			{
				UPrimalCharacterStatusComponent* charStatus = dino->GetCharacterStatusComponent();

				//var for dino name for debugging (and one day Logging)
				FString DinoName;

				//variable declarations for potential protection parameters
				bool isPassiveAggressive;
				bool isPassiveFlee;
				bool hasNoRider;
				bool hasNoInventory;
				//bool isNotFollowing;
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

				//get inventory weight and check if it's empty
				float* currentWeight = charStatus->CurrentStatusValuesField()() + 7;
				if (*currentWeight > 0)
				{
					hasNoInventory = false;
				}
				else
				{
					hasNoInventory = true;
				}

				//check if dino is following something
				//AActor *act = dino->GetTamedFollowTarget();
				////log line to check distance to structure
				//if (DinoPassiveProtection::EnableConsoleDebugging)
				//{
				//	//FString * actName = act->GetFullName();
				//	//Log::GetLog()->warn(actName->ToString());
				//}

				//if (act)
				//{
				//	Log::GetLog()->warn("not following if");
				//	isNotFollowing = false;
				//}
				//else
				//{
				//	Log::GetLog()->warn("not following else");
				//	isNotFollowing = true;
				//}

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
						//log line to check distance to structure
						if (DinoPassiveProtection::EnableConsoleDebugging)
						{
							Log::GetLog()->warn(FVector::Distance(dino->RootComponentField()->RelativeLocationField(), actor->RootComponentField()->RelativeLocationField()));
						}

						isNotNearEnemyStructures = false;
						break;
					}
					else
					{
						isNotNearEnemyStructures = true;
					}
				}

				//get Turret mode status
				isNotInTurretMode = !(dino->bIsInTurretMode()());

				



				#pragma region COMMENTED CODE FOR LOGGING/DEBUGGING PURPOSES

				if (DinoPassiveProtection::EnableConsoleDebugging)
				{
					//LOGGING

					Log::GetLog()->warn("## Beginning of debug ##");
					Log::GetLog()->warn("############################");

					/*
					if (EventInstigator)
					{
						FString EIName;
						EventInstigator->NameField().ToString(&EIName);
						Log::GetLog()->warn("EventInstigator name: {}", EIName.ToString());

					}
					if (DamageCauser)
					{
						FString DCName;
						DamageCauser->NameField().ToString(&DCName);
						Log::GetLog()->warn("DamageCauser name: {}", DCName.ToString());

					}
					*/

					Log::GetLog()->warn("Dino name: {}", DinoName.ToString());

					Log::GetLog()->warn("Dino is Passive: {}", isPassiveAggressive);
					Log::GetLog()->warn("Dino is Passive flee: {}", isPassiveFlee);
					Log::GetLog()->warn("Dino has No rider: {}", hasNoRider);
					Log::GetLog()->warn("Dino has No inventory: {}", hasNoInventory);
					//Log::GetLog()->warn("Dino isn't Following a target: ", isNotFollowing);
					Log::GetLog()->warn("Dino is Ignoring whistle: {}", isIgnoringWhistles);
					Log::GetLog()->warn("Dino is Neutered: {}", isNeutered);
					Log::GetLog()->warn("Dino is Above min health: {}", isHealthAboveMin);
					Log::GetLog()->warn("Dino isn't Near enemy Structures: {}", isNotNearEnemyStructures);
					Log::GetLog()->warn("Dino isn't in Turret mode: {}", isNotInTurretMode);
					Log::GetLog()->warn("Dino is a Baby: {}", isBaby);


					Log::GetLog()->warn("############################");
					Log::GetLog()->warn("## End of debug ##");	
				}

				#pragma endregion Comment out for release builds


				//build config array for comparing against
				bool configConditions[] = 
				{
					true,	//Always requires passive = 0
					DinoPassiveProtection::RequiresPassiveFlee,
					DinoPassiveProtection::RequiresNoRider,
					DinoPassiveProtection::RequiresNoInventory,
					//DinoPassiveProtection::RequiresNotFollowing,
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
					//isNotFollowing,
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
						return false;
					}
				}				
				return true;
			}
		}
		return false;
	}

#pragma region OLD CODE FOR ACHIVE PURPOSES

	//Check Arkhomes for better structure detection functions
	//check for enemy structures nearby		
	/*
	TArray<AActor*> AllStructures;
	UGameplayStatics::GetAllActorsOfClass(reinterpret_cast<UObject*>
		(ArkApi::GetApiUtils().GetWorld()), APrimalStructure::GetPrivateStaticClass(), &AllStructures);

	for (AActor* StructureActor : AllStructures)
	{
		if (StructureActor)
		{
			if (StructureActor->TargetingTeamField() > 0 && _this->TargetingTeamField() != StructureActor->TargetingTeamField())
			{
				APrimalStructure* Structure = static_cast<APrimalStructure*>(StructureActor);
				Log::GetLog()->warn(FVector::Distance(_this->RootComponentField()->RelativeLocationField(), Structure->RootComponentField()->RelativeLocationField()));
				if (FVector::Distance(_this->RootComponentField()->RelativeLocationField(), Structure->RootComponentField()->RelativeLocationField()) <= (DinoPassiveProtection::MinimumEnemyStructureDistance * 300))
				{
					isNotNearEnemyStructures = false;
					break;
				}
				else
				{
					isNotNearEnemyStructures = true;
				}
			}
		}
	}
	*/

#pragma endregion OLD CODE FOR ACHIVE PURPOSES
	
}
