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
#include "faction.h"
#include "alpha.h"
#include "base.h"
#include "council.h"
#include "game.h"
#include "map.h"
#include "probe.h"
#include "technology.h"
#include "veh.h"

player *Players = (player *)0x00946A50;
player_data *PlayersData = (player_data *)0x0096C9E0;
faction_art *FactionArt = (faction_art *)0x0078E978;
rules_social_category *SocialCategory = (rules_social_category *)0x0094B000;
rules_social_effect *SocialEffect = (rules_social_effect *)0x00946580;
LPSTR *Mood = (LPSTR *)0x0094C9E4;
LPSTR *Repute = (LPSTR *)0x00946A30;
rules_might *Might = (rules_might *)0x0094C558;
rules_bonusname *BonusName = (rules_bonusname *)0x009461A8;
uint8_t *FactionsStatus = (uint8_t *)0x009A64E8;

/*
Purpose: Check whether specified faction is a human player or computer controlled AI.
Original Offset: n/a
Return Value: Is faction a human? true/false
Status: Complete
*/
BOOL __cdecl is_human(uint32_t factionID) {
	return FactionsStatus[0] & (1 << factionID);
}

/*
Purpose: Check whether specified faction is alive or whether they've been eliminated.
Original Offset: n/a
Return Value: Is faction alive? true/false
Status: Complete
*/
BOOL __cdecl is_alive(uint32_t factionID) {
	return FactionsStatus[1] & (1 << factionID);
}

/*
Purpose: Get Player's faction name adjective.
Original Offset: 0050B910
Return Value: Faction name adjective
Status: Complete
*/
LPSTR __cdecl get_adjective(int factionID) {
	return Players[factionID].adjNameFaction;
}

/*
Purpose: Get Player's faction noun.
Original Offset: 0050B930
Return Value: Faction noun
Status: Complete
*/
LPSTR __cdecl get_noun(int factionID) {
	parse_set(Players[factionID].nounGender, Players[factionID].isNounPlural);
	return Players[factionID].nounFaction;
}

/*
Purpose: Determine whether automatic contact is enabled for net or PBEM games.
Original Offset: 00539160
Return Value: Is always contact enabled? true/false
Status: Complete
*/
BOOL __cdecl auto_contact() {
	if (*IsMultiplayerNet && Rules->TglHumanAlwaysContactNet) {
		return true;
	}
	return *IsMultiplayerPBEM && Rules->TglHumansAlwaysContactPBEM;
}

/*
Purpose: Check whether specified faction is nearing the diplomatic victory requirements to be able
		 to call a Supreme Leader vote. Optional 2nd parameter (0/-1 to disable) specifies a faction
		 to skip if they have pact with faction from 1st parameter.
Original Offset: 00539D40
Return Value: factionID nearing diplomatic victory or zero
Status: Complete
*/
uint32_t __cdecl aah_ooga(int factionID, int pactFactionID) {
	if (!(*GameRules & RULES_VICTORY_DIPLOMATIC)) {
		return 0; // Diplomatic Victory not allowed
	}
	uint32_t votesTotal = 0;
	for (uint32_t i = 1; i < MaxPlayerNum; i++) {
		votesTotal += council_votes(i);
	}
	uint32_t factionIDRet = 0;
	for (int i = 1; i < MaxPlayerNum; i++) {
		if (i != pactFactionID
			&& (pactFactionID <= 0 || !(PlayersData[i].diploTreaties[pactFactionID] & DTREATY_PACT)
				|| !(*GameRules & RULES_VICTORY_COOPERATIVE))) {
			int proposalPreq = Proposal[PROP_UNITE_SUPREME_LEADER].preqTech;
			if ((has_tech(proposalPreq, factionID)
				|| (proposalPreq >= 0 && (has_tech(Technology[proposalPreq].preqTech1, factionID)
					|| has_tech(Technology[proposalPreq].preqTech2, factionID))))
				&& council_votes(i) * 2 >= votesTotal && (!factionIDRet || i == factionID)) {
				factionIDRet = i;
			}
		}
	}
	return factionIDRet;
}

