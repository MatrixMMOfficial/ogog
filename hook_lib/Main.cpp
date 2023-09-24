#include "Main.hpp"

#include "csv.hpp"

void* exception_handler;

void backtrace(const char* func) {
	const int trace_count = 15;
	void* trace_back[trace_count];
	DWORD hash;
	RtlCaptureStackBackTrace(1, trace_count, trace_back, &hash);
	nlog("%s callstack: ", func);
	printf("%s callstack: ", func);
	for (int i = 0; i < trace_count; i++) {
		if (i == trace_count - 1) {
			nlog("%p\n", (uintptr_t)trace_back[i]);
			printf("%p\n", (uintptr_t)trace_back[i]);
		}
		else {
			nlog("%p:", (uintptr_t)trace_back[i]);
			printf("%p:", (uintptr_t)trace_back[i]);
		}
	}
}

int iTick = 0;
bool bFinished;
bool btoggle;

uintptr_t xuid_generated;
int collision_ticker;
utils::hook::detour r_endframe;
void R_EndFrame_Detour() {
	if (strcmp(Dvar_GetStringSafe("NSQLTTMRMP"), "mp_donetsk") == 0) {
		*reinterpret_cast<int*>(0x14E385A68_g) = 80;
		*reinterpret_cast<int*>(0x14E385A78_g) = 80;
		if (collision_ticker == 60) {
			btoggle = !btoggle;
			*reinterpret_cast<int*>(0x145CC7555_g) = btoggle; // s_transientsCollisionMP_LobbyToGameStart
		}
		collision_ticker++;
	}
	else {
		*reinterpret_cast<int*>(0x14E385A68_g) = 1000;
		*reinterpret_cast<int*>(0x14E385A78_g) = 1000;
	}

	if (!bFinished) {
		if (iTick == 500) {
			DWORD flOldProtect;
			XUID xuid;
			xuid.RandomXUID();
			utils::hook::set<int>(0x144622BE0_g, 1);

			utils::hook::set<uintptr_t>(0x14E5C07C0_g, 0x11CB1243B8D7C31E | xuid.m_id * xuid.m_id);
			utils::hook::set<uintptr_t>(0x14F05ACE8_g, 0x11CB1243B8D7C31E | xuid.m_id * xuid.m_id);

			utils::hook::set<uintptr_t>(0x14E5C07E8_g, 0x11CB1243B8D7C31E | (xuid.m_id * xuid.m_id) / 6); // s_presenceData

			utils::hook::set<int>(0x14E371231_g, 1);
			utils::hook::set<int>(0x144622910_g, 2);
			utils::hook::set<int>(0x144622BE0_g, 1);

			utils::hook::set<char>(*reinterpret_cast<uintptr_t*>(0x14EE560B0_g) + 0x28, 0);
			utils::hook::set(0x14E5C0730_g, 2);

			auto get_bnet_class = reinterpret_cast<uintptr_t(*)()>(0x141660280_g);
			uintptr_t bnet_class = get_bnet_class();
			*(DWORD*)(bnet_class + 0x2F4) = 0x795230F0;
			*(DWORD*)(bnet_class + 0x2FC) = 0;
			*(BYTE*)(bnet_class + 0x2F8) = 31;

			printf("LOADED!\n");
			bFinished = true;
		}
		else {
			iTick += 1;
		}
	}

	r_endframe.stub<void>();
}

bool initiatedevgui;

void CG_DrawWaterMark() {
	float white[4] = { 1.0f, 1.0f, 1.0f, 0.2f };
	CL_DrawText(0x14EF2DEA0_g, "Fuck off activision you cunts", 0x7FFFFFFF, *reinterpret_cast<uintptr_t*>(0x14EEB0C68_g), 0, 400.0f, 1, 1, 0.80000001, 0.80000001, white, 7);
}

void CL_ScreenMP_DrawOverlay_Detour() {
	auto DevGui_Draw = reinterpret_cast<void(*)(int)>(0x1417E5CD0_g);
	auto Con_DrawConsole = reinterpret_cast<void(*)(int)>(0x1415AE0B0_g);

	Con_DrawConsole(0);
	DevGui_Draw(0);

	if (show_watermark->current.enabled) {
		CG_DrawWaterMark();
	}
}

utils::hook::detour cl_createdevgui;
void CL_CreateDevGui_Detour(int fsMenuEntries, const char* modeCfg) {
	auto DevGui_AddCommand = reinterpret_cast<void(*)(const char* path, const char* command)>(0x1417E58B0_g);
	auto DevGui_AddDvar = reinterpret_cast<void(*)(const char* path, uintptr_t dvar)>(0x1417E5940_g);

	cl_createdevgui.stub<void>(fsMenuEntries, modeCfg);
}

const char* username_Detour() {
	if (player_name) {
		return player_name->current.string;
	}
	else {
		return "Unknown Name";
	}
}

utils::hook::detour lui_cod_registerdvars;
void LUI_CoD_RegisterDvars_Detour() {
	nlog("registering lui dvars\n");
	player_name = Dvar_RegisterString("player_name", "Player1", 0, "Sets the player name.");
	sv_cheats = Dvar_RegisterBool("sv_cheats", false, 0, "Enables cheats to be used on a server");
	spawn_br_gas = Dvar_RegisterBool("spawn_br_gas", true, 0, "Disables gas in battle royale maps");
	show_watermark = Dvar_RegisterBool("show_watermark", false, 0, "Shows the watermark for codUPLOADER");

	player_sustainammo = Dvar_RegisterBool("player_sustainAmmo", false, 0, "Firing weapon will not decrease clip ammo.");

	lui_cod_registerdvars.stub<void>();
}

utils::hook::detour db_zones_performzoneload;
__int64 DB_Zones_PerformZoneLoad_Detour(bool processingPreloadedFiles, bool isBaseMap, bool wasPaused, int failureMode) {
	failureMode = 1;

	return db_zones_performzoneload.stub<__int64>(processingPreloadedFiles, isBaseMap, wasPaused, failureMode);
}

void CL_TransientsCollisionMP_SetTransientMode_Detour(int mode) {
	if (strcmp(Dvar_GetStringSafe("NSQLTTMRMP"), "mp_donetsk") == 0) {
		*reinterpret_cast<int*>(0x145CC7534_g) = 1;
	}
	else {
		*reinterpret_cast<int*>(0x145CC7534_g) = mode;
	}
}

utils::hook::detour net_outofbanddata;
bool NET_OutOfBandData_Detour(int sock, netadr_t* adr, const unsigned __int8* format, int len) {
	return net_outofbanddata.stub<bool>(sock, adr, format, len);
}

