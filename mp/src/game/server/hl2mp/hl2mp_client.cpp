//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
/*

===== tf_client.cpp ========================================================

HL2 client/server game specific stuff

*/

#include "cbase.h"
#include "hl2mp_player.h"
#include "hl2mp_gamerules.h"
#include "gamerules.h"
#include "teamplay_gamerules.h"
#include "entitylist.h"
#include "physics.h"
#include "game.h"
#include "player_resource.h"
#include "engine/IEngineSound.h"
#include "team.h"
#include "viewport_panel_names.h"

#include "tier0/vprof.h"

#ifdef SecobMod__SAVERESTORE
#include "filesystem.h"
#endif //SecobMod__SAVERESTORE

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void Host_Say(edict_t *pEdict, bool teamonly);

ConVar sv_motd_unload_on_dismissal("sv_motd_unload_on_dismissal", "0", 0, "If enabled, the MOTD contents will be unloaded when the player closes the MOTD.");

extern CBaseEntity*	FindPickerEntityClass(CBasePlayer *pPlayer, char *classname);
extern bool			g_fGameOver;

#ifdef SecobMod__USE_PLAYERCLASSES
void SSPlayerClassesBGCheck(CHL2MP_Player *pPlayer)
{
	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();
	UserMessageBegin(user, "SSPlayerClassesBGCheck");
	MessageEnd();
}

void ShowSSPlayerClasses(CHL2MP_Player *pPlayer)
{
	CSingleUserRecipientFilter user(pPlayer);
	user.MakeReliable();
	UserMessageBegin(user, "ShowSSPlayerClasses");
	MessageEnd();
}

CON_COMMAND(ss_classes_default, "The current map is a background level - do default spawn")
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer(UTIL_GetCommandClient());
	if (pPlayer != NULL)
	{
		CSingleUserRecipientFilter user(pPlayer);
		user.MakeReliable();
		pPlayer->InitialSpawn();
		pPlayer->Spawn();
		pPlayer->m_Local.m_iHideHUD |= HIDEHUD_ALL;
		pPlayer->RemoveAllItems(true);

		//SecobMod__Information  These are now commented out because for your own mod you'll have to use the black room spawn method anyway.
		// That is for your own maps, you create a seperate room with fully black textures, no light and a single info_player_start.
		//You may end up needing to uncomment it if you don't use playerclasses, but you'll figure that out for yourself when you cant see anything but your HUD.
		//color32 black = {0,0,0,255};
		//UTIL_ScreenFade( pPlayer, black, 0.0f, 0.0f, FFADE_IN|FFADE_PURGE );
	}
}
#endif //SecobMod__USE_PLAYERCLASSES

