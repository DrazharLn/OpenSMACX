/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2020 Brendan Casey
 *
 * OpenSMACX is free software: you can redistribute it and / or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenSMACX is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenSMACX. If not, see <http://www.gnu.org/licenses/>.
 */
#include "stdafx.h"
#include "temp.h"
#include "alpha.h"
#include "council.h"
#include "faction.h"
#include "general.h"
#include "game.h"
#include "base.h"
#include "map.h"
#include "veh.h"
#include "maininterface.h"
#include "technology.h"
#include "terraforming.h"
#include "log.h"

// built-in functions > used to prevent crash from mixed alloc/free SDKs
func1 *_malloc = (func1 *)0x006470A6;
func2 *_free = (func2 *)0x00644EF2;
func3 *_fopen = (func3 *)0x00645646;
func4 *_srand = (func4 *)0x00646010;
func10 *_fread = (func10 *)0x00646178;
func10 *_fwrite = (func10 *)0x0064603F;
//func7 *__rand = (func7 *)0x0064601D;
//func12 *_realloc = (func12 *)0x00647132;
//func18 *_fclose = (func18 *)0x00645598;
//func19 *_fgets = (func19 *)0x0064726A;

// other
func5 *load_faction_art = (func5 *)0x00453710;
// TODO: crash bug; X_pop > ... > BasePop::start > text_close > NULLs 009B7CF4 (FILE *Txt.textFile)
// Next call to text_get() tries to access 009B7CF4 and the game crashes.
func6 *X_pop = (func6 *)0x005BF310;
func9 *fixup_landmarks = (func9 *)0x00592940;
func9 *mapwin_terrain_fixup = (func9 *)0x00471240;
func9 *do_video = (func9 *)0x00636300;
func9 *check_net = (func9 *)0x0062D5D0;
func9 *do_net = (func9 *)0x0062D5B0;
func14 *base_at = (func14 *)0x004E3A50;
func11 *wants_to_attack = (func11 *)0x0055BC80;

// Time
func30* blink_timer = (func30*)0x0050EA40;
func30* blink2_timer = (func30*)0x0050EE30;
func30* line_timer = (func30*)0x0050EE80;
func30* turn_timer = (func30*)0x0050EF10;

// testing
func8 *parse_string_OG = (func8 *)0x00625880;
func12 *enemy_capabilities_OG = (func12 *)0x00560DD0;

func16 *tech_rate_OG = (func16 *)0x005BE6B0;
func17 *wants_prototype_OG = (func17 *)0x005BE100;

///
char1032 *stringTemp = (char1032 *)0x009B86A0;
char256 *ParseStrBuffer = (char256 *)0x009BB5E8;

//  ; int 
int *BufferStrHeight = (int *)0x009B3A9C;
int *ParseNumTable = (int *)0x009BB598;
int *ParseStrPlurality = (int *)0x009BB570;
int *ParseStrGender = (int *)0x009BB5C0;
int *GenderDefault = (int *)0x009BBFEC;
int *PluralityDefault = (int *)0x009BBFF0;
int *Language = (int *)0x009BC054;
BOOL *IsLoggingDisabled = (BOOL *)0x009BC004;
BOOL *MultiplayerToggle = (BOOL *)0x0093F660;
int *MsgStatus = (int *)0x009B7B9C;
HWND *HandleMain = (HWND *)0x009B7B28;

uint32_t *UnkBitfield1 = (uint32_t *)0x0090D91C;

Filefind *FilefindPath = (Filefind *)0x009B8198;
MainInterface *MainInterfaceVar = (MainInterface *)0x007AE820;