utils::hook::detour g_cmdsmp_clientcommand;
void G_CmdsMP_ClientCommand_Detour(int clientNum) {
	g_entities = *reinterpret_cast<gentity_s**>(0x14BC20F00_g);

	uintptr_t client = g_entities[clientNum].get<uintptr_t>(0x150);

	char command[1024];
	SV_Cmd_ArgvBuffer(0, command, 1024);

	if (client) {
		if (strcmp(command, "noclip") == 0) {
			if (CheatsOk(clientNum)) {
				Cmd_Noclip_f(clientNum);
			}
			return;
		}
		if (strcmp(command, "give") == 0) {
			if (CheatsOk(clientNum)) {
				SV_Cmd_ArgvBuffer(1, command, 1024);
				Weapon weap;
				if (BG_Weapons_GetFullWeaponForName(command, &weap, BG_FindBaseWeaponForName)) {
					if (SV_Cmd_Argc() == 3) {
						SV_Cmd_ArgvBuffer(2, command, 1024);
						weap.weaponCamo = atoi(command);
					}
					if (G_Weapon_GivePlayerWeapon(client, 0, &weap, 0, 0, 0)) {
						G_Items_AddAmmo(client, &weap, 0, 9999, 1);
						G_Weapon_SelectWeapon(clientNum, &weap);
					}
				}
			}
		}
		if (strcmp(command, "ks_give") == 0) {
			if (CheatsOk(clientNum)) {
				SV_Cmd_ArgvBuffer(1, command, 1024);
				scrContext_t* ctx = ScriptContext_Server();
				Scr_AddString(ctx, command);

				Scr_FreeThread(ctx, GScr_ExecEntThread(&g_entities[clientNum], 0x1B65FC, 1));
			}
		}
		if (strcmp(command, "bold_msg") == 0) {
			char msgbuf[500];
			SV_Cmd_ArgvBuffer(1, command, 1024);
			if (strlen(command) < 500) {
				for (int i = 0; i < 30; i++) {
					SvClient* ms_clients = *reinterpret_cast<SvClient**>(0x14E17F690_g + (8 * i));
					if (ms_clients) {
						snprintf(msgbuf, 500, "g \"%s\"", command);
						ms_clients->SendServerCommand(1, msgbuf);
					}
				}
			}
		}
		if (strcmp(command, "remove_barriers") == 0) {
			if (CheatsOk(clientNum)) {
				auto SL_ConvertToString = reinterpret_cast<const char* (*)(int)>(0x14131AA20_g);
				for (int i = 0; i < 1024; i++) {
					int classname = g_entities[i].get<int>(0x17C);
					if (classname) {
						if (strcmp(SL_ConvertToString(classname), "trigger_hurt") == 0 ||
							strcmp(SL_ConvertToString(classname), "trigger_multiple") == 0 ||
							strcmp(SL_ConvertToString(classname), "trigger_damage") == 0) {
							auto G_SetOrigin = reinterpret_cast<bool(*)(gentity_s * ent, const vec3_t * origin, bool warpPhysics, bool updateBroadphase)>(0x140FD4CC0_g);
							vec3_t gone = { 0, 0, -9999999 };
							G_SetOrigin(&g_entities[i], &gone, true, true);
						}
					}
				}
				for (int i = 0; i < 30; i++) {
					SvClient* ms_clients = *reinterpret_cast<SvClient**>(0x14E17F690_g + (8 * i));
					if (ms_clients) {
						ms_clients->SendServerCommand(1, "g \"Death barriers removed!\"");
					}
				}
			}
		}
		if (strcmp(command, "viewpos") == 0) {
			if (CheatsOk(clientNum)) {
				char msgbuf[500];
				SvClient* ms_clients = *reinterpret_cast<SvClient**>(0x14E17F690_g + (8 * clientNum));
				if (ms_clients) {
					snprintf(msgbuf, 500, "f \"viewpos: (%.2f, %.2f, %.2f)\"", g_entities[clientNum].r_currentOrigin[0], g_entities[clientNum].r_currentOrigin[1], g_entities[clientNum].r_currentOrigin[2]);
					ms_clients->SendServerCommand(1, msgbuf);
				}
			}
		}
	}

	g_cmdsmp_clientcommand.stub<void>(clientNum);
}

utils::hook::detour cl_inputmp_execbinding;
void CL_InputMP_ExecBinding_Detour(int localClientNum, int kb, int key, int forceNotify) {
	switch (key) {
	case K_N:
		CL_Main_AddReliableCommand("noclip");
		break;
	}

	cl_inputmp_execbinding.stub<void>(localClientNum, kb, key, forceNotify);
}

void Cmd_Exec_Internal(bool isSuperUser) {
	const char* cmdbuf;
	const char* file;
	auto DB_FindXAssetHeader = reinterpret_cast<uintptr_t(*)(XAssetType type, const char* givenName, int allowCreateDefault)>(0x1411AA890_g);
	auto DB_ReadRawFile = reinterpret_cast<const char* (*)(unsigned int a1, unsigned int a2, const char* a3, char a4)>(0x141297140_g);
	auto Core_strcpy_truncate = reinterpret_cast<bool(*)(char* dest, unsigned __int64 destsize, const char* src)>(0x142036A90_g);
	auto Com_DefaultExtension = reinterpret_cast<void(*)(char* path, int maxSize, const char* extension)>(0x1413F1AE0_g);
	char path[64];

	if (cmd_args->argc[cmd_args->nesting] == 2)
	{
		Core_strcpy_truncate(path, 64, *(cmd_args->argv[cmd_args->nesting] + 1));
		Com_DefaultExtension(path, 64, ".cfg");
		if (DB_FindXAssetHeader(ASSET_TYPE_RAWFILE, path, 0))
		{
			if (!DB_ReadRawFile(0, cmd_args->controllerIndex[cmd_args->nesting], path, isSuperUser))
			{
				if (cmd_args->argc[cmd_args->nesting] <= 1)
					file = "";
				else
					file = *(cmd_args->argv[cmd_args->nesting] + 1);
				printf("couldn't exec %s\n", file);
			}
		}
		else
		{
			FS_ReadFile(path, &cmdbuf);
			LUI_CoD_LuaCall_ExecNow(*reinterpret_cast<uintptr_t*>(0x151868880_g), cmdbuf);
		}
	}
	else
	{
		printf(0, "exec <filename> : execute a script file\n");
	}
}

utils::hook::detour gscr_spawnbrcircle;
void GScr_SpawnBrCircle_Detour(uintptr_t scrContext) {
	if (spawn_br_gas->current.enabled) {
		gscr_spawnbrcircle.stub<void>(scrContext);
	}
}

void entry_point() {
	XUID xuid;
	xuid.RandomXUID();

	printf("%i\n", xuid.m_id);

	r_endframe.create(0x141966950_g, R_EndFrame_Detour);
	utils::hook::jump(0x141297580_g, Cmd_Exec_Internal);
	utils::hook::jump(0x1415E1340_g, CL_ScreenMP_DrawOverlay_Detour);
	utils::hook::jump(0x1413FD3A0_g, username_Detour);

	db_zones_performzoneload.create(0x140F677A0_g, DB_Zones_PerformZoneLoad_Detour);

	g_cmdsmp_clientcommand.create(0x14120B6A0_g, G_CmdsMP_ClientCommand_Detour);
	cl_inputmp_execbinding.create(0x1415E1AB0_g, CL_InputMP_ExecBinding_Detour);
	gscr_spawnbrcircle.create(0x141243AB0_g, GScr_SpawnBrCircle_Detour);

	utils::hook::jump(0x140D6B7D0_g, CL_TransientsCollisionMP_SetTransientMode_Detour);

	printf("hooked!\n");
}