/*
Purpose: Human controlled player nearing endgame.
Original Offset: 00539E40
Return Value: Is human player nearing endgame? true/false
Status: Complete
*/
BOOL __cdecl climactic_battle() {
	for (uint32_t i = 1; i < MaxPlayerNum; i++) {
		if (is_human(i) && PlayersData[i].cornerMarketTurn > * TurnCurrentNum) {
			return true; // Human controlled player initiated Corner Global Energy Market
		}
	}
	if (aah_ooga(0, -1)) { // nearing Supreme Leader, these parameters will always return false
		return true; // TODO: Revisit in future once more end game code is complete. This may have
					 //       been effectively disabled as a design decision rather than a bug.
	}
	if (ascending(0)) {
		for (uint32_t i = 1; i < MaxPlayerNum; i++) {
			if (is_human(i) && (has_tech(Facility[FAC_PSI_GATE].preqTech, i)
				|| has_tech(Facility[FAC_VOICE_OF_PLANET].preqTech, i))) {
				return true; // Human controlled player has tech to build PSI Gates or VoP
			}
		}
	}
	return false;
}

/*
Purpose: Determine ideal unit count to protect faction's bases in the specified land region.
Original Offset: 00560D50
Return Value: Amount of non-offensive units needed to guard region
Status: Complete
*/
uint32_t __cdecl guard_check(uint32_t factionID, uint32_t region) {
	if (region >= MaxRegionLandNum) {
		return 0;
	}
	int factor = 2 - PlayersData[factionID].AI_Fight;
	uint32_t planRegion = PlayersData[factionID].regionBasePlan[region];
	if (planRegion == PLAN_COLONIZATION) {
		factor += 2;
	}
	else if (planRegion == PLAN_DEFENSIVE) {
		factor = 1; // 1-1 unit per base ratio
	}
	if (PlayersData[factionID].playerFlags & PFLAG_STRAT_DEF_OBJECTIVES) {
		factor = 1; // 1-1 unit per base ratio
	}
	return (PlayersData[factionID].regionTotalBases[region] + factor - 1) / factor;
}

/*
Purpose: Add the specific goal to the faction's goals for the specified tile. Optional baseID param.
Original Offset: 00579A30
Return Value: n/a
Status:  Complete
*/
void __cdecl add_goal(uint32_t factionID, int type, int priority, int xCoord, int yCoord,
	int baseID) {
	if (!on_map(xCoord, yCoord)) {
		return;
	}
	for (int i = 0; i < MaxGoalsNum; i++) {
		goal &goals = PlayersData[factionID].goals[i];
		if (goals.xCoord == xCoord && goals.yCoord == yCoord && goals.type == type) {
			if (goals.priority <= priority) {
				goals.priority = (int16_t)priority;
			}
			return;
		}
	}
	int prioritySearch = 0, goalID = -1;
	for (int i = 0; i < MaxGoalsNum; i++) {
		goal &goals = PlayersData[factionID].goals[i];
		int typeCmp = goals.type, prirotyCmp = goals.priority;
		if (typeCmp < 0 || prirotyCmp < priority) {
			int cmp = typeCmp >= 0 ? 0 : 1000;
			if (!cmp) {
				cmp = prirotyCmp > 0 ? 20 - prirotyCmp : prirotyCmp + 100;
			}
			if (cmp > prioritySearch) {
				prioritySearch = cmp;
				goalID = i;
			}
		}
	}
	if (goalID >= 0) {
		goal &goals = PlayersData[factionID].goals[goalID];
		goals.type = (int16_t)type;
		goals.priority = (int16_t)priority;
		goals.xCoord = xCoord;
		goals.yCoord = yCoord;
		goals.baseID = baseID;
	}
}

