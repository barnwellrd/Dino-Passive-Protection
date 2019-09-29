#include <API/ARK/Ark.h>
#include "DPP.h"
#include "DPPConfig.h"
#include "DPPCommands.h"

#pragma comment(lib, "ArkApi.lib")

DECLARE_HOOK(APrimalDinoCharacter_TakeDamage, float, APrimalDinoCharacter*, float, FDamageEvent*, AController*, AActor*);

float Hook_APrimalDinoCharacter_TakeDamage(APrimalDinoCharacter* _this, float Damage, FDamageEvent* DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{	

	//_this != NULL
	if (_this)
	{

		//NEEDS ability to ignore environment damage like falling through the map and lava
		//Dino is a tribe dino
		if (_this->TargetingTeamField() > 10000)
		{

			//if EventInstigator != Null
			if (EventInstigator)
			{

				//allow owning team to damage dino
				if (_this->TargetingTeamField() == EventInstigator->TargetingTeamField())
				{
					return APrimalDinoCharacter_TakeDamage_original(_this, Damage, DamageEvent, EventInstigator, DamageCauser);
				}
			}

			//checks dinos protection
			// isProtected >= 0 means not protected
			// isProtected = -1 means protected
			int isProtected = DinoPassiveProtection::CheckDinoProtection(_this);

			//if dino is protected
			if (isProtected == -1)
			{
				//sends notification if event instagator is another player
				if (EventInstigator && !EventInstigator->IsLocalController() && EventInstigator->IsA(AShooterPlayerController::GetPrivateStaticClass()))
				{
					uint64 steam_id = ArkApi::GetApiUtils().GetSteamIdFromController(EventInstigator);
					AShooterPlayerController* player = ArkApi::GetApiUtils().FindPlayerFromSteamId(steam_id);
					ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr, *DinoPassiveProtection::PassiveProtectedDinoMessage);
				}

				//get dino health variable
				UPrimalCharacterStatusComponent* charStatus = _this->GetCharacterStatusComponent();
				float* currentHealth = charStatus->CurrentStatusValuesField()();

				//Cause 5 damage and then heal the damage done (This allows passive flee to work if enabled)
				//Damage taken will always be < 5 (instantly healed) and never exceed 5
				*currentHealth += APrimalDinoCharacter_TakeDamage_original(_this, 5, DamageEvent, EventInstigator, DamageCauser);

				//return 0 for no damage taken
				return 0;
			}
			else
			{
				return APrimalDinoCharacter_TakeDamage_original(_this, Damage, DamageEvent, EventInstigator, DamageCauser);
			}
		}
	}
	return APrimalDinoCharacter_TakeDamage_original(_this, Damage, DamageEvent, EventInstigator, DamageCauser);
}

void Load()
{
	Log::Get().Init("DinoPassiveProtection");

	InitConfig();
	InitCommands();

	ArkApi::GetHooks().SetHook("APrimalDinoCharacter.TakeDamage", &Hook_APrimalDinoCharacter_TakeDamage, &APrimalDinoCharacter_TakeDamage_original);
}

void Unload()
{
	RemoveCommands();
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