extern "C" __declspec(dllexport) int DiscordCreate() {
	CreateThread(0, 0xA0, (LPTHREAD_START_ROUTINE)entry_point, 0, 0, 0);
	return 1;
}

utils::hook::detour cl_keys_event;
void CL_Keys_Event_Detour(int localClientNum, int key, bool down, unsigned int time, int virtualKey, int controllerIndex) {
	auto Con_ToggleConsole = reinterpret_cast<void(*)()>(0x1415B18C0_g);
	auto DevGui_Toggle = reinterpret_cast<void(*)()>(0x1417E9DA0_g);

	if (down) {
		switch (key) {
		case K_GRAVE:
			Con_ToggleConsole();
			return;
			break;
		case K_F1:
			DevGui_Toggle();
			return;
			break;
		}
	}

	cl_keys_event.stub<void>(localClientNum, key, down, time, virtualKey, controllerIndex);
}

utils::hook::detour PM_WeaponUseAmmo;
void PM_WeaponUseAmmo_Detour(__int64 playerstate, Weapon* weapon, char a3, int a4, int hand) {
	if (!player_sustainammo->current.enabled) {
		PM_WeaponUseAmmo.stub<void>(playerstate, weapon, a3, a4, hand);
	}
}

utils::hook::detour dvar_registerbool;
dvar_t* Dvar_RegisterBool_Detour(const char* dvarName, bool value, unsigned int flags, const char* description) {
	if (strcmp(dvarName, "LSTQOKLTRN") == 0) {
		nlog("dvar registered!\n");
		value = true;
	}
	if (strcmp(dvarName, "MPSSOTQQPM") == 0) {
		nlog("dvar registered!\n");
		value = true;
	}
	dvar_t* ret = dvar_registerbool.stub<dvar_t*>(dvarName, value, flags, description);
	return ret;
}

utils::hook::detour dvar_registerstring;
dvar_t* Dvar_RegisterString_Detour(const char* dvarName, const char* value, unsigned int flags, const char* description) {
	return dvar_registerstring.stub<dvar_t*>(dvarName, value, flags, description);
}

utils::hook::detour seh_stringed_getstring;
const char* SEH_StringEd_GetString_Detour(const char* pszReference) {
	const char* ret = seh_stringed_getstring.stub<const char*>(pszReference);

	if (!pszReference[1])
	{
		if ((*pszReference & 0x80) != 0)
			return "t";
		return pszReference;
	}

	if (
		strstr(pszReference, "LUA_MENU/MAPNAME_DEADZONE") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_M_CAGE") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_CAVE_AM") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_CAVE") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_M_CARGO") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_CRASH2") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_M_OVERUNDER") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_EUPHRATES") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_RAID") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_M_SHOWERS") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_RUNNER_AM") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_RUNNER") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_HACKNEY_AM") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_HACKNEY_YARD") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_M_HILL") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_PICCADILLY") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_M_PINE") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_SPEAR_AM") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_SPEAR") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_PETROGRAD") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_M_STACK") ||
		strstr(pszReference, "LUA_MENU/MAPNAME_VACANT")) {
		return "^1no work";
	}

	return ret;
}

char buffer[0x5000];

utils::hook::detour db_loadxfile;
int DB_LoadXFile_Detour(const char* zoneName, uintptr_t zoneMem, uintptr_t assetList, int zoneFlags, bool wasPaused, int failureMode, uintptr_t outSignature) {
	return db_loadxfile.stub<int>(zoneName, zoneMem, assetList, zoneFlags, wasPaused, failureMode, outSignature);
}

utils::hook::detour CL_TransientsMP_ProcessLoadingQueue;
char CL_TransientsMP_ProcessLoadingQueue_Detour() {
	return CL_TransientsMP_ProcessLoadingQueue.stub<char>();
}

utils::hook::detour partyhost_startprivateparty;
void PartyHost_StartPrivateParty_Detour(int localClientNum, int localControllerIndex, bool currentlyActive, int hostType) {
	Cbuf_AddText("exec autoexec.cfg");

	partyhost_startprivateparty.stub<void>(localClientNum, localControllerIndex, currentlyActive, hostType);
}

bool Live_IsUserSignedInToDemonware_Detour() {
	return true;
}

int dwGetLogOnStatus_Detour() {
	return 2;
}

int LiveStorage_GetActiveStatsSource_Detour() {
	return 1;
}

void set_byte_f() {
	char command[500];
	if (Cmd_Argc() == 3) {
		Cmd_ArgvBuffer(1, command, 500);
		uintptr_t address = atoll(command) + base;
		Cmd_ArgvBuffer(2, command, 500);
		utils::hook::set<unsigned char>(address, atoi(command));
	}
}

void set_short_f() {
	char command[500];
	if (Cmd_Argc() == 3) {
		Cmd_ArgvBuffer(1, command, 500);
		uintptr_t address = atoll(command) + base;
		Cmd_ArgvBuffer(2, command, 500);
		utils::hook::set<unsigned short>(address, atol(command));
	}
}

void set_int_f() {
	char command[500];
	if (Cmd_Argc() == 3) {
		Cmd_ArgvBuffer(1, command, 500);
		uintptr_t address = atoll(command) + base;
		Cmd_ArgvBuffer(2, command, 500);
		utils::hook::set<unsigned int>(address, _atoi64(command));
	}
}

void set_pointer_f() {
	char command[500];
	if (Cmd_Argc() == 3) {
		Cmd_ArgvBuffer(1, command, 500);
		uintptr_t address = atoll(command) + base;
		Cmd_ArgvBuffer(2, command, 500);
		utils::hook::set<unsigned __int64>(address, _atoi64(command));
	}
}

void Cmd_Quit_f() {
	ExitProcess(0x1);
}

void Cmd_OpenMenu_f() {
	char command[500];
	if (Cmd_Argc() == 2) {
		auto LUI_OpenMenu = reinterpret_cast<void(*)(int localClientNum, const char* menuName, int isPopup, int isModal, int isExclusive)>(0x141B9BDB0_g);
		Cmd_ArgvBuffer(1, command, 500);
		LUI_OpenMenu(0, command, true, false, false);
	}
}

void Cmd_AddBot_f() {
	auto SV_ClientMP_AddTestClient = reinterpret_cast<uintptr_t(*)()>(0x14136E570_g);
	SV_ClientMP_AddTestClient();
}

