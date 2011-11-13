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
#include "ScriptedEscortAI.h"

enum Yells
{
    SAY_JAINA_INTRO_1       = -1668001,
    SAY_JAINA_INTRO_2       = -1668002,
    SAY_JAINA_INTRO_3       = -1668003,
    SAY_JAINA_INTRO_4       = -1668004,
    SAY_UTHER_INTRO_A2_1    = -1668005,
    SAY_JAINA_INTRO_5       = -1668006,
    SAY_UTHER_INTRO_A2_2    = -1668007,
    SAY_JAINA_INTRO_6       = -1668008,
    SAY_UTHER_INTRO_A2_3    = -1668009,
    SAY_JAINA_INTRO_7       = -1668010,
    SAY_UTHER_INTRO_A2_4    = -1668011,
    SAY_JAINA_INTRO_8       = -1668012,
    SAY_UTHER_INTRO_A2_5    = -1668013,
    SAY_JAINA_INTRO_9       = -1668014,
    SAY_UTHER_INTRO_A2_6    = -1668015,
    SAY_UTHER_INTRO_A2_7    = -1668016,
    SAY_JAINA_INTRO_10      = -1668017,
    SAY_UTHER_INTRO_A2_8    = -1668018,
    SAY_JAINA_INTRO_11      = -1668019,
    SAY_UTHER_INTRO_A2_9    = -1668020,

    SAY_SYLVANAS_INTRO_1    = -1668021,
    SAY_SYLVANAS_INTRO_2    = -1668022,
    SAY_SYLVANAS_INTRO_3    = -1668023,
    SAY_UTHER_INTRO_H2_1    = -1668024,
    SAY_SYLVANAS_INTRO_4    = -1668025,
    SAY_UTHER_INTRO_H2_2    = -1668026,
    SAY_SYLVANAS_INTRO_5    = -1668027,
    SAY_UTHER_INTRO_H2_3    = -1668028,
    SAY_SYLVANAS_INTRO_6    = -1668029,
    SAY_UTHER_INTRO_H2_4    = -1668030,
    SAY_SYLVANAS_INTRO_7    = -1668031,
    SAY_UTHER_INTRO_H2_5    = -1668032,
    SAY_UTHER_INTRO_H2_6    = -1668033,
    SAY_SYLVANAS_INTRO_8    = -1668034,
    SAY_UTHER_INTRO_H2_7    = -1668035,

    SAY_LK_INTRO_1      = -1668036,
    SAY_LK_INTRO_2      = -1668037,
    SAY_LK_INTRO_3      = -1668038,
    SAY_FALRIC_INTRO_1  = -1668039,
    SAY_MARWYN_INTRO_1  = -1668040,
    SAY_FALRIC_INTRO_2  = -1668041,

    SAY_JAINA_INTRO_END         = -1668042,
    SAY_SYLVANAS_INTRO_END      = -1668043,
    SAY_LK_JAINA_INTRO_END      = -1594473,
    SAY_LK_SYLVANAS_INTRO_END   = -1594474
};

enum Events
{
    EVENT_NONE,

    EVENT_START_PREINTRO,
    EVENT_PREINTRO_1,
    EVENT_PREINTRO_2,

    EVENT_START_INTRO,
    EVENT_SKIP_INTRO,

    EVENT_INTRO_A2_1,
    EVENT_INTRO_A2_2,
    EVENT_INTRO_A2_3,
    EVENT_INTRO_A2_4,
    EVENT_INTRO_A2_5,
    EVENT_INTRO_A2_6,
    EVENT_INTRO_A2_7,
    EVENT_INTRO_A2_8,
    EVENT_INTRO_A2_9,
    EVENT_INTRO_A2_10,
    EVENT_INTRO_A2_11,
    EVENT_INTRO_A2_12,
    EVENT_INTRO_A2_13,
    EVENT_INTRO_A2_14,
    EVENT_INTRO_A2_15,
    EVENT_INTRO_A2_16,
    EVENT_INTRO_A2_17,
    EVENT_INTRO_A2_18,
    EVENT_INTRO_A2_19,

    EVENT_INTRO_H2_1,
    EVENT_INTRO_H2_2,
    EVENT_INTRO_H2_3,
    EVENT_INTRO_H2_4,
    EVENT_INTRO_H2_5,
    EVENT_INTRO_H2_6,
    EVENT_INTRO_H2_7,
    EVENT_INTRO_H2_8,
    EVENT_INTRO_H2_9,
    EVENT_INTRO_H2_10,
    EVENT_INTRO_H2_11,
    EVENT_INTRO_H2_12,
    EVENT_INTRO_H2_13,
    EVENT_INTRO_H2_14,
    EVENT_INTRO_H2_15,

    EVENT_INTRO_LK_1,
    EVENT_INTRO_LK_2,
    EVENT_INTRO_LK_3,
    EVENT_INTRO_LK_4,
    EVENT_INTRO_LK_5,
    EVENT_INTRO_LK_6,
    EVENT_INTRO_LK_7,
    EVENT_INTRO_LK_8,
    EVENT_INTRO_LK_9,
    EVENT_INTRO_LK_10,

    EVENT_INTRO_END
};

enum eEnum
{
    ACTION_START_INTRO,
    ACTION_SKIP_INTRO,

    QUEST_DELIVRANCE_FROM_THE_PIT_A2    = 24710,
    QUEST_DELIVRANCE_FROM_THE_PIT_H2    = 24712,
    QUEST_WRATH_OF_THE_LICH_KING_A2     = 24500,
    QUEST_WRATH_OF_THE_LICH_KING_H2     = 24802
};

enum Spells
{
    SPELL_CAST_VISUAL           = 65633, // Jaina/Sylavana lo lanzan para invocar a uther
    SPELL_BOSS_SPAWN_AURA       = 72712, // Falric and Marwyn
    SPELL_UTHER_DESPAWN         = 70693,
    SPELL_TAKE_FROSTMOURNE      = 72729,
    SPELL_FROSTMOURNE_DESPAWN   = 72726,
    SPELL_FROSTMOURNE_VISUAL    = 73220,
    SPELL_FROSTMOURNE_SOUNDS    = 70667
};

const Position HallsofReflectionLocs[]=
{
    {5283.234863f, 1990.946777f, 707.695679f, 0.929097f}, // 2 Loralen Follows
    {5408.031250f, 2102.918213f, 707.695251f, 0.792756f}, // 9 Sylvanas Follows
    {5401.866699f, 2110.837402f, 707.695251f, 0.800610f}  // 10 Loralen follows
};

const Position SpawnPos              = {5262.540527f, 1949.693726f, 707.695007f, 0.808736f}; // Jaina/Sylvanas Beginning Position
const Position MoveDoorPos           = {5268.254395f, 1955.522705f, 707.699585f, 1.277278f};
const Position MoveThronePos         = {5306.952148f, 1998.499023f, 709.341431f, 1.277278f}; // Jaina/Sylvanas walks to throne
const Position UtherSpawnPos         = {5308.310059f, 2003.857178f, 709.341431f, 4.650315f};
const Position LichKingSpawnPos      = {5362.917480f, 2062.307129f, 707.695374f, 3.945812f};
const Position LichKingMoveThronePos = {5312.080566f, 2009.172119f, 709.341431f, 3.973301f}; // Lich King walks to throne
const Position LichKingMoveAwayPos   = {5400.069824f, 2102.7131689f, 707.69525f, 0.843803f}; // Lich King walks away

class npc_jaina_or_sylvanas_hor : public CreatureScript
{
private:
    bool m_isSylvana;

public:
    npc_jaina_or_sylvanas_hor(bool isSylvana, const char* name) : CreatureScript(name), m_isSylvana(isSylvana) { }