void FinishClientPutInServer(CHL2MP_Player *pPlayer)
{
#ifdef SecobMod__USE_PLAYERCLASSES
	pPlayer->InitialSpawn();
	pPlayer->Spawn();
	pPlayer->RemoveAllItems(true);
	SSPlayerClassesBGCheck(pPlayer);
#else
	pPlayer->InitialSpawn();
	pPlayer->Spawn();
#endif //SecobMod__USE_PLAYERCLASSES		
	char sName[128];
	Q_strncpy(sName, pPlayer->GetPlayerName(), sizeof(sName));

	// First parse the name and remove any %'s
	for (char *pApersand = sName; pApersand != NULL && *pApersand != 0; pApersand++)
	{
		// Replace it with a space
		if (*pApersand == '%')
			*pApersand = ' ';
	}

	// notify other clients of player joining the game
	UTIL_ClientPrintAll(HUD_PRINTNOTIFY, "#Game_connected", sName[0] != 0 ? sName : "<unconnected>");

	if (HL2MPRules()->IsTeamplay() == true)
	{
		ClientPrint(pPlayer, HUD_PRINTTALK, "You are on team %s1\n", pPlayer->GetTeam()->GetName());
	}

	const ConVar *hostname = cvar->FindVar("hostname");
	const char *title = (hostname) ? hostname->GetString() : "MESSAGE OF THE DAY";

	KeyValues *data = new KeyValues("data");
	data->SetString("title", title);		// info panel title
	data->SetString("type", "1");			// show userdata from stringtable entry
	data->SetString("msg", "motd");		// use this stringtable entry
	data->SetBool("unload", sv_motd_unload_on_dismissal.GetBool());

	pPlayer->ShowViewPortPanel(PANEL_INFO, true, data);


#ifndef SecobMod__SAVERESTORE
#ifdef SecobMod__USE_PLAYERCLASSES
	pPlayer->ShowViewPortPanel(PANEL_CLASS, true, NULL);
#endif //SecobMod__USE_PLAYERCLASSES	
#endif //SecobMod__SAVERESTORE


#ifdef SecobMod__SAVERESTORE 

	//if (Transitioned)
	//{
	//SecobMod
	KeyValues *pkvTransitionRestoreFile = new KeyValues("transition.cfg");

	//Msg ("Transition path is: %s !!!!!\n",TransitionPath);

	if (pkvTransitionRestoreFile->LoadFromFile(filesystem, "transition.cfg"))
	{
		while (pkvTransitionRestoreFile)
		{
			const char *pszSteamID = pkvTransitionRestoreFile->GetName(); //Gets our header, which we use the players SteamID for.
			const char *PlayerSteamID = engine->GetPlayerNetworkIDString(pPlayer->edict()); //Finds the current players Steam ID.

			Msg("In-File SteamID is %s.\n", pszSteamID);
			Msg("In-Game SteamID is %s.\n", PlayerSteamID);

			if (Q_strcmp(PlayerSteamID, pszSteamID) != 0)
			{
				if (pkvTransitionRestoreFile == NULL)
				{
					break;
				}
				//SecobMod__Information  No SteamID found for this person, maybe they're new to the game or have "STEAM_ID_PENDING". Show them the class menu and break the loop.
				pPlayer->ShowViewPortPanel(PANEL_CLASS, true, NULL);
				break;
			}

			Msg("SteamID Match Found!");

#ifdef SecobMod__USE_PLAYERCLASSES
			//Class.
			KeyValues *pkvCurrentClass = pkvTransitionRestoreFile->FindKey("CurrentClass");
#endif //SecobMod__USE_PLAYERCLASSES
			//Health
			KeyValues *pkvHealth = pkvTransitionRestoreFile->FindKey("Health");

			//Armour
			KeyValues *pkvArmour = pkvTransitionRestoreFile->FindKey("Armour");

			//CurrentHeldWeapon
			KeyValues *pkvActiveWep = pkvTransitionRestoreFile->FindKey("ActiveWeapon");

			//Weapon_0.
			KeyValues *pkvWeapon_0 = pkvTransitionRestoreFile->FindKey("Weapon_0");
			KeyValues *pkvWeapon_0_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_0_PriClip");
			KeyValues *pkvWeapon_0_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_0_SecClip");
			KeyValues *pkvWeapon_0_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_0_PriClipAmmo");
			KeyValues *pkvWeapon_0_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_0_SecClipAmmo");
			KeyValues *pkvWeapon_0_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_0_PriClipAmmoLeft");
			KeyValues *pkvWeapon_0_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_0_SecClipAmmoLeft");
			//Weapon_1.
			KeyValues *pkvWeapon_1 = pkvTransitionRestoreFile->FindKey("Weapon_1");
			KeyValues *pkvWeapon_1_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_1_PriClip");
			KeyValues *pkvWeapon_1_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_1_SecClip");
			KeyValues *pkvWeapon_1_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_1_PriClipAmmo");
			KeyValues *pkvWeapon_1_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_1_SecClipAmmo");
			KeyValues *pkvWeapon_1_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_1_PriClipAmmoLeft");
			KeyValues *pkvWeapon_1_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_1_SecClipAmmoLeft");
			//Weapon_2.
			KeyValues *pkvWeapon_2 = pkvTransitionRestoreFile->FindKey("Weapon_2");
			KeyValues *pkvWeapon_2_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_2_PriClip");
			KeyValues *pkvWeapon_2_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_2_SecClip");
			KeyValues *pkvWeapon_2_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_2_PriClipAmmo");
			KeyValues *pkvWeapon_2_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_2_SecClipAmmo");
			KeyValues *pkvWeapon_2_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_2_PriClipAmmoLeft");
			KeyValues *pkvWeapon_2_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_2_SecClipAmmoLeft");
			//Weapon_3.
			KeyValues *pkvWeapon_3 = pkvTransitionRestoreFile->FindKey("Weapon_3");
			KeyValues *pkvWeapon_3_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_3_PriClip");
			KeyValues *pkvWeapon_3_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_3_SecClip");
			KeyValues *pkvWeapon_3_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_3_PriClipAmmo");
			KeyValues *pkvWeapon_3_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_3_SecClipAmmo");
			KeyValues *pkvWeapon_3_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_3_PriClipAmmoLeft");
			KeyValues *pkvWeapon_3_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_3_SecClipAmmoLeft");
			//Weapon_4.
			KeyValues *pkvWeapon_4 = pkvTransitionRestoreFile->FindKey("Weapon_4");
			KeyValues *pkvWeapon_4_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_4_PriClip");
			KeyValues *pkvWeapon_4_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_4_SecClip");
			KeyValues *pkvWeapon_4_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_4_PriClipAmmo");
			KeyValues *pkvWeapon_4_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_4_SecClipAmmo");
			KeyValues *pkvWeapon_4_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_4_PriClipAmmoLeft");
			KeyValues *pkvWeapon_4_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_4_SecClipAmmoLeft");
			//Weapon_5.
			KeyValues *pkvWeapon_5 = pkvTransitionRestoreFile->FindKey("Weapon_5");
			KeyValues *pkvWeapon_5_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_5_PriClip");
			KeyValues *pkvWeapon_5_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_5_SecClip");
			KeyValues *pkvWeapon_5_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_5_PriClipAmmo");
			KeyValues *pkvWeapon_5_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_5_SecClipAmmo");
			KeyValues *pkvWeapon_5_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_5_PriClipAmmoLeft");
			KeyValues *pkvWeapon_5_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_5_SecClipAmmoLeft");
			//Weapon_6.
			KeyValues *pkvWeapon_6 = pkvTransitionRestoreFile->FindKey("Weapon_6");
			KeyValues *pkvWeapon_6_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_6_PriClip");
			KeyValues *pkvWeapon_6_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_6_SecClip");
			KeyValues *pkvWeapon_6_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_6_PriClipAmmo");
			KeyValues *pkvWeapon_6_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_6_SecClipAmmo");
			KeyValues *pkvWeapon_6_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_6_PriClipAmmoLeft");
			KeyValues *pkvWeapon_6_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_6_SecClipAmmoLeft");
			//Weapon_7.
			KeyValues *pkvWeapon_7 = pkvTransitionRestoreFile->FindKey("Weapon_7");
			KeyValues *pkvWeapon_7_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_7_PriClip");
			KeyValues *pkvWeapon_7_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_7_SecClip");
			KeyValues *pkvWeapon_7_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_7_PriClipAmmo");
			KeyValues *pkvWeapon_7_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_7_SecClipAmmo");
			KeyValues *pkvWeapon_7_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_7_PriClipAmmoLeft");
			KeyValues *pkvWeapon_7_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_7_SecClipAmmoLeft");
			//Weapon_8.
			KeyValues *pkvWeapon_8 = pkvTransitionRestoreFile->FindKey("Weapon_8");
			KeyValues *pkvWeapon_8_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_8_PriClip");
			KeyValues *pkvWeapon_8_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_8_SecClip");
			KeyValues *pkvWeapon_8_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_8_PriClipAmmo");
			KeyValues *pkvWeapon_8_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_8_SecClipAmmo");
			KeyValues *pkvWeapon_8_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_8_PriClipAmmoLeft");
			KeyValues *pkvWeapon_8_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_8_SecClipAmmoLeft");
			//Weapon_9.
			KeyValues *pkvWeapon_9 = pkvTransitionRestoreFile->FindKey("Weapon_9");
			KeyValues *pkvWeapon_9_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_9_PriClip");
			KeyValues *pkvWeapon_9_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_9_SecClip");
			KeyValues *pkvWeapon_9_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_9_PriClipAmmo");
			KeyValues *pkvWeapon_9_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_9_SecClipAmmo");
			KeyValues *pkvWeapon_9_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_9_PriClipAmmoLeft");
			KeyValues *pkvWeapon_9_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_9_SecClipAmmoLeft");
			//Weapon_10.
			KeyValues *pkvWeapon_10 = pkvTransitionRestoreFile->FindKey("Weapon_10");
			KeyValues *pkvWeapon_10_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_10_PriClip");
			KeyValues *pkvWeapon_10_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_10_SecClip");
			KeyValues *pkvWeapon_10_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_10_PriClipAmmo");
			KeyValues *pkvWeapon_10_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_10_SecClipAmmo");
			KeyValues *pkvWeapon_10_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_10_PriClipAmmoLeft");
			KeyValues *pkvWeapon_10_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_10_SecClipAmmoLeft");
			//Weapon_11.
			KeyValues *pkvWeapon_11 = pkvTransitionRestoreFile->FindKey("Weapon_11");
			KeyValues *pkvWeapon_11_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_11_PriClip");
			KeyValues *pkvWeapon_11_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_11_SecClip");
			KeyValues *pkvWeapon_11_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_11_PriClipAmmo");
			KeyValues *pkvWeapon_11_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_11_SecClipAmmo");
			KeyValues *pkvWeapon_11_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_11_PriClipAmmoLeft");
			KeyValues *pkvWeapon_11_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_11_SecClipAmmoLeft");
			//Weapon_12.
			KeyValues *pkvWeapon_12 = pkvTransitionRestoreFile->FindKey("Weapon_12");
			KeyValues *pkvWeapon_12_PriClip = pkvTransitionRestoreFile->FindKey("Weapon_12_PriClip");
			KeyValues *pkvWeapon_12_SecClip = pkvTransitionRestoreFile->FindKey("Weapon_12_SecClip");
			KeyValues *pkvWeapon_12_PriClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_12_PriClipAmmo");
			KeyValues *pkvWeapon_12_SecClipAmmo = pkvTransitionRestoreFile->FindKey("Weapon_12_SecClipAmmo");
			KeyValues *pkvWeapon_12_PriClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_12_PriClipAmmoLeft");
			KeyValues *pkvWeapon_12_SecClipAmmoLeft = pkvTransitionRestoreFile->FindKey("Weapon_12_SecClipAmmoLeft");


			//=====================================================================	
			if (pszSteamID)
			{

				//Set ints for the class,health and armour.
				#ifdef SecobMod__USE_PLAYERCLASSES
				int PlayerClassValue = pkvCurrentClass->GetInt();
				#endif

				int PlayerHealthValue = pkvHealth->GetInt();
				int PlayerArmourValue = pkvArmour->GetInt();

				//Current Active Weapon
				const char *pkvActiveWep_Value = pkvActiveWep->GetString();


				//============================================================================================
				#ifdef SecobMod__USE_PLAYERCLASSES
				if (PlayerClassValue == 1)
				{
					pPlayer->m_iCurrentClass = 1;
					pPlayer->m_iClientClass = pPlayer->m_iCurrentClass;
					pPlayer->ForceHUDReload(pPlayer);
					Msg("Respawning...\n");
					pPlayer->PlayerCanChangeClass = false;
					pPlayer->RemoveAllItems(true);
					pPlayer->m_iHealth = PlayerHealthValue;
					pPlayer->m_iMaxHealth = 125;
					pPlayer->SetArmorValue(PlayerArmourValue);
					pPlayer->SetMaxArmorValue(0);
					pPlayer->CBasePlayer::SetWalkSpeed(50);
					pPlayer->CBasePlayer::SetNormSpeed(190);
					pPlayer->CBasePlayer::SetSprintSpeed(640);
					pPlayer->CBasePlayer::SetJumpHeight(200.0);

					//SecobMod__Information This allows you to use filtering while mapping. Such as only a trigger one class may actually trigger. Thanks to Alters for providing this fix.
					pPlayer->CBasePlayer::KeyValue("targetname", "Assaulter");
					pPlayer->SetModel("models/sdk/Humans/Group03/male_06_sdk.mdl");

					//SecobMod__Information Due to the way our player classes now work, the first spawn of any class has to teleport to their specific player start.
					CBaseEntity *pEntity = NULL;
					Vector pEntityOrigin;
					pEntity = gEntList.FindEntityByClassnameNearest("info_player_assaulter", pEntityOrigin, 0);
					if (pEntity != NULL)
					{
						pEntityOrigin = pEntity->GetAbsOrigin();
						pPlayer->SetAbsOrigin(pEntityOrigin);
					}
					//PlayerClass bug fix - if the below lines are removed the player is stuck with 0 movement, once they're able to move again we can remove the suit as required.
					pPlayer->EquipSuit();
					pPlayer->StartSprinting();
					pPlayer->StopSprinting();
				}
				else if (PlayerClassValue == 2)
				{
					pPlayer->m_iCurrentClass = 2;
					pPlayer->m_iClientClass = pPlayer->m_iCurrentClass;
					pPlayer->ForceHUDReload(pPlayer);
					Msg("Respawning...\n");
					pPlayer->PlayerCanChangeClass = false;
					pPlayer->RemoveAllItems(true);
					pPlayer->m_iHealth = PlayerHealthValue;
					pPlayer->m_iMaxHealth = 100;
					pPlayer->SetArmorValue(PlayerArmourValue);
					pPlayer->SetMaxArmorValue(0);
					pPlayer->CBasePlayer::SetWalkSpeed(150);
					pPlayer->CBasePlayer::SetNormSpeed(190);
					pPlayer->CBasePlayer::SetSprintSpeed(500);
					pPlayer->CBasePlayer::SetJumpHeight(150.0);

					//SecobMod__Information This allows you to use filtering while mapping. Such as only a trigger one class may actually trigger. Thanks to Alters for providing this fix.				
					pPlayer->CBasePlayer::KeyValue("targetname", "Supporter");
					pPlayer->SetModel("models/sdk/Humans/Group03/l7h_rebel.mdl");

					//SecobMod__Information Due to the way our player classes now work, the first spawn of any class has to teleport to their specific player start.
					CBaseEntity *pEntity = NULL;
					Vector pEntityOrigin;
					pEntity = gEntList.FindEntityByClassnameNearest("info_player_supporter", pEntityOrigin, 0);
					if (pEntity != NULL)
					{
						pEntityOrigin = pEntity->GetAbsOrigin();
						pPlayer->SetAbsOrigin(pEntityOrigin);
					}
					//PlayerClass bug fix - if the below lines are removed the player is stuck with 0 movement, once they're able to move again we can remove the suit as required.
					pPlayer->EquipSuit();
					pPlayer->StartSprinting();
					pPlayer->StopSprinting();
				}
				else if (PlayerClassValue == 3)
				{
					pPlayer->m_iCurrentClass = 3;
					pPlayer->m_iClientClass = pPlayer->m_iCurrentClass;
					pPlayer->ForceHUDReload(pPlayer);
					pPlayer->PlayerCanChangeClass = false;
					pPlayer->RemoveAllItems(true);
					pPlayer->m_iHealth = PlayerHealthValue;
					pPlayer->m_iMaxHealth = 80;
					pPlayer->SetArmorValue(PlayerArmourValue);
					pPlayer->SetMaxArmorValue(0);
					pPlayer->CBasePlayer::SetWalkSpeed(150);
					pPlayer->CBasePlayer::SetNormSpeed(190);
					pPlayer->CBasePlayer::SetSprintSpeed(320);
					pPlayer->CBasePlayer::SetJumpHeight(100.0);

					//SecobMod__Information This allows you to use filtering while mapping. Such as only a trigger one class may actually trigger. Thanks to Alters for providing this fix.				
					pPlayer->CBasePlayer::KeyValue("targetname", "Medic");
					pPlayer->SetModel("models/sdk/Humans/Group03/male_05.mdl");

					//SecobMod__Information Due to the way our player classes now work, the first spawn of any class has to teleport to their specific player start.
					CBaseEntity *pEntity = NULL;
					Vector pEntityOrigin;
					pEntity = gEntList.FindEntityByClassnameNearest("info_player_medic", pEntityOrigin, 0);
					if (pEntity != NULL)
					{
						pEntityOrigin = pEntity->GetAbsOrigin();
						pPlayer->SetAbsOrigin(pEntityOrigin);
					}
					//PlayerClass bug fix - if the below lines are removed the player is stuck with 0 movement, once they're able to move again we can remove the suit as required.
					pPlayer->EquipSuit();
					pPlayer->StartSprinting();
					pPlayer->StopSprinting();
					pPlayer->EquipSuit(false);
				}
				else if (PlayerClassValue == 4)
				{
					pPlayer->m_iCurrentClass = 4;
					pPlayer->m_iClientClass = pPlayer->m_iCurrentClass;
					pPlayer->ForceHUDReload(pPlayer);
					pPlayer->PlayerCanChangeClass = false;
					pPlayer->RemoveAllItems(true);
					pPlayer->m_iHealth = PlayerHealthValue;
					pPlayer->m_iMaxHealth = 150;
					pPlayer->SetArmorValue(PlayerArmourValue);
					pPlayer->SetMaxArmorValue(200);
					pPlayer->CBasePlayer::SetWalkSpeed(150);
					pPlayer->CBasePlayer::SetNormSpeed(190);
					pPlayer->CBasePlayer::SetSprintSpeed(320);
					pPlayer->CBasePlayer::SetJumpHeight(40.0);

					//SecobMod__Information  This allows you to use filtering while mapping. Such as only a trigger one class may actually trigger. Thanks to Alters for providing this fix.				
					pPlayer->CBasePlayer::KeyValue("targetname", "Heavy");
					pPlayer->SetModel("models/sdk/Humans/Group03/police_05.mdl");

					//SecobMod__Information  Due to the way our player classes now work, the first spawn of any class has to teleport to their specific player start.
					CBaseEntity *pEntity = NULL;
					Vector pEntityOrigin;
					pEntity = gEntList.FindEntityByClassnameNearest("info_player_heavy", pEntityOrigin, 0);
					if (pEntity != NULL)
					{
						pEntityOrigin = pEntity->GetAbsOrigin();
						pPlayer->SetAbsOrigin(pEntityOrigin);
					}
					//PlayerClass bug fix - if the below lines are removed the player is stuck with 0 movement, once they're able to move again we can remove the suit as required.
					pPlayer->EquipSuit();
					pPlayer->StartSprinting();
					pPlayer->StopSprinting();
				}
				#endif //SecobMod__USE_PLAYERCLASSES

				#ifndef SecobMod__USE_PLAYERCLASSES
				pPlayer->m_iHealth = PlayerHealthValue;
				pPlayer->m_iMaxHealth = 125;
				pPlayer->SetArmorValue(PlayerArmourValue);
				pPlayer->SetModel("models/sdk/Humans/Group03/male_06_sdk.mdl");
				//Bug fix - if the below lines are removed the player is stuck with 0 movement, once they're able to move again we can remove the suit as required.
				pPlayer->EquipSuit();
				pPlayer->StartSprinting();
				pPlayer->StopSprinting();
				#endif //SecobMod__USE_PLAYERCLASSES NOT

				const char *pkvWeapon_Value = NULL;
				int Weapon_PriClip_Value = 0;
				const char *pkvWeapon_PriClipAmmo_Value = NULL;
				int Weapon_SecClip_Value = 0;
				const char *pkvWeapon_SecClipAmmo_Value = NULL;
				int Weapon_PriClipCurrent_Value = 0;
				int Weapon_SecClipCurrent_Value = 0;

				//Loop through all of our weapon slots.
				for (int i = 0; i < 12; i++)
				{
					if (i == 0)
					{
						pkvWeapon_Value = pkvWeapon_0->GetString();
						Weapon_PriClip_Value = pkvWeapon_0_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_0_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_0_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_0_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_0_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_0_SecClipAmmoLeft->GetInt();
					}
					else if (i == 1)
					{
						pkvWeapon_Value = pkvWeapon_1->GetString();
						Weapon_PriClip_Value = pkvWeapon_1_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_1_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_1_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_1_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_1_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_1_SecClipAmmoLeft->GetInt();

					}
					else if (i == 2)
					{
						pkvWeapon_Value = pkvWeapon_2->GetString();
						Weapon_PriClip_Value = pkvWeapon_2_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_2_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_2_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_2_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_2_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_2_SecClipAmmoLeft->GetInt();

					}
					else if (i == 3)
					{
						pkvWeapon_Value = pkvWeapon_3->GetString();
						Weapon_PriClip_Value = pkvWeapon_3_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_3_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_3_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_3_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_3_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_3_SecClipAmmoLeft->GetInt();
					}
					else if (i == 4)
					{
						pkvWeapon_Value = pkvWeapon_4->GetString();
						Weapon_PriClip_Value = pkvWeapon_4_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_4_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_4_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_4_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_4_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_4_SecClipAmmoLeft->GetInt();
					}
					else if (i == 5)
					{
						pkvWeapon_Value = pkvWeapon_5->GetString();
						Weapon_PriClip_Value = pkvWeapon_5_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_5_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_5_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_5_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_5_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_5_SecClipAmmoLeft->GetInt();
					}
					else if (i == 6)
					{
						pkvWeapon_Value = pkvWeapon_6->GetString();
						Weapon_PriClip_Value = pkvWeapon_6_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_6_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_6_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_6_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_6_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_6_SecClipAmmoLeft->GetInt();
					}
					else if (i == 7)
					{
						pkvWeapon_Value = pkvWeapon_7->GetString();
						Weapon_PriClip_Value = pkvWeapon_7_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_7_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_7_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_7_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_7_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_7_SecClipAmmoLeft->GetInt();
					}
					else if (i == 8)
					{
						pkvWeapon_Value = pkvWeapon_8->GetString();
						Weapon_PriClip_Value = pkvWeapon_8_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_8_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_8_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_8_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_8_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_8_SecClipAmmoLeft->GetInt();
					}
					else if (i == 9)
					{
						pkvWeapon_Value = pkvWeapon_9->GetString();
						Weapon_PriClip_Value = pkvWeapon_9_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_9_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_9_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_9_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_9_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_9_SecClipAmmoLeft->GetInt();
					}
					else if (i == 10)
					{
						pkvWeapon_Value = pkvWeapon_10->GetString();
						Weapon_PriClip_Value = pkvWeapon_10_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_10_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_10_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_10_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_10_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_10_SecClipAmmoLeft->GetInt();
					}
					else if (i == 11)
					{
						pkvWeapon_Value = pkvWeapon_11->GetString();
						Weapon_PriClip_Value = pkvWeapon_11_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_11_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_11_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_11_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_11_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_11_SecClipAmmoLeft->GetInt();
					}
					else if (i == 12)
					{
						pkvWeapon_Value = pkvWeapon_12->GetString();
						Weapon_PriClip_Value = pkvWeapon_12_PriClip->GetInt();
						pkvWeapon_PriClipAmmo_Value = pkvWeapon_12_PriClipAmmo->GetString();
						Weapon_SecClip_Value = pkvWeapon_12_SecClip->GetInt();
						pkvWeapon_SecClipAmmo_Value = pkvWeapon_12_SecClipAmmo->GetString();
						Weapon_PriClipCurrent_Value = pkvWeapon_12_PriClipAmmoLeft->GetInt();
						Weapon_SecClipCurrent_Value = pkvWeapon_12_SecClipAmmoLeft->GetInt();
					}

					//Now give the weapon and ammo.
					pPlayer->GiveNamedItem((pkvWeapon_Value));
					pPlayer->Weapon_Switch(pPlayer->Weapon_OwnsThisType(pkvWeapon_Value));
					if (pPlayer->GetActiveWeapon()->UsesClipsForAmmo1())
					{
						if (Weapon_PriClipCurrent_Value != -1)
						{
							if (strcmp(pkvWeapon_Value, "weapon_crossbow") == 0)
							{
								pPlayer->GetActiveWeapon()->m_iClip1 = Weapon_PriClipCurrent_Value;
								pPlayer->GetActiveWeapon()->m_iPrimaryAmmoType = Weapon_PriClip_Value;
								pPlayer->GetActiveWeapon()->SetPrimaryAmmoCount(int(Weapon_PriClip_Value));
								pPlayer->CBasePlayer::GiveAmmo(Weapon_PriClip_Value, Weapon_PriClip_Value);
							}
							else
							{
								pPlayer->GetActiveWeapon()->m_iClip1 = Weapon_PriClipCurrent_Value;
								pPlayer->CBasePlayer::GiveAmmo(Weapon_PriClip_Value, pkvWeapon_PriClipAmmo_Value);
								//Msg("Weapon primary clip value should be: %i\n", Weapon_PriClipCurrent_Value);
							}
						}
					}
					else
					{
						pPlayer->GetActiveWeapon()->m_iPrimaryAmmoType = Weapon_PriClip_Value;
						pPlayer->GetActiveWeapon()->SetPrimaryAmmoCount(int(Weapon_PriClip_Value));
						pPlayer->CBasePlayer::GiveAmmo(Weapon_PriClip_Value, Weapon_PriClip_Value);
					}
					if (pPlayer->GetActiveWeapon()->UsesClipsForAmmo2())
					{
						if (Weapon_SecClipCurrent_Value != -1)
						{
							if (strcmp(pkvWeapon_Value, "weapon_crossbow") == 0)
							{
								pPlayer->GetActiveWeapon()->m_iClip2 = Weapon_SecClipCurrent_Value;
								pPlayer->GetActiveWeapon()->m_iSecondaryAmmoType = Weapon_SecClip_Value;
								pPlayer->GetActiveWeapon()->SetSecondaryAmmoCount(int(Weapon_SecClip_Value));
								pPlayer->CBasePlayer::GiveAmmo(Weapon_SecClip_Value, Weapon_SecClip_Value);
							}
							else
							{
								pPlayer->GetActiveWeapon()->m_iClip2 = Weapon_SecClipCurrent_Value;
								pPlayer->CBasePlayer::GiveAmmo(Weapon_SecClip_Value, pkvWeapon_SecClipAmmo_Value);
							}
						}
					}
					else
					{
						pPlayer->GetActiveWeapon()->m_iSecondaryAmmoType = Weapon_SecClip_Value;
						pPlayer->GetActiveWeapon()->SetSecondaryAmmoCount(int(Weapon_SecClip_Value));
						pPlayer->CBasePlayer::GiveAmmo(Weapon_SecClip_Value, Weapon_SecClip_Value);
					}
				}
				//Now restore the players Active Weapon (the weapon they had out at the level transition).
				pPlayer->Weapon_Switch(pPlayer->Weapon_OwnsThisType(pkvActiveWep_Value));
			}
			else
			{
				//Something went wrong show the class panel.
				pPlayer->ShowViewPortPanel(PANEL_CLASS, true, NULL);
			}
			break;
		}
	}
	else
	{
		//Something went wrong show the class panel.
		pPlayer->ShowViewPortPanel(PANEL_CLASS, true, NULL);
	}
#endif //SecobMod__SAVERESTORE 

#ifdef SecobMod__ENABLE_MAP_BRIEFINGS
	pPlayer->ShowViewPortPanel(PANEL_INFO, false, NULL);
	const char *brief_title = "Briefing: ";//(hostname) ? hostname->GetString() : "MESSAGE OF THE DAY";

	KeyValues *brief_data = new KeyValues("brief_data");

	brief_data->SetString("title", brief_title);		// info panel title
	brief_data->SetString("type", "1");			// show userdata from stringtable entry
	brief_data->SetString("msg", "briefing");		// use this stringtable entry
	
	pPlayer->ShowViewPortPanel(PANEL_INFO, true, brief_data);

#endif //SecobMod__ENABLE_MAP_BRIEFINGS

	data->deleteThis();
}

