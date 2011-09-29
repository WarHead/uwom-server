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
#include "oculus.h"

#define MAX_ENCOUNTER 4
#define MAX_CONSTRUCTS 10

/* The Occulus encounters:
0 - Drakos the Interrogator
1 - Varos Cloudstrider
2 - Mage-Lord Urom
3 - Ley-Guardian Eregos */

class instance_oculus : public InstanceMapScript
{
public:
    instance_oculus() : InstanceMapScript("instance_oculus", 578) { }

    InstanceScript* GetInstanceScript(InstanceMap * map) const
    {
        return new instance_oculus_InstanceMapScript(map);
    }

    struct instance_oculus_InstanceMapScript : public InstanceScript
    {
        instance_oculus_InstanceMapScript(Map * map) : InstanceScript(map) {}

        void Initialize()
        {
            SetBossNumber(MAX_ENCOUNTER);

            drakosGUID = 0;
            varosGUID = 0;
            uromGUID = 0;
            eregosGUID = 0;

            platformUrom = 0;
            CentrifugeConstructCnt = MAX_CONSTRUCTS;

            eregosCacheGUID = 0;
            RampenlichtGUID = 0;

            DragonList.clear();
            GOList.clear();
        }

        void OnCreatureDeath(Creature * creature)
        {
            if (creature->GetEntry() != NPC_CENTRIFUGE_CONSTRUCT)
                return;

             if (CentrifugeConstructCnt)
             {
                 DoUpdateWorldState(WORLD_STATE_CENTRIFUGE_CONSTRUCT_AMOUNT, --CentrifugeConstructCnt);
                 SaveToDB();
             }
        }

        void OnPlayerEnter(Player * player)
        {
            if (GetBossState(DATA_DRAKOS_EVENT) == DONE && GetBossState(DATA_VAROS_EVENT) != DONE)
            {
                player->SendUpdateWorldState(WORLD_STATE_CENTRIFUGE_CONSTRUCT_SHOW, 1);
                player->SendUpdateWorldState(WORLD_STATE_CENTRIFUGE_CONSTRUCT_AMOUNT, CentrifugeConstructCnt);
            }
            else
            {
                player->SendUpdateWorldState(WORLD_STATE_CENTRIFUGE_CONSTRUCT_SHOW, 0);
                player->SendUpdateWorldState(WORLD_STATE_CENTRIFUGE_CONSTRUCT_AMOUNT, 0);
            }
        }

        void ProcessEvent(WorldObject * /*unit*/, uint32 eventId)
        {
            if (eventId != EVENT_CALL_DRAGON)
                return;

            if (Creature * varos = instance->GetCreature(varosGUID))
                if (Creature * drake = varos->SummonCreature(NPC_AZURE_RING_GUARDIAN, varos->GetPositionX(), varos->GetPositionY(), varos->GetPositionZ()+20))
                    drake->AI()->DoAction(ACTION_CALL_DRAGON_EVENT);
        }

        void OnCreatureCreate(Creature * creature)
        {
            switch(creature->GetEntry())
            {
                case NPC_VERDISA:
                case NPC_BELGARISTRASZ:
                case NPC_ETERNOS:
                    DragonList.push_back(creature->GetGUID());
                    break;
                case NPC_DRAKOS:
                    drakosGUID = creature->GetGUID();
                    break;
                case NPC_VAROS:
                    varosGUID = creature->GetGUID();
                    break;
                case NPC_UROM:
                    uromGUID = creature->GetGUID();
                    break;
                case NPC_EREGOS:
                    eregosGUID = creature->GetGUID();
                    break;
                case NPC_CENTRIFUGE_CONSTRUCT:
                    break;
            }
        }

        void OnGameObjectCreate(GameObject * go)
        {
            switch (go->GetEntry())
            {
                case GO_DRAGON_CAGE_DOOR:
                    if (GetBossState(DATA_DRAKOS_EVENT) == DONE)
                        go->SetGoState(GO_STATE_ACTIVE);
                    else
                        go->SetGoState(GO_STATE_READY);
                    GOList.push_back(go->GetGUID());
                    break;
                case GO_EREGOS_CACHE_N:
                case GO_EREGOS_CACHE_H:
                    eregosCacheGUID = go->GetGUID();
                    break;
                case GO_RAMPENLICHT:
                    RampenlichtGUID = go->GetGUID();
                default:
                    break;
            }
        }