int __cdecl tester() {
	log_set_state(true);
	log_say("Start test", 0, 0, 0);


	for (int k = 0; k < MaxVehProtoNum; k++) {
		//if (VehPrototype[k].flags & 4) {
			//
		//}
	//	log_say(Players[k / 64].searchKey, VehPrototype[k].vehName, VehPrototype[k].unk1, k, 0);
	}

	for (int i = 0; i < MaxPlayerNum; i++) {
		for (int k = 0; k < MaxVehProtoNum; k++) {

			BOOL val5 = wants_prototype(k, i);
			BOOL val6 = wants_prototype_OG(k, i);
			if (val5 != val6) {
				log_say("wants_prototype error: ", k, i, 0);
				log_say("good : bad:", val6, val5, 0);
			}
		}
		
		/*
		PlayersData[i].techCost = -1;
		uint32_t val3 = tech_rate_OG(i);
		PlayersData[i].techCost = -1;
		uint32_t val4 = tech_rate(i);
		if (val3 != val4) {
			log_say("tech_rate error: ", i, 0, 0);
			log_say("good : bad:", val3, val4, 0);
		}
		*/
	}

	//tech_calc_output();
	/*
	for (int i = 0; i < MaxPlayerNum; i++) {
		log_say(Players[i].searchKey, "PSI Atk", weap_strat(WPN_PSI_ATTACK, i), 0, 0);
		log_say(Players[i].searchKey, "PSI Def", arm_strat(ARM_PSI_DEFENSE, i), 0, 0);
	}
	*/
	/*
	for (int z = 0; z < 10; z++) {
		for (int i = 1; i < MaxPlayerNum; i++) {
			int t1 = PlayersData[i].enemyBestWeaponVal;
			int t2 = PlayersData[i].enemyBestArmorVal;
			int t3 = PlayersData[i].enemyBestLandSpeed;
			int t4 = PlayersData[i].enemyBestPsiAtkVal;
			int t5 = PlayersData[i].enemyBestPsiDefVal;
			int t6 = PlayersData[i].bestLandSpeed;
			int t7 = PlayersData[i].bestPsiDefVal;
			int t8 = PlayersData[i].bestArmorVal;
			int t9 = PlayersData[i].bestPsiAtkVal;
			int t0 = PlayersData[i].bestWeaponVal;
			enemy_capabilities_t(i);
			int a1 = PlayersData[i].enemyBestWeaponVal;
			int a2 = PlayersData[i].enemyBestArmorVal;
			int a3 = PlayersData[i].enemyBestLandSpeed;
			int a4 = PlayersData[i].enemyBestPsiAtkVal;
			int a5 = PlayersData[i].enemyBestPsiDefVal;
			int a6 = PlayersData[i].bestLandSpeed;
			int a7 = PlayersData[i].bestPsiDefVal;
			int a8 = PlayersData[i].bestArmorVal;
			int a9 = PlayersData[i].bestPsiAtkVal;
			int a0 = PlayersData[i].bestWeaponVal;
			PlayersData[i].enemyBestWeaponVal = t1;
			PlayersData[i].enemyBestArmorVal = t2;
			PlayersData[i].enemyBestLandSpeed = t3;
			PlayersData[i].enemyBestPsiAtkVal = t4;
			PlayersData[i].enemyBestPsiDefVal = t5;
			PlayersData[i].bestLandSpeed = t6;
			PlayersData[i].bestPsiDefVal = t7;
			PlayersData[i].bestArmorVal = t8;
			PlayersData[i].bestPsiAtkVal = t9;
			PlayersData[i].bestWeaponVal = t0;
			enemy_capabilities_t(i);
			int b1 = PlayersData[i].enemyBestWeaponVal;
			int b2 = PlayersData[i].enemyBestArmorVal;
			int b3 = PlayersData[i].enemyBestLandSpeed;
			int b4 = PlayersData[i].enemyBestPsiAtkVal;
			int b5 = PlayersData[i].enemyBestPsiDefVal;
			int b6 = PlayersData[i].bestLandSpeed;
			int b7 = PlayersData[i].bestPsiDefVal;
			int b8 = PlayersData[i].bestArmorVal;
			int b9 = PlayersData[i].bestPsiAtkVal;
			int b0 = PlayersData[i].bestWeaponVal;

			if (a1 != b1) {
				log_say("enemyBestWeaponVal error", a1, b1, i);
			}
			if (a2 != b2) {
				log_say("enemyBestArmorVal error", a2, b2, i);
			}
			if (a3 != b3) {
				log_say("enemyBestLandSpeed error", a3, b3, i);
			}
			if (a4 != b4) {
				log_say("enemyBestPsiAtkVal error", a4, b4, i);
			}
			if (a5 != b5) {
				log_say("enemyBestPsiDefVal error", a5, b5, i);
			}
			if (a6 != b6) {
				log_say("bestLandSpeed error", a6, b6, i);
			}
			if (a7 != b7) {
				log_say("bestPsiDefVal error", a7, b7, i);
			}
			if (a8 != b8) {
				log_say("bestArmorVal error", a8, b8, i);
			}
			if (a9 != b9) {
				log_say("bestPsiAtkVal error", a9, b9, i);
			}
			if (a0 != b0) {
				log_say("bestWeaponVal error", a0, b0, i);
			}
			log_say("results: ", i, a1, a2);
			log_say("results: ", i, a3, a4);
			log_say("results: ", i, a5, a6);
			log_say("results: ", i, a7, a8);
			log_say("results: ", i, a9, a0);
		}
	}
	log_say("original start", 0, 0, 0);
	for (int z = 0; z < 10; z++) {
		for (int i = 1; i < MaxPlayerNum; i++) {
			int t1 = PlayersData[i].enemyBestWeaponVal;
			int t2 = PlayersData[i].enemyBestArmorVal;
			int t3 = PlayersData[i].enemyBestLandSpeed;
			int t4 = PlayersData[i].enemyBestPsiAtkVal;
			int t5 = PlayersData[i].enemyBestPsiDefVal;
			int t6 = PlayersData[i].bestLandSpeed;
			int t7 = PlayersData[i].bestPsiDefVal;
			int t8 = PlayersData[i].bestArmorVal;
			int t9 = PlayersData[i].bestPsiAtkVal;
			int t0 = PlayersData[i].bestWeaponVal;
			enemy_capabilities_OG(i);
			int a1 = PlayersData[i].enemyBestWeaponVal;
			int a2 = PlayersData[i].enemyBestArmorVal;
			int a3 = PlayersData[i].enemyBestLandSpeed;
			int a4 = PlayersData[i].enemyBestPsiAtkVal;
			int a5 = PlayersData[i].enemyBestPsiDefVal;
			int a6 = PlayersData[i].bestLandSpeed;
			int a7 = PlayersData[i].bestPsiDefVal;
			int a8 = PlayersData[i].bestArmorVal;
			int a9 = PlayersData[i].bestPsiAtkVal;
			int a0 = PlayersData[i].bestWeaponVal;
			PlayersData[i].enemyBestWeaponVal = t1;
			PlayersData[i].enemyBestArmorVal = t2;
			PlayersData[i].enemyBestLandSpeed = t3;
			PlayersData[i].enemyBestPsiAtkVal = t4;
			PlayersData[i].enemyBestPsiDefVal = t5;
			PlayersData[i].bestLandSpeed = t6;
			PlayersData[i].bestPsiDefVal = t7;
			PlayersData[i].bestArmorVal = t8;
			PlayersData[i].bestPsiAtkVal = t9;
			PlayersData[i].bestWeaponVal = t0;
			enemy_capabilities_OG(i);
			int b1 = PlayersData[i].enemyBestWeaponVal;
			int b2 = PlayersData[i].enemyBestArmorVal;
			int b3 = PlayersData[i].enemyBestLandSpeed;
			int b4 = PlayersData[i].enemyBestPsiAtkVal;
			int b5 = PlayersData[i].enemyBestPsiDefVal;
			int b6 = PlayersData[i].bestLandSpeed;
			int b7 = PlayersData[i].bestPsiDefVal;
			int b8 = PlayersData[i].bestArmorVal;
			int b9 = PlayersData[i].bestPsiAtkVal;
			int b0 = PlayersData[i].bestWeaponVal;

			if (a1 != b1) {
				log_say("enemyBestWeaponVal error", a1, b1, i);
			}
			if (a2 != b2) {
				log_say("enemyBestArmorVal error", a2, b2, i);
			}
			if (a3 != b3) {
				log_say("enemyBestLandSpeed error", a3, b3, i);
			}
			if (a4 != b4) {
				log_say("enemyBestPsiAtkVal error", a4, b4, i);
			}
			if (a5 != b5) {
				log_say("enemyBestPsiDefVal error", a5, b5, i);
			}
			if (a6 != b6) {
				log_say("bestLandSpeed error", a6, b6, i);
			}
			if (a7 != b7) {
				log_say("bestPsiDefVal error", a7, b7, i);
			}
			if (a8 != b8) {
				log_say("bestArmorVal error", a8, b8, i);
			}
			if (a9 != b9) {
				log_say("bestPsiAtkVal error", a9, b9, i);
			}
			if (a0 != b0) {
				log_say("bestWeaponVal error", a0, b0, i);
			}
			log_say("results: ", i, a1, a2);
			log_say("results: ", i, a3, a4);
			log_say("results: ", i, a5, a6);
			log_say("results: ", i, a7, a8);
			log_say("results: ", i, a9, a0);
		}
	}


	/*
	for (int i = 0; i < MaxPlayerNum; i++) {
		for (int j = 0; j < 128; j++) {
			if (PlayersData[i].baseCountByRegion[j]) {
				log_say(Players[i].searchKey, " ? baseCountByRegion ? ", j, PlayersData[i].baseCountByRegion[j], 0);
			}
		}
	}
	
	for (int i = 0; i < MaxPlayerNum; i++) {
		for (int j = 0; j < 128; j++) {
			if (PlayersData[i].unk_81[j]) {
				log_say(Players[i].searchKey, "? unk_81 ?", j, PlayersData[i].unk_81[j], 0);
			}
		}
	}

	for (int i = 0; i < MaxPlayerNum; i++) {
		for (int j = 0; j < 128; j++) {
			if (PlayersData[i].unk_84[j]) {
				//log_say(Players[i].searchKey, "? unk_84 ? ", j, PlayersData[i].unk_84[j], 0);
			}
		}
	}
	
	for (int i = 0; i < MaxPlayerNum; i++) {
		//log_say(Players[i].searchKey, "unk_70 - sea?", PlayersData[i].unk_70, 0, 0);
		//log_say(Players[i].searchKey, "unk_71", PlayersData[i].unk_72, 0, 0);
		log_say(Players[i].searchKey, " ? unk_26 | earnedTechsSaved | techRanking ? ", PlayersData[i].unk_26, PlayersData[i].earnedTechsSaved, PlayersData[i].techRanking);

		//log_say(Players[i].searchKey, "unk_48 - base support sum", PlayersData[i].unk_48, 0, 0);
		//log_say(Players[i].searchKey, "? unk_49 - 4xecon dmg sum ? ", PlayersData[i].unk_49, 0, 0);
		//log_say(Players[i].searchKey, "unk_48 - support", PlayersData[i].unk_48, 0, 0);
	}
	*/
	/*
	for (int i = 0; i < MaxBaseNum; i++) {
		char szTemp[512];
		szTemp[0] = 0;
		strcat_s(szTemp, " - ");
		strcat_s(szTemp, Players[Base[i].factionIDCurrent].nounFaction);
		strcat_s(szTemp, " > unk3 > ");
		log_say(Base[i].nameString, szTemp, Base[i].unk3, 0, 0);
		/*
		szTemp[0] = 0;
		strcat_s(szTemp, " - ");
		strcat_s(szTemp, Players[Base[i].factionIDCurrent].nounFaction);
		strcat_s(szTemp, " - unk4 - ");
		log_say(Base[i].nameString, szTemp, Base[i].unk4, 0, 0);
		szTemp[0] = 0;
		strcat_s(szTemp, " - ");
		strcat_s(szTemp, Players[Base[i].factionIDCurrent].nounFaction);
		strcat_s(szTemp, " - unk5 - ");
		log_say(Base[i].nameString, szTemp, Base[i].unk5, 0, 0);
		*/
	//}
	

	//

	log_say("End test", 0, 0, 0);
	log_set_state(false);
	/*
	for (uint32_t y = 0; y < *MapVerticalBounds; y++) {
		for (uint32_t x = y & 1; x < *MapHorizontalBounds; x += 2) {
			uint8_t att = abstract_at(x, y);
			if (att) {
				log_say("abstract_at: ", x, y, att);
				//log_say("region:      ", x, y, region_at(x,y));
			}
			
			
			//if (goody) {
			//	log_say("goody_at: ", x, y, goody);
			//}
			/*
			uint32_t bit = bit_at(x, y);
			if (bit & BIT_UNK_4000000) {
				log_say("found BIT_UNK_4000000: ", x, y, bit);
			}
			if (bit & BIT_UNK_8000000) {
				log_say("found BIT_UNK_8000000: ", x, y, bit);
			}
			/*
			for (uint32_t alt = 0; alt < 8; alt++) {
				map_loc(x, y)->val1 &= 0x1F;
				map_loc(x, y)->val1 |= alt << 5;
				uint32_t min = minerals_at_OG(x, y);
				if (minerals_at(x, y) != min) {
					log_say("minerals_at error: ", x, y, min);
				}
			}
			*/
			//for (int f = 0; f < MaxPlayerNum; f++) {
				//
			//}
			//int goody = goody_at_OG(x, y);
			//if (bonus != bonus_at_OG(x, y, 0)) {
			//	MessageBoxA(NULL, "bonus_at Error", "FATAL ERROR", MB_ICONWARNING);
			//}
			//if (goody) {
			//	log_say("goody_at: ", x, y, goody);
			//}
			/*
			uint32_t bit = bit_at(x, y);
			
			
			if (bit & BIT_UNK_200) {
				log_say("BIT_UNK_200: ", x, y, bit);
			}
			if (bit & BIT_UNK_4000) {
				log_say("BIT_UNK_4000: ", x, y, bit);
			}
			//if (bit & BIT_SUPPLY_REMOVE) {
			//	log_say("BIT_SUPPLY_REMOVE: ", x, y, bit);
			//}
			if (bit & BIT_UNK_2000000) {
				log_say("BIT_UNK_2000000: ", x, y, bit);
			}
			if (bit & BIT_UNK_4000000) {
				log_say("BIT_UNK_4000000: ", x, y, bit);
			}
			if (bit & BIT_UNK_8000000) {
				log_say("BIT_UNK_8000000: ", x, y, bit);
			}
			//if (bit & BIT_UNK_40000000) {
			//	log_say("BIT_UNK_40000000: ", x, y, bit);
			//}
			*/
		//}
	//}
	
	/*
	for (int y = -10; y < (int)*MapVerticalBounds; y++) {
		for (int x = -1; x < (int)*MapHorizontalBounds; x++) {
			for (int f = -1; f < 10; f++) {
				int FMTemp = *MapFlatToggle;
				for (int t = 0; t < 2; t++) {
					*MapFlatToggle = t;
					BOOL MPtemp = *MultiplayerToggle;
					for (int m = 0; m < 2; m++) {
						*MultiplayerToggle = m;
						if (zoc_any_OG(x, y, f) != zoc_any(x, y, f)) {
							MessageBoxA(NULL, "zoc_any Error", "FATAL ERROR", MB_ICONWARNING);
						}
						if (zoc_veh_OG(x, y, f) != zoc_veh(x, y, f)) {
							MessageBoxA(NULL, "zoc_veh Error", "FATAL ERROR", MB_ICONWARNING);
						}
						if (zoc_sea_OG(x, y, f) != zoc_sea(x, y, f)) {
							MessageBoxA(NULL, "zoc_sea Error", "FATAL ERROR", MB_ICONWARNING);
						}
						if (zoc_move_OG(x, y, f) != zoc_move(x, y, f)) {
							MessageBoxA(NULL, "zoc_move Error", "FATAL ERROR", MB_ICONWARNING);
						}
					}
					*MultiplayerToggle = MPtemp;
				}
				*MapFlatToggle = FMTemp;
			}
		}
	}
	*/
	return 0;
}