    bool OnGossipSelect(Player * plr, Creature * creature, uint32 /*sender*/, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                plr->CLOSE_GOSSIP_MENU();
                if (creature->AI())
                    creature->AI()->DoAction(ACTION_START_INTRO);
                creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                plr->CLOSE_GOSSIP_MENU();
                if (creature->AI())
                    creature->AI()->DoAction(ACTION_SKIP_INTRO);
                creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                break;
        }
        return true;
    }

    bool OnGossipHello(Player * plr, Creature * creature)
    {
        if (creature->isQuestGiver())
            plr->PrepareQuestMenu(creature->GetGUID());

        QuestStatus status = plr->GetQuestStatus(m_isSylvana ? QUEST_DELIVRANCE_FROM_THE_PIT_H2 : QUEST_DELIVRANCE_FROM_THE_PIT_A2);
        if (status == QUEST_STATUS_COMPLETE || status == QUEST_STATUS_REWARDED)
            plr->ADD_GOSSIP_ITEM( 0, "Könnt ihr das Schwert entfernen?", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        QuestStatus status2 = plr->GetQuestStatus(m_isSylvana ? QUEST_WRATH_OF_THE_LICH_KING_H2 : QUEST_WRATH_OF_THE_LICH_KING_A2);
        if (status2 == QUEST_STATUS_COMPLETE || status2 == QUEST_STATUS_REWARDED)
            plr->ADD_GOSSIP_ITEM( 0, "Herrin, ich glaube, ich höre Arthas kommen. Was auch immer ihr vor habt, tut es schnell!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+2);

        plr->SEND_GOSSIP_MENU(DEFAULT_GOSSIP_MESSAGE, creature->GetGUID());
        return true;
    }

    // AI of Part1: handle the intro till start of gauntlet event.
    struct npc_jaina_or_sylvanas_horAI : public ScriptedAI
    {
        npc_jaina_or_sylvanas_horAI(Creature * creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        InstanceScript * instance;
        uint64 UtherGUID;
        uint64 LichKingGUID;

        EventMap events;

        void Reset()
        {
            events.Reset();

            UtherGUID = 0;
            LichKingGUID = 0;

            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            me->SetStandState(UNIT_STAND_STATE_STAND);
            me->SetVisible(true);
        }

        void DoAction(const int32 actionId)
        {
            switch (actionId)
            {
                case ACTION_START_INTRO:
                    events.ScheduleEvent(EVENT_START_PREINTRO, 0);
                    break;
                case ACTION_SKIP_INTRO:
                    events.ScheduleEvent(EVENT_SKIP_INTRO, 0);
                    break;
            }
        }

        void UpdateAI(const uint32 diff)
        {
            events.Update(diff);

            switch (events.ExecuteEvent())
            {
                case EVENT_START_PREINTRO:
                    me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                    me->GetMotionMaster()->MovePoint(0, MoveDoorPos);

                    if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                        events.ScheduleEvent(EVENT_PREINTRO_1, 0);
                    else
                        events.ScheduleEvent(EVENT_START_INTRO, 0);
                    break;

                case EVENT_PREINTRO_1:
                        DoScriptText(SAY_JAINA_INTRO_1, me);
                        events.ScheduleEvent(EVENT_PREINTRO_2, 6000);
                    break;

                case EVENT_PREINTRO_2:
                        DoScriptText(SAY_JAINA_INTRO_2, me);
                        events.ScheduleEvent(EVENT_START_INTRO, 7000);
                    break;

                case EVENT_START_INTRO:
                    me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                    me->GetMotionMaster()->MovePoint(0, MoveThronePos);
                    // Begining of intro is differents between factions as the speech sequence and timers are differents.
                    if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                        events.ScheduleEvent(EVENT_INTRO_A2_1, 0);
                    else
                        events.ScheduleEvent(EVENT_INTRO_H2_1, 0);
                    break;

            // A2 Intro Events
                case EVENT_INTRO_A2_1:
                    DoScriptText(SAY_JAINA_INTRO_3, me);
                    events.ScheduleEvent(EVENT_INTRO_A2_2, 5000);
                    break;
                case EVENT_INTRO_A2_2:
                    DoScriptText(SAY_JAINA_INTRO_4, me);
                    events.ScheduleEvent(EVENT_INTRO_A2_3, 10000);
                    break;
                case EVENT_INTRO_A2_3:
                    DoCast(me, SPELL_CAST_VISUAL);
                    me->CastSpell(me, SPELL_FROSTMOURNE_SOUNDS, true);
                    instance->HandleGameObject(instance->GetData64(DATA_FROSTMOURNE), true);
                    events.ScheduleEvent(EVENT_INTRO_A2_4, 10000);
                    break;
                case EVENT_INTRO_A2_4:
                    // spawn UTHER during speach 2
                    if (Creature * Uther = me->SummonCreature(NPC_UTHER, UtherSpawnPos))
                    {
                        Uther->GetMotionMaster()->MoveIdle();
                        Uther->SetReactState(REACT_PASSIVE); // be sure he will not aggro arthas
                        UtherGUID = Uther->GetGUID();
                        me->SetUInt64Value(UNIT_FIELD_TARGET, UtherGUID);
                        Uther->SetUInt64Value(UNIT_FIELD_TARGET, me->GetGUID());
                    }
                    events.ScheduleEvent(EVENT_INTRO_A2_5, 2000);
                    break;
                case EVENT_INTRO_A2_5:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                        DoScriptText(SAY_UTHER_INTRO_A2_1, Uther);
                    events.ScheduleEvent(EVENT_INTRO_A2_6, 3000);
                    break;
                case EVENT_INTRO_A2_6:
                    DoScriptText(SAY_JAINA_INTRO_5, me);
                    events.ScheduleEvent(EVENT_INTRO_A2_7, 6000);
                    break;
                case EVENT_INTRO_A2_7:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                        DoScriptText(SAY_UTHER_INTRO_A2_2, Uther);
                    events.ScheduleEvent(EVENT_INTRO_A2_8, 6500);
                    break;
                case EVENT_INTRO_A2_8:
                    DoScriptText(SAY_JAINA_INTRO_6, me);
                    events.ScheduleEvent(EVENT_INTRO_A2_9, 2000);
                    break;
                case EVENT_INTRO_A2_9:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                        DoScriptText(SAY_UTHER_INTRO_A2_3, Uther);
                    events.ScheduleEvent(EVENT_INTRO_A2_10, 9000);
                    break;
                case EVENT_INTRO_A2_10:
                    DoScriptText(SAY_JAINA_INTRO_7, me);
                    events.ScheduleEvent(EVENT_INTRO_A2_11, 5000);
                    break;
                case EVENT_INTRO_A2_11:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                        DoScriptText(SAY_UTHER_INTRO_A2_4, Uther);
                    events.ScheduleEvent(EVENT_INTRO_A2_12, 11000);
                    break;
                case EVENT_INTRO_A2_12:
                    DoScriptText(SAY_JAINA_INTRO_8, me);
                    events.ScheduleEvent(EVENT_INTRO_A2_13, 4000);
                    break;
                case EVENT_INTRO_A2_13:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                        DoScriptText(SAY_UTHER_INTRO_A2_5, Uther);
                    events.ScheduleEvent(EVENT_INTRO_A2_14, 12500);
                    break;
                case EVENT_INTRO_A2_14:
                    DoScriptText(SAY_JAINA_INTRO_9, me);
                    events.ScheduleEvent(EVENT_INTRO_A2_15, 10000);
                    break;
                case EVENT_INTRO_A2_15:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                        DoScriptText(SAY_UTHER_INTRO_A2_6, Uther);
                    events.ScheduleEvent(EVENT_INTRO_A2_16, 22000);
                    break;
                case EVENT_INTRO_A2_16:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                        DoScriptText(SAY_UTHER_INTRO_A2_7, Uther);
                    events.ScheduleEvent(EVENT_INTRO_A2_17, 4000);
                    break;
                case EVENT_INTRO_A2_17:
                    DoScriptText(SAY_JAINA_INTRO_10, me);
                    events.ScheduleEvent(EVENT_INTRO_A2_18, 2000);
                    break;
                case EVENT_INTRO_A2_18:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                    {
                        Uther->HandleEmoteCommand(EMOTE_ONESHOT_NO);
                        DoScriptText(SAY_UTHER_INTRO_A2_8, Uther);
                    }
                    events.ScheduleEvent(EVENT_INTRO_A2_19, 11000);
                    break;
                case EVENT_INTRO_A2_19:
                    DoScriptText(SAY_JAINA_INTRO_11, me);
                    events.ScheduleEvent(EVENT_INTRO_LK_1, 2000);
                    break;

                // H2 Intro Events
                case EVENT_INTRO_H2_1:
                    DoScriptText(SAY_SYLVANAS_INTRO_1, me);
                    events.ScheduleEvent(EVENT_INTRO_H2_2, 8000);
                    break;
                case EVENT_INTRO_H2_2:
                    DoScriptText(SAY_SYLVANAS_INTRO_2, me);
                    events.ScheduleEvent(EVENT_INTRO_H2_3, 6000);
                    break;
                case EVENT_INTRO_H2_3:
                    DoScriptText(SAY_SYLVANAS_INTRO_3, me);
                    DoCast(me, SPELL_CAST_VISUAL);
                    instance->HandleGameObject(instance->GetData64(DATA_FROSTMOURNE), true);
                    me->CastSpell(me, SPELL_FROSTMOURNE_SOUNDS, true);
                    events.ScheduleEvent(EVENT_INTRO_H2_4, 6000);
                    break;
                case EVENT_INTRO_H2_4:
                    // spawn UTHER during speach 2
                    if (Creature * Uther = me->SummonCreature(NPC_UTHER, UtherSpawnPos))
                    {
                        Uther->GetMotionMaster()->MoveIdle();
                        Uther->SetReactState(REACT_PASSIVE); // be sure he will not aggro arthas
                        UtherGUID = Uther->GetGUID();
                        me->SetUInt64Value(UNIT_FIELD_TARGET, UtherGUID);
                        Uther->SetUInt64Value(UNIT_FIELD_TARGET, me->GetGUID());
                    }
                    events.ScheduleEvent(EVENT_INTRO_H2_5, 2000);
                    break;
                case EVENT_INTRO_H2_5:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                        DoScriptText(SAY_UTHER_INTRO_H2_1, Uther);
                    events.ScheduleEvent(EVENT_INTRO_H2_6, 11000);
                    break;
                case EVENT_INTRO_H2_6:
                    DoScriptText(SAY_SYLVANAS_INTRO_4, me);
                    events.ScheduleEvent(EVENT_INTRO_H2_7, 3000);
                    break;
                case EVENT_INTRO_H2_7:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                        DoScriptText(SAY_UTHER_INTRO_H2_2, Uther);
                    events.ScheduleEvent(EVENT_INTRO_H2_8, 6000);
                    break;
                case EVENT_INTRO_H2_8:
                    DoScriptText(SAY_SYLVANAS_INTRO_5, me);
                    events.ScheduleEvent(EVENT_INTRO_H2_9, 5000);
                    break;
                case EVENT_INTRO_H2_9:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                        DoScriptText(SAY_UTHER_INTRO_H2_3, Uther);
                    events.ScheduleEvent(EVENT_INTRO_H2_10, 19000);
                    break;
                case EVENT_INTRO_H2_10:
                    DoScriptText(SAY_SYLVANAS_INTRO_6, me);
                    events.ScheduleEvent(EVENT_INTRO_H2_11, 1500);
                    break;
                case EVENT_INTRO_H2_11:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                        DoScriptText(SAY_UTHER_INTRO_H2_4, Uther);
                    events.ScheduleEvent(EVENT_INTRO_H2_12, 19500);
                    break;
                case EVENT_INTRO_H2_12:
                    DoScriptText(SAY_SYLVANAS_INTRO_7, me);
                    events.ScheduleEvent(EVENT_INTRO_H2_13, 2000);
                    break;
                case EVENT_INTRO_H2_13:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                    {
                        Uther->HandleEmoteCommand(EMOTE_ONESHOT_NO);
                        DoScriptText(SAY_UTHER_INTRO_H2_5, Uther);
                    }
                    events.ScheduleEvent(EVENT_INTRO_H2_14, 12000);
                    break;
                case EVENT_INTRO_H2_14:
                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                        DoScriptText(SAY_UTHER_INTRO_H2_6, Uther);
                    events.ScheduleEvent(EVENT_INTRO_H2_15, 8000);
                    break;
                case EVENT_INTRO_H2_15:
                    DoScriptText(SAY_SYLVANAS_INTRO_8, me);
                    events.ScheduleEvent(EVENT_INTRO_LK_1, 2000);
                    break;

                // Remaining Intro Events common for both faction
                case EVENT_INTRO_LK_1:
                    // Spawn LK in front of door, and make him move to the sword.
                    if (Creature * LichKing = me->SummonCreature(NPC_LICH_KING_EVENT, LichKingSpawnPos))
                    {
                        LichKing->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        LichKing->GetMotionMaster()->MovePoint(0, LichKingMoveThronePos);
                        LichKing->SetReactState(REACT_PASSIVE);
                        LichKingGUID = LichKing->GetGUID();

                        if (GameObject * gate = instance->instance->GetGameObject(instance->GetData64(DATA_FROSTWORN_DOOR)))
                             gate->SetGoState(GO_STATE_ACTIVE);

                        me->SetUInt64Value(UNIT_FIELD_TARGET, LichKingGUID);
                        LichKing->SetUInt64Value(UNIT_FIELD_TARGET, me->GetGUID());
                    }

                    if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                    {
                        Uther->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_COWER);
                        if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                            DoScriptText(SAY_UTHER_INTRO_A2_9, Uther);
                        else
                            DoScriptText(SAY_UTHER_INTRO_H2_7, Uther);
                    }

                    events.ScheduleEvent(EVENT_INTRO_LK_2, 11000);
                    break;

                case EVENT_INTRO_LK_2:
                     if (Creature * LichKing = me->GetCreature(*me, LichKingGUID))
                         DoScriptText(SAY_LK_INTRO_1, LichKing);
                    if (GameObject * gate = instance->instance->GetGameObject(instance->GetData64(DATA_FROSTWORN_DOOR)))
                        gate->SetGoState(GO_STATE_READY);
                     events.ScheduleEvent(EVENT_INTRO_LK_3, 2000);
                     break;

                case EVENT_INTRO_LK_3:
                     // The Lich King banishes Uther to the abyss.
                     if (Creature * Uther = me->GetCreature(*me, UtherGUID))
                     {
                         Uther->CastSpell(Uther, SPELL_UTHER_DESPAWN, true);
                         Uther->DisappearAndDie();
                         UtherGUID = 0;
                     }
                     events.ScheduleEvent(EVENT_INTRO_LK_4, 5000);
                     break;

                case EVENT_INTRO_LK_4:
                      if (Creature * LichKing = me->GetCreature(*me, LichKingGUID))
                          DoScriptText(SAY_LK_INTRO_2, LichKing);
                     // He steps forward and removes the runeblade from the heap of skulls.
                      if (GameObject *pFrostmourne = me->FindNearestGameObject(GO_FROSTMOURNE, 11.0f))
                         pFrostmourne->SetPhaseMask(0,true);

                      if (Creature * LichKing = me->GetCreature(*me, LichKingGUID))
                      {
                          LichKing->CastSpell(LichKing, SPELL_TAKE_FROSTMOURNE, true);
                          LichKing->CastSpell(LichKing, SPELL_FROSTMOURNE_VISUAL, true);
                      }

                      me->RemoveAllAuras();

                    events.ScheduleEvent(EVENT_INTRO_LK_5, 10000);
                    break;

                case EVENT_INTRO_LK_5:
                    // summon Falric and Marwyn. then go back to the door
                    if (Creature * Falric = me->GetCreature(*me, instance->GetData64(DATA_FALRIC)))
                    {
                        Falric->CastSpell(Falric, SPELL_BOSS_SPAWN_AURA, true);
                        Falric->SetVisible(true);
                        Falric->GetMotionMaster()->MovePoint(0, 5283.309f, 2031.173f, 709.319f);
                    }
                    if (Creature * Marwyn = me->GetCreature(*me, instance->GetData64(DATA_MARWYN)))
                    {
                        Marwyn->CastSpell(Marwyn, SPELL_BOSS_SPAWN_AURA, true);
                        Marwyn->SetVisible(true);
                        Marwyn->GetMotionMaster()->MovePoint(0, 5335.585f, 1981.439f, 709.319f);
                    }

                    if (Creature * LichKing = me->GetCreature(*me, LichKingGUID))
                        DoScriptText(SAY_LK_INTRO_3, LichKing);

                    events.ScheduleEvent(EVENT_INTRO_LK_6, 8000);
                    break;

                case EVENT_INTRO_LK_6:
                    if (Creature * Falric = me->GetCreature(*me, instance->GetData64(DATA_FALRIC)))
                        DoScriptText(SAY_FALRIC_INTRO_1, Falric);

                    events.ScheduleEvent(EVENT_INTRO_LK_7, 2000);
                    break;

                case EVENT_INTRO_LK_7:
                    if (Creature * Marwyn = me->GetCreature(*me, instance->GetData64(DATA_MARWYN)))
                        DoScriptText(SAY_MARWYN_INTRO_1, Marwyn);

                    if (Creature * LichKing = me->GetCreature(*me, LichKingGUID))
                    {
                        if (GameObject * gate = instance->instance->GetGameObject(instance->GetData64(DATA_FROSTWORN_DOOR)))
                            gate->SetGoState(GO_STATE_ACTIVE);
                        LichKing->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        LichKing->GetMotionMaster()->MovePoint(0, LichKingMoveAwayPos);
                    }

                    events.ScheduleEvent(EVENT_INTRO_LK_8, 2000);
                    break;

                case EVENT_INTRO_LK_8:
                    if (Creature * Falric = me->GetCreature(*me, instance->GetData64(DATA_FALRIC)))
                        DoScriptText(SAY_FALRIC_INTRO_2, Falric);

                    events.ScheduleEvent(EVENT_INTRO_LK_9, 5000);
                    break;

                case EVENT_INTRO_LK_9:
                    if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                        DoScriptText(SAY_JAINA_INTRO_END, me);
                    else
                        DoScriptText(SAY_SYLVANAS_INTRO_END, me);

                    me->GetMotionMaster()->MovePoint(0, LichKingMoveAwayPos);

                    if (Creature * LichKing = me->GetCreature(*me, LichKingGUID))
                    {
                        LichKing->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                        LichKing->GetMotionMaster()->MovePoint(0, LichKingMoveAwayPos);
                    }

                    events.ScheduleEvent(EVENT_INTRO_LK_10, 5000);
                    break;

                case EVENT_INTRO_LK_10:
                     if (Creature * LichKing = me->GetCreature(*me, LichKingGUID))
                     {
                         if (instance->GetData(DATA_TEAM_IN_INSTANCE) == ALLIANCE)
                            DoScriptText(SAY_LK_JAINA_INTRO_END, LichKing);
                         else
                             DoScriptText(SAY_LK_SYLVANAS_INTRO_END, LichKing);
                     }
                     events.ScheduleEvent(EVENT_INTRO_END, 7000);
                     break;

                case EVENT_INTRO_END:
                    if (instance)
                    {
                        instance->SetData(DATA_INTRO_EVENT, DONE);
                        instance->SetData(DATA_WAVE_COUNT, 99); // start first wave
                        instance->SetData(DATA_FALRIC_EVENT, IN_PROGRESS);
                    }

                    if (GameObject * gate = instance->instance->GetGameObject(instance->GetData64(DATA_FROSTWORN_DOOR)))
                        gate->SetGoState(GO_STATE_READY);

                    // Loralen or Koreln disappearAndDie()
                    me->DisappearAndDie();
                    if (Creature * LichKing = me->GetCreature(*me, LichKingGUID))
                        LichKing->DisappearAndDie();

                    break;

                case EVENT_SKIP_INTRO:
                   // Spawn LK in front of door, and make him move to the sword.
                    if (Creature * LichKing = me->SummonCreature(NPC_LICH_KING_EVENT, LichKingSpawnPos, TEMPSUMMON_MANUAL_DESPAWN))
                    {
                        LichKing->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                        LichKing->GetMotionMaster()->MovePoint(0, LichKingMoveThronePos);
                        LichKing->SetReactState(REACT_PASSIVE);
                        LichKingGUID = LichKing->GetGUID();
                        me->SetUInt64Value(UNIT_FIELD_TARGET, LichKingGUID);
                        LichKing->SetUInt64Value(UNIT_FIELD_TARGET, me->GetGUID());
                    }

                    me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                    me->GetMotionMaster()->MovePoint(0, MoveThronePos);
                    events.ScheduleEvent(EVENT_INTRO_LK_4, 20000);
                    break;
            }
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_jaina_or_sylvanas_horAI(creature);
    }
};

enum TrashSpells
{
    // Ghostly Priest
    SPELL_SHADOW_WORD_PAIN          = 72318,
    SPELL_CIRCLE_OF_DESTRUCTION     = 72320,
    SPELL_COWER_IN_FEAR             = 72321,
    SPELL_DARK_MENDING              = 72322,

    // Phantom Mage
    SPELL_FIREBALL                  = 72163,
    SPELL_FLAMESTRIKE               = 72169,
    SPELL_FROSTBOLT                 = 72166,
    SPELL_CHAINS_OF_ICE             = 72121,
    SPELL_HALLUCINATION             = 72342,

    // Phantom Hallucination (same as phantom mage + HALLUCINATION_2 when dies)
    SPELL_HALLUCINATION_2           = 72344,

    // Shadowy Mercenary
    SPELL_SHADOW_STEP               = 72326,
    SPELL_DEADLY_POISON             = 72329,
    SPELL_ENVENOMED_DAGGER_THROW    = 72333,
    SPELL_KIDNEY_SHOT               = 72335,

    // Spectral Footman
    SPELL_SPECTRAL_STRIKE           = 72198,
    SPELL_SHIELD_BASH               = 72194,
    SPELL_TORTURED_ENRAGE           = 72203,

    // Tortured Rifleman
    SPELL_SHOOT                     = 72208,
    SPELL_CURSED_ARROW              = 72222,
    SPELL_FROST_TRAP                = 72215,
    SPELL_ICE_SHOT                  = 72268,

    // Quel'Delar Event
    SPELL_HELDENHAFTER_STOSS        = 29426,
    SPELL_KLINGENSTURM              = 67541,
    SPELL_WHIRLWIND_VISUAL          = 70300,
    SPELL_TOEDLICHER_STOSS          = 16856
};

enum TrashEvents
{
    EVENT_TRASH_NONE,

    // Ghostly Priest
    EVENT_SHADOW_WORD_PAIN,
    EVENT_CIRCLE_OF_DESTRUCTION,
    EVENT_COWER_IN_FEAR,
    EVENT_DARK_MENDING,

    // Phantom Mage
    EVENT_FIREBALL,
    EVENT_FLAMESTRIKE,
    EVENT_FROSTBOLT,
    EVENT_CHAINS_OF_ICE,
    EVENT_HALLUCINATION,

    // Shadowy Mercenary
    EVENT_SHADOW_STEP,
    EVENT_DEADLY_POISON,
    EVENT_ENVENOMED_DAGGER_THROW,
    EVENT_KIDNEY_SHOT,

    // Spectral Footman
    EVENT_SPECTRAL_STRIKE,
    EVENT_SHIELD_BASH,
    EVENT_TORTURED_ENRAGE,

    // Tortured Rifleman
    EVENT_SHOOT,
    EVENT_CURSED_ARROW,
    EVENT_FROST_TRAP,
    EVENT_ICE_SHOT,

    // Quel'Delar
    EVENT_HELDENHAFTER_STOSS,
    EVENT_KLINGENSTURM,
    EVENT_TOEDLICHER_STOSS
};

class npc_ghostly_priest : public CreatureScript
{
public:
    npc_ghostly_priest() : CreatureScript("npc_ghostly_priest") { }

    struct npc_ghostly_priestAI: public ScriptedAI
    {
        npc_ghostly_priestAI(Creature * creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_SHADOW_WORD_PAIN, SEKUNDEN_05);
            events.ScheduleEvent(EVENT_CIRCLE_OF_DESTRUCTION, SEKUNDEN_10);
            events.ScheduleEvent(EVENT_COWER_IN_FEAR, SEKUNDEN_15);
            events.ScheduleEvent(EVENT_DARK_MENDING, SEKUNDEN_20);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SHADOW_WORD_PAIN:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        {
                            DoCast(target, SPELL_SHADOW_WORD_PAIN);
                            events.RescheduleEvent(EVENT_SHADOW_WORD_PAIN, 8 * IN_MILLISECONDS);
                        }
                        else
                            events.RescheduleEvent(EVENT_SHADOW_WORD_PAIN, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_CIRCLE_OF_DESTRUCTION:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 10.0f, true))
                        {
                            DoCast(target, SPELL_CIRCLE_OF_DESTRUCTION);
                            events.RescheduleEvent(EVENT_CIRCLE_OF_DESTRUCTION, 12 * IN_MILLISECONDS);
                        }
                        else
                            events.RescheduleEvent(EVENT_CIRCLE_OF_DESTRUCTION, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_COWER_IN_FEAR:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 20.0f, true))
                        {
                            DoCast(target, SPELL_COWER_IN_FEAR);
                            events.RescheduleEvent(EVENT_COWER_IN_FEAR, SEKUNDEN_10);
                        }
                        else
                            events.RescheduleEvent(EVENT_COWER_IN_FEAR, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_DARK_MENDING:
                        if (Unit * target = DoSelectLowestHpFriendly(40.0f, 30000))
                        {
                            DoCast(target, SPELL_DARK_MENDING);
                            events.RescheduleEvent(EVENT_DARK_MENDING, SEKUNDEN_20);
                        }
                        else
                        {
                            DoCast(SPELL_DARK_MENDING);
                            events.RescheduleEvent(EVENT_DARK_MENDING, urand(SEKUNDEN_10, SEKUNDEN_20));
                        }
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_ghostly_priestAI(creature);
    }
};

class npc_phantom_mage : public CreatureScript
{
public:
    npc_phantom_mage() : CreatureScript("npc_phantom_mage") { }

    struct npc_phantom_mageAI: public ScriptedAI
    {
        npc_phantom_mageAI(Creature * creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_FIREBALL, 1 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_FLAMESTRIKE, SEKUNDEN_05);
            events.ScheduleEvent(EVENT_FROSTBOLT, SEKUNDEN_10);
            events.ScheduleEvent(EVENT_CHAINS_OF_ICE, SEKUNDEN_15);
            events.ScheduleEvent(EVENT_HALLUCINATION, SEKUNDEN_30);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_FIREBALL:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        {
                            DoCast(target, SPELL_FIREBALL);
                            events.RescheduleEvent(EVENT_FIREBALL, urand(SEKUNDEN_10, SEKUNDEN_15));
                        }
                        else
                            events.RescheduleEvent(EVENT_FIREBALL, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_FLAMESTRIKE:
                        DoCast(SPELL_FLAMESTRIKE);
                        events.ScheduleEvent(EVENT_FLAMESTRIKE, urand(SEKUNDEN_10, SEKUNDEN_15));
                        break;
                    case EVENT_FROSTBOLT:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        {
                            DoCast(target, SPELL_FROSTBOLT);
                            events.RescheduleEvent(EVENT_FROSTBOLT, urand(SEKUNDEN_10, SEKUNDEN_15));
                        }
                        else
                            events.RescheduleEvent(EVENT_FROSTBOLT, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_CHAINS_OF_ICE:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                        {
                            DoCast(target, SPELL_CHAINS_OF_ICE);
                            events.RescheduleEvent(EVENT_CHAINS_OF_ICE, urand(SEKUNDEN_10, SEKUNDEN_20));
                        }
                        else
                            events.RescheduleEvent(EVENT_CHAINS_OF_ICE, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_HALLUCINATION:
                        DoCast(SPELL_HALLUCINATION);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_phantom_mageAI(creature);
    }
};

class npc_phantom_hallucination : public CreatureScript
{
public:
    npc_phantom_hallucination() : CreatureScript("npc_phantom_hallucination") { }

    struct npc_phantom_hallucinationAI : public npc_phantom_mage::npc_phantom_mageAI
    {
        npc_phantom_hallucinationAI(Creature * creature) : npc_phantom_mage::npc_phantom_mageAI(creature)
        {
        }

        void JustDied(Unit * /*killer*/)
        {
            DoCast(SPELL_HALLUCINATION_2);
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_phantom_hallucinationAI(creature);
    }
};

class npc_shadowy_mercenary : public CreatureScript
{
public:
    npc_shadowy_mercenary() : CreatureScript("npc_shadowy_mercenary") { }

    struct npc_shadowy_mercenaryAI: public ScriptedAI
    {
        npc_shadowy_mercenaryAI(Creature * creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_SHADOW_STEP, SEKUNDEN_10);
            events.ScheduleEvent(EVENT_DEADLY_POISON, SEKUNDEN_05);
            events.ScheduleEvent(EVENT_ENVENOMED_DAGGER_THROW, SEKUNDEN_10);
            events.ScheduleEvent(EVENT_KIDNEY_SHOT, SEKUNDEN_15);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SHADOW_STEP:
                        DoCast(SPELL_SHADOW_STEP);
                        events.RescheduleEvent(EVENT_SHADOW_STEP, urand(SEKUNDEN_10, SEKUNDEN_15));
                        break;
                    case EVENT_DEADLY_POISON:
                        DoCastVictim(SPELL_DEADLY_POISON);
                        events.ScheduleEvent(EVENT_DEADLY_POISON, urand(SEKUNDEN_05, SEKUNDEN_15));
                        break;
                    case EVENT_ENVENOMED_DAGGER_THROW:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        {
                            DoCast(target, SPELL_ENVENOMED_DAGGER_THROW);
                            events.RescheduleEvent(EVENT_ENVENOMED_DAGGER_THROW, urand(SEKUNDEN_05, SEKUNDEN_10));
                        }
                        else
                            events.RescheduleEvent(EVENT_ENVENOMED_DAGGER_THROW, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_KIDNEY_SHOT:
                        DoCastVictim(SPELL_KIDNEY_SHOT);
                        events.RescheduleEvent(EVENT_KIDNEY_SHOT, urand(SEKUNDEN_05, SEKUNDEN_10));
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_shadowy_mercenaryAI(creature);
    }
};

class npc_spectral_footman : public CreatureScript
{
public:
    npc_spectral_footman() : CreatureScript("npc_spectral_footman") { }

    struct npc_spectral_footmanAI: public ScriptedAI
    {
        npc_spectral_footmanAI(Creature * creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_SPECTRAL_STRIKE, SEKUNDEN_05);
            events.ScheduleEvent(EVENT_SHIELD_BASH, SEKUNDEN_10);
            events.ScheduleEvent(EVENT_TORTURED_ENRAGE, SEKUNDEN_15);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SPECTRAL_STRIKE:
                        DoCastVictim(SPELL_SPECTRAL_STRIKE);
                        events.RescheduleEvent(EVENT_SPECTRAL_STRIKE, SEKUNDEN_05);
                        break;
                    case EVENT_SHIELD_BASH:
                        DoCastVictim(SPELL_SHIELD_BASH);
                        events.RescheduleEvent(EVENT_SHIELD_BASH, SEKUNDEN_05);
                        break;
                    case EVENT_TORTURED_ENRAGE:
                        DoCast(SPELL_TORTURED_ENRAGE);
                        events.RescheduleEvent(EVENT_TORTURED_ENRAGE, SEKUNDEN_15);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_spectral_footmanAI(creature);
    }
};

class npc_tortured_rifleman : public CreatureScript
{
public:
    npc_tortured_rifleman() : CreatureScript("npc_tortured_rifleman") { }

    struct npc_tortured_riflemanAI  : public ScriptedAI
    {
        npc_tortured_riflemanAI(Creature * creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_SHOOT, 2 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_CURSED_ARROW, SEKUNDEN_05);
            events.ScheduleEvent(EVENT_FROST_TRAP, 1 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_ICE_SHOT, SEKUNDEN_10);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SHOOT:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        {
                            DoCast(target, SPELL_SHOOT);
                            events.RescheduleEvent(EVENT_SHOOT, 2 * IN_MILLISECONDS);
                        }
                        else
                            events.RescheduleEvent(EVENT_SHOOT, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_CURSED_ARROW:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        {
                            DoCast(target, SPELL_CURSED_ARROW);
                            events.RescheduleEvent(EVENT_CURSED_ARROW, urand(SEKUNDEN_05, SEKUNDEN_15));
                        }
                        else
                            events.RescheduleEvent(EVENT_CURSED_ARROW, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_FROST_TRAP:
                        DoCast(SPELL_FROST_TRAP);
                        events.RescheduleEvent(EVENT_FROST_TRAP, SEKUNDEN_30);
                        break;
                    case EVENT_ICE_SHOT:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f, true))
                        {
                            DoCast(target, SPELL_ICE_SHOT);
                            events.RescheduleEvent(EVENT_ICE_SHOT, urand(SEKUNDEN_10, SEKUNDEN_15));
                        }
                        else
                            events.RescheduleEvent(EVENT_ICE_SHOT, 1 * IN_MILLISECONDS);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_tortured_riflemanAI(creature);
    }
};

enum GENERAL_EVENT
{
    EVENT_SHIELD = 1,
    EVENT_SPIKE,

    SAY_AGGRO   = -1594519,
    SAY_DEATH   = -1594520,

    SPELL_SHIELD_THROWN     = 69222,
    H_SPELL_SHIELD_THROWN   = 73076,
    SPELL_SPIKE             = 69184,
    H_SPELL_SPIKE           = 70399,
    SPELL_CLONE_NAME        = 57507,
    SPELL_CLONE_MODEL       = 45204,

    // Reflection'Spells
    SPELL_BALEFUL_STRIKE    = 69933,
    SPELL_SPIRIT_BURST      = 69900,
    H_SPELL_BALEFUL_STRIKE  = 70400,
    H_SPELL_SPIRIT_BURST    = 73046
};

class npc_frostworn_general : public CreatureScript
{
public:
    npc_frostworn_general() : CreatureScript("npc_frostworn_general") { }

    struct npc_frostworn_generalAI : public ScriptedAI
    {
        npc_frostworn_generalAI(Creature * creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript * instance;
        EventMap events;

        bool CloneDone;

        void Reset()
        {
            if (instance)
                instance->SetData(DATA_FROSWORN_EVENT, NOT_STARTED);

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            events.Reset();

            CloneDone = false;
        }

        void SummonClones()
        {
            std::list<Unit *> playerList;
            SelectTargetList(playerList, 5, SELECT_TARGET_TOPAGGRO, 0, true);
            for (std::list<Unit *>::const_iterator itr = playerList.begin(); itr != playerList.end(); ++itr)
            {
                Unit * tmp = (*itr);
                Creature * Reflection = me->SummonCreature(NPC_REFLECTION, tmp->GetPositionX(), tmp->GetPositionY(), tmp->GetPositionZ(), tmp->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 3000);
                Reflection->SetName(tmp->GetName());
                tmp->CastSpell(Reflection, SPELL_CLONE_NAME, true);
                tmp->CastSpell(Reflection, SPELL_CLONE_MODEL, true);
                Reflection->setFaction(me->getFaction());
                Reflection->AI()->AttackStart(tmp);
            }
            CloneDone = true;
        }

        void JustDied(Unit * /*killer*/)
        {
            DoScriptText(SAY_DEATH, me);

            if (instance)
                instance->SetData(DATA_FROSWORN_EVENT, DONE);
        }

        void MoveInLineOfSight(Unit * who)
        {
            if (!instance || !who)
                return;

            if (me->getVictim())
                return;

            if (who->GetTypeId() != TYPEID_PLAYER || instance->GetData(DATA_MARWYN_EVENT) != DONE || !me->IsWithinDistInMap(who, 20.0f))
                return;

            if (Player * plr = who->ToPlayer())
                if (plr->isGameMaster())
                    return;

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            AttackStart(who);
        }

        void EnterCombat(Unit * /*victim*/)
        {
            events.ScheduleEvent(EVENT_SHIELD, SEKUNDEN_05);
            events.ScheduleEvent(EVENT_SPIKE, urand(SEKUNDEN_10, SEKUNDEN_15));

            DoScriptText(SAY_AGGRO, me);

            if (instance)
                instance->SetData(DATA_FROSWORN_EVENT, IN_PROGRESS);

            DoZoneInCombat();
        }

        void DamageTaken(Unit * /*attacker*/, uint32 & dmg)
        {
            if (!CloneDone && me->HealthBelowPctDamaged(50, dmg))
                SummonClones();
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STAT_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_SHIELD:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 45.0f, true))
                        {
                            DoCast(target, SPELL_SHIELD_THROWN);
                            events.RescheduleEvent(EVENT_SHIELD, 2 * IN_MILLISECONDS);
                        }
                        else
                            events.RescheduleEvent(EVENT_SHIELD, 1 * IN_MILLISECONDS);
                        break;
                    case EVENT_SPIKE:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 45.0f, true))
                        {
                            DoCast(target, SPELL_SPIKE);
                            events.RescheduleEvent(EVENT_SPIKE, urand(SEKUNDEN_05, SEKUNDEN_15));
                        }
                        else
                            events.RescheduleEvent(EVENT_SPIKE, 1 * IN_MILLISECONDS);
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_frostworn_generalAI(creature);
    }
};

class npc_spiritual_reflection : public CreatureScript
{
public:
    npc_spiritual_reflection() : CreatureScript("npc_spiritual_reflection") { }

    struct npc_spiritual_reflectionAI : public ScriptedAI
    {
        npc_spiritual_reflectionAI(Creature * creature) : ScriptedAI(creature) { }

        uint32 StrikeTimer;

        void Reset()
        {
            StrikeTimer = urand(1000,3000);
        }

        void JustDied(Unit * killer)
        {
            DoCast(killer, SPELL_SPIRIT_BURST);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (StrikeTimer < diff)
            {
                if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 8.0f))
                {
                    DoCast(target, SPELL_BALEFUL_STRIKE);
                    StrikeTimer = urand(3000, 8000);
                }
                else
                    StrikeTimer = 1 * IN_MILLISECONDS;
            }
            else
                StrikeTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_spiritual_reflectionAI(creature);
    }
};

enum Defs
{
    /*INTRO - Pre Escape*/
    SAY_LICH_KING_AGGRO_A   = -1594477,
    SAY_LICH_KING_AGGRO_H   = -1594478,
    SAY_JAINA_AGGRO         = -1594479,
    SAY_SYLVANA_AGGRO       = -1594480,

    /*ESCAPE*/
    SAY_JAINA_WALL_01       = -1594487,
    SAY_SYLVANA_WALL_01     = -1594488,
    SAY_JAINA_WALL_02       = -1594489,
    SAY_SYLVANA_WALL_02     = -1594490,
    SAY_LICH_KING_WALL_02   = -1594491,
    SAY_LICH_KING_WALL_03   = -1594492,
    SAY_LICH_KING_WALL_04   = -1594493,
    SAY_JAINA_WALL_03       = -1594494,
    SAY_JAINA_WALL_04       = -1594495,
    SAY_SYLVANA_WALL_03     = -1594496,
    SAY_SYLVANA_WALL_04     = -1594497,
    SAY_JAINA_ESCAPE_01     = -1594498,
    SAY_JAINA_ESCAPE_02     = -1594499,
    SAY_SYLVANA_ESCAPE_01   = -1594500,
    SAY_SYLVANA_ESCAPE_02   = -1594501,
    SAY_JAINA_TRAP          = -1594502,
    SAY_SYLVANA_TRAP        = -1594503,
    SAY_MATHEAS_JAINA       = -1594505,
    SAY_JAINA_FIRE          = -1594526,
    SAY_SYLVANA_FIRE        = -1594522,
    SAY_SYLVANA_FINAL_1     = -1594523,
    SAY_SYLVANA_FINAL_2     = -1594521,
    SAY_JAINA_FINAL_1       = -1594527,
    SAY_JAINA_FINAL_2       = -1594524,
    SAY_JAINA_FINAL_3       = -1594525,
    SAY_LICH_KING_END_01    = -1594506,
    SAY_LICH_KING_END_02    = -1594507,
    SAY_LICH_KING_END_03    = -1594508,

    /*SPELLS AND VISUAL EFFECTS*/
    SPELL_WINTER                = 69780,
    SPELL_FURY_OF_FROSTMOURNE   = 70063,
    SPELL_ICE_PRISON_VISUAL     = 69708,
    SPELL_DARK_ARROW            = 70194,
    SPELL_ICE_BARRIER           = 69787, // Jaina
    SPELL_DESTROY_ICE_WALL_01   = 69784, // Jaina
    SPELL_DESTROY_ICE_WALL_03   = 70225, // Sylvana
    SPELL_SYLVANA_JUMP          = 68339,
    SPELL_SYLVANA_STEP          = 69087,
    SPELL_FIRE_CANNON           = 67461,

    FACTION = 2076
};

const Position CannonSpawns[4] =
{
    {5230.00f, 1658.75f, 802.22f, 0.00f},
    {5245.74f, 1644.44f, 802.35f, 0.00f},
    {5260.64f, 1636.07f, 802.16f, 0.00f},
    {5275.90f, 1633.22f, 802.25f, 0.00f}
};

class npc_jaina_and_sylvana_hor_part2 : public CreatureScript
{
public:
    npc_jaina_and_sylvana_hor_part2() : CreatureScript("npc_jaina_and_sylvana_hor_part2") { }

    bool OnGossipSelect(Player * plr, Creature * creature, uint32 /*sender*/, uint32 action)
    {
        InstanceScript * instance = creature->GetInstanceScript();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                plr->CLOSE_GOSSIP_MENU();
                CAST_AI(npc_jaina_and_sylvana_hor_part2AI, creature->AI())->Start(false,true);
                creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                creature->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                creature->SetUInt64Value(UNIT_FIELD_TARGET, 0);
                creature->setActive(true);

                if (instance)
                {
                    instance->SetData64(DATA_ESCAPE_LEADER, creature->GetGUID());
                    instance->SetData(DATA_LICHKING_EVENT, IN_PROGRESS);
                }
                return true;
            default:
                return false;
        }
    }

    bool OnGossipHello(Player * plr, Creature * creature)
    {
        InstanceScript * instance = creature->GetInstanceScript();

        if (!instance)
            return false;

        if (instance->GetData(DATA_LICHKING_EVENT) == DONE)
            return false;

        if (creature->isQuestGiver())
           plr->PrepareQuestMenu( creature->GetGUID());

        plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Wir sind bereit!", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        plr->SEND_GOSSIP_MENU(plr->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }

    struct npc_jaina_and_sylvana_hor_part2AI : public npc_escortAI
    {
        npc_jaina_and_sylvana_hor_part2AI(Creature * creature) : npc_escortAI(creature)
        {
            instance = creature->GetInstanceScript();
        }

        InstanceScript * instance;

        uint32 CastTimer;
        uint32 StepTimer;
        uint32 Step;
        int32 HoldTimer;
        uint32 Count;
        bool Fight;
        bool Event;
        bool PreFight;
        bool WallCast;
        uint64 LichKingGUID;
        //uint64 LeaderGUID;
        uint64 IceWallGUID;
        uint64 WallTargetGUID;
        uint64 CaptainGUID;
        Creature * LichKing;
        //uint32 ChestID;

        void Reset()
        {
            if (instance && instance->GetData(DATA_LICHKING_EVENT) == IN_PROGRESS)
                return;

            Step = 0;
            StepTimer = 500;
            Fight = true;
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            WallTargetGUID = 0;

            if (me->GetEntry() == NPC_JAINA_OUTRO)
            {
                me->CastSpell(me, SPELL_ICE_BARRIER, false);
                me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY2HL);
            }

            if (instance && instance->GetData(DATA_LICHKING_EVENT) == DONE)
                me->SetVisible(false);
        }

        void AttackStart(Unit * who)
        {
            if (!who)
                return;

            if (me->GetEntry() != NPC_SYLVANA_OUTRO)
                return;

            if (instance->GetData(DATA_LICHKING_EVENT) == IN_PROGRESS || Fight != true)
                return;

            npc_escortAI::AttackStart(who);

        }

        void JustDied(Unit * /*killer*/)
        {
            if (instance)
                instance->SetData(DATA_LICHKING_EVENT, FAIL);
        }

        void WaypointReached(uint32 i)
        {
            switch(i)
            {
                case 3:
                    instance->SetData(DATA_ICE_WALL_1, IN_PROGRESS);
                    if (GameObject * gate = instance->instance->GetGameObject(instance->GetData64(GO_ICE_WALL_1)))
                    {
                        gate->SetGoState(GO_STATE_READY);
                        IceWallGUID = gate->GetGUID();
                    }
                    break;
                case 4:
                    if (me->GetEntry() == NPC_JAINA_OUTRO)
                        DoScriptText(SAY_JAINA_WALL_01, me);
                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                        DoScriptText(SAY_SYLVANA_WALL_01, me);
                    CastTimer = 1000;
                    HoldTimer = 30000;
                    SetEscortPaused(true);
                    if (Creature *WallTarget = me->SummonCreature(NPC_ICE_WALL,me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation(),TEMPSUMMON_MANUAL_DESPAWN,720000))
                    {
                        WallTargetGUID = WallTarget->GetGUID();
                        WallTarget->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        if (me->GetEntry() == NPC_JAINA_OUTRO)
                            me->CastSpell(me, SPELL_DESTROY_ICE_WALL_01, false);
                    }
                    WallCast = true;
                    break;
                case 6:
                    instance->SetData(DATA_ICE_WALL_2, IN_PROGRESS);
                    if (Creature * WallTarget = instance->instance->GetCreature(WallTargetGUID))
                    {
                        if (WallTarget->isAlive())
                        {
                            WallTarget->ForcedDespawn();
                            WallTargetGUID = 0;
                        }
                    }
                    break;
                case 8:
                    if (me->GetEntry() == NPC_JAINA_OUTRO)
                        DoScriptText(SAY_JAINA_WALL_02, me);
                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                        DoScriptText(SAY_SYLVANA_WALL_02, me);
                    CastTimer = 1000;
                    HoldTimer = 30000;
                    SetEscortPaused(true);
                    if (Creature *WallTarget = me->SummonCreature(NPC_ICE_WALL,me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation(),TEMPSUMMON_MANUAL_DESPAWN,720000))
                    {
                        WallTargetGUID = WallTarget->GetGUID();
                        WallTarget->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        if (me->GetEntry() == NPC_JAINA_OUTRO)
                            me->CastSpell(me, SPELL_DESTROY_ICE_WALL_01, false);
                    }
                    WallCast = true;
                    break;
                case 9:
                    if (me->GetEntry() == NPC_JAINA_OUTRO)
                        DoScriptText(SAY_JAINA_ESCAPE_01, me);
                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                        DoScriptText(SAY_SYLVANA_ESCAPE_01, me);
                    break;
                case 11:
                    instance->SetData(DATA_ICE_WALL_3, IN_PROGRESS);
                    if (Creature * WallTarget = instance->instance->GetCreature(WallTargetGUID))
                    {
                        if (WallTarget->isAlive())
                        {
                            WallTarget->ForcedDespawn();
                            WallTargetGUID = 0;
                        }
                    }
                    break;
                case 12:
                    if (me->GetEntry() == NPC_JAINA_OUTRO)
                        DoScriptText(SAY_JAINA_WALL_03, me);
                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                        DoScriptText(SAY_SYLVANA_WALL_03, me);
                    CastTimer = 1000;
                    HoldTimer = 30000;
                    SetEscortPaused(true);
                    if (Creature *WallTarget = me->SummonCreature(NPC_ICE_WALL,me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation(),TEMPSUMMON_MANUAL_DESPAWN,720000))
                    {
                        WallTargetGUID = WallTarget->GetGUID();
                        WallTarget->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        if (me->GetEntry() == NPC_JAINA_OUTRO)
                            me->CastSpell(me, SPELL_DESTROY_ICE_WALL_01, false);
                    }
                    WallCast = true;
                    break;
                case 13:
                    if (me->GetEntry() == NPC_JAINA_OUTRO)
                        DoScriptText(SAY_JAINA_ESCAPE_02, me);
                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                        DoScriptText(SAY_SYLVANA_ESCAPE_02, me);
                    break;
                case 15:
                    instance->SetData(DATA_ICE_WALL_4, IN_PROGRESS);
                    if (Creature * WallTarget = instance->instance->GetCreature(WallTargetGUID))
                    {
                        if (WallTarget->isAlive())
                        {
                            WallTarget->ForcedDespawn();
                            WallTargetGUID = 0;
                        }
                    }
                    break;
                case 16:
                    if (me->GetEntry() == NPC_JAINA_OUTRO)
                        DoScriptText(SAY_JAINA_WALL_04, me);
                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                        DoScriptText(SAY_SYLVANA_WALL_04, me);
                    CastTimer = 1000;
                    HoldTimer = 30000;
                    SetEscortPaused(true);
                    if (Creature *WallTarget = me->SummonCreature(NPC_ICE_WALL,me->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation(),TEMPSUMMON_MANUAL_DESPAWN,720000))
                    {
                        WallTargetGUID = WallTarget->GetGUID();
                        WallTarget->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        if (me->GetEntry() == NPC_JAINA_OUTRO)
                            me->CastSpell(me, SPELL_DESTROY_ICE_WALL_01, false);
                    }
                    WallCast = true;
                    break;
                case 19:
                    if (me->GetEntry() == NPC_JAINA_OUTRO)
                        DoScriptText(SAY_JAINA_TRAP, me);
                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                        DoScriptText(SAY_SYLVANA_TRAP, me);
                    break;
                case 20:
                    if (Creature * WallTarget = instance->instance->GetCreature(WallTargetGUID))
                    {
                        if (WallTarget->isAlive())
                        {
                            WallTarget->ForcedDespawn();
                            WallTargetGUID = 0;
                        }
                    }
                    SetEscortPaused(true);
                    if (me->GetEntry() == NPC_JAINA_OUTRO)
                        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY2HL);
                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                        me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_READY1H);
                    break;
            }
        }

        void MoveInLineOfSight(Unit * who)
        {
            if (!who || !instance)
                return;

            if (who->GetTypeId() != TYPEID_PLAYER)
                return;

            Player * plr = (Player *)who;

            if (plr->GetTeam() == ALLIANCE && me->GetEntry() == NPC_SYLVANA_OUTRO)
                return;

            if (plr->GetTeam() == HORDE && me->GetEntry() == NPC_JAINA_OUTRO)
                return;

            if (me->IsWithinDistInMap(who, 50.0f) && instance->GetData(DATA_FROSWORN_EVENT) == DONE && instance->GetData(DATA_PHASE) == 3)
            {
                plr = (Player *)who;
                Event = true;
                me->setFaction(FACTION);
                instance->SetData(DATA_PHASE, 4);
            }
        }

        void DamageTaken(Unit * /*DoneBy*/, uint32 & dmg)
        {
            if (!instance)
                return;

            if (instance->GetData(DATA_LICHKING_EVENT) != IN_PROGRESS)
            {
                dmg = 0;
                return;
            }

            if (instance->GetData(DATA_LICHKING_EVENT) == IN_PROGRESS && WallCast == true)
            {
                HoldTimer = HoldTimer + 100;
                return;
            }
        }

        void JumpNextStep(uint32 Time)
        {
            StepTimer = Time;
            Step++;
        }

        void Intro()
        {
            switch(Step)
            {
                case 0:
                    me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                    LichKingGUID = instance->GetData64(DATA_LICHKING);
                    LichKing = instance->instance->GetCreature(LichKingGUID);
                    me->SetUInt64Value(UNIT_FIELD_TARGET, LichKing->GetGUID());
                    JumpNextStep(100);
                    break;
                case 1:
                    HoRQuestComplete(me->GetEntry());
                    if (LichKing)
                    {
                        if (me->GetEntry() == NPC_JAINA_OUTRO)
                            DoScriptText(SAY_LICH_KING_AGGRO_A, LichKing);
                        if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                            DoScriptText(SAY_LICH_KING_AGGRO_H, LichKing);
                        LichKing->AI()->AttackStart(me);
                        me->AI()->AttackStart(LichKing);
                    }
                    JumpNextStep(3000);
                    break;
                case 2:
                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                    {
                        Fight = false;
                        if (LichKing)
                            me->GetMotionMaster()->MovePoint(0, (me->GetPositionX()-5)+rand()%10, (me->GetPositionY()-5)+rand()%10, me->GetPositionZ());
                        JumpNextStep(3000);
                    }
                    else
                        JumpNextStep(100);
                    break;
                case 3:
                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                        Fight = true;
                    JumpNextStep(100);
                    break;
                case 4:
                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                    {
                        if (LichKing)
                            me->CastSpell(LichKing, SPELL_SYLVANA_STEP, false);
                        JumpNextStep(3000);
                    }
                    else
                        JumpNextStep(100);
                    break;
                case 5:
                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                    {
                        Fight = false;
                        if (LichKing)
                            me->GetMotionMaster()->MovePoint(0, (me->GetPositionX()-5)+rand()%10, (me->GetPositionY()-5)+rand()%10, me->GetPositionZ());
                        JumpNextStep(3000);
                    }
                    else
                        JumpNextStep(12000);
                    break;
                case 6:
                    Fight = true;

                    if (LichKing)
                    {
                        if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                            DoCast(SPELL_SYLVANA_JUMP);
                        DoCast(LichKing, me->GetEntry() == NPC_JAINA_OUTRO ? SPELL_ICE_PRISON_VISUAL : SPELL_DARK_ARROW, false);
                        LichKing->AttackStop();
                    }
                    JumpNextStep(1480);
                    break;
                case 7:
                    me->RemoveAllAuras();

                    if (LichKing && !LichKing->HasAura(SPELL_ICE_PRISON_VISUAL))
                        LichKing->AddAura(me->GetEntry() == NPC_JAINA_OUTRO ? SPELL_ICE_PRISON_VISUAL : SPELL_DARK_ARROW, LichKing);

                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                    me->AttackStop();

                    if (me->GetEntry() == NPC_JAINA_OUTRO)
                    {
                        me->RemoveAurasDueToSpell(SPELL_ICE_BARRIER);
                        DoScriptText(SAY_JAINA_AGGRO, me);
                    }

                    if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                        DoScriptText(SAY_SYLVANA_AGGRO, me);

                    JumpNextStep(3000);
                    break;
                case 8:
                    me->GetMotionMaster()->MovePoint(0, 5577.187f, 2236.003f, 733.012f);
                    if (LichKing && !LichKing->HasAura(SPELL_ICE_PRISON_VISUAL))
                    {
                        LichKing->AddAura(me->GetEntry() == NPC_JAINA_OUTRO ? SPELL_ICE_PRISON_VISUAL : SPELL_DARK_ARROW, LichKing);
                        me->SetUInt64Value(UNIT_FIELD_TARGET, LichKing->GetGUID());
                    }
                    JumpNextStep(10000);
                    break;
                case 9:
                    if (LichKing && (!LichKing->HasAura(SPELL_ICE_PRISON_VISUAL) || !LichKing->HasAura(SPELL_DARK_ARROW)))
                    {
                        LichKing->AddAura(me->GetEntry() == NPC_JAINA_OUTRO ? SPELL_ICE_PRISON_VISUAL : SPELL_DARK_ARROW, LichKing);
                        me->SetUInt64Value(UNIT_FIELD_TARGET, LichKing->GetGUID());
                    }
                    me->RemoveAllAuras();
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    Count = 1;
                    JumpNextStep(100);
                    break;
            }
        }

        void Outro()
        {
            switch(Step)
            {
                case 10:
                    JumpNextStep(10000);
                    break;
                case 11:
                    if (GameObject * Gunship = instance->instance->GetGameObject(instance->GetData64(me->GetEntry() == NPC_JAINA_OUTRO ? GO_SKYBREAKER : GO_ORGRIM_HAMMER)))
                        Gunship->SetPhaseMask(1, true);

                    if (Creature * Captain = me->SummonCreature(me->GetEntry() == NPC_JAINA_OUTRO? NPC_BARTLETT : NPC_KORM, 5251.17f, 1610.2f, 795.812f, 2.07997f))
                    {
                        DoScriptText(me->GetEntry() == NPC_JAINA_OUTRO ? SAY_JAINA_FIRE : SAY_SYLVANA_FIRE, Captain);
                        CaptainGUID = Captain->GetGUID();
                    }
                    if (GameObject * Cave = instance->instance->GetGameObject(instance->GetData64(GO_CAVE)))
                    {
                        Creature * CaveTarget = me->SummonCreature(NPC_ICE_WALL,Cave->GetPositionX(),me->GetPositionY(),me->GetPositionZ(),me->GetOrientation(),TEMPSUMMON_MANUAL_DESPAWN,720000);
                        CaveTarget->SetFloatValue(OBJECT_FIELD_SCALE_X, 4);

                        for (int8 i=0; i<4; ++i)
                        {
                            if (Creature * Cannoner = me->SummonCreature(NPC_ICE_WALL, CannonSpawns[i],TEMPSUMMON_MANUAL_DESPAWN,720000))
                            {
                                Cannoner->setFaction(me->getFaction());
                                Cannoner->CastSpell(CaveTarget, SPELL_FIRE_CANNON, true);
                            }

                        }
                    }
                    JumpNextStep(6000);
                    break;
                case 12:
                    if (GameObject * Cave = instance->instance->GetGameObject(instance->GetData64(GO_CAVE)))
                        Cave->SetGoState(GO_STATE_READY);
                    me->RemoveAllAuras();

                    if (LichKing)
                        LichKing->DespawnOrUnsummon();

                    HoRQuestComplete(38211);
                    JumpNextStep(10000);
                    break;
                case 13:
                    me->SetUInt32Value(UNIT_NPC_EMOTESTATE, EMOTE_STATE_STAND);
                    instance->SetData(DATA_LICHKING_EVENT, DONE);
                    if (Creature * Captain = me->GetCreature(*me, CaptainGUID))
                        DoScriptText(me->GetEntry() == NPC_JAINA_OUTRO ? SAY_JAINA_FINAL_1 : SAY_SYLVANA_FINAL_1, Captain);
                    me->SummonGameObject(me->GetEntry() == NPC_JAINA_OUTRO ? GO_STAIRS_SKYBREAKER : GO_STAIRS_ORGRIM_HAMMER, 5247.45f, 1627.72f, 784.302f, 5.88208f, 0, 0, 0.199211f, -0.979957f, 1*DAY);
                    JumpNextStep(10000);
                    break;
                case 14:
                    DoScriptText(me->GetEntry() == NPC_JAINA_OUTRO ? SAY_JAINA_FINAL_2 : SAY_SYLVANA_FINAL_2, me);
                    JumpNextStep(20000);
                    break;
                case 15:
                    DoScriptText(me->GetEntry() == NPC_JAINA_OUTRO ? SAY_JAINA_FINAL_3 : 0, me);
                    JumpNextStep(5000);
                    break;
                case 16:
                    me->SetOrientation(0.68f);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                    me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    JumpNextStep(5000);
                    break;
            }
        }

        void HoRQuestComplete(uint32 killCredit)
        {
            if (instance)
            {
                Map::PlayerList const & PlayerList = instance->instance->GetPlayers();
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                    if (i->getSource())
                        i->getSource()->KilledMonsterCredit(killCredit, 0);
            }
        }

        void UpdateEscortAI(const uint32 diff)
        {
            if (!instance || !Event)
                return;

            DoMeleeAttackIfReady();

            if (instance->GetData(DATA_PHASE) == 4 && instance->GetData(DATA_LICHKING_EVENT) != IN_PROGRESS)
            {
                if (StepTimer < diff)
                    Intro();
                else
                    StepTimer -= diff;
            }

            if (instance->GetData(DATA_LICHKING_EVENT) == SPECIAL && instance->GetData(DATA_PHASE) != 6) // End Cinematic
            {
                instance->SetData(DATA_PHASE, 6);
                Step = 10;
            }

            if (instance->GetData(DATA_PHASE) == 6)
            {
                if (StepTimer < diff)
                    Outro();
                else
                    StepTimer -= diff;
                return;
            }

            if (WallCast == true && CastTimer < diff)
            {
                if (me->GetEntry() == NPC_SYLVANA_OUTRO)
                {
                   if (Creature * WallTarget = instance->instance->GetCreature(WallTargetGUID))
                        me->CastSpell(WallTarget, SPELL_DESTROY_ICE_WALL_03, false);
                    CastTimer = 1000;
                }
            }
            else
                CastTimer -= diff;

            if (WallCast == true && HoldTimer < 10000 && ( instance->GetData(DATA_SUMMONS) == 0 || !me->isInCombat()))
            {
                WallCast = false;
                me->InterruptNonMeleeSpells(false);
                SetEscortPaused(false);
                if (GameObject * gate = instance->instance->GetGameObject(IceWallGUID))
                    gate->SetGoState(GO_STATE_ACTIVE);
                ++Count;
                switch(Count)
                {
                    case 2:
                        if (GameObject * gate = instance->instance->GetGameObject(instance->GetData64(GO_ICE_WALL_2)))
                        {
                            gate->SetGoState(GO_STATE_READY);
                            if (LichKing && LichKing->isAlive())
                                DoScriptText(SAY_LICH_KING_WALL_02, LichKing);
                            IceWallGUID = gate->GetGUID();
                        }
                        break;
                    case 3:
                        if (GameObject * gate = instance->instance->GetGameObject(instance->GetData64(GO_ICE_WALL_3)))
                        {
                            gate->SetGoState(GO_STATE_READY);
                            if (LichKing && LichKing->isAlive())
                                DoScriptText(SAY_LICH_KING_WALL_03, LichKing);
                            IceWallGUID = gate->GetGUID();
                        }
                        break;
                    case 4:
                        if (GameObject * gate = instance->instance->GetGameObject(instance->GetData64(GO_ICE_WALL_4)))
                        {
                            gate->SetGoState(GO_STATE_READY);
                            if (LichKing && LichKing->isAlive())
                                DoScriptText(SAY_LICH_KING_WALL_04, LichKing);
                            IceWallGUID = gate->GetGUID();
                        }
                        break;
                    case 5:
                        if (LichKing && LichKing->isAlive())
                        {
                            LichKing->RemoveAurasDueToSpell(SPELL_WINTER);
                            LichKing->SetSpeed(MOVE_WALK, 2.5f, true);
                            Step = 0;
                        }
                        break;
                }
            }
            else
            {
                HoldTimer -= diff;
                if (HoldTimer <= 0)
                    HoldTimer = 0;
            }

            return;
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_jaina_and_sylvana_hor_part2AI(creature);
    }
};

class at_hor_waves_restarter : public AreaTriggerScript
{
public:
    at_hor_waves_restarter() : AreaTriggerScript("at_hor_waves_restarter") {}

    bool OnTrigger(Player * plr, AreaTriggerEntry const * /*trigger*/)
    {
        InstanceScript * instance = plr->GetInstanceScript();

        if (!instance)
            return true;

        if (plr->isGameMaster() || !plr->isGMVisible())
            return true;

        if (instance->GetData(DATA_INTRO_EVENT) != DONE)
            return true;

        if (instance->GetData(DATA_MARWYN_EVENT) == IN_PROGRESS || instance->GetData(DATA_MARWYN_EVENT) == DONE)
            return true;

        if (instance->GetData(DATA_FALRIC_EVENT) != DONE)
            if (Creature * Falric = plr->GetCreature(*plr, instance->GetData64(DATA_FALRIC)))
            {
                Falric->SetVisible(true);
                Falric->CastSpell(Falric, SPELL_BOSS_SPAWN_AURA, true);
            }

        if (instance->GetData(DATA_MARWYN_EVENT) != DONE)
        {
            if (Creature * Marwyn = plr->GetCreature(*plr, instance->GetData64(DATA_MARWYN)))
            {
                Marwyn->SetVisible(true);
                Marwyn->CastSpell(Marwyn, SPELL_BOSS_SPAWN_AURA, true);
            }
            instance->SetData(DATA_WAVE_COUNT, START_WAVES);
        }
        return true;
    }
};

class npc_queldelar_trigger : public CreatureScript
{
public:
    npc_queldelar_trigger() : CreatureScript("npc_queldelar_trigger") { }

    struct npc_queldelar_triggerAI  : public ScriptedAI
    {
        npc_queldelar_triggerAI(Creature * creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();

            me->setActive(true);

            if (instance)
                PlrGUID = instance->GetData64(DATA_QUELDELAR_PLR);
            else
                PlrGUID = 0;

            checktimer = SEKUNDEN_05;
        }

        void Reset()
        {
        }

        void CheckQuelDelarPlr()
        {
            if (!instance)
                return;

            if (Player * plr = ObjectAccessor::GetPlayer(*me, PlrGUID))
                if (!instance->GetData64(DATA_QUELDELAR))
                    if (me->IsWithinDistInMap(plr, 14.99f) && plr->HasItemCount(ITEM_GETEMPERTES_QUELDELAR, 1))
                    {
                        plr->SetMover(me);
                        plr->CastSpell(me, SPELL_QUELDELAR_WERFEN_1, false);
                        plr->CastSpell(me, SPELL_QUELDELAR_WERFEN_2, true);
                    }
        }

        void SpellHit(Unit * /*caster*/, const SpellInfo * SI)
        {
            if (SI && SI->Id == SPELL_QUELDELAR_WERFEN_1)
                if (instance && !ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_QUELDELAR)))
                {
                    DoCast(SPELL_BOESES_QUELDELAR_BESCHWOEREN);

                    if (Player * plr = ObjectAccessor::GetPlayer(*me, PlrGUID))
                    {
                        plr->SetMover(plr);
                        plr->DestroyItemCount(ITEM_GETEMPERTES_QUELDELAR, 1, true);
                    }
                    checktimer = 0;
                    me->ForcedDespawn();
                }
        }

        void UpdateAI(const uint32 diff)
        {
            if (PlrGUID && checktimer && checktimer <= diff)
            {
                CheckQuelDelarPlr();
                checktimer = SEKUNDEN_05;
            }
            else
                checktimer -= diff;
        }
private:
    InstanceScript * instance;
    uint64 PlrGUID;
    uint32 checktimer;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_queldelar_triggerAI(creature);
    }
};