/*
Purpose: Add the specific site to the faction's site goals for the specified tile.
Original Offset: 00579B70
Return Value: n/a
Status: Complete
*/
void __cdecl add_site(uint32_t factionID, int type, int priority, int xCoord, int yCoord) {
	if ((xCoord ^ yCoord) & 1 && *GameState & STATE_DEBUG_MODE) {
		danger("Bad SITE", "", xCoord, yCoord, type);
	}
	for (int i = 0; i < MaxSitesNum; i++) {
		goal &sites = PlayersData[factionID].sites[i];
		if (sites.xCoord == xCoord && sites.yCoord == yCoord && sites.type == type) {
			if (sites.priority <= priority) {
				sites.priority = (int16_t)priority;
			}
			return;
		}
	}
	int prioritySearch = 0, siteID = -1;
	for (int i = 0; i < MaxSitesNum; i++) {
		goal &sites = PlayersData[factionID].sites[i];
		int typeCmp = sites.type, prirotyCmp = sites.priority;
		if (typeCmp < 0 || prirotyCmp < priority) {
			int cmp = typeCmp >= 0 ? 0 : 1000;
			if (!cmp) {
				cmp = 20 - prirotyCmp;
			}
			if (cmp > prioritySearch) {
				prioritySearch = cmp;
				siteID = i;
			}
		}
	}
	if (siteID >= 0) {
		goal &sites = PlayersData[factionID].sites[siteID];
		sites.type = (int16_t)type;
		sites.priority = (int16_t)priority;
		sites.xCoord = xCoord;
		sites.yCoord = yCoord;
		add_goal(factionID, type, priority, xCoord, yCoord, -1);
	}
}

/*
Purpose: Check if a goal exists at the tile for the specified faction and type.
Original Offset: 00579CC0
Return Value: Does specific goal exist for faction at tile? true/false
Status: Complete
*/
BOOL __cdecl at_goal(uint32_t factionID, int type, int xCoord, int yCoord) {
	for (int i = 0; i < MaxGoalsNum; i++) {
		goal &goals = PlayersData[factionID].goals[i];
		if (goals.xCoord == xCoord && goals.yCoord == yCoord && goals.type == type) {
			return true;
		}
	}
	return false;
}

/*
Purpose: Check if a site exists at the tile for the specified faction and type.
Original Offset: 00579D20
Return Value: Does specific site exist for faction at tile? true/false
Status: Complete
*/
BOOL __cdecl at_site(uint32_t factionID, int type, int xCoord, int yCoord) {
	for (int i = 0; i < MaxSitesNum; i++) {
		goal &sites = PlayersData[factionID].sites[i];
		if (sites.xCoord == xCoord && sites.yCoord == yCoord && sites.type == type) {
			return true;
		}
	}
	return false;
}

/*
Purpose: Wipe all goals for the specified faction then recreate any active site related goals.
Original Offset: 00579D80
Return Value: n/a
Status: Complete
*/
void __cdecl wipe_goals(uint32_t factionID) {
	for (int i = 0; i < MaxGoalsNum; i++) {
		goal &goals = PlayersData[factionID].goals[i];
		int16_t priority = goals.priority;
		if (priority < 0) {
			goals.type = AI_GOAL_UNUSED;
		}
		else {
			goals.priority = -priority;
		}
	}
	for (int i = 0; i < MaxSitesNum; i++) {
		goal &sites = PlayersData[factionID].sites[i];
		int16_t type = sites.type;
		if (type >= 0) {
			add_goal(factionID, type, sites.priority, sites.xCoord, sites.yCoord, -1);
		}
	}
}

/*
Purpose: Initialize all goals for the specified faction.
Original Offset: 00579E00
Return Value: n/a
Status: Complete
*/
void __cdecl init_goals(uint32_t factionID) {
	for (int i = 0; i < MaxGoalsNum; i++) {
		goal &goals = PlayersData[factionID].goals[i];
		goals.type = -1;
		goals.priority = 0;
		goals.xCoord = 0;
		goals.yCoord = 0;
		goals.baseID = 0;
	}
	for (int i = 0; i < MaxSitesNum; i++) {
		goal &sites = PlayersData[factionID].sites[i];
		sites.type = -1;
		sites.priority = 0;
		sites.xCoord = 0;
		sites.yCoord = 0;
		sites.baseID = 0;
	}
}

