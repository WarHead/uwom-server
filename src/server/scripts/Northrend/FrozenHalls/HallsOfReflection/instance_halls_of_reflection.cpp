/*
 * Copyright (C) 2008-2011 by WarHead - United Worlds of MaNGOS - http://www.uwom.de
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "ScriptPCH.h"
#include "halls_of_reflection.h"

/* Halls of Reflection encounters:
0- Falric
1- Marwyn
2- Frostworn General
3- The Lich King
*/

enum eEnum
{
    ENCOUNTER_WAVE_MERCENARY    = 6,
    ENCOUNTER_WAVE_FOOTMAN      = 10,
    ENCOUNTER_WAVE_RIFLEMAN     = 6,
    ENCOUNTER_WAVE_PRIEST       = 6,
    ENCOUNTER_WAVE_MAGE         = 6
};

enum Events
{
    EVENT_NONE,
    EVENT_NEXT_WAVE,
    EVENT_START_LICH_KING
};

static Position PriestSpawnPos[ENCOUNTER_WAVE_PRIEST] =
{
    {5277.74f, 2016.88f, 707.778f, 5.96903f},
    {5295.88f, 2040.34f, 707.778f, 5.07891f},
    {5320.37f, 1980.13f, 707.778f, 2.00713f},
    {5280.51f, 1997.84f, 707.778f, 0.296706f},
    {5302.45f, 2042.22f, 707.778f, 4.90438f},
    {5306.57f, 1977.47f, 707.778f, 1.50098f}
};

static Position MageSpawnPos[ENCOUNTER_WAVE_MAGE] =
{
    {5312.75f, 2037.12f, 707.778f, 4.59022f},
    {5309.58f, 2042.67f, 707.778f, 4.69494f},
    {5275.08f, 2008.72f, 707.778f, 6.21337f},
    {5279.65f, 2004.66f, 707.778f, 0.069813f},
    {5275.48f, 2001.14f, 707.778f, 0.174533f},
    {5316.7f, 2041.55f, 707.778f, 4.50295f}
};

static Position MercenarySpawnPos[ENCOUNTER_WAVE_MERCENARY] =
{
    {5302.25f, 1972.41f, 707.778f, 1.37881f},
    {5311.03f, 1972.23f, 707.778f, 1.64061f},
    {5277.36f, 1993.23f, 707.778f, 0.401426f},
    {5318.7f, 2036.11f, 707.778f, 4.2237f},
    {5335.72f, 1996.86f, 707.778f, 2.74017f},
    {5299.43f, 1979.01f, 707.778f, 1.23918f}
};

static Position FootmenSpawnPos[ENCOUNTER_WAVE_FOOTMAN] =
{
    {5306.06f, 2037, 707.778f, 4.81711f},
    {5344.15f, 2007.17f, 707.778f, 3.15905f},
    {5337.83f, 2010.06f, 707.778f, 3.22886f},
    {5343.29f, 1999.38f, 707.778f, 2.9147f},
    {5340.84f, 1992.46f, 707.778f, 2.75762f},
    {5325.07f, 1977.6f, 707.778f, 2.07694f},
    {5336.6f, 2017.28f, 707.778f, 3.47321f},
    {5313.82f, 1978.15f, 707.778f, 1.74533f},
    {5280.63f, 2012.16f, 707.778f, 6.05629f},
    {5322.96f, 2040.29f, 707.778f, 4.34587f}
};

static Position RiflemanSpawnPos[ENCOUNTER_WAVE_RIFLEMAN] =
{
    {5343.47f, 2015.95f, 707.778f, 3.49066f},
    {5337.86f, 2003.4f, 707.778f, 2.98451f},
    {5319.16f, 1974, 707.778f, 1.91986f},
    {5299.25f, 2036, 707.778f, 5.02655f},
    {5295.64f, 1973.76f, 707.778f, 1.18682f},
    {5282.9f, 2019.6f, 707.778f, 5.88176f}
};

class instance_halls_of_reflection : public InstanceMapScript
{
public:
    instance_halls_of_reflection() : InstanceMapScript("instance_halls_of_reflection", 668) { }

    struct instance_halls_of_reflection_InstanceMapScript : public InstanceScript
    {
        instance_halls_of_reflection_InstanceMapScript(Map * map) : InstanceScript(map) {};

