#pragma once

inline void ConsoleReloadConfig(APlayerController* player, FString* cmd, bool boolean)
{
	const auto shooter_controller = static_cast<AShooterPlayerController*>(player);

	//if not Admin
	if (!shooter_controller || !shooter_controller->PlayerStateField() || !shooter_controller->bIsAdmin().Get())
		return;
	InitConfig();
}

inline void RconReloadConfig(RCONClientConnection* rcon_connection, RCONPacket* rcon_packet, UWorld*)
{
	FString reply;
	
	try
	{
		InitConfig();
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
	ArkApi::GetCommands().AddConsoleCommand("DPP.Reload", &ConsoleReloadConfig);
	ArkApi::GetCommands().AddRconCommand("DPP.Reload", &RconReloadConfig);
}

inline void RemoveCommands()
{
	ArkApi::GetCommands().RemoveConsoleCommand("DPP.Reload");
	ArkApi::GetCommands().RemoveRconCommand("DPP.Reload");
}