/*
Purpose: Delete sites of the specified type within proximity of the tile along with related goals.
Original Offset: 00579E70
Return Value: n/a
Status: Complete
*/
void __cdecl del_site(uint32_t factionID, int type, int xCoord, int yCoord, int proximity) {
	for (int i = 0; i < MaxSitesNum; i++) {
		goal &sites = PlayersData[factionID].sites[i];
		if (sites.type == type) {
			int dist = vector_dist(xCoord, yCoord, sites.xCoord, sites.yCoord);
			if (dist <= proximity) {
				sites.type = AI_GOAL_UNUSED;
				sites.priority = 0;
				for (int j = 0; j < MaxGoalsNum; j++) {
					goal &goalCompare = PlayersData[factionID].goals[j];
					if (goalCompare.xCoord == sites.xCoord && goalCompare.yCoord == sites.yCoord &&
						goalCompare.type == type) {
						goalCompare.type = AI_GOAL_UNUSED;
					}
				}
			}
		}
	}
}

/*
Purpose: Calculate the cost for the faction to corner the Global Energy Market (Economic Victory).
Original Offset: 0059EE50
Return Value: Cost to corner the Global Energy Market
Status: Complete
*/
uint32_t __cdecl corner_market(uint32_t factionID) {
	int cost = 0;
	for (int i = 0; i < *BaseCurrentCount; i++) {
		uint32_t targetFactionID = Base[i].factionIDCurrent;
		if (targetFactionID != factionID) {
			uint32_t treaties = PlayersData[targetFactionID].diploTreaties[factionID];
			if (!(treaties & DTREATY_PACT) || !(treaties & DTREATY_HAVE_SURRENDERED)) {
				cost += mind_control(i, factionID, true);
			}
		}
	}
	return (cost < 1000) ? 1000 : cost;
}

/*
Purpose: Validate whether each faction meets the requirements to have the Map revealed. Added some
		 minor tweaks to improve performance.
Original Offset: 005A96D0
Return Value: n/a
Status: Complete
*/
void __cdecl see_map_check() {
	for (int factionID = 1; factionID < MaxPlayerNum; factionID++) {
		PlayersData[factionID].playerFlags &= ~PFLAG_MAP_REVEALED;
		uint32_t *satellites = &PlayersData[factionID].satellitesNutrient;
		for (int i = 0; i < 4; i++, satellites++) {
			if (*satellites) {
				PlayersData[factionID].playerFlags |= PFLAG_MAP_REVEALED;
				break; // end satellite loop once set
			}
		}
		if (!(PlayersData[factionID].playerFlags & PFLAG_MAP_REVEALED)) { // skip Tech check if set
			for (int techID = 0; techID < MaxTechnologyNum; techID++) {
				if (Technology[techID].flags & TFLAG_REVEALS_MAP && has_tech(techID, factionID)) {
					PlayersData[factionID].playerFlags |= PFLAG_MAP_REVEALED;
				}
			}
		}
	}
}

