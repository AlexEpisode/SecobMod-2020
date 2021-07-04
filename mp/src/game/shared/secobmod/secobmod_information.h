//====================================================//
//  Source Engine: CoOperative Base Mod Information.  //
//====================================================//
//SecobMod__IFDEF_Fixes  These are mainly kept to show where if defined lines have had to be changed to ifdef for code to compile.
//SecobMod__IFDEF_Info   These are like the above but have information describing why the fix is in place.
//SecobMod__MiscFixes    These are small fixes to the code to allow things to run correctly.
//SecobMod__MiscFixes	   These are like the above but have information describing why the fix was required.
//SecobMod__Information  These are the main comment lines found in the source code explaining many things in detail.
//SecobMod__ChangeME!    These are lines of code that require changing/you may wish to change for the game to run correctly once you start modifying the base.
//SecobMod__FixMe		   These are bugs in the code which require a fix before they'll work.
//SecobMod.

//NOTE: The game has only been tested using HL2, Ep1, Ep2 and Portal 1 Content. Other games may not mount correctly but some paths in the GameInfo.txt have been added for your convenience.

//===============================//
//  Known Bugs Requiring Fixes.  //
//===============================//

//2020 Bugs:
//* HIGH - The PANEL_INFO (Text Window) which is used for MOTD and Briefings has an invalid texture for its background applied somewhere in the engine. While technically low priority it's pretty awful to look at a pink and black checkerboard texture.
//* MEDIUM - The vehicle is locked on spawn which stops AI (e.g. Alyx) from entering it. If forced into it the AI will remain standing attached to the vehicle.
//* MEDIUM - The player in a vehicle is also now standing up, originally they were put into a sitting animation so not sure why this has happened.
//* MEDIUM - Weapon strip/dissolve zones (as used to make the Super Gravity Gun in HL2/EP1 -e.g. d3_citadel_03) don't work. The hl2_triggers.cpp file has been modified to get this SEMI-working, downside is they're currently left with just a normal gravity gun. The problem is that it doesn't throw the HL2MP gravity gun on drop, it deletes it from the game and hangs because it can never find a gravity gun. A forced spawn of a gravity gun after other weapons have been dissolved is the temp. fix.


//2013 Bugs:
//* MEDIUM - Armour not working on non-playerclass save/restore transitions. This should work according to the code! - may be fixed now.
//* MEDIUM - Leaving tilted vehicles causes players to appear tilted as well. This affects only their player model, their actual views are normal. This is hard to reproduce all the time which makes fixing it that much harder.
//* LOW - Super Gravity Gun - Some particle effects are missing, but the gun itself works as in singleplayer.
//* LOW - Blood splashes being emitted when shooting other players despite no damage actually being dealt, also affects AI that can take damage but take none from the player.
// (only applicable when "SecobMod__FRIENDLY_FIRE_ENABLED" is disabled, at least for players).
//* LOW - AI bend their legs when going up in lifts (elevators), and hover slightly in the air going down.
//* LOW - You can rapidly fire the crossbow, presumably the same fix for this the shotgun has can be implemented.
//* LOW - RPG - Particles show without alphas/blobs for the laser.

//================================================//
//  Co-Op Singleplayer Maps Information.          //
//================================================//
// To those wishing to make a co-op version of the Half-Life 2/Ep1/2 maps, Mulekick on the steam forums gave this advice for older maps:
// - This does mean that you will need to start using the Everything solution file, which means more work settings things up again -
//Basically brushes that block LOS from AI's won't work with older maps.
//To fix this:
//In public/bspflags.h, change the line starting with
//#define MASK_BLOCKLOS (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_BLOCKLOS)
//to:
//#define MASK_BLOCKLOS (CONTENTS_SOLID|CONTENTS_MOVEABLE|CONTENTS_BLOCKLOS|CONTENTS_OPAQUE)

