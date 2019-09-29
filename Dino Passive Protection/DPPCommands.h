#pragma once

// "!dpp status"
//command for retrieving aimed, owned dino's status
inline void GetProtectionStatus(AShooterPlayerController* player)
{
	if (!player || !player->PlayerStateField() || ArkApi::IApiUtils::IsPlayerDead(player))
		return;

	//get aimed target
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
			//checks dinos protection
			// isProtected >= 0 means not protected
			// isProtected = -1 means protected
			int isProtected = DinoPassiveProtection::CheckDinoProtection(Dino);

			//if dino is protected
			if (isProtected == -1)
			{
				ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr,
					*DinoPassiveProtection::PassiveProtectedDinoMessage);
			}
			else if (isProtected == 999)
			{
				//won't likely happen???
			}
			//not protected
			else
			{
				ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr,
					*DinoPassiveProtection::UnprotectedDinoMessage);
				ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr,
					*DinoPassiveProtection::MissingProtectionHintMessages[isProtected]);
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

// "!dpp getpath"
//Get the PLugin's blueprint path of target dino or structure
//May vary wildly from spawn Blueprint, so use this command's output for Whitelisting and Blacklisting
inline void GetTargetPath(AShooterPlayerController* player)
{
	//if player is dead or doesn't exist, break
	if (!player || !player->PlayerStateField() || ArkApi::IApiUtils::IsPlayerDead(player))
		return;

	//get aimed target
	AActor* Actor = player->GetPlayerCharacter()->GetAimedActor(ECC_GameTraceChannel2, nullptr, 0.0, 0.0, nullptr, nullptr,
		false, false);

	//check if target is a dino or structure
	if (Actor && (Actor->IsA(APrimalDinoCharacter::GetPrivateStaticClass()) || Actor->IsA(APrimalStructure::GetPrivateStaticClass())))
	{
		
		ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr,
			"{}", DinoPassiveProtection::GetBlueprint(Actor).ToString());
		Log::GetLog()->info("Blueprint Path From Command: {}", DinoPassiveProtection::GetBlueprint(Actor).ToString());
	}
	//target not a dino or structure
	else
	{
		ArkApi::GetApiUtils().SendNotification(player, DinoPassiveProtection::MessageColor, DinoPassiveProtection::MessageTextSize, DinoPassiveProtection::MessageDisplayDelay, nullptr,
			*DinoPassiveProtection::NotADinoOrStructureMessage);
	}
}

//Function that filters chat commands
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
		else if (input.Compare("getpath") == 0)
		{
			GetTargetPath(player);
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

//initialize chat commands
inline void InitChatCommands()
{
	FString cmd1 = DinoPassiveProtection::DPPChatCommandPrefix;
	ArkApi::GetCommands().AddChatCommand(cmd1, &ChatCommand);
}

//remove chat commands
inline void RemoveChatCommands()
{
	FString cmd1 = DinoPassiveProtection::DPPChatCommandPrefix;
	cmd1 = cmd1.Append("dpp");
	ArkApi::GetCommands().RemoveChatCommand(cmd1);
}

//Console command for reloading the config
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

//Rcon reload config command
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

//initialize commands
inline void InitCommands()
{
	InitChatCommands();

	ArkApi::GetCommands().AddConsoleCommand("DPP.Reload", &ConsoleReloadConfig);
	ArkApi::GetCommands().AddRconCommand("DPP.Reload", &RconReloadConfig);
}

//remove commands
inline void RemoveCommands()
{
	RemoveChatCommands();

	ArkApi::GetCommands().RemoveConsoleCommand("DPP.Reload");
	ArkApi::GetCommands().RemoveRconCommand("DPP.Reload");
}