class npc_queldelar : public CreatureScript
{
public:
    npc_queldelar() : CreatureScript("npc_queldelar") { }

    struct npc_queldelarAI  : public ScriptedAI
    {
        npc_queldelarAI(Creature * creature) : ScriptedAI(creature)
        {
            me->SetDisplayId(30547);
            me->setActive(false);
            waittimer = SEKUNDEN_20;
            instance = me->GetInstanceScript();
            // TODO: Spell finden, der den NPC als Schwert sichtbar macht!
        }

        void Reset()
        {
            events.Reset();
            me->RemoveAurasDueToSpell(SPELL_WHIRLWIND_VISUAL);
        }

        void EnterCombat(Unit * /*who*/)
        {
            events.ScheduleEvent(EVENT_HELDENHAFTER_STOSS, SEKUNDEN_05);
            events.ScheduleEvent(EVENT_KLINGENSTURM, SEKUNDEN_10);
            events.ScheduleEvent(EVENT_TOEDLICHER_STOSS, 8 * IN_MILLISECONDS);
        }

        void JustDied(Unit * /*killer*/)
        {
            if (instance)
                if (Player * plr = ObjectAccessor::GetPlayer(*me, instance->GetData64(DATA_QUELDELAR_PLR)))
                    plr->AddItem(ITEM_GEDAEMPFTES_QUELDELAR, 1);
        }