/*
Purpose: Calculate the social engineering effect modifiers for the specified faction.
Original Offset: 005B4210
Return Value: n/a
Status: Complete - testing
*/
void __cdecl social_calc(social_category *category, social_effect *effect, uint32_t factionID, 
	BOOL UNUSED(tgl1), BOOL isQuickCalc) {
	ZeroMemory(effect, sizeof(social_effect));
	BOOL hasCloningVatSP = has_project(SP_CLONING_VATS, factionID);
	BOOL hasNetBackboneSP = has_project(SP_NETWORK_BACKBONE, factionID);
	for (int i = 0; i < MaxSocialCatNum; i++) {
		uint32_t modelVal = *(&category->politics + i);
		for (int j = 0; j < MaxSocialEffectNum; j++) {
			int effectVal = *(&SocialCategory[i].modelEffect[modelVal].economy + j);
			if (effectVal < 0) {
				if ((i == 3 && j == 1 && hasNetBackboneSP) ||
					(((i == 3 && j == 3) || (i == 2 && j == 1)) && hasCloningVatSP)) {
					effectVal = 0;
				}
				else {
					for (int k = 0; k < Players[factionID].factionBonusCount; k++) {
						if (Players[factionID].factionBonusVal1[k] == i
							&& Players[factionID].factionBonusVal2[k] == j) {
							if (Players[factionID].factionBonusID[i] == RULE_IMPUNITY) {
								*(&effect->economy + j) -= effectVal;
							}
							else if (Players[factionID].factionBonusID[i] == RULE_PENALTY) {
								*(&effect->economy + j) += effectVal;
							}
						}
					}
				}
			}
			*(&effect->economy + j) += effectVal;
		}
	}
	if (!isQuickCalc) {
		if (has_project(SP_ASCETIC_VIRTUES, factionID)) {
			effect->police++;
		}
		if (has_project(SP_LIVING_REFINERY, factionID)) {
			effect->support += 2;
		}
		if (has_temple(factionID)) {
			effect->planet++;
			if (Players[factionID].ruleFlags & RFLAG_ALIEN) {
				effect->research++; // undocumented bonus for ALIEN faction
			}
		}
		int count = 11;
		social_effect *effectCalc = effect, *effectBase = &PlayersData[factionID].socEffectBase;
		do {
			effectCalc->economy += effectBase->economy;
			effectCalc++;
			effectBase++;
			--count;
		} while (count);

		for (int i = 0; i < Players[factionID].factionBonusCount; i++) {
			if (Players[factionID].factionBonusID[i] == RULE_IMMUNITY) {
				*(&effect->economy + Players[factionID].factionBonusVal1[i]) 
					= range(*(&effect->economy + Players[factionID].factionBonusVal1[i]), 0, 999);
			}
			else if (Players[factionID].factionBonusID[i] == RULE_ROBUST) {
				int effectVal = *(&effect->economy + Players[factionID].factionBonusVal1[i]);
				if (effectVal < 0) {
					*(&effect->economy + Players[factionID].factionBonusVal1[i]) = effectVal / 2;
				}
			}
		}
	}
}

/*
Purpose: Handle the social engineering turn upkeep for the specified faction.
Original Offset: 005B44D0
Return Value: n/a
Status: Complete - testing
*/
void __cdecl social_upkeep(uint32_t factionID) {
	for (int i = 0; i < MaxSocialCatNum; i++) {
		*(&PlayersData[factionID].socCategoryActive.politics + i) =
			*(&PlayersData[factionID].socCategoryPending.politics + i);
	}
	social_calc(&PlayersData[factionID].socCategoryPending, 
		&PlayersData[factionID].socEffectPending, factionID, false, false);
	social_calc(&PlayersData[factionID].socCategoryPending,
		&PlayersData[factionID].socEffectActive, factionID, false, false);
	social_calc(&PlayersData[factionID].socCategoryPending,
		&PlayersData[factionID].socEffectTemp, factionID, true, false);
	PlayersData[factionID].socUpheavalCostPaid = 0;
}

/*
Purpose: Calculate the cost of social upheaval for the specified faction.
Original Offset: 005B4550
Return Value: Social upheaval cost
Status: Complete
*/
uint32_t __cdecl social_upheaval(uint32_t factionID, social_category *categoryNew) {
	uint32_t changeCount = 0;
	for (int i = 0; i < MaxSocialCatNum; i++) {
		if (*(&categoryNew->politics + i) != 
			*(&PlayersData[factionID].socCategoryActive.politics + i)) {
			changeCount++;
		}
	}
	if (!changeCount) {
		return 0;
	}
	changeCount++;
	uint32_t diffLvl = is_human(factionID) ? PlayersData[factionID].diffLevel : DLVL_LIBRARIAN;
	uint32_t cost = changeCount * changeCount * changeCount * diffLvl;
	if (Players[factionID].ruleFlags & RFLAG_ALIEN) {
		cost += cost / 2;
	}
	return cost;
}

/*
Purpose: Check to see whether provided faction can utilize a specific social category and model.
Original Offset: 005B4730
Return Value: Is social category/model available to faction? true/false
Status: Complete
*/
BOOL __cdecl society_avail(int socCategory, int socModel, int factionID) {
	if (Players[factionID].socAntiIdeologyCategory == socCategory
		&& Players[factionID].socAntiIdeologyModel == socModel) {
		return false;
	}
	return has_tech(SocialCategory[socCategory].preqTech[socModel], factionID);
}