        uint64 FalricGUID;
        uint64 MarwynGUID;
        uint64 LichKingGUID;
        uint64 JainaPart1GUID;
        uint64 SylvanasPart1GUID;
        uint64 LeaderGUID;

        uint64 GunshipGUID;
        uint64 ChestGUID;
        uint64 PortalGUID;

        uint64 FrostmourneGUID;
        uint64 FrontDoorGUID;
        uint64 FrostwornDoorGUID;
        uint64 ArthasDoorGUID;
        uint64 RunDoorGUID;
        uint64 WallGUID[4];
        uint64 CaveDoorGUID;
        uint64 QuelDelarTriggerGUID;
        uint64 QuelDelarGUID;
        uint64 QuelDelarPlrGUID;

        uint32 WallData[4];
        uint32 Encounter[MAX_ENCOUNTER];
        uint32 Team;
        uint32 WaveCnt;
        uint32 IntroDone;
        uint32 Summons;
        uint32 PhaseData;

        EventMap events;

        void Initialize()
        {
            events.Reset();

            FalricGUID = 0;
            MarwynGUID = 0;
            LichKingGUID = 0;
            JainaPart1GUID = 0;
            SylvanasPart1GUID = 0;
            LeaderGUID = 0;

            GunshipGUID = 0;
            ChestGUID = 0;
            PortalGUID = 0;

            FrostmourneGUID = 0;
            ArthasDoorGUID = 0;
            FrostwornDoorGUID = 0;
            FrontDoorGUID = 0;
            CaveDoorGUID = 0;
            QuelDelarTriggerGUID = 0;
            QuelDelarGUID = 0;
            QuelDelarPlrGUID = 0;

            Team = 0;
            WaveCnt = 0;
            IntroDone = 0;

            for (uint8 i=0; i<4; ++i)
            {
                WallData[i] = 0;
                WallGUID[i] = 0;
            }

            for (uint8 i=0; i<MAX_ENCOUNTER; ++i)
                Encounter[i] = NOT_STARTED;
        }

        void OpenDoor(uint64 guid)
        {
            if (GameObject * go = instance->GetGameObject(guid))
                go->SetGoState(GO_STATE_ACTIVE);
        }

        void CloseDoor(uint64 guid)
        {
            if (GameObject * go = instance->GetGameObject(guid))
                go->SetGoState(GO_STATE_READY);
        }

        void OnPlayerEnter(Player * plr)
        {
            if (!plr || !plr->IsInWorld())
                return;

            Position pos = { 5304.132324f, 2001.124634f, 709.341431f, 3.895033f };

            if (plr->HasAura(SPELL_QUELDELARS_ZWANG) && !GetData64(DATA_QUELDELAR_PLR))
            {
                SetData64(DATA_QUELDELAR_PLR, plr->GetGUID());

                if (!instance->GetCreature(QuelDelarGUID))
                    instance->SummonCreature(NPC_QUELDELAR_TRIGGER, pos);
            }

            if (WaveCnt)
                DoUpdateWorldState(WORLD_STATE_HOR, 1);
        }

        void OnCreatureCreate(Creature * creature)
        {
            Map::PlayerList const &players = instance->GetPlayers();
            if (!players.isEmpty())
                if (Player * plr = players.begin()->getSource())
                    Team = plr->GetTeam();

            switch (creature->GetEntry())
            {
                case NPC_QUELDELAR_TRIGGER:
                    QuelDelarTriggerGUID = creature->GetGUID();
                    creature->SetRespawnDelay(86400);
                    break;
                case NPC_QUELDELAR:
                    QuelDelarGUID = creature->GetGUID();
                    creature->SetRespawnDelay(86400);
                    break;
                case NPC_FALRIC:
                    FalricGUID = creature->GetGUID();
                    break;
                case NPC_MARWYN:
                    MarwynGUID = creature->GetGUID();
                    break;
                case NPC_LICH_KING_EVENT:
                    break;
                case NPC_JAINA_PART1:
                    if (Team == HORDE)
                    {
                        creature->UpdateEntry(NPC_SYLVANAS_PART1, HORDE);
                        creature->UpdateObjectVisibility(true);
                    }
                    else
                    {
                        creature->SetHealth(5040000);
                        creature->SetPower(POWER_MANA, 1762000);
                    }
                    JainaPart1GUID = creature->GetGUID();
                    SylvanasPart1GUID = creature->GetGUID();
                    break;
                case NPC_SYLVANAS_PART1:
                    SylvanasPart1GUID = creature->GetGUID();
                    break;
                case NPC_FROSTWORN_GENERAL:
                    creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    break;
                case NPC_JAINA_OUTRO:
                    if (Team == HORDE)
                    {
                        creature->UpdateEntry(NPC_SYLVANA_OUTRO, HORDE);
                        creature->UpdateObjectVisibility(true);
                    }
                    else
                    {
                        creature->SetHealth(5040000);
                        creature->SetPower(POWER_MANA, 1762000);
                    }
                    LeaderGUID = creature->GetGUID();
                    break;
                case BOSS_LICH_KING:
                    creature->SetHealth(207890000);
                    LichKingGUID = creature->GetGUID();
                    break;
            }
        }