//=================================//
//  GameInfo.txt Default Backup.   //
//=================================//
/*
"GameInfo"
{
	game	"SecobMod 2020"
	title	""
	title2	""
	type multiplayer_only
	nomodels 0
	nohimodel 1
	nocrosshair 1
	hidden_maps
	{
		"test_speakers"		1
		"test_hardware"		1
	}
	supportsvr	0


	FileSystem
	{
		SteamAppId				243750
		
		SearchPaths
		{	
			//===================
			//Secob Search Paths.
			//===================
			game+mod+mod_write+default_write_path		|gameinfo_path|.
			gamebin				|gameinfo_path|bin
			Game				|all_source_engine_paths|hl2
			Game				|all_source_engine_paths|hl2mp
			game				|all_source_engine_paths|hl2/hl2_textures.vpk
			game				|all_source_engine_paths|hl2/hl2_sound_misc.vpk
			game				|all_source_engine_paths|hl2/hl2_misc.vpk
			platform			|all_source_engine_paths|platform/platform_misc.vpk
			//Custom .vpk files - Remember to rename mod_hl2mp to that of your mods folder name!
			game+mod			mod_hl2mp/custom

			//========================================================================================================================================================================================
			//Valve Game Search Paths - Uncomment to access non-code related items from games (e.g. models, textures, sounds etc.). 
			//The top lines in each game list allow SecobMod to see map files for loading in game, only HL:S,HL2/Ep1/Ep2,CS:S and Portal 1 work for map loading.
			//========================================================================================================================================================================================
			
			//Half-Life 2	
			game "|all_source_engine_paths|..\..\common\Half-Life 2\hl2"
			game "|all_source_engine_paths|..\..\common\Half-Life 2\hl2\hl2_sound_vo_english.vpk"	
			game "|all_source_engine_paths|..\..\common\Half-Life 2\hl2\hl2_pak.vpk"

			//Half-Life 2: Episode1
			//game "|all_source_engine_paths|..\..\common\Half-Life 2\episodic"
			//game "|all_source_engine_paths|..\..\common\Half-Life 2\episodic\ep1_pak.vpk"
			//game "|all_source_engine_paths|..\..\common\Half-Life 2\episodic\ep1_sound_vo_english.vpk"	

			//Half-Life 2: Episode 2	
			//game "|all_source_engine_paths|..\..\common\Half-Life 2\ep2"
			//game "|all_source_engine_paths|..\..\common\Half-Life 2\ep2\ep2_sound_vo_english.vpk"	
			//game "|all_source_engine_paths|..\..\common\Half-Life 2\ep2\ep2_pak.vpk"

			//Half-Life Source
			//game "|all_source_engine_paths|..\..\common\Half-Life 2\hl1"
			//game "|all_source_engine_paths|..\..\common\Half-Life 2\hl1_hd\hl1_hd_pak.vpk"
			//game "|all_source_engine_paths|..\..\common\Half-Life 2\hl1\hl1_sound_vo_english.vpk"
			//game "|all_source_engine_paths|..\..\common\Half-Life 2\hl1\hl1_pak.vpk"
			
			//Counter Strike Source
			//game "|all_source_engine_paths|..\..\commonCounter-Strike Source\cstrike"
			//game "|all_source_engine_paths|..\..\common\Counter-Strike Source\cstrike\cstrike_english.vpk"
			//game "|all_source_engine_paths|..\..\common\Counter-Strike Source\cstrike\cstrike_pak.vpk"
			
			//Portal1
			//game "|all_source_engine_paths|..\..\common\Portal\portal\"
			//game "|all_source_engine_paths|..\..\common\Portal\portal\portal_pak.vpk"
			
			//Portal2
			//game "|all_source_engine_paths|..\..\common\Portal 2\portal2\pak01_dir.vpk"
			//game "|all_source_engine_paths|..\..\common\Portal 2\portal2\pak01_000.vpk"

			//Team Fortress 2
			//game "|all_source_engine_paths|..\..\common\Team Fortress 2\tf\tf2_textures.vpk"
			//game "|all_source_engine_paths|..\..\common\Team Fortress 2\tf\tf2_sound_vo_english.vpk"
			//game "|all_source_engine_paths|..\..\common\Team Fortress 2\tf\tf2_sound_misc.vpk"
			//game "|all_source_engine_paths|..\..\common\Team Fortress 2\tf\tf2_misc.vpk"	

			//The below games need filling in and may or may not work:

			//Left 4 Dead

			//Left 4 Dead 2

			//Counter Strike: Global Offensive
		}
	}
}
*/