/*
===========
ClientPutInServer

called each time a player is spawned into the game
============
*/
void ClientPutInServer(edict_t *pEdict, const char *playername)
{
	// Allocate a CBaseTFPlayer for pev, and call spawn
	CHL2MP_Player *pPlayer = CHL2MP_Player::CreatePlayer("player", pEdict);
	pPlayer->SetPlayerName(playername);
}


void ClientActive(edict_t *pEdict, bool bLoadGame)
{
	// Can't load games in CS!
	Assert(!bLoadGame);

	CHL2MP_Player *pPlayer = ToHL2MPPlayer(CBaseEntity::Instance(pEdict));
	FinishClientPutInServer(pPlayer);
}


/*
===============
const char *GetGameDescription()

Returns the descriptive name of this .dll.  E.g., Half-Life, or Team Fortress 2
===============
*/
const char *GetGameDescription()
{
	if (g_pGameRules) // this function may be called before the world has spawned, and the game rules initialized
		return g_pGameRules->GetGameDescription();
	else
		return "Half-Life 2 Deathmatch";
}

//-----------------------------------------------------------------------------
// Purpose: Given a player and optional name returns the entity of that 
//			classname that the player is nearest facing
//			
// Input  :
// Output :
//-----------------------------------------------------------------------------
CBaseEntity* FindEntity(edict_t *pEdict, char *classname)
{
	// If no name was given set bits based on the picked
	if (FStrEq(classname, ""))
	{
		return (FindPickerEntityClass(static_cast<CBasePlayer*>(GetContainingEntity(pEdict)), classname));
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Precache game-specific models & sounds
//-----------------------------------------------------------------------------
void ClientGamePrecache(void)
{
	CBaseEntity::PrecacheModel("models/player.mdl");
	CBaseEntity::PrecacheModel("models/gibs/agibs.mdl");
	CBaseEntity::PrecacheModel("models/weapons/v_hands.mdl");

	CBaseEntity::PrecacheScriptSound("HUDQuickInfo.LowAmmo");
	CBaseEntity::PrecacheScriptSound("HUDQuickInfo.LowHealth");

	CBaseEntity::PrecacheScriptSound("FX_AntlionImpact.ShellImpact");
	CBaseEntity::PrecacheScriptSound("Missile.ShotDown");
	CBaseEntity::PrecacheScriptSound("Bullets.DefaultNearmiss");
	CBaseEntity::PrecacheScriptSound("Bullets.GunshipNearmiss");
	CBaseEntity::PrecacheScriptSound("Bullets.StriderNearmiss");

	CBaseEntity::PrecacheScriptSound("Geiger.BeepHigh");
	CBaseEntity::PrecacheScriptSound("Geiger.BeepLow");
}


// called by ClientKill and DeadThink
void respawn(CBaseEntity *pEdict, bool fCopyCorpse)
{
	CHL2MP_Player *pPlayer = ToHL2MPPlayer(pEdict);

	if (pPlayer)
	{
		if (gpGlobals->curtime > pPlayer->GetDeathTime() + DEATH_ANIMATION_TIME)
		{
			// respawn player
			pPlayer->Spawn();
		}
		else
		{
			pPlayer->SetNextThink(gpGlobals->curtime + 0.1f);
		}
	}
}

void GameStartFrame(void)
{
	VPROF("GameStartFrame()");
	if (g_fGameOver)
		return;

	gpGlobals->teamplay = (teamplay.GetInt() != 0);

#ifdef DEBUG
	extern void Bot_RunAll();
	Bot_RunAll();
#endif
}

//=========================================================
// instantiate the proper game rules object
//=========================================================
void InstallGameRules()
{
	// vanilla deathmatch
	CreateGameRulesObject("CHL2MPRules");
}