// tech_val output
func13 *tech_val_OG = (func13 *)0x005BCBE0;
void tech_calc_output() {
	for (int i = 0; i < MaxPlayerNum; i++) {
		for (int j = 0; j < MaxTechnologyNum; j++) {
			for (int k = 0; k < 2; k++) {
				int techVal1 = tech_val(j, i, k);
				int techVal2 = tech_val_OG(j, i, k);
				if (techVal1 != techVal2) {
					log_say("tech_val error: ", j, i, k);
					log_say("good:", techVal2, 0, 0);
					log_say("bad: ", techVal1, 0, 0);
				}
				else {
					log_say(Players[i].searchKey, Technology[j].name, techVal1, k, 0);
				}
			}
		}
		for (int j = 89; j < 609; j++) {
			int techVal1 = tech_val(j, i, 0);
			int techVal2 = tech_val_OG(j, i, 0);
			if (techVal1 != techVal2) {
				log_say("tech_val error: ", j, i, 0);
				if (j < 97) {
					log_say(Players[i].searchKey, Players[j - 89].searchKey, techVal1, techVal2, 0);
				}
				else {
					log_say(Players[i].searchKey, VehPrototype[j - 97].vehName, techVal1, techVal2, 0);
				}
			}
			else {
				if (j < 97) {
					log_say(Players[i].searchKey, Players[j - 89].searchKey, techVal1, 0, 0);
				}
				else {
					log_say(Players[i].searchKey, VehPrototype[j - 97].vehName, techVal1, 0, 0);
				}
			}
		}
	}
}