        bool SetBossState(uint32 type, EncounterState state)
        {
            if (!InstanceScript::SetBossState(type, state))
                return false;

            switch (type)
            {
                case DATA_DRAKOS_EVENT:
                    if (state == DONE)
                    {
                        DoUpdateWorldState(WORLD_STATE_CENTRIFUGE_CONSTRUCT_SHOW, 1);
                        DoUpdateWorldState(WORLD_STATE_CENTRIFUGE_CONSTRUCT_AMOUNT, CentrifugeConstructCnt);
                        OpenCageDoors();
                        MoveDrakeNPCs();
                    }
                    break;
                case DATA_VAROS_EVENT:
                    if (state == DONE)
                        DoUpdateWorldState(WORLD_STATE_CENTRIFUGE_CONSTRUCT_SHOW, 0);
                    break;
                case DATA_EREGOS_EVENT:
                    if (state == DONE)
                    {
                        DoRespawnGameObject(RampenlichtGUID, 7 * DAY);
                        DoRespawnGameObject(eregosCacheGUID, 7 * DAY);
                    }
                    break;
            }
            return true;
        }

        void SetData(uint32 type, uint32 data)
        {
            switch(type)
            {
                case DATA_UROM_PLATAFORM: platformUrom = data; break;
            }
        }

        uint32 GetData(uint32 type)
        {
            switch(type)
            {
                case DATA_CONSTRUCT_CNT:    return CentrifugeConstructCnt;
                case DATA_UROM_PLATAFORM:   return platformUrom;
                // used by condition system
                case DATA_UROM_EVENT:       return GetBossState(DATA_UROM_EVENT);
            }
            return 0;
        }

        uint64 GetData64(uint32 identifier)
        {
            switch(identifier)
            {
                case DATA_DRAKOS:   return drakosGUID;
                case DATA_VAROS:    return varosGUID;
                case DATA_UROM:     return uromGUID;
                case DATA_EREGOS:   return eregosGUID;
            }
            return 0;
        }

        void OpenCageDoors()
        {
            if (GOList.empty())
                return;

            for (std::list<uint64>::const_iterator itr = GOList.begin(); itr != GOList.end(); ++itr)
                if (GameObject * go = instance->GetGameObject(*itr))
                    go->SetGoState(GO_STATE_ACTIVE);
        }

        void MoveDrakeNPCs()
        {
            if (DragonList.empty())
                return;

            for (std::list<uint64>::const_iterator itr = DragonList.begin(); itr != DragonList.end(); ++itr)
                if (Creature * cr = instance->GetCreature(*itr))
                {
                    // Dies wird korrekt ausgeführt, aber dennoch bewegen sich die NPCs nicht an die Position / vom Fleck! :-( Kein Plan imo... :-(
                    switch(cr->GetEntry())
                    {
                        case NPC_VERDISA:       cr->GetMotionMaster()->MovePoint(0, MovePos[0]); break;
                        case NPC_BELGARISTRASZ: cr->GetMotionMaster()->MovePoint(0, MovePos[1]); break;
                        case NPC_ETERNOS:       cr->GetMotionMaster()->MovePoint(0, MovePos[2]); break;
                    }
                }
        }

        std::string GetSaveData()
        {
            OUT_SAVE_INST_DATA;

            std::ostringstream saveStream;
            saveStream << "T O " << CentrifugeConstructCnt << ' ' << GetBossSaveData();

            str_data = saveStream.str();

            OUT_SAVE_INST_DATA_COMPLETE;
            return str_data;
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

            std::istringstream loadStream(in);
            loadStream >> dataHead1 >> dataHead2 >> CentrifugeConstructCnt;

            if (dataHead1 == 'T' && dataHead2 == 'O')
            {
                for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                {
                    uint32 tmpState;
                    loadStream >> tmpState;
                    if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;
                    SetBossState(i, EncounterState(tmpState));
                }
            } else OUT_LOAD_INST_DATA_FAIL;

            OUT_LOAD_INST_DATA_COMPLETE;
        }
        private:
            uint8 platformUrom;
            uint32 CentrifugeConstructCnt;
            uint64 drakosGUID;
            uint64 varosGUID;
            uint64 uromGUID;
            uint64 eregosGUID;
            uint64 eregosCacheGUID;
            uint64 RampenlichtGUID;
            std::string str_data;
            std::list<uint64> GOList;
            std::list<uint64> DragonList;
    };

};

void AddSC_instance_oculus()
{
    new instance_oculus();
}
