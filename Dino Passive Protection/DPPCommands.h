#pragma once

inline void GetProtectionStatus(AShooterPlayerController* player)
{
	if (!player || !player->PlayerStateField() || ArkApi::IApiUtils::IsPlayerDead(player))
		return;

	AActor* Actor = player->GetPlayerCharacter()->GetAimedActor(ECC_GameTraceChannel2, nullptr, 0.0, 0.0, nullptr, nullptr,
		false, false);

	//check if target is a dino
	if (Actor && Actor->IsA(APrimalDinoCharacter::GetPrivateStaticClass()))
	{
		APrimalDinoCharacter* Dino = static_cast<APrimalDinoCharacter*>(Actor);
		const int teamId = Dino->TargetingTeamField();

		//dino in same tribe
		if (Dino->TargetingTeamField() == player->TargetingTeamField())
		{
			//check if protected
			bool isProtected = DinoPassiveProtection::CheckDinoProtection(Dino);

			//is protected
			if (isProtected)
			{
				ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr,
					*DinoPassiveProtection::PassiveProtectedDinoMessage);
			}
			//not protected
			else
			{
				ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr,
					*DinoPassiveProtection::UnprotectedDinoMessage);
			}
		}
		//dino not in same tribe
		else
		{
			ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr,
				*DinoPassiveProtection::NotTribeSameMessage);
		}
	}
	//target not a dino
	else
	{
		ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr,
			*DinoPassiveProtection::NotADinoMessage);
	}
}

inline void ChatCommand(AShooterPlayerController* player, FString* message, int mode)
{
	TArray<FString> parsed;
	message->ParseIntoArray(parsed, L" ", true);

	if (parsed.IsValidIndex(1))
	{
		FString input = parsed[1];
		if (input.Compare("status") == 0)
		{
			GetProtectionStatus(player);
		}
		else
		{
			ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr,
				*DinoPassiveProtection::DPPInvalidCommand);
		}
	}
	else
	{
		ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr,
			*DinoPassiveProtection::DPPInvalidCommand);
	}
}

inline void InitChatCommands()
{
	FString cmd1 = DinoPassiveProtection::DPPChatCommandPrefix;
	cmd1 = cmd1.Append("dpp");
	ArkApi::GetCommands().AddChatCommand(cmd1, &ChatCommand);
}

inline void RemoveChatCommands()
{
	FString cmd1 = DinoPassiveProtection::DPPChatCommandPrefix;
	cmd1 = cmd1.Append("dpp");
	ArkApi::GetCommands().RemoveChatCommand(cmd1);
}

inline void ConsoleReloadConfig(APlayerController* player, FString* cmd, bool boolean)
{
	const auto shooter_controller = static_cast<AShooterPlayerController*>(player);

	//if not Admin
	if (!shooter_controller || !shooter_controller->PlayerStateField() || !shooter_controller->bIsAdmin().Get())
	{
		return;
	}
	RemoveChatCommands();
	InitConfig();
	InitChatCommands();
}

inline void RconReloadConfig(RCONClientConnection* rcon_connection, RCONPacket* rcon_packet, UWorld*)
{
	FString reply;
	
	try
	{
		RemoveChatCommands();
		InitConfig();
		InitChatCommands();
	}
	catch (const std::exception& error)
	{
		Log::GetLog()->error(error.what());

		reply = error.what();
		rcon_connection->SendMessageW(rcon_packet->Id, 0, &reply);
		return;
	}

	reply = "Reloaded config";
	rcon_connection->SendMessageW(rcon_packet->Id, 0, &reply);
}

inline void InitCommands()
{
	InitChatCommands();
	ArkApi::GetCommands().AddConsoleCommand("DPP.Reload", &ConsoleReloadConfig);
	ArkApi::GetCommands().AddRconCommand("DPP.Reload", &RconReloadConfig);
	ArkApi::GetCommands().AddChatCommand("DPP.Reload", &ConsoleReloadConfig);
}

inline void RemoveCommands()
{
	RemoveChatCommands();
	ArkApi::GetCommands().RemoveConsoleCommand("DPP.Reload");
	ArkApi::GetCommands().RemoveRconCommand("DPP.Reload");
}
