/*
 * OpenSMACX - an open source clone of Sid Meier's Alpha Centauri.
 * Copyright (C) 2013-2019 Brendan Casey
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
#include "base.h"
#include "alpha.h"
#include "faction.h" // PlayerData
#include "map.h" // x_dist(), region_at()
#include "strings.h"

rules_facility *Facility = (rules_facility *)0x009A4B68;
rules_citizen *Citizen = (rules_citizen *)0x00946020;
base *Base = (base *)0x0097D040; // 512
base_secret_project *SecretProject = (base_secret_project *)0x009A6514; // 64
int *BaseIDCurrentSelected = (int *)0x00689370;
int *BaseCurrentCount = (int *)0x009A64CC;
int *BaseFindDist = (int *)0x0090EA04;
base **BaseCurrent = (base **)0x0090EA30;

/*
Purpose: Set current base globals.
Original Offset: 004E39D0
Return Value: n/a
Status: Complete
*/
void __cdecl set_base(int baseID) {
	*BaseIDCurrentSelected = baseID;
	*BaseCurrent = &Base[baseID];
}

/*
Purpose: Get base name string from baseID and store it in strBase. If baseID is -1, use 'NONE'.
Original Offset: 004E3A00
Return Value: n/a
Status: Complete
*/
void __cdecl say_base(LPSTR strBase, int baseID) {
	std::string output = (baseID >= 0) ? Base[baseID].nameString 
		: StringTable->get((int)*((LPSTR *)Label->stringsPtr + 0x19)); // 'NONE'
	// assumes 1032 char buffer via stringTemp, eventually remove
	strcat_s(strBase, 1032, output.c_str());
}

/*
Purpose: Find baseID nearest to coordinates.
Original Offset: 004E3B80
Return Value: -1 if not found, otherwise baseID
Status: Complete
*/
int __cdecl base_find(int xCoord, int yCoord) {
	if (*BaseCurrentCount <= 0) {
		return -1;
	}
	int proximity = 9999, baseID = -1;
	for (int i = 0; i < *BaseCurrentCount; i++) {
		int distHorz = abs(xCoord - Base[i].xCoord);
		if (!(*MapFlatToggle & 1) && distHorz > (int)*MapHorizontal) {
			distHorz = *MapHorizontalBounds - distHorz;
		}
		int dist = x_dist(distHorz, yCoord - Base[i].yCoord); // removed extra abs() yCoord diff
		if (dist <= proximity) {
			proximity = dist;
			baseID = i;
		}
	}
	if (baseID >= 0) {
		*BaseFindDist = proximity;
	}
	return baseID;
}

/*
Purpose: Find baseID nearest to coordinates owned by faction.
Original Offset: 004E3C60
Return Value: -1 if not found, otherwise baseID
Status: Complete
*/
int __cdecl base_find(int xCoord, int yCoord, uint32_t factionID) {
	if (*BaseCurrentCount <= 0) {
		return -1;
	}
	int proximity = 9999, baseID = -1;
	for (int i = 0; i < *BaseCurrentCount; i++) {
		if (Base[i].factionIDCurrent == factionID) {
			int distHorz = abs(xCoord - Base[i].xCoord);
			if (!(*MapFlatToggle & 1) && distHorz > (int)*MapHorizontal) {
				distHorz = *MapHorizontalBounds - distHorz;
			}
			int dist = x_dist(distHorz, yCoord - Base[i].yCoord); // removed extra abs() yCoord diff
			if (dist <= proximity) {
				proximity = dist;
				baseID = i;
			}
		}
	}
	if (baseID >= 0) {
		*BaseFindDist = proximity;
	}
	return baseID;
}

/*
Purpose: Find baseID nearest to coordinates meeting various conditional checks.
Original Offset: 004E3D50
Return Value: -1 if not found, otherwise baseID
Status: Complete
*/
int __cdecl base_find(int xCoord, int yCoord, int factionID, int region, int factionID2, 
	int factionID3) {
	int proximity = 9999, baseID = -1;
	*BaseFindDist = 9999; // difference from other two functions where this is reset at start
	if (*BaseCurrentCount <= 0) {
		return -1;
	}
	for (int i = 0; i < *BaseCurrentCount; i++) {
		if (region < 0 || region_at(Base[i].xCoord, Base[i].yCoord) == (uint32_t)region) {
			if (factionID < 0 ? (factionID2 < 0 || Base[i].factionIDCurrent != factionID2) 
				: (factionID == Base[i].factionIDCurrent || (factionID2 == -2 
					? PlayersData[factionID].diploStatus[Base[i].factionIDCurrent] & DSTATE_PACT 
					: (factionID2 >= 0 && factionID2 == Base[i].factionIDCurrent)))) {
				if (factionID3 < 0 || Base[i].factionIDCurrent == factionID3 
					|| ((1 << factionID3) & Base[i].unk2)) {
					int distHorz = abs(xCoord - Base[i].xCoord);
					if (!(*MapFlatToggle & 1) && distHorz > (int)*MapHorizontal) {
						distHorz = *MapHorizontalBounds - distHorz;
					}
					// removed extra abs() yCoord diff
					int dist = x_dist(distHorz, yCoord - Base[i].yCoord);
					if (dist <= proximity) {
						proximity = dist;
						baseID = i;
					}
				}
			}
		}
	}
	if (baseID >= 0) {
		*BaseFindDist = proximity;
	}
	return baseID;
}