void Cmd_DDLDump_f() {
	int cur = 0;
	DDLFile* g_assets = *(DDLFile**)(0x14B8F5C48_g);
	if (g_assets[cur].name) {
		while (g_assets[cur].name != 0 && g_assets[cur].ddlDef != 0) {
			nlog("%s\n", g_assets[cur].name);
			for (int istruct = 0; istruct < g_assets[cur].ddlDef->structCount; istruct++) {
				nlog("struct %s { //count: %i\n", g_assets[cur].ddlDef->structList[istruct].name, g_assets[cur].ddlDef->structList[istruct].memberCount);
				for (int imember = 0; imember < g_assets[cur].ddlDef->structList[istruct].memberCount; imember++) {
					if (g_assets[cur].ddlDef->structList[istruct].members[imember].arraySize == 1) {
						switch (g_assets[cur].ddlDef->structList[istruct].members[imember].type) {
						case DDL_STRING_TYPE: nlog("\tconst char* %s;\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						case DDL_FIXEDPOINT_TYPE:
						case DDL_FLOAT_TYPE: nlog("\tfloat %s;\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						case DDL_BYTE_TYPE: nlog("\tbyte %s;\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						case DDL_SHORT_TYPE: nlog("\tshort %s;\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						case DDL_INT_TYPE: nlog("\tint %s;\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						case DDL_UINT_TYPE: nlog("\tuint %s;\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						case DDL_PAD_TYPE: nlog("\tchar %s;\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						case DDL_HASH_TYPE: nlog("\thash %s;\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						case DDL_UINT64_TYPE: nlog("\tuint64_t %s;\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						case DDL_ENUM_TYPE: nlog("\t%s %s;\n", g_assets[cur].ddlDef->enumList[g_assets[cur].ddlDef->structList[istruct].members[imember].externalIndex].name, g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						case DDL_STRUCT_TYPE: nlog("\t%s %s;\n", g_assets[cur].ddlDef->structList[g_assets[cur].ddlDef->structList[istruct].members[imember].externalIndex].name, g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						default:nlog("\t%s;\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						}
					}
					else {
						switch (g_assets[cur].ddlDef->structList[istruct].members[imember].type) {
						case DDL_FIXEDPOINT_TYPE:
						case DDL_FLOAT_TYPE: nlog("\tfloat %s[%i];\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name, g_assets[cur].ddlDef->structList[istruct].members[imember].arraySize); break;
						case DDL_STRING_TYPE: nlog("\tconst char* %s[%i];\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name, g_assets[cur].ddlDef->structList[istruct].members[imember].arraySize); break;
						case DDL_BYTE_TYPE: nlog("\tbyte %s[%i];\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name, g_assets[cur].ddlDef->structList[istruct].members[imember].arraySize); break;
						case DDL_SHORT_TYPE: nlog("\tshort %s[%i];\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name, g_assets[cur].ddlDef->structList[istruct].members[imember].arraySize); break;
						case DDL_INT_TYPE: nlog("\tint %s[%i];\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name, g_assets[cur].ddlDef->structList[istruct].members[imember].arraySize); break;
						case DDL_UINT_TYPE: nlog("\tuint %s[%i];\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name, g_assets[cur].ddlDef->structList[istruct].members[imember].arraySize); break;
						case DDL_PAD_TYPE: nlog("\tchar %s[%i];\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name, g_assets[cur].ddlDef->structList[istruct].members[imember].arraySize); break;
						case DDL_HASH_TYPE: nlog("\thash %s[%i];\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name, g_assets[cur].ddlDef->structList[istruct].members[imember].arraySize); break;
						case DDL_ENUM_TYPE: nlog("\t%s %s[%i];\n", g_assets[cur].ddlDef->enumList[g_assets[cur].ddlDef->structList[istruct].members[imember].externalIndex].name, g_assets[cur].ddlDef->structList[istruct].members[imember].name, g_assets[cur].ddlDef->structList[istruct].members[imember].arraySize); break;
						case DDL_UINT64_TYPE: nlog("\tuint64_t %s[%i];\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name, g_assets[cur].ddlDef->structList[istruct].members[imember].arraySize); break;
						case DDL_STRUCT_TYPE: nlog("\t%s %s[%i];\n", g_assets[cur].ddlDef->structList[g_assets[cur].ddlDef->structList[istruct].members[imember].externalIndex].name, g_assets[cur].ddlDef->structList[istruct].members[imember].name, g_assets[cur].ddlDef->structList[istruct].members[imember].arraySize); break;
						default:nlog("\t%s;\n", g_assets[cur].ddlDef->structList[istruct].members[imember].name); break;
						}
					}
				}
				nlog("}\n");
			}
			for (int ienum = 0; ienum < g_assets[cur].ddlDef->enumCount; ienum++) {
				nlog("enum %s { //count: %i\n", g_assets[cur].ddlDef->enumList[ienum].name, g_assets[cur].ddlDef->enumList[ienum].memberCount);
				for (int imember = 0; imember < g_assets[cur].ddlDef->enumList[ienum].memberCount; imember++) {
					nlog("\t%s,\n", g_assets[cur].ddlDef->enumList[ienum].members[imember]);
				}
				nlog("}\n\n\n\n");
			}
			cur++;
		}
	}
}

void Cmd_WeaponDefDump_f() {
	//Globals
	uintptr_t* bg_weaponCompleteDefs = reinterpret_cast<uintptr_t*>(0x14C6EC870_g);

	nlog("DUMPING WEAPON DEFINITIONS!!! --- \n");

	for (int i = 0; i < 550; i++) {
		WeaponCompleteDef* weap = reinterpret_cast<WeaponCompleteDef*>(bg_weaponCompleteDefs[i]);

		if (!weap) continue;
		nlog("szInternalName: %s\n", weap->szInternalName);
		nlog("szDisplayName: %s\n", weap->szDisplayName);
	}

	nlog("FINISHED WEAPON DEFINITION DUMP YAY!!! --- \n");
}

void Cmd_ViewVehicleEnts_f() {
	auto SL_ConvertToString = reinterpret_cast<const char* (*)(int)>(0x14131AA20_g);
	for (int i = 0; i < 1024; i++) {
		int classname = g_entities[i].get<int>(0x17C);
		if (classname) {
			const char* s_classname = SL_ConvertToString(classname);
			if (g_entities[i].get<uintptr_t>(0x160)) {
				nlog("vehicle %s\n", s_classname);
			}
		}
	}
}

void Cmd_LoadoutSave_f() {
	auto Cmd_LocalControllerIndex = reinterpret_cast<int(*)()>(0x141298040_g);
	auto LiveStorage_GetActiveStatsSource = reinterpret_cast<int(*)(int)>(0x1412A1EB0_g);
	auto LiveStorage_CreateDDLContext = reinterpret_cast<bool(*)(const int controllerIndex, int statsGroup, int statsSource, DDLContext * context, uintptr_t a5, void* userData)>(0x1412A13C0_g);
	auto DDL_MoveToPath = reinterpret_cast<bool(*)(const DDLState * fromState, DDLState * toState, int depth, const char** path)>(0x1420524F0_g);
	//auto Com_DDL_ConvertNavStringToHash = reinterpret_cast<int(*)(const char*)>(0x14129EE80_g);
	//auto LiveStorage_InitializeDDLStateForStatsGroup = reinterpret_cast<void(*)(const DDLDef * def, DDLState * state, int statsGroup)>(0x1410CAD70_g);
	auto Com_PlayerData_GetStatsBlob = reinterpret_cast<int(*)(int)>(0x1410CA7A0_g);
	auto DDL_GetType = reinterpret_cast<DDLType(*)(const DDLState*)>(0x142051DD0_g);
	auto DDL_GetString = reinterpret_cast<const char* (*)(const DDLState * state, const DDLContext * ddlContext)>(0x142051CD0_g);
	auto DDL_GetEnum = reinterpret_cast<const char* (*)(const DDLState * state, const DDLContext * ddlContext)>(0x1420519E0_g);
	auto DDL_GetInt = reinterpret_cast<int(*)(const DDLState * state, const DDLContext * ddlContext)>(0x142051BF0_g);
	auto DDL_GetRootState = reinterpret_cast<DDLState * (*)(DDLState * result, const DDLDef * ddlDef)>(0x142051C70_g);
	auto CL_PlayerData_GetDDLBuffer = reinterpret_cast<bool(*)(DDLContext * context, int controllerIndex, int statsSource, __int32 statsGroup)>(0x1415C7940_g);
	auto Com_DDL_LoadAsset = reinterpret_cast<const DDLDef * (*)(const char* fileName)>(0x14129F3B0_g);
	auto Com_ParseNavStrings = reinterpret_cast<bool(*)(const char* pStr, const char** navStrings, int navStringMax, int* navStringCount)>(0x1412A02E0_g);
	auto Com_DDL_CreateContext = reinterpret_cast<bool(*)(void* buffer, int len, const DDLDef * def, DDLContext* const ddlContext, uintptr_t a5, void* userData)>(0x14129EEC0_g);

	DDLContext context;
	DDLState state;
	char buffer[200];
	char* navStrings[32]{};
	int navStringCount;
	if (CL_PlayerData_GetDDLBuffer(&context, 0, 1, 4)) {
		DDL_GetRootState(&state, context.def);
		sprintf_s(buffer, "loadouts.0.name");
		Com_ParseNavStrings(buffer, (const char**)navStrings, 32, &navStringCount);
		if (DDL_MoveToPath(&state, &state, navStringCount, (const char**)navStrings)) {
			printf("%s\n", DDL_GetString(&state, &context));
		}
	}
}

void SaveOperatorSkins()
{
	auto Cmd_LocalControllerIndex = reinterpret_cast<int(*)()>(0x141298040_g);
	auto LiveStorage_GetActiveStatsSource = reinterpret_cast<int(*)(int)>(0x1412A1EB0_g);
	auto LiveStorage_CreateDDLContext = reinterpret_cast<bool(*)(const int controllerIndex, int statsGroup, int statsSource, DDLContext * context, uintptr_t a5, void* userData)>(0x1412A13C0_g);
	auto DDL_MoveToPath = reinterpret_cast<bool(*)(const DDLState * fromState, DDLState * toState, int depth, const char** path)>(0x1420524F0_g);
	//auto Com_DDL_ConvertNavStringToHash = reinterpret_cast<int(*)(const char*)>(0x14129EE80_g);
	//auto LiveStorage_InitializeDDLStateForStatsGroup = reinterpret_cast<void(*)(const DDLDef * def, DDLState * state, int statsGroup)>(0x1410CAD70_g);
	auto Com_PlayerData_GetStatsBlob = reinterpret_cast<int(*)(int)>(0x1410CA7A0_g);
	auto DDL_GetType = reinterpret_cast<DDLType(*)(const DDLState*)>(0x142051DD0_g);
	auto DDL_GetString = reinterpret_cast<const char* (*)(const DDLState * state, const DDLContext * ddlContext)>(0x142051CD0_g);
	auto DDL_GetEnum = reinterpret_cast<const char* (*)(const DDLState * state, const DDLContext * ddlContext)>(0x1420519E0_g);
	auto DDL_GetInt = reinterpret_cast<int(*)(const DDLState * state, const DDLContext * ddlContext)>(0x142051BF0_g);
	auto DDL_GetRootState = reinterpret_cast<DDLState * (*)(DDLState * result, const DDLDef * ddlDef)>(0x142051C70_g);
	auto CL_PlayerData_GetDDLBuffer = reinterpret_cast<bool(*)(DDLContext * context, int controllerIndex, int statsSource, __int32 statsGroup)>(0x1415C7940_g);
	auto Com_DDL_LoadAsset = reinterpret_cast<const DDLDef * (*)(const char* fileName)>(0x14129F3B0_g);
	auto Com_ParseNavStrings = reinterpret_cast<bool(*)(const char* pStr, const char** navStrings, int navStringMax, int* navStringCount)>(0x1412A02E0_g);
	auto Com_DDL_CreateContext = reinterpret_cast<bool(*)(void* buffer, int len, const DDLDef * def, DDLContext* const ddlContext, uintptr_t a5, void* userData)>(0x14129EEC0_g);
	auto StringTable_GetColumnValueForRow = reinterpret_cast<const char* (*)(StringTable*, int, int)>(0x1413E2B40_g);

	DDLContext context;
	DDLState state;
	char buffer[200];
	char* navStrings[32]{};
	int navStringCount;
	char path[MAX_PATH + 1];
	strcpy(path, Dvar_GetStringSafe("LOOQOTRNTN"));
	strcat(path, "\\players\\inventory.json");
	nlohmann::json inventoryJson;
	if (CL_PlayerData_GetDDLBuffer(&context, 0, 1, 4)) {
		auto operatorCsv = DB_FindXAssetHeader(ASSET_TYPE_STRINGTABLE, "operators.csv", 0).stringTable;
		for (int i = 0; i < operatorCsv->rowCount; ++i) {
			const char* operatorName = StringTable_GetColumnValueForRow(operatorCsv, i, 1);
			DDL_GetRootState(&state, context.def);
			sprintf_s(buffer, "customizationSetup.operatorCustomization.%s.skin", operatorName);
			Com_ParseNavStrings(buffer, (const char**)navStrings, 32, &navStringCount);
			if (DDL_MoveToPath(&state, &state, navStringCount, (const char**)navStrings))
			{
				inventoryJson["Operator"]["OperatorSkin"][operatorName] = DDL_GetInt(&state, &context);
			}
		}
		for (int i = 0; i < 2; ++i)
		{
			DDL_GetRootState(&state, context.def);
			sprintf_s(buffer, "customizationSetup.operators.%d", i);
			Com_ParseNavStrings(buffer, (const char**)navStrings, 32, &navStringCount);
			if (DDL_MoveToPath(&state, &state, navStringCount, (const char**)navStrings))
			{
				inventoryJson["Operator"]["SelectedOperator"][i] = DDL_GetEnum(&state, &context);
			}
		}
		std::ofstream JsonOut(path);
		JsonOut << inventoryJson;
	}
}

int LuaShared_LuaCall_IsDemoBuild_Detour(uintptr_t luaVM) {
	lua_pushboolean(luaVM, 1);
	return 1;
}

utils::hook::detour dvar_findvarbyname;
dvar_t* Dvar_FindVarByName_Detour(const char* dvarName) {
	dvar_t* ret = dvar_findvarbyname.stub<dvar_t*>(dvarName);
	return ret;
}

utils::hook::detour db_findxassetheader;
XAssetHeader DB_FindXAssetHeader_Detour(XAssetType type, const char* givenName, int allowCreateDefault) {
	XAssetHeader temp = db_findxassetheader.stub<XAssetHeader>(type, givenName, allowCreateDefault);

	//if (type == ASSET_TYPE_XMODEL) {
	//	if (strcmp(temp.model->name, "head_mp_western_ghost_1_1") == 0) {
	//		return db_findxassetheader.stub<XAssetHeader>(type, "head_opforce_juggernaut", allowCreateDefault);
	//	}
	//	if (strcmp(temp.model->name, "mp_western_vm_arms_ghost_1_1") == 0) {
	//		return db_findxassetheader.stub<XAssetHeader>(type, "viewhands_opforce_juggernaut", allowCreateDefault);
	//	}
	//	if (strcmp(temp.model->name, "body_mp_western_ghost_1_1_lod1") == 0) {
	//		return db_findxassetheader.stub<XAssetHeader>(type, "body_opforce_juggernaut_mp_lod1", allowCreateDefault);
	//	}
	//	if (strcmp(temp.model->name, "military_carepackage_01_friendly") == 0) {
	//		return db_findxassetheader.stub<XAssetHeader>(type, "opforce_juggernaut_prop_static", allowCreateDefault);
	//	}
	//	if (strstr(temp.model->name, "veh8_mil_air_")) {
	//		return db_findxassetheader.stub<XAssetHeader>(type, "veh8_mil_air_acharlie130", allowCreateDefault);
	//	}
	//}

	//if (type == ASSET_TYPE_STRINGTABLE)
	//{
	//	char path[MAX_PATH + 1];
	//	memset(path, 0, MAX_PATH + 1);
	//	snprintf(path, MAX_PATH + 1, "%s\\players\\raw\\%s", Dvar_GetStringSafe("LOOQOTRNTN"), givenName);
	//	if (file_exists(path))
	//	{
	//		printf("replacing file %s\n", givenName);

	//		csv::CSVFileInfo info = csv::get_file_info(path);
	//		StringTable* table = temp.stringTable;
	//		table->rowCount = info.n_rows;
	//		table->columnCount = info.n_cols;

	//		//table->name =
	//	}
	//}

	return temp;
}

utils::hook::detour db_getrawbufferinflate;
const char* DB_GetRawBufferInflate_Detour(const char* file, char* buffer, int length) {
	char path[MAX_PATH + 1];
	memset(path, 0, MAX_PATH + 1);
	std::string filecontents;
	std::string curline;
	snprintf(path, MAX_PATH + 1, "%s\\players\\raw\\%s", Dvar_GetStringSafe("LOOQOTRNTN"), file);
	if (file_exists(path)) {
		printf("replacing file %s\n", file);
		std::ifstream myfile;
		myfile.open(path);
		filecontents = "";
		while (myfile) {
			std::getline(myfile, curline);
			filecontents += curline + "\n";
		}
		myfile.close();
		strcpy(buffer, filecontents.c_str());
		return filecontents.c_str();;
	}
	printf("loading %s\n", file);
	return db_getrawbufferinflate.stub<const char*>(file, buffer, length);
}

const char* _va(const char* format, ...) {
	char _buf[2048];
	va_list ap;

	va_start(ap, format);
	vsnprintf(_buf, 2048, format, ap);
	_buf[2047] = 0;
	return _buf;
}
SpawnPointEntityRecord* g_customSpawns;
char g_customEntityString[0xFFFFFFF];
utils::hook::detour load_mapentsasset;
void Load_MapEntsAsset_Detour(XAssetHeader* mapEnts) {
	auto Scr_AllocGlobalString = reinterpret_cast<scr_string_t(*)(const char*)>(0x14131B2C0_g);
	char path[MAX_PATH + 1];
	snprintf(path, MAX_PATH + 1, "%s\\players\\raw\\%s", Dvar_GetStringSafe("LOOQOTRNTN"), mapEnts->image->name);
	if (file_exists(path)) {
		printf("loading %s\n", path);
		HANDLE mapEntsFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		int numberOfBytesRead = GetFileSize(mapEntsFile, NULL);
		if (mapEntsFile != INVALID_HANDLE_VALUE)
		{
			memset(g_customEntityString, 0, 0xFFFFFFF);
			ReadFile(mapEntsFile, g_customEntityString, numberOfBytesRead, (LPDWORD)&numberOfBytesRead, 0);
			mapEnts->mapEnts->entityString = g_customEntityString;
			mapEnts->mapEnts->numEntityChars = strlen(g_customEntityString) + 1;
			CloseHandle(mapEntsFile);
			memset(path, 0, MAX_PATH + 1);
			snprintf(path, MAX_PATH + 1, "%s\\players\\raw\\%s.spawnlist", Dvar_GetStringSafe("LOOQOTRNTN"), mapEnts->image->name);
			if (!file_exists(path)) {
			}
			else {
				nlohmann::json json;
				std::ifstream file(path);
				file >> json;
				file.close();
				mapEnts->mapEnts->spawnList.spawnsCount = json["spawnList"]["spawnsCount"];
				for (int i = 0; i < mapEnts->mapEnts->spawnList.spawnsCount; i++) {
					mapEnts->mapEnts->spawnList.spawns[i].index = json["spawnList"][_va("spawns[%i]", i)]["index"];
					mapEnts->mapEnts->spawnList.spawns[i].name = Scr_AllocGlobalString(std::string(json["spawnList"][_va("spawns[%i]", i)]["name"]).c_str());
					mapEnts->mapEnts->spawnList.spawns[i].target = Scr_AllocGlobalString(std::string(json["spawnList"][_va("spawns[%i]", i)]["target"]).c_str());
					mapEnts->mapEnts->spawnList.spawns[i].script_noteworthy = Scr_AllocGlobalString(std::string(json["spawnList"][_va("spawns[%i]", i)]["script_noteworthy"]).c_str());

					mapEnts->mapEnts->spawnList.spawns[i].origin.v[0] = json["spawnList"][_va("spawns[%i]", i)]["origin"][0];
					mapEnts->mapEnts->spawnList.spawns[i].origin.v[1] = json["spawnList"][_va("spawns[%i]", i)]["origin"][1];
					mapEnts->mapEnts->spawnList.spawns[i].origin.v[2] = json["spawnList"][_va("spawns[%i]", i)]["origin"][2];

					mapEnts->mapEnts->spawnList.spawns[i].angles.v[0] = json["spawnList"][_va("spawns[%i]", i)]["angles"][0];
					mapEnts->mapEnts->spawnList.spawns[i].angles.v[1] = json["spawnList"][_va("spawns[%i]", i)]["angles"][1];
					mapEnts->mapEnts->spawnList.spawns[i].angles.v[2] = json["spawnList"][_va("spawns[%i]", i)]["angles"][2];
				}
			}
		}
	}

	printf("%s\n", mapEnts->mapEnts->clientTrigger.triggerString);

	load_mapentsasset.stub<void>(mapEnts);
}

utils::hook::detour load_clipmapasset;
void Load_ClipMapAsset_Detour(XAssetHeader* clipMap) {
	load_clipmapasset.stub<void>(clipMap);
}
char g_customBuffer[0x18000];
utils::hook::detour db_getrawbuffer;
void DB_GetRawBuffer_Detour(const RawFile* rawfile, char* buf, int size) {
	char path[MAX_PATH + 1];
	snprintf(path, MAX_PATH + 1, "%s\\players\\raw\\%s", Dvar_GetStringSafe("LOOQOTRNTN"), rawfile->name);
	if (file_exists(path)) {
		HANDLE mapEntsFile = CreateFile(path, GENERIC_READ, 0, NULL, OPEN_EXISTING, 0, NULL);
		int numberOfBytesRead = GetFileSize(mapEntsFile, NULL);
		if (mapEntsFile != INVALID_HANDLE_VALUE)
		{
			ReadFile(mapEntsFile, g_customBuffer, numberOfBytesRead, (LPDWORD)&numberOfBytesRead, 0);
			CloseHandle(mapEntsFile);
			strcpy(buf, g_customBuffer);
			printf("replacing %s\n", rawfile->name);
		}
		return;
	}
	printf("loading %s\n", rawfile->name);

	db_getrawbuffer.stub<void>(rawfile, buf, size);
}

utils::hook::detour db_pollfastfilestate;
int DB_PollFastfileState_Detour(const char* zoneName) {
	if (strcmp(zoneName, "mp_donetsk_cg_ls_tr") == 0) {
		return 2;
	}

	//printf("%s\n", zoneName);

	return db_pollfastfilestate.stub<int>(zoneName);
}

utils::hook::detour load_xmodelasset;
void Load_XModelAsset_Detour(XAssetHeader* model) {
	load_xmodelasset.stub<void>(model);
}

utils::hook::detour ddl_setuint;
bool DDL_SetUInt_Detour(const DDLState* state, DDLContext* ddlContext, unsigned int val) {
	//ddlContext->obfuscated = false;
	//printf("%p - %p\n", state->offset, (uintptr_t)(ddlContext->buff) + state->offset);

	return ddl_setuint.stub<bool>(state, ddlContext, val);
}

//UserInfo Crash fix
utils::hook::detour sv_updateuserinfo_f;
void SV_UpdateUserinfo_f_Detour(__int64 cl) {
	auto SV_Cmd_Argv = reinterpret_cast<char* (*)(int)>(0x141298B10_g);
	auto Info_ValueForKey = reinterpret_cast<char* (*)(const char*, const char*)>(0x1413F2A10_g);
	// more checks can be added here (it's patched in current mw19, vanguard, and mwii, could probably find the actual fix there)
	if (!strlen(Info_ValueForKey(SV_Cmd_Argv(1), "platform"))) {
		return;
	}
	sv_updateuserinfo_f.stub<void>(cl);
}

void* exception_handler_handle;
BOOL WINAPI DllMain(HMODULE hModule, DWORD Reason, LPVOID lpVoid) {
	g_Addrs.ModuleBase = (uintptr_t)(GetModuleHandle(0));
	utils::hook::set<char>(0x1403061A0_g, 0xC3); // Mystery function 1
	if (Reason == DLL_PROCESS_ATTACH) {
		AllocConsole();
		FILE* Dummy;
		freopen_s(&Dummy, "CONOUT$", "w", stdout);
		freopen_s(&Dummy, "CONIN$", "r", stdin);

		utils::nt::library game{};
		utils::nt::library user32("user32.dll");
		utils::nt::library ntdll("ntdll.dll");
		utils::nt::library kernel32("kernel32.dll");

		va = (const char* (*)(const char*, ...))0x1413F3010_g; //j_va

		nlog("Base Address: %p\n", base);

		cmd_args = (CmdArgs*)(0x14D20CBD0_g);

		//utils::hook::jump(0x141BD3360_g, sub_141BD3360_Detour);

		//sub_141BD3360.create(0x141BD3360_g, sub_141BD3360_Detour);

		utils::hook::copy(0x1530AD525_g, data_buf, 0x12856B); // Splash screen data

		Cmd_AddCommandInternal("set_byte", set_byte_f, &set_byte_f_VAR);
		Cmd_AddCommandInternal("set_short", set_short_f, &set_short_f_VAR);
		Cmd_AddCommandInternal("set_int", set_int_f, &set_int_f_VAR);
		Cmd_AddCommandInternal("set_pointer", set_pointer_f, &set_pointer_f_VAR);
		Cmd_AddCommandInternal("quit", Cmd_Quit_f, &quit_f_VAR);
		Cmd_AddCommandInternal("openmenu", Cmd_OpenMenu_f, &openmenu_f_VAR);
		//Cmd_AddCommandInternal("addbot", Cmd_AddBot_f, &addbot_f_VAR);
		Cmd_AddCommandInternal("ddldump", Cmd_DDLDump_f, &ddldump_f_VAR);
		Cmd_AddCommandInternal("weapondefdump", Cmd_WeaponDefDump_f, &weapondefdump_f_VAR);
		//Cmd_AddCommandInternal("view_vehicle_ents", Cmd_ViewVehicleEnts_f, &view_vehicle_ents_f_VAR);
		//Cmd_AddCommandInternal("loadout_save", Cmd_LoadoutSave_f, &loadout_save_f_VAR);
		Cmd_AddCommandInternal("loadout_save", SaveOperatorSkins, &loadout_save_f_VAR);

		// patch ui_maxclients limit
		utils::hook::nop(0x140F30210_g, 5);
		utils::hook::nop(0x14119E51D_g, 5);
		utils::hook::nop(0x14136B8F8_g, 5);
		utils::hook::nop(0x1416029F0_g, 5);
		utils::hook::nop(0x1419E19A3_g, 5);

		// patch party_maxplayers limit
		utils::hook::nop(0x140F252EE_g, 5);
		utils::hook::nop(0x14119D23F_g, 5);
		utils::hook::nop(0x1410769B9_g, 5);
		utils::hook::set(0x1410769B9_g, 0xC3);
		utils::hook::nop(0x140F24B4B_g, 5);
		utils::hook::set(0x140F24B4B_g, 0xC3);
		utils::hook::nop(0x1416029E2_g, 5);
		utils::hook::nop(0x14119E52B_g, 5);
		utils::hook::nop(0x140f252EE_g, 5);
		utils::hook::nop(0x14119F13A_g, 5);
		utils::hook::nop(0x1410D32E2_g, 5);

		// Fix crash exploit
		utils::hook::nop(0x14136D0EA_g, 5);

		db_findxassetheader.create(0x1411AA890_g, DB_FindXAssetHeader_Detour);
		db_getrawbufferinflate.create(0x1412C2AE0_g, DB_GetRawBufferInflate_Detour);
		ddl_setuint.create(0x1420529C0_g, DDL_SetUInt_Detour);
		//db_pollfastfilestate.create(0x1411ADD00_g, DB_PollFastfileState_Detour);

		load_mapentsasset.create(0x140F61690_g, Load_MapEntsAsset_Detour);
		load_clipmapasset.create(0x140F60F40_g, Load_ClipMapAsset_Detour);
		load_xmodelasset.create(0x140F62290_g, Load_XModelAsset_Detour);
		//load_ttfasset.create(0x140F61F40_g, Load_TTFAsset_Detour);

		db_getrawbuffer.create(0x1412C29A0_g, DB_GetRawBuffer_Detour);

		utils::hook::jump(0x141528490_g, Live_IsUserSignedInToDemonware_Detour);
		utils::hook::jump(0x1417EC930_g, dwGetLogOnStatus_Detour);
		utils::hook::jump(0x1412A1EB0_g, LiveStorage_GetActiveStatsSource_Detour);
		//utils::hook::jump(0x1419B96A0_g, LuaShared_LuaCall_IsDemoBuild_Detour);

		dvar_findvarbyname.create(0x1413E63A0_g, Dvar_FindVarByName_Detour);

		db_loadxfile.create(0x1411A79F0_g, DB_LoadXFile_Detour);
		CL_TransientsMP_ProcessLoadingQueue.create(0x1415F7BF0_g, CL_TransientsMP_ProcessLoadingQueue_Detour);
		sv_updateuserinfo_f.create(0x14136D0C0_g, SV_UpdateUserinfo_f_Detour);

		lui_cod_registerdvars.create(0x1419D4500_g, LUI_CoD_RegisterDvars_Detour);
		net_outofbanddata.create(0x1412BB350_g, NET_OutOfBandData_Detour);
		cl_keys_event.create(0x1415BEB80_g, CL_Keys_Event_Detour);
		dvar_registerbool.create(0x1413E7670_g, Dvar_RegisterBool_Detour);
		dvar_registerstring.create(0x1413E7A70_g, Dvar_RegisterString_Detour);
		seh_stringed_getstring.create(0x1413CC2A0_g, SEH_StringEd_GetString_Detour);

		cl_createdevgui.create(0x1415B2080_g, CL_CreateDevGui_Detour);
		partyhost_startprivateparty.create(0x14119F0D0_g, PartyHost_StartPrivateParty_Detour);

		PM_WeaponUseAmmo.create(0x141155AF0_g, PM_WeaponUseAmmo_Detour); //TODO: add sv_cheat validation

		clientUIActives = (clientUIActive_t*)(0x14EEF1280_g);
	}

	return TRUE;
}

void nlog(const char* str, ...) {
	va_list ap;
	HWND notepad, edit;
	char buf[256];

	va_start(ap, str);
	vsprintf(buf, str, ap);
	va_end(ap);
	strcat(buf, "");
	log(buf);
}
void nlog(const char* file, const char* str, ...) {
	va_list ap;
	HWND notepad, edit;
	char buf[256];

	va_start(ap, str);
	vsprintf(buf, str, ap);
	va_end(ap);
	strcat(buf, "");
	log(file, buf);
}

uintptr_t find_pattern(const char* module_name, const char* pattern) {
	const auto get_module_size = [=](uintptr_t module_base)
		{
			return reinterpret_cast<PIMAGE_NT_HEADERS>(module_base + reinterpret_cast<PIMAGE_DOS_HEADER>(module_base)->e_lfanew)->OptionalHeader.SizeOfImage;
		};
	const auto module_start = (uintptr_t)GetModuleHandle(module_name);
	if (module_start != 0ULL)
	{
		const auto module_end = module_start + get_module_size(module_start);

		const char* pattern_current = pattern;
		uintptr_t current_match = NULL;

		MEMORY_BASIC_INFORMATION64 page_information = {};
		for (auto current_page = reinterpret_cast<unsigned char*>(module_start); current_page < reinterpret_cast<unsigned char*>(module_end); current_page = reinterpret_cast<unsigned char*>(page_information.BaseAddress + page_information.RegionSize))
		{
			VirtualQuery(reinterpret_cast<LPCVOID>(current_page), reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&page_information), sizeof(MEMORY_BASIC_INFORMATION));
			if (page_information.Protect == PAGE_NOACCESS)
				continue;

			if (page_information.State != MEM_COMMIT)
				continue;

			if (page_information.Protect & PAGE_GUARD)
				continue;

			for (auto current_address = reinterpret_cast<unsigned char*>(page_information.BaseAddress); current_address < reinterpret_cast<unsigned char*>(page_information.BaseAddress + page_information.RegionSize - 0x8); current_address++)
			{
				if (*current_address != GET_BYTE(pattern_current) && *pattern_current != '\?') {
					current_match = 0ULL;
					pattern_current = pattern;
					continue;
				}

				if (!current_match)
					current_match = reinterpret_cast<uintptr_t>(current_address);

				pattern_current += 3;
				if (pattern_current[-1] == NULL)
					return current_match;
			}
		}
	}

	return 0ULL;
}

uintptr_t find_pattern(uintptr_t start, const char* module_name, const char* pattern) {
	const auto get_module_size = [=](uintptr_t module_base)
		{
			return reinterpret_cast<PIMAGE_NT_HEADERS>(module_base + reinterpret_cast<PIMAGE_DOS_HEADER>(module_base)->e_lfanew)->OptionalHeader.SizeOfImage;
		};
	const auto module_start = start;
	if (module_start != 0ULL)
	{
		const auto module_end = module_start + get_module_size(module_start);

		const char* pattern_current = pattern;
		uintptr_t current_match = NULL;

		MEMORY_BASIC_INFORMATION64 page_information = {};
		for (auto current_page = reinterpret_cast<unsigned char*>(module_start); current_page < reinterpret_cast<unsigned char*>(module_end); current_page = reinterpret_cast<unsigned char*>(page_information.BaseAddress + page_information.RegionSize))
		{
			VirtualQuery(reinterpret_cast<LPCVOID>(current_page), reinterpret_cast<PMEMORY_BASIC_INFORMATION>(&page_information), sizeof(MEMORY_BASIC_INFORMATION));
			if (page_information.Protect == PAGE_NOACCESS)
				continue;

			if (page_information.State != MEM_COMMIT)
				continue;

			if (page_information.Protect & PAGE_GUARD)
				continue;

			for (auto current_address = reinterpret_cast<unsigned char*>(page_information.BaseAddress); current_address < reinterpret_cast<unsigned char*>(page_information.BaseAddress + page_information.RegionSize - 0x8); current_address++)
			{
				if (*current_address != GET_BYTE(pattern_current) && *pattern_current != '\?') {
					current_match = 0ULL;
					pattern_current = pattern;
					continue;
				}

				if (!current_match)
					current_match = reinterpret_cast<uintptr_t>(current_address);

				pattern_current += 3;
				if (pattern_current[-1] == NULL)
					return current_match;
			}
		}
	}

	return 0ULL;
}
menu_variables vars;

size_t operator"" _b(const size_t val)
{
	return base + val;
}

size_t reverse_b(const size_t val)
{
	return val - base;
}

size_t reverse_b(const void* val)
{
	return reverse_b(reinterpret_cast<size_t>(val));
}

size_t operator"" _g(const size_t val)
{
	return base + (val - 0x140000000);
}

size_t reverse_g(const size_t val)
{
	return (val - base) + 0x140000000;
}

size_t reverse_g(const void* val)
{
	return reverse_g(reinterpret_cast<size_t>(val));
}

void log(const char* str) {
	std::ofstream outputFile("output.log", std::ios::app);
	if (outputFile.is_open()) {
		outputFile << str;
		outputFile.close();
	}
	else {
		std::cout << "Failed to open file for appending." << std::endl;
	}
}

void log(const char* file, const char* str) {
	try {
		std::ofstream outputFile(file, std::ios::app);
		if (!outputFile.is_open()) {
			throw std::runtime_error("Failed to open file for appending.");
		}

		outputFile << str;
	}
	catch (const std::exception& e) {
		printf("%s\n", e.what());
	}
}