/*
Purpose: Calculate social engineering for AI.
Original Offset: 005B4790
Return Value: n/a
Status: wip
*/
void __cdecl social_ai(uint32_t factionID, int tgl1, int tgl2, int tgl3, int tgl4, BOOL tgl5) {
	if (!tgl5) {
		if (is_human(factionID)) {
			return;
		}
	}
	else if (tgl1 < 0) {
		return;
	}
}

/*
Purpose: Calculate specified faction's best available weapon and armor ratings as well as the
		 fastest moving ground Veh chassis. Compare these capabilities to faction's best opponent
		 capabilities based on current diplomacy.
Original Offset: 00560DD0
Return Value: n/a
Status: Complete - testing / WIP
*/
void __cdecl enemy_capabilities(uint32_t factionID) {
	BOOL hasWorms = veh_avail(BSC_MIND_WORMS, factionID, -1);
	PlayersData[factionID].bestPsiOffense = hasWorms ? weap_strat(WPN_PSI_ATTACK, factionID) : 0;
	PlayersData[factionID].bestWeaponValue = 1;
	for (int i = 0; i < MaxWeaponNum; i++) {
		if (has_tech(Weapon[i].preqTech, factionID) && Weapon[i].offenseRating < 99) {
			int weapVal = weap_strat(i, factionID);
			if (Weapon[i].offenseRating < 0 && weapVal > PlayersData[factionID].bestPsiOffense) {
				PlayersData[factionID].bestPsiOffense = weapVal;
			}
			if (weapVal > PlayersData[factionID].bestWeaponValue) {
				PlayersData[factionID].bestWeaponValue = weapVal;
			}
		}
	}
	PlayersData[factionID].bestPsiDefense = hasWorms ? arm_strat(ARM_PSI_DEFENSE, factionID) : 0;
	PlayersData[factionID].bestArmorValue = 1;
	for (int i = 0; i < MaxArmorNum; i++) {
		if (has_tech(Armor[i].preqTech, factionID)) {
			int armVal = arm_strat(i, factionID);
			if (Armor[i].defenseRating < 0 && armVal > PlayersData[factionID].bestPsiDefense) {
				PlayersData[factionID].bestPsiDefense = armVal;
			}
			if (armVal > PlayersData[factionID].bestArmorValue) {
				PlayersData[factionID].bestArmorValue = armVal;
			}
		}
	}
	PlayersData[factionID].bestLandSpeed = 1;
	for (int i = 0; i < MaxChassisNum; i++) {
		if (has_tech(Chassis[i].preqTech, factionID) && Chassis[i].triad == TRIAD_LAND) {
			if (Chassis[i].speed > PlayersData[factionID].bestLandSpeed) {
				PlayersData[factionID].bestLandSpeed = Chassis[i].speed;
			}
		}
	}
	PlayersData[factionID].enemyBestWeaponValue = 0;
	PlayersData[factionID].enemyBestArmorValue = 0;
	PlayersData[factionID].enemyBestLandSpeed = 0;
	PlayersData[factionID].enemyBestPsiOffense = 0;
	PlayersData[factionID].enemyBestPsiDefense = 0;
	for (int i = 0; i < 4 && !PlayersData[factionID].enemyBestWeaponValue; i++) {
		// 1st pass: vendetta, no treaty, has commlink
		// 2nd pass: no treaty, has commlink
		// 3rd pass: has commlink
		// 4th pass: any non-pact faction
		for (uint32_t j = 1, treaties; j < MaxPlayerNum; j++) {
			if (j != factionID
				&& (treaties = PlayersData[i].diploTreaties[j], !(treaties & DTREATY_PACT))
				&& ((!i && treaties & DTREATY_VENDETTA && !(treaties & DTREATY_TREATY)
					&& treaties & DTREATY_COMMLINK)
					|| (i == 1 && !(treaties & DTREATY_TREATY) && treaties & DTREATY_COMMLINK)
					|| (i == 2 && treaties & DTREATY_COMMLINK) || (i == 3))) {
				if (PlayersData[factionID].enemyBestWeaponValue < PlayersData[j].bestWeaponValue) {
					PlayersData[factionID].enemyBestWeaponValue = PlayersData[j].bestWeaponValue;
				}
				if (PlayersData[factionID].enemyBestArmorValue < PlayersData[j].bestArmorValue) {
					PlayersData[factionID].enemyBestArmorValue = PlayersData[j].bestArmorValue;
				}
				if (PlayersData[factionID].enemyBestLandSpeed < PlayersData[j].bestLandSpeed) {
					PlayersData[factionID].enemyBestLandSpeed = PlayersData[j].bestLandSpeed;
				}
				if (PlayersData[factionID].enemyBestPsiOffense < PlayersData[j].bestPsiOffense) {
					PlayersData[factionID].enemyBestPsiOffense = PlayersData[j].bestPsiOffense;
				}
				if (PlayersData[factionID].enemyBestPsiDefense < PlayersData[j].bestPsiDefense) {
					PlayersData[factionID].enemyBestPsiDefense = PlayersData[j].bestPsiDefense;
				}
			}
		}
	}
}