/*
Purpose: Check what facility (if any) a base needs for additional population growth. Stand alone
         function unused in original game and likely optimized out.
Original Offset: 004EEF80
Return Value: Facility id needed for pop growth or zero if base already has Hab Complex and Dome.
Status: WIP - test
*/
uint32_t __cdecl pop_goal_fac(int baseID) {
	uint32_t factionID = Base[baseID].factionIDCurrent;
	uint32_t limitMod = has_project(SP_ASCETIC_VIRTUES, factionID) ? 2 : 0;
	int pop = Base[baseID].populationSize - limitMod + Players[factionID].rulePopulation;
	if (pop >= Rules->PopLimitSansHabComplex) {
		uint32_t offset, mask;
		bitmask(FAC_HAB_COMPLEX, &offset, &mask);
		if (!(Base[baseID].facilitiesPresentTable[offset] & mask)) {
			return FAC_HAB_COMPLEX;
		}
	}
	if (pop >= Rules->PopLimitSansHabDome) {
		uint32_t offset, mask;
		bitmask(FAC_HABITATION_DOME, &offset, &mask);
		if (!(Base[baseID].facilitiesPresentTable[offset] & mask)) {
			return FAC_HABITATION_DOME;
		}
	}
	return 0; // Base already has Hab Complex and Dome
}

/*
Purpose: ?
Original Offset: 004EF090
Return Value: ?
Status: WIP - test
*/
int __cdecl pop_goal(int baseID) {
	uint32_t factionID = Base[baseID].factionIDCurrent;
	uint32_t limitMod = has_project(SP_ASCETIC_VIRTUES, factionID) ? 2 : 0;
	int goal = (36 - Base[baseID].populationSize) / 6 + Base[baseID].populationSize;
	if (goal <= 6) {
		goal = 6;
	}
	uint32_t offset, mask;
	bitmask(FAC_HAB_COMPLEX, &offset, &mask);
	if (!(Base[baseID].facilitiesPresentTable[offset] & mask)) {
		int compare = Rules->PopLimitSansHabComplex - Players[factionID].rulePopulation + limitMod;
		if (goal >= compare) {
			goal = compare;
		}
	}
	bitmask(FAC_HABITATION_DOME, &offset, &mask);
	if (!(Base[baseID].facilitiesPresentTable[offset] & mask)) {
		int compare = Rules->PopLimitSansHabDome - Players[factionID].rulePopulation + limitMod;
		if (goal >= compare) {
			goal = compare;
		}
	}
	return goal;
}

/*
Purpose: Check if faction has secret project in a base they control.
Original Offset: 004F80D0
Return Value: Does faction have Secret Project? true/false
Status: Complete
*/
BOOL __cdecl has_project(uint32_t projectID, uint32_t factionID) {
	int baseID = base_project(projectID);
	if (baseID >= 0) {
		return (Base[baseID].factionIDCurrent == factionID);
	}
	return false; // Not built or destroyed
}

/*
Purpose: Checks whether facility (non-SP) has been build in currently selected base.
Original Offset: 00500290
Return Value: Does current base have facility? true/false
Status: Complete
*/
BOOL __cdecl has_fac_built(uint32_t facilityID) {
	if (facilityID >= FAC_SKY_HYDRO_LAB) {
		return false;
	}
	uint32_t offset, mask;
	bitmask(facilityID, &offset, &mask);
	return (Base[*BaseIDCurrentSelected].facilitiesPresentTable[offset] & mask) != 0;
}

/*
Purpose: Get current status of project.
Original Offset: 005002E0
Return Value: baseID or -1/-2 if not built or destroyed
Status: Complete
*/
int __cdecl base_project(uint32_t projectID) {
	return *(&SecretProject->HumanGenomeProject + projectID);
}

/*
Purpose: Calculate offset & bitmask from facilityID.
Original Offset: 0050BA00
Return Value: n/a
Status: Complete
*/
void __cdecl bitmask(uint32_t facilityID, uint32_t *offset, uint32_t *mask) {
	*offset = facilityID / 8;
	*mask = 1 << (facilityID & 7);
}

/*
Purpose: Check if base is a port.
Original Offset: 00579A00
Return Value: Is base port? true/false
Status: Complete
*/
BOOL __cdecl is_port(int baseID, BOOL isBaseRadius) {
	return is_coast(Base[baseID].xCoord, Base[baseID].yCoord, isBaseRadius);
}