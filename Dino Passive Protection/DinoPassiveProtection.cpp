#include <API/ARK/Ark.h>
#include "DPP.h"
#include "DPPConfig.h"
//#include "NewPlayerProtectionHooks.h"
//#include "NewPlayerProtectionCommands.h"

#pragma comment(lib, "ArkApi.lib")

DECLARE_HOOK(APrimalDinoCharacter_TakeDamage, float, APrimalDinoCharacter*, float, FDamageEvent*, AController*, AActor*);

float Hook_APrimalDinoCharacter_TakeDamage(APrimalDinoCharacter* _this, float Damage, FDamageEvent* DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	//_this != NULL
	if (_this)
	{
		//Dino is a tribe dino
		if (_this->TargetingTeamField() > 10000)
		{
			UPrimalCharacterStatusComponent* charStatus = _this->GetCharacterStatusComponent();

			FString DinoName;
			bool isNotFollowing;
			bool isPassiveAggressive;
			bool isPassiveFlee;
			bool isIgnoringWhistles;
			bool isNeutered;
			bool hasNoRider;
			bool hasNoInventory;
			bool isNotNearEnemyStructures;
			bool isHealthAboveMin;

			//get and print Dino Name
			_this->GetDinoDescriptiveName(&DinoName);
			int32 Index = 0;
			if (DinoName.FindLastChar('-', Index))
			{
				DinoName = DinoName.RightChop(Index + 2);
				if (DinoName.FindLastChar('(', Index)) DinoName.RemoveAt(Index);
				if (DinoName.FindLastChar(')', Index)) DinoName.RemoveAt(Index);
			}

			//check if dino is following something
			if (_this->GetTamedFollowTarget())
			{
				isNotFollowing = false;
			}
			else
			{
				isNotFollowing = true;
			}

			//0 is passive or passive flee
			if (_this->TamedAggressionLevelField() == 0)
			{
				isPassiveAggressive = true;
			}
			else
			{
				isPassiveAggressive = false;
			}

			//get passive flee status
			isPassiveFlee = _this->bPassiveFlee()();

			//get ingoring whistle status
			isIgnoringWhistles = _this->bIgnoreAllWhistles()();

			//get neutered status
			isNeutered = _this->bNeutered()();

			//get rider status
			hasNoRider = !(_this->bHasRider()());

			//get inventory weight and check if it's empty
			float* currentWeight = charStatus->CurrentStatusValuesField()() + 7;
			if (*currentWeight > 0)
			{
				hasNoInventory = true;
			}
			else
			{
				hasNoInventory = false;
			}

			//check for enemy structures nearby		
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
						if (FVector::Distance(_this->RootComponentField()->RelativeLocationField(), Structure->RootComponentField()->RelativeLocationField()) <= DinoPassiveProtection::MinimumEnemyStructureDistance)
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
			Log::GetLog()->warn("Dino name: {}", DinoName.ToString());
			Log::GetLog()->warn("Not Following a target: ", isNotFollowing);
			Log::GetLog()->warn("Dino is Passive: {}", isPassiveAggressive);
			Log::GetLog()->warn("Dino is passive flee: {}", isPassiveFlee);
			Log::GetLog()->warn("Dino is ignore whistle: {}", isIgnoringWhistles);
			Log::GetLog()->warn("Dino is neutered: {}", isNeutered);
			Log::GetLog()->warn("Dino has rider: {}", hasNoRider);
			Log::GetLog()->warn("Dino has no inventory: {}", hasNoInventory);
			Log::GetLog()->warn("Dino not near enemy Structures: {}", isNotNearEnemyStructures);
			Log::GetLog()->warn("Dino is above min health: {}", isHealthAboveMin);

			if (EventInstigator && !EventInstigator->IsLocalController() && EventInstigator->IsA(AShooterPlayerController::StaticClass()))
			{
				uint64 steam_id = ArkApi::GetApiUtils().GetSteamIdFromController(EventInstigator);
				AShooterPlayerController* player = ArkApi::GetApiUtils().FindPlayerFromSteamId(steam_id);
				ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr, *DinoPassiveProtection::PassiveProtectedDinoTakingDamageMessage);
			}

			//build config array
			bool configConditions[] = {
				DinoPassiveProtection::RequiresNotFollowing,
				DinoPassiveProtection::RequiresPassiveFlee,
				DinoPassiveProtection::RequiresIgnoreWhistle,
				DinoPassiveProtection::RequiresNeutered,
				DinoPassiveProtection::RequiresNoRider,
				DinoPassiveProtection::RequiresNoInventory,
				DinoPassiveProtection::RequiresNoNearbyEnemyStructures,
				true
			};

			//build hook vars array
			bool hookActuals[] = {
				 isNotFollowing,
				 (isPassiveAggressive && isPassiveFlee),
				 isIgnoringWhistles,
				 isNeutered,
				 hasNoRider,
				 hasNoInventory,
				 isNotNearEnemyStructures,
				 isHealthAboveMin
			};

			for (int i = 0; i < 8 ; ++i)
			{
				if (configConditions[i] && !hookActuals[i])
					return APrimalDinoCharacter_TakeDamage_original(_this, Damage, DamageEvent, EventInstigator, DamageCauser);
			}
			
			*currentHealth += APrimalDinoCharacter_TakeDamage_original(_this, 5, DamageEvent, EventInstigator, DamageCauser);;
			return 0;
		}
	}
	return APrimalDinoCharacter_TakeDamage_original(_this, Damage, DamageEvent, EventInstigator, DamageCauser);
}

void Load()
{
	Log::Get().Init("DinoPassiveProtection");

	ArkApi::GetHooks().SetHook("APrimalDinoCharacter.TakeDamage", &Hook_APrimalDinoCharacter_TakeDamage,
		&APrimalDinoCharacter_TakeDamage_original);
}

void Unload()
{

	ArkApi::GetHooks().DisableHook("APrimalDinoCharacter.TakeDamage", &Hook_APrimalDinoCharacter_TakeDamage);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Load();
		break;
	case DLL_PROCESS_DETACH:
		Unload();
		break;
	}
	return TRUE;
}