//====================================================//
//  Complete list of modified files. 				  //
//====================================================//
/*
README
mp\src\creategameprojects.bat
mp\src\createallprojects.bat
mp\src\game\shared\util_shared.cpp
mp\src\game\shared\usercmd.h
mp\src\game\shared\usercmd.cpp
mp\src\game\shared\teamplayroundbased_gamerules.cpp
mp\src\game\shared\shareddefs.h
mp\src\game\shared\ragdoll_shared.cpp
mp\src\game\shared\particlesystemquery.cpp
mp\src\game\shared\multiplay_gamerules.cpp
mp\src\game\shared\movevars_shared.cpp
mp\src\game\shared\gamerules.cpp
mp\src\game\shared\gamemovement.h
mp\src\game\shared\gamemovement.cpp
mp\src\game\shared\baseviewmodel_shared.h
mp\src\game\shared\baseviewmodel_shared.cpp
mp\src\game\shared\baseplayer_shared.cpp
mp\src\game\shared\basegrenade_shared.cpp
mp\src\game\shared\baseentity_shared.cpp
mp\src\game\shared\basecombatweapon_shared.cpp
mp\src\game\shared\ai_activity.h
mp\src\game\shared\activitylist.cpp
mp\src\game\shared\achievements_hlx.cpp
mp\src\game\shared\Multiplayer\multiplayer_animstate.h
mp\src\game\shared\Multiplayer\multiplayer_animstate.cpp
mp\src\game\shared\hl2mp\weapon_stunstick.cpp
mp\src\game\shared\hl2mp\weapon_smg1.cpp
mp\src\game\shared\hl2mp\weapon_slam.cpp
mp\src\game\shared\hl2mp\weapon_shotgun.cpp
mp\src\game\shared\hl2mp\weapon_rpg.h
mp\src\game\shared\hl2mp\weapon_rpg.cpp
mp\src\game\shared\hl2mp\weapon_pistol.cpp
mp\src\game\shared\hl2mp\weapon_physcannon.h
mp\src\game\shared\hl2mp\weapon_physcannon.cpp
mp\src\game\shared\hl2mp\weapon_hl2mpbasebasebludgeon.h
mp\src\game\shared\hl2mp\weapon_hl2mpbase_machinegun.h
mp\src\game\shared\hl2mp\weapon_hl2mpbase.h
mp\src\game\shared\hl2mp\weapon_hl2mpbase.cpp
mp\src\game\shared\hl2mp\weapon_frag.cpp
mp\src\game\shared\hl2mp\weapon_crowbar.h
mp\src\game\shared\hl2mp\weapon_crowbar.cpp
mp\src\game\shared\hl2mp\weapon_crossbow.cpp
mp\src\game\shared\hl2mp\weapon_ar2.h
mp\src\game\shared\hl2mp\weapon_ar2.cpp
mp\src\game\shared\hl2mp\hl2mp_weapon_parse.h
mp\src\game\shared\hl2mp\hl2mp_weapon_parse.cpp
mp\src\game\shared\hl2mp\hl2mp_gamerules.h
mp\src\game\shared\hl2mp\hl2mp_gamerules.cpp
mp\src\game\shared\hl2\hl_gamemovement.h
mp\src\game\shared\hl2\hl_gamemovement.cpp
mp\src\game\shared\hl2\hl2_usermessages.cpp
mp\src\game\shared\hl2\hl2_gamerules.h
mp\src\game\shared\hl2\hl2_gamerules.cpp
mp\src\game\server\util.h
mp\src\game\server\util.cpp
mp\src\game\server\triggers.cpp
mp\src\game\server\trigger_portal.cpp
mp\src\game\server\test_stressentities.cpp
mp\src\game\server\subs.cpp
mp\src\game\server\server_hl2mp.vpc
mp\src\game\server\sceneentity.cpp
mp\src\game\server\recipientfilter.cpp
mp\src\game\server\player_pickup.cpp
mp\src\game\server\player_lagcompensation.cpp
mp\src\game\server\player_command.cpp
mp\src\game\server\player.h
mp\src\game\server\player.cpp
mp\src\game\server\physics_npc_solver.cpp
mp\src\game\server\physics.h
mp\src\game\server\physics.cpp
mp\src\game\server\physconstraint.cpp
mp\src\game\server\npc_talker.cpp
mp\src\game\server\monstermaker.cpp
mp\src\game\server\message_entity.cpp
mp\src\game\server\maprules.cpp
mp\src\game\server\logicentities.cpp
mp\src\game\server\ilagcompensationmanager.h
mp\src\game\server\globals.h
mp\src\game\server\globals.cpp
mp\src\game\server\gib.cpp
mp\src\game\server\genericactor.cpp
mp\src\game\server\gameweaponmanager.cpp
mp\src\game\server\gameinterface.h
mp\src\game\server\gameinterface.cpp
mp\src\game\server\game.cpp
mp\src\game\server\func_break.cpp
mp\src\game\server\EnvMessage.cpp
mp\src\game\server\EnvHudHint.cpp
mp\src\game\server\env_zoom.cpp
mp\src\game\server\entitylist.cpp
mp\src\game\server\EntityDissolve.cpp
mp\src\game\server\effects.cpp
mp\src\game\server\CommentarySystem.cpp
mp\src\game\server\colorcorrectionvolume.cpp
mp\src\game\server\client.cpp
mp\src\game\server\baseflex.cpp
mp\src\game\server\baseentity.h
mp\src\game\server\baseentity.cpp
mp\src\game\server\basecombatcharacter.cpp
mp\src\game\server\baseanimating.h
mp\src\game\server\ai_utils.h
mp\src\game\server\ai_speech.cpp
mp\src\game\server\ai_scriptconditions.h
mp\src\game\server\ai_scriptconditions.cpp
mp\src\game\server\ai_relationship.cpp
mp\src\game\server\ai_playerally.cpp
mp\src\game\server\ai_planesolver.cpp
mp\src\game\server\ai_moveprobe.cpp
mp\src\game\server\ai_hint.cpp
mp\src\game\server\ai_behavior_standoff.cpp
mp\src\game\server\ai_behavior_passenger.h
mp\src\game\server\ai_behavior_passenger.cpp
mp\src\game\server\ai_behavior_lead.cpp
mp\src\game\server\ai_behavior_follow.cpp
mp\src\game\server\ai_behavior_fear.cpp
mp\src\game\server\ai_basenpc_schedule.cpp
mp\src\game\server\ai_basenpc.h
mp\src\game\server\ai_basenpc.cpp
mp\src\game\server\ai_activity.cpp
mp\src\game\server\hl2mp\hl2mp_player.h
mp\src\game\server\hl2mp\hl2mp_player.cpp
mp\src\game\server\hl2mp\hl2mp_client.cpp
mp\src\game\server\hl2\weapon_physcannon.cpp
mp\src\game\server\hl2\vehicle_viewcontroller.cpp
mp\src\game\server\hl2\vehicle_prisoner_pod.cpp
mp\src\game\server\hl2\vehicle_jeep.h
mp\src\game\server\hl2\vehicle_jeep.cpp
mp\src\game\server\hl2\vehicle_crane.cpp
mp\src\game\server\hl2\vehicle_airboat.cpp
mp\src\game\server\hl2\script_intro.cpp
mp\src\game\server\hl2\proto_sniper.cpp
mp\src\game\server\hl2\prop_thumper.cpp
mp\src\game\server\hl2\prop_combine_ball.cpp
mp\src\game\server\hl2\npc_zombine.cpp
mp\src\game\server\hl2\npc_vortigaunt_episodic.cpp
mp\src\game\server\hl2\npc_turret_floor.h
mp\src\game\server\hl2\npc_turret_floor.cpp
mp\src\game\server\hl2\npc_strider.cpp
mp\src\game\server\hl2\npc_scanner.cpp
mp\src\game\server\hl2\npc_rollermine.cpp
mp\src\game\server\hl2\npc_playercompanion.h
mp\src\game\server\hl2\npc_playercompanion.cpp
mp\src\game\server\hl2\npc_metropolice.cpp
mp\src\game\server\hl2\npc_headcrab.h
mp\src\game\server\hl2\npc_headcrab.cpp
mp\src\game\server\hl2\npc_enemyfinder.cpp
mp\src\game\server\hl2\npc_dog.cpp
mp\src\game\server\hl2\npc_combines.h
mp\src\game\server\hl2\npc_combines.cpp
mp\src\game\server\hl2\npc_combinegunship.cpp
mp\src\game\server\hl2\npc_combinedropship.cpp
mp\src\game\server\hl2\npc_combine.cpp
mp\src\game\server\hl2\npc_citizen17.h
mp\src\game\server\hl2\npc_citizen17.cpp
mp\src\game\server\hl2\npc_BaseZombie.cpp
mp\src\game\server\hl2\npc_basescanner.cpp
mp\src\game\server\hl2\npc_barnacle.h
mp\src\game\server\hl2\npc_barnacle.cpp
mp\src\game\server\hl2\npc_attackchopper.cpp
mp\src\game\server\hl2\npc_antlionguard.cpp
mp\src\game\server\hl2\npc_antlion.h
mp\src\game\server\hl2\npc_antlion.cpp
mp\src\game\server\hl2\npc_alyx_episodic.cpp
mp\src\game\server\hl2\item_suit.cpp
mp\src\game\server\hl2\item_dynamic_resupply.cpp
mp\src\game\server\hl2\item_battery.cpp
mp\src\game\server\hl2\hl2_player.cpp
mp\src\game\server\hl2\func_tank.h
mp\src\game\server\hl2\func_tank.cpp
mp\src\game\server\hl2\func_recharge.cpp
mp\src\game\server\hl2\env_headcrabcanister.cpp
mp\src\game\server\hl2\combine_mine.cpp
mp\src\game\server\hl2\antlion_maker.cpp
mp\src\game\server\hl2\ai_behavior_actbusy.cpp
mp\src\game\server\hl2\ai_allymanager.cpp
mp\src\game\server\episodic\weapon_striderbuster.cpp
mp\src\game\server\episodic\vehicle_jeep_episodic.cpp
mp\src\game\server\episodic\ep2_gamestats.cpp
mp\src\game\client\weapon_selection.cpp
mp\src\game\client\viewrender.h
mp\src\game\client\view.cpp
mp\src\game\client\iviewrender.h
mp\src\game\client\in_main.cpp
mp\src\game\client\hud_squadstatus.cpp
mp\src\game\client\hud_posture.cpp
mp\src\game\client\hud_basechat.cpp
mp\src\game\client\geiger.cpp
mp\src\game\client\fx_impact.cpp
mp\src\game\client\clientmode_shared.cpp
mp\src\game\client\client_hl2mp.vpc
mp\src\game\client\cdll_client_int.cpp
mp\src\game\client\c_vehicle_jeep.cpp
mp\src\game\client\c_team_train_watcher.cpp
mp\src\game\client\c_dynamiclight.cpp
mp\src\game\client\c_colorcorrection.cpp
mp\src\game\client\c_baseplayer.h
mp\src\game\client\c_baseplayer.cpp
mp\src\game\client\c_baseentity.cpp
mp\src\game\client\hl2mp\c_hl2mp_player.h
mp\src\game\client\hl2mp\c_hl2mp_player.cpp
mp\src\game\client\hl2\hud_zoom.cpp
mp\src\game\client\hl2\hud_suitpower.cpp
mp\src\game\client\hl2\hud_poisondamageindicator.cpp
mp\src\game\client\hl2\hud_locator.cpp
mp\src\game\client\hl2\hud_health.cpp
mp\src\game\client\hl2\hud_flashlight.cpp
mp\src\game\client\hl2\hud_damageindicator.cpp
mp\src\game\client\hl2\hud_battery.cpp
mp\src\game\client\hl2\hud_ammo.cpp
mp\src\game\client\hl2\c_vehicle_airboat.cpp
mp\src\game\client\hl2\c_npc_antlionguard.cpp
mp\src\game\client\game_controls\vguitextwindow.cpp
mp\src\game\client\game_controls\classmenu.cpp
mp\src\game\client\game_controls\baseviewport.cpp
mp\src\game\client\episodic\c_prop_scalable.cpp
*/