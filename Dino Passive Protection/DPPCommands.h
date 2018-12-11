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
	InitConfig();
}

inline void InitCommands()
{
	ArkApi::GetCommands().AddConsoleCommand("DPP.ReloadConfig", &ConsoleReloadConfig);
	ArkApi::GetCommands().AddRconCommand("DPP.ReloadConfig", &RconReloadConfig);
}

inline void RemoveCommands()
{
	ArkApi::GetCommands().RemoveConsoleCommand("DPP.ReloadConfig");
	ArkApi::GetCommands().RemoveRconCommand("DPP.ReloadConfig");
}