void __cdecl enemy_capabilities_t(uint32_t factionID) {
	// * PSI could potentially be best weapon?
	// * PSI should always be last Weapon
	// * faction order will affect initial run through
	// > potential fix would be to calc all factions at once before enemy best compares

	//BOOL hasWorms = veh_avail(BSC_MIND_WORMS, factionID, -1);
	//PlayersData[factionID].bestPsiAtkVal = hasWorms ? weap_strat(WPN_PSI_ATTACK, factionID) : 0;
	PlayersData[factionID].bestWeaponValue = 1;
	for (int i = 0; i < MaxWeaponNum; i++) {
		if (has_tech(Weapon[i].preqTech, factionID) && Weapon[i].offenseRating < 99) {
			int weapVal = weap_strat(i, factionID);
			//if (Weapon[i].offenseRating < 0 && weapVal > PlayersData[factionID].bestPsiAtkVal) {
			//	PlayersData[factionID].bestPsiAtkVal = weapVal;
			//}
			if (Weapon[i].offenseRating >= 0 && weapVal > PlayersData[factionID].bestWeaponValue) {
				PlayersData[factionID].bestWeaponValue = weapVal;
			}
		}
	}
	//PlayersData[factionID].bestPsiDefVal = hasWorms ? arm_strat(ARM_PSI_DEFENSE, factionID) : 0;
	PlayersData[factionID].bestArmorValue = 1;
	for (int i = 0; i < MaxArmorNum; i++) {
		if (has_tech(Armor[i].preqTech, factionID)) {
			int armVal = arm_strat(i, factionID);
			//if (Armor[i].defenseRating < 0 && armVal > PlayersData[factionID].bestPsiDefVal) {
			//	PlayersData[factionID].bestPsiDefVal = armVal;
			//}
			if (Armor[i].defenseRating >= 0 && armVal > PlayersData[factionID].bestArmorValue) {
				PlayersData[factionID].bestArmorValue = armVal;
			}
		}
	}
	PlayersData[factionID].bestLandSpeed = 1;
	for (int i = 0; i < MaxChassisNum; i++) {
		if (has_tech(Chassis[i].preqTech, factionID) && Chassis[i].triad == TRIAD_LAND) {
			if (Chassis[i].speed > PlayersData[factionID].bestLandSpeed) {
				PlayersData[factionID].bestLandSpeed = Chassis[i].speed;
			}
		}
	}
	PlayersData[factionID].enemyBestWeaponValue = 0;
	PlayersData[factionID].enemyBestArmorValue = 0;
	PlayersData[factionID].enemyBestLandSpeed = 0;
	//PlayersData[factionID].enemyBestPsiAtkVal = 0;
	//PlayersData[factionID].enemyBestPsiDefVal = 0;
	for (int i = 0; i < 4 && !PlayersData[factionID].enemyBestWeaponValue; i++) {
		// 1st pass: vendetta, no treaty, has commlink
		// 2nd pass: no treaty, has commlink
		// 3rd pass: has commlink
		// 4th pass: any non-pact faction
		for (uint32_t j = 1, treaties; j < MaxPlayerNum; j++) {
			if (j != factionID
				&& (treaties = PlayersData[i].diploTreaties[j], !(treaties & DTREATY_PACT))
				&& ((!i && treaties & DTREATY_VENDETTA && !(treaties & DTREATY_TREATY)
					&& treaties & DTREATY_COMMLINK)
					|| (i == 1 && !(treaties & DTREATY_TREATY) && treaties & DTREATY_COMMLINK)
					|| (i == 2 && treaties & DTREATY_COMMLINK) || (i == 3))) {
				if (PlayersData[factionID].enemyBestWeaponValue < PlayersData[j].bestWeaponValue) {
					PlayersData[factionID].enemyBestWeaponValue = PlayersData[j].bestWeaponValue;
				}
				if (PlayersData[factionID].enemyBestArmorValue < PlayersData[j].bestArmorValue) {
					PlayersData[factionID].enemyBestArmorValue = PlayersData[j].bestArmorValue;
				}
				if (PlayersData[factionID].enemyBestLandSpeed < PlayersData[j].bestLandSpeed) {
					PlayersData[factionID].enemyBestLandSpeed = PlayersData[j].bestLandSpeed;
				}
				/*
				if (PlayersData[factionID].enemyBestPsiAtkVal < PlayersData[j].bestPsiAtkVal) {
					PlayersData[factionID].enemyBestPsiAtkVal = PlayersData[j].bestPsiAtkVal;
				}
				if (PlayersData[factionID].enemyBestPsiDefVal < PlayersData[j].bestPsiDefVal) {
					PlayersData[factionID].enemyBestPsiDefVal = PlayersData[j].bestPsiDefVal;
				}
				*/
			}
		}
	}
	// PSI
	BOOL hasWorms = veh_avail(BSC_MIND_WORMS, factionID, -1);
	PlayersData[factionID].bestPsiOffense = hasWorms ? weap_strat(WPN_PSI_ATTACK, factionID) : 0;
	for (int i = 0; i < MaxWeaponNum; i++) {
		if (has_tech(Weapon[i].preqTech, factionID) && Weapon[i].offenseRating < 99) {
			int weapVal = weap_strat(i, factionID);
			if (Weapon[i].offenseRating < 0 && weapVal > PlayersData[factionID].bestPsiOffense) {
				PlayersData[factionID].bestPsiOffense = weapVal;
			}
		}
	}
	PlayersData[factionID].bestPsiDefense = hasWorms ? arm_strat(ARM_PSI_DEFENSE, factionID) : 0;
	for (int i = 0; i < MaxArmorNum; i++) {
		if (has_tech(Armor[i].preqTech, factionID)) {
			int armVal = arm_strat(i, factionID);
			if (Armor[i].defenseRating < 0 && armVal > PlayersData[factionID].bestPsiDefense) {
				PlayersData[factionID].bestPsiDefense = armVal;
			}
		}
	}
	PlayersData[factionID].enemyBestPsiOffense = 0;
	PlayersData[factionID].enemyBestPsiDefense = 0;
	for (int i = 0; i < 4 && !PlayersData[factionID].enemyBestPsiOffense; i++) {
		// 1st pass: vendetta, no treaty, has commlink
		// 2nd pass: no treaty, has commlink
		// 3rd pass: has commlink
		// 4th pass: any non-pact faction
		for (uint32_t j = 1, treaties; j < MaxPlayerNum; j++) {
			if (j != factionID
				&& (treaties = PlayersData[i].diploTreaties[j], !(treaties & DTREATY_PACT))
				&& ((!i && treaties & DTREATY_VENDETTA && !(treaties & DTREATY_TREATY)
					&& treaties & DTREATY_COMMLINK)
					|| (i == 1 && !(treaties & DTREATY_TREATY) && treaties & DTREATY_COMMLINK)
					|| (i == 2 && treaties & DTREATY_COMMLINK) || (i == 3))) {
				if (PlayersData[factionID].enemyBestPsiOffense < PlayersData[j].bestPsiOffense) {
					PlayersData[factionID].enemyBestPsiOffense = PlayersData[j].bestPsiOffense;
				}
				if (PlayersData[factionID].enemyBestPsiDefense < PlayersData[j].bestPsiDefense) {
					PlayersData[factionID].enemyBestPsiDefense = PlayersData[j].bestPsiDefense;
				}
			}
		}
	}
}
