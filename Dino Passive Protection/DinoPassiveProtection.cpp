#include <iostream>
#include <API/ARK/Ark.h>

#pragma comment(lib, "ArkApi.lib")

DECLARE_HOOK(APrimalDinoCharacter_TakeDamage, float, APrimalDinoCharacter*, float, FDamageEvent*, AController*, AActor*);

float Hook_APrimalDinoCharacter_TakeDamage(APrimalDinoCharacter* _this, float Damage, FDamageEvent* DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	if (_this)
	{
		if (_this->TargetingTeamField() > 10000)
		{
			FString descr;
			//_this->GetHumanReadableName(&descr);
			_this->DinoNameTagField().ToString(&descr);
			//not displaying??
			Log::GetLog()->warn("Dino name: {}", descr.ToString());

			int following = 0;
			if (_this->GetTamedFollowTarget())
			{
				following = 1;
				//Log::GetLog()->warn("Dino is following a target!");
			}
			Log::GetLog()->warn("Following a target: ", following);

			int AggressionField = _this->TamedAggressionLevelField();
			//0 is passive or passive flee
			Log::GetLog()->warn("Dino AggressionField: {}", AggressionField);


			bool passiveflee = _this->bPassiveFlee().Get();
			// true when on passive flee
			Log::GetLog()->warn("Dino is passive flee: {}", passiveflee);

			bool ignorewhistle = _this->bIgnoreAllWhistles().Get();
			//true when set to ignore
			Log::GetLog()->warn("Dino is ignore whistle: {}", ignorewhistle);

			bool neutered = _this->bNeutered().Get();
			//true when spayed or neutered
			Log::GetLog()->warn("Dino is neutered: {}", neutered);

			bool hasrider = _this->bHasRider().Get();
			Log::GetLog()->warn("Dino has rider: {}", hasrider);

			float dragWeight = _this->DragWeightField();
			//does not change with inventory
			Log::GetLog()->warn("Dino dragWeight: {}", dragWeight);

			float basedragWeight = _this->GetBaseDragWeight();
			Log::GetLog()->warn("Dino basedragWeight: {}", basedragWeight);


			float getdragWeight = _this->GetDragWeight(_this);;
			Log::GetLog()->warn("Dino getdragWeight: {}", getdragWeight);

			APrimalDinoCharacter* default = static_cast<APrimalDinoCharacter*>(_this->ClassField()->GetDefaultObject(true));
			float modDragWeight = default->DragWeightField();
			//does not change with inventory
			Log::GetLog()->warn("Dino Modded dragweight: {}", modDragWeight);


			//Log::GetLog()->warn("Dino name: {}", descr.ToString());


			//Log::GetLog()->warn("Dino name: {}", descr.ToString());

			//bIsCarryingCharacter()
			//bIsCarryingPassenger()
			//bIsInTurretMode()
			//bMovementInProgress()

			//needs weight (No saddle or inventory)  && full health && not near enemy structures
			if (AggressionField == 0 && passiveflee && ignorewhistle && !following && neutered && !hasrider)
			{
				return 0.01;
			}
		}
	}

	/*if (DamageEvent)
	{
		FString descr;
		DamageEvent->DamageTypeClassField().uClass->ClassConfigNameField().ToString(&descr);
		Log::GetLog()->warn("DamageEvent: {}", descr.ToString());
	}

	if (EventInstigator)
	{
		FString descr;
		EventInstigator->GetHumanReadableName(&descr);
		Log::GetLog()->warn("EventInstigator: {}", descr.ToString());
	}

	if (DamageCauser)
	{
		FString descr;
		DamageCauser->GetHumanReadableName(&descr);
		Log::GetLog()->warn("DamageCauser: {}", descr.ToString());
	}*/
	return APrimalDinoCharacter_TakeDamage_original(_this, Damage, DamageEvent, EventInstigator, DamageCauser);
}

void Load()
{
	Log::Get().Init("DinoPassiveProtection");

	ArkApi::GetHooks().SetHook("APrimalDinoCharacter.TakeDamage", &Hook_APrimalDinoCharacter_TakeDamage,
		&APrimalDinoCharacter_TakeDamage_original);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		Load();
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}