        void OnGameObjectCreate(GameObject * go)
        {
            // TODO: init state depending on encounters
            switch (go->GetEntry())
            {
                case GO_FROSTMOURNE:
                    FrostmourneGUID = go->GetGUID();
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                    HandleGameObject(0, false, go);
                    break;
                case GO_FROSTMOURNE_ALTAR:
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                    HandleGameObject(0, true, go);
                    break;
                case GO_FRONT_DOOR:
                    FrontDoorGUID = go->GetGUID();
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);
                    OpenDoor(FrontDoorGUID);
                    break;
                case GO_FROSTWORN_DOOR:
                    FrostwornDoorGUID = go->GetGUID();
                    go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_INTERACT_COND);

                    if (Encounter[1] == DONE)
                        OpenDoor(FrostwornDoorGUID);
                    else
                        CloseDoor(FrostwornDoorGUID);
                    break;
                case GO_RUN_DOOR:
                    RunDoorGUID = go->GetGUID();
                    break;
                case GO_ARTHAS_DOOR:
                    ArthasDoorGUID = go->GetGUID();
                    break;
                case GO_ICE_WALL_1:
                    WallGUID[0] = go->GetGUID();
                    break;
                case GO_ICE_WALL_2:
                    WallGUID[1] = go->GetGUID();
                    break;
                case GO_ICE_WALL_3:
                    WallGUID[2] = go->GetGUID();
                    break;
                case GO_ICE_WALL_4:
                    WallGUID[3] = go->GetGUID();
                    break;
                case GO_CAVE:
                    CaveDoorGUID = go->GetGUID();
                    break;
                case GO_CAPTAIN_CHEST_1:
                    go->SetPhaseMask(2, true);
                    if (!instance->IsHeroic() && Team == HORDE)
                        ChestGUID = go->GetGUID();
                    break;
                case GO_CAPTAIN_CHEST_3:
                    go->SetPhaseMask(2, true);
                    if (instance->IsHeroic() && Team == HORDE)
                        ChestGUID = go->GetGUID();
                    break;
                case GO_CAPTAIN_CHEST_2:
                    go->SetPhaseMask(2, true);
                    if (!instance->IsHeroic() && Team == ALLIANCE)
                        ChestGUID = go->GetGUID();
                    break;
                case GO_CAPTAIN_CHEST_4:
                    go->SetPhaseMask(2, true);
                    if (instance->IsHeroic() && Team == ALLIANCE)
                        ChestGUID = go->GetGUID();
                    break;
                case GO_SKYBREAKER:
                    go->SetPhaseMask(2, true);
                    if (Team == ALLIANCE)
                        GunshipGUID = go->GetGUID();
                    break;
                case GO_ORGRIM_HAMMER:
                    go->SetPhaseMask(2, true);
                    if (Team == HORDE)
                        GunshipGUID = go->GetGUID();
                    break;
                case GO_PORTAL:
                    go->SetPhaseMask(2, true);
                    PortalGUID = go->GetGUID();
                    break;
            }
        }

        void SetData(uint32 type, uint32 data)
        {
            switch (type)
            {
                case DATA_WAVE_COUNT:
                    if (GetData(DATA_MARWYN_EVENT) == DONE)
                    {
                        WaveCnt = 0;
                        DoUpdateWorldState(WORLD_STATE_HOR_WAVE_COUNT, 0);
                        DoUpdateWorldState(WORLD_STATE_HOR, WaveCnt);
                        break;
                    }
                    if (data == START_WAVES) // Start der Wellen
                    {
                        CloseDoor(FrontDoorGUID);
                        events.ScheduleEvent(EVENT_NEXT_WAVE, 0);
                        break;
                    }
                    if (data == START_NEXT_WAVE_INSTANT) // Nächste Welle sofort
                    {
                        events.RescheduleEvent(EVENT_NEXT_WAVE, 0);
                        break;
                    }
                    if (data == START_NEXT_WAVE) // Nächste Welle starten
                    {
                        events.RescheduleEvent(EVENT_NEXT_WAVE, SEKUNDEN_60);
                        break;
                    }
                    if (WaveCnt && data == START_RESET) // Wipe
                    {
                        DoWipe();
                        break;
                    }
                    break;
                case DATA_INTRO_EVENT:
                    IntroDone = data;
                    break;
                case DATA_FALRIC_EVENT:
                    Encounter[0] = data;
                    if (data == DONE)
                        events.RescheduleEvent(EVENT_NEXT_WAVE, SEKUNDEN_60);
                    break;
                case DATA_MARWYN_EVENT:
                    Encounter[1] = data;
                    if (data == DONE)
                    {
                        DoUpdateWorldState(WORLD_STATE_HOR, 0);
                        OpenDoor(FrostwornDoorGUID);
                        OpenDoor(FrontDoorGUID);
                    }
                    break;
                case DATA_FROSWORN_EVENT:
                    Encounter[2] = data;
                    if (data == DONE)
                    {
                        OpenDoor(ArthasDoorGUID);
                        SetData(DATA_PHASE, 3);
                        instance->SummonCreature(BOSS_LICH_KING, OutroSpawns[0]);
                        instance->SummonCreature(NPC_JAINA_OUTRO, OutroSpawns[1]);
                    }
                    break;
                case DATA_LICHKING_EVENT:
                    Encounter[3] = data;
                    switch(data)
                    {
                        case IN_PROGRESS:
                            OpenDoor(RunDoorGUID);
                            if (instance->IsHeroic())
                                DoStartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, ACHIEV_NOT_RETREATING_EVENT);
                            break;
                        case FAIL:
                            for (uint8 i=0; i<4; ++i)
                                OpenDoor(WallGUID[i]);

                            CloseDoor(RunDoorGUID);

                            if (Creature * LichKing = instance->GetCreature(LichKingGUID))
                                LichKing->DespawnOrUnsummon(10000);
                            if (Creature * Leader = instance->GetCreature(LeaderGUID))
                                Leader->DespawnOrUnsummon(10000);

                            DoStopTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, ACHIEV_NOT_RETREATING_EVENT);
                            DoCastSpellOnPlayers(SPELL_SUICIDE); // Alle Spieler begehen Selbsmord!

                            SetData(DATA_PHASE, 3);
                            instance->SummonCreature(BOSS_LICH_KING, OutroSpawns[0]);
                            instance->SummonCreature(NPC_JAINA_OUTRO, OutroSpawns[1]);
                            break;
                        case DONE:
                            if (GameObject * Chest = instance->GetGameObject(ChestGUID))
                                Chest->SetPhaseMask(1, true);
                            if (GameObject * Portal = instance->GetGameObject(PortalGUID))
                                Portal->SetPhaseMask(1, true);

                            DoCompleteAchievement(ACHIEV_HALLS_OF_REFLECTION_N);

                            if (instance->IsHeroic())
                            {
                                DoCompleteAchievement(ACHIEV_HALLS_OF_REFLECTION_H);
                                DoCastSpellOnPlayers(SPELL_ACHIEV_CHECK);
                                DoStopTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, ACHIEV_NOT_RETREATING_EVENT);
                            }
                            break;
                    }
                    break;
                case DATA_SUMMONS:
                    if (data == 3)
                        Summons = 0;
                    else if (data == 1)
                        ++Summons;
                    else if (data == 0)
                        --Summons;
                    data = NOT_STARTED;
                    break;
                case DATA_ICE_WALL_1:
                    WallData[0] = data;
                    break;
                case DATA_ICE_WALL_2:
                    WallData[1] = data;
                    break;
                case DATA_ICE_WALL_3:
                    WallData[2] = data;
                    break;
                case DATA_ICE_WALL_4:
                    WallData[3] = data;
                    break;
                case DATA_PHASE:
                    PhaseData = data;
                    break;
            }

            if (data == DONE)
                SaveToDB();
        }

        uint32 GetData(uint32 type)
        {
            switch (type)
            {
                case DATA_INTRO_EVENT:          return IntroDone;
                case DATA_TEAM_IN_INSTANCE:     return Team;

                case DATA_FALRIC_EVENT:         return Encounter[0];
                case DATA_MARWYN_EVENT:         return Encounter[1];
                case DATA_WAVE_COUNT:           return WaveCnt;

                case DATA_FROSWORN_EVENT:       return Encounter[2];

                case DATA_LICHKING_EVENT:       return Encounter[3];
                case DATA_ICE_WALL_1:           return WallData[0];
                case DATA_ICE_WALL_2:           return WallData[1];
                case DATA_ICE_WALL_3:           return WallData[2];
                case DATA_ICE_WALL_4:           return WallData[3];
                case DATA_SUMMONS:              return Summons;

                case DATA_PHASE:                return PhaseData;
            }

            return 0;
        }

        void SetData64(uint32 type, uint64 data)
        {
            if (type == DATA_QUELDELAR_PLR)
                QuelDelarPlrGUID = data;
        }

        uint64 GetData64(uint32 identifier)
        {
            switch (identifier)
            {
                case DATA_FALRIC:               return FalricGUID;
                case DATA_MARWYN:               return MarwynGUID;
                case DATA_LICHKING:             return LichKingGUID;
                case DATA_ESCAPE_LEADER:        return LeaderGUID;
                case DATA_FROSTMOURNE:          return FrostmourneGUID;
                case DATA_FRONT_DOOR:           return FrontDoorGUID;
                case DATA_FROSTWORN_DOOR:       return FrostwornDoorGUID;
                case DATA_ARTHAS_DOOR:          return ArthasDoorGUID;
                case DATA_QUELDELAR_TRIGGER:    return QuelDelarTriggerGUID;
                case DATA_QUELDELAR:            return QuelDelarGUID;
                case DATA_QUELDELAR_PLR:        return QuelDelarPlrGUID;

                case GO_ICE_WALL_1:         return WallGUID[0];
                case GO_ICE_WALL_2:         return WallGUID[1];
                case GO_ICE_WALL_3:         return WallGUID[2];
                case GO_ICE_WALL_4:         return WallGUID[3];
                case GO_CAVE:               return CaveDoorGUID;
                case GO_CAPTAIN_CHEST_1:
                case GO_CAPTAIN_CHEST_2:
                case GO_CAPTAIN_CHEST_3:
                case GO_CAPTAIN_CHEST_4:    return ChestGUID;
                case GO_SKYBREAKER:
                case GO_ORGRIM_HAMMER:      return GunshipGUID;
            }
            return 0;
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;
            saveStream << "H R 1 " << Encounter[0] << " " << Encounter[1] << " " << Encounter[2] << " " << Encounter[3] << " " << IntroDone;

            OUT_SAVE_INST_DATA_COMPLETE;
            return saveStream.str();
        }

        void Load(const char* in)
        {
            if (!in)
            {
                OUT_LOAD_INST_DATA_FAIL;
                return;
            }

            OUT_LOAD_INST_DATA(in);

            char dataHead1, dataHead2;
            uint16 version;
            uint16 data0, data1, data2, data3, data4;

            std::istringstream loadStream(in);
            loadStream >> dataHead1 >> dataHead2 >> version >> data0 >> data1 >> data2 >> data3 >> data4;

            if (dataHead1 == 'H' && dataHead2 == 'R')
            {
                Encounter[0] = data0;
                Encounter[1] = data1;
                Encounter[2] = data2;
                Encounter[3] = data3;
                IntroDone = data4;

                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    if (Encounter[i] == IN_PROGRESS)
                        Encounter[i] = NOT_STARTED;

                OpenDoor(FrontDoorGUID);
                if (Encounter[1] == DONE)
                    OpenDoor(FrostwornDoorGUID);
                if (Encounter[2] == DONE)
                    OpenDoor(ArthasDoorGUID);

            } else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }

        void AddWave()
        {
            DoUpdateWorldState(WORLD_STATE_HOR, 1);
            DoUpdateWorldState(WORLD_STATE_HOR_WAVE_COUNT, WaveCnt);

            switch (WaveCnt)
            {
                case 1:
                case 2:
                case 3:
                case 4:
                    if (Creature * Falric = instance->GetCreature(FalricGUID))
                        SpawnWave(Falric);
                    break;
                case 5:
                    if (Creature * Falric = instance->GetCreature(FalricGUID))
                        Falric->AI()->DoAction(ACTION_ENTER_COMBAT);
                    break;
                case 6:
                case 7:
                case 8:
                case 9:
                    if (Creature * Marwyn  = instance->GetCreature(MarwynGUID))
                        SpawnWave(Marwyn);
                    break;
                case 10:
                    if (Creature * Marwyn = instance->GetCreature(MarwynGUID))
                        Marwyn->AI()->DoAction(ACTION_ENTER_COMBAT);
                    break;
                default:
                    break;
            }
        }

        void DoWipe()
        {
            if (GetData(DATA_FALRIC_EVENT) == DONE && GetData(DATA_MARWYN_EVENT) != DONE)
                WaveCnt = 5;
            else
                WaveCnt = 0;

            events.Reset();

            DoUpdateWorldState(WORLD_STATE_HOR, 0);
            DoUpdateWorldState(WORLD_STATE_HOR_WAVE_COUNT, WaveCnt);

            OpenDoor(FrontDoorGUID);

            if (Creature * Falric = instance->GetCreature(FalricGUID))
                if (Falric->isAlive())
                    Falric->AI()->EnterEvadeMode();

            if (Creature * Marwyn = instance->GetCreature(MarwynGUID))
                if (Marwyn->isAlive())
                    Marwyn->AI()->EnterEvadeMode();
        }

        void RemoveFlags(Unit * unit)
        {
            unit->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PASSIVE | UNIT_FLAG_OOC_NOT_ATTACKABLE | UNIT_FLAG_UNK_6);
            unit->CastSpell(unit, 53708, true); // Shadow Summon Visual - Kann den orig. Spell nicht finden. Ist eigentlich eine lila Zone unter den NPCs, mit aufsteigenden "Dämpfen".
        }

        // TODO: Alle WellenNPCs müssen sofort beim Ruf erscheinen, und zufällig (in Gruppen) nacheinander aktiviert werden!
        void SpawnWave(Creature * summoner)
        {
            if (TempSummon * TS = summoner->SummonCreature(NPC_WAVE_MERCENARY, MercenarySpawnPos[urand(0, ENCOUNTER_WAVE_MERCENARY-1)]))
                RemoveFlags(TS);

            if (TempSummon * TS = summoner->SummonCreature(NPC_WAVE_FOOTMAN, FootmenSpawnPos[urand(0, ENCOUNTER_WAVE_FOOTMAN-1)]))
                RemoveFlags(TS);

            if (TempSummon * TS = summoner->SummonCreature(NPC_WAVE_RIFLEMAN, RiflemanSpawnPos[urand(0, ENCOUNTER_WAVE_RIFLEMAN-1)]))
                RemoveFlags(TS);

            if (TempSummon * TS = summoner->SummonCreature(NPC_WAVE_PRIEST, PriestSpawnPos[urand(0, ENCOUNTER_WAVE_PRIEST-1)]))
                RemoveFlags(TS);

            if (TempSummon * TS = summoner->SummonCreature(NPC_WAVE_MAGE, MageSpawnPos[urand(0, ENCOUNTER_WAVE_MAGE-1)]))
                RemoveFlags(TS);
        }

        bool AlivePlr()
        {
            if (instance->GetPlayers().isEmpty())
                return false;

            return true;
        }

        bool NurGM()
        {
            Map::PlayerList const & PlList = instance->GetPlayers();

            if (PlList.isEmpty())
                return true;

            for (Map::PlayerList::const_iterator i = PlList.begin(); i != PlList.end(); ++i)
                if (Player * plr = i->getSource())
                    if (!plr->isGameMaster() && plr->isGMVisible())
                        return false;

            return true;
        }

        void Update(uint32 diff)
        {
            if (WaveCnt && (!instance->HavePlayers() || !AlivePlr() || NurGM()))
            {
                DoWipe();
                return;
            }

            events.Update(diff);

            switch (events.ExecuteEvent())
            {
                case EVENT_NEXT_WAVE:
                    ++WaveCnt;
                    AddWave();
                    break;
                default:
                    break;
            }
        }
    };

    InstanceScript * GetInstanceScript(InstanceMap * map) const
    {
        return new instance_halls_of_reflection_InstanceMapScript(map);
    }
};

void AddSC_instance_halls_of_reflection()
{
    new instance_halls_of_reflection();
}