        void UpdateAI(const uint32 diff)
        {
            if (waittimer && waittimer <= diff)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_SERVER_CONTROLLED);
                me->setActive(true);
                waittimer = 0;
            }
            else
                waittimer -= diff;

            if (!UpdateVictim())
                return;

            if (me->HasAura(SPELL_WHIRLWIND_VISUAL))
                if (!me->HasAura(SPELL_KLINGENSTURM))
                    me->RemoveAurasDueToSpell(SPELL_WHIRLWIND_VISUAL);

            events.Update(diff);

            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_HELDENHAFTER_STOSS:
                        DoCastVictim(SPELL_HELDENHAFTER_STOSS);
                        events.RescheduleEvent(EVENT_HELDENHAFTER_STOSS, SEKUNDEN_10);
                        break;
                    case EVENT_KLINGENSTURM:
                        DoCastAOE(SPELL_KLINGENSTURM, true);
                        DoCastAOE(SPELL_WHIRLWIND_VISUAL, true);
                        events.RescheduleEvent(EVENT_KLINGENSTURM, urand(SEKUNDEN_10, SEKUNDEN_20));
                        break;
                    case EVENT_TOEDLICHER_STOSS:
                        DoCastVictim(SPELL_TOEDLICHER_STOSS);
                        events.RescheduleEvent(EVENT_TOEDLICHER_STOSS, urand(SEKUNDEN_10, SEKUNDEN_20));
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
private:
    EventMap events;
    uint32 waittimer;
    InstanceScript * instance;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_queldelarAI(creature);
    }
};

void AddSC_halls_of_reflection()
{
    new npc_jaina_or_sylvanas_hor(true, "npc_sylvanas_hor_part1");
    new npc_jaina_or_sylvanas_hor(false, "npc_jaina_hor_part1");
    new npc_jaina_and_sylvana_hor_part2();
    new npc_ghostly_priest();
    new npc_phantom_mage();
    new npc_phantom_hallucination();
    new npc_shadowy_mercenary();
    new npc_spectral_footman();
    new npc_tortured_rifleman();
    new at_hor_waves_restarter();
    new npc_frostworn_general();
    new npc_spiritual_reflection();
    new npc_queldelar_trigger();
    new npc_queldelar();
}