// 005FCA30
BOOL __cdecl do_non_input() {
	do_video();
	check_net();
	do_net();
	MSG Msg;
	/*
	DWORD *msgTime = &Msg.time;
	for (int i = 3; i > 0; i--) {
		*msgTime = -1;
		msgTime += 7;
	}
	*/
	if (!PeekMessage(&Msg, 0, WM_NULL, WM_INPUT, PM_NOREMOVE) 
		|| !PeekMessage(&Msg, 0, WM_UNICHAR, WM_KEYDOWN | WM_INPUT, PM_NOREMOVE)
		|| !PeekMessage(&Msg, 0, WM_MOUSELAST, 0xFFFF, PM_NOREMOVE)) {
		return false;
	}
	/*
	DWORD *msgTime2 = &Msg.time;
	for (uint32_t i = 1; i < 4; i++) {

	}
	*/
	// PeekMessage
	TranslateMessage(&Msg);
	DispatchMessage(&Msg);
	return true;
}

// 005FCB20
void __cdecl do_all_non_input() {
	do {
		*MsgStatus = 32;
	} while (do_non_input());
	*MsgStatus = 0;
	do_net();
	check_net();
}

BOOL __cdecl do_draw() {
	return false;
}

void __cdecl do_all_draws() {
	//
}

BOOL __cdecl do_keyboard() {
	return false;
}

void __cdecl do_all_keyboard() {
	//
}
