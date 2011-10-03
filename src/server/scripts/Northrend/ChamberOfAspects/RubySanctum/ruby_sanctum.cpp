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
#include "ruby_sanctum.h"

enum Texts
{
    SAY_XERESTRASZA_EVENT,
    SAY_XERESTRASZA_EVENT_1,
    SAY_XERESTRASZA_EVENT_2,
    SAY_XERESTRASZA_EVENT_3,
    SAY_XERESTRASZA_EVENT_4,
    SAY_XERESTRASZA_EVENT_5,
    SAY_XERESTRASZA_EVENT_6,
    SAY_XERESTRASZA_EVENT_7,
    SAY_XERESTRASZA_INTRO
};

enum Events
{
    EVENT_XERESTRASZA_EVENT_1 = 1,
    EVENT_XERESTRASZA_EVENT_2,
    EVENT_XERESTRASZA_EVENT_3,
    EVENT_XERESTRASZA_EVENT_4,
    EVENT_XERESTRASZA_EVENT_5,
    EVENT_XERESTRASZA_EVENT_6,
    EVENT_XERESTRASZA_EVENT_7
};

const Position xerestraszaMovePos = { 3151.236f, 379.8733f, 86.31996f, 0.0f };

class npc_xerestrasza : public CreatureScript
{
public:
    npc_xerestrasza() : CreatureScript("npc_xerestrasza") { }

    struct npc_xerestraszaAI : public ScriptedAI
    {
        npc_xerestraszaAI(Creature* creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            events.Reset();
            if (instance && instance->GetData(DATA_XERESTRASZA_ALLOWED))
            {
                introDone = true;
                DoAction(ACTION_BALTHARUS_DEATH);
            }
            else
            {
                isIntro = true;
                introDone = false;
                me->RemoveFlag(UNIT_NPC_FLAGS, GOSSIP_OPTION_QUESTGIVER);
            }
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_BALTHARUS_DEATH)
            {
                me->setActive(true);
                isIntro = false;

                Talk(SAY_XERESTRASZA_EVENT);
                me->AddUnitMovementFlag(MOVEMENTFLAG_WALKING);
                me->GetMotionMaster()->MovePoint(0, xerestraszaMovePos);

                events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_1, 16000);
                events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_2, 25000);
                events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_3, 32000);
                events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_4, 42000);
                events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_5, 51000);
                events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_6, 61000);
                events.ScheduleEvent(EVENT_XERESTRASZA_EVENT_7, 69000);

                if (instance)
                    instance->SetData(DATA_XERESTRASZA_ALLOWED, 1);
            }
            else if (action == ACTION_INTRO_BALTHARUS && !introDone)
            {
                introDone = true;
                Talk(SAY_XERESTRASZA_INTRO);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (isIntro)
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_XERESTRASZA_EVENT_1: Talk(SAY_XERESTRASZA_EVENT_1); break;
                    case EVENT_XERESTRASZA_EVENT_2: Talk(SAY_XERESTRASZA_EVENT_2); break;
                    case EVENT_XERESTRASZA_EVENT_3: Talk(SAY_XERESTRASZA_EVENT_3); break;
                    case EVENT_XERESTRASZA_EVENT_4: Talk(SAY_XERESTRASZA_EVENT_4); break;
                    case EVENT_XERESTRASZA_EVENT_5: Talk(SAY_XERESTRASZA_EVENT_5); break;
                    case EVENT_XERESTRASZA_EVENT_6: Talk(SAY_XERESTRASZA_EVENT_6); break;
                    case EVENT_XERESTRASZA_EVENT_7:
                        me->SetFlag(UNIT_NPC_FLAGS, GOSSIP_OPTION_QUESTGIVER);
                        Talk(SAY_XERESTRASZA_EVENT_7);
                        me->setActive(false);
                        break;
                    default:
                        break;
                }
            }
        }
private:
        EventMap events;
        bool isIntro;
        bool introDone;
        InstanceScript * instance;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return GetRubySanctumAI<npc_xerestraszaAI>(creature);
    }
};

class at_baltharus_plateau : public AreaTriggerScript
{
    public:
        at_baltharus_plateau() : AreaTriggerScript("at_baltharus_plateau") { }

        bool OnTrigger(Player * player, AreaTriggerEntry const * /*areaTrigger*/)
        {
            // Only trigger once
            if (InstanceScript * instance = player->GetInstanceScript())
            {
                if (Creature * xerestrasza = ObjectAccessor::GetCreature(*player, instance->GetData64(DATA_XERESTRASZA)))
                    xerestrasza->AI()->DoAction(ACTION_INTRO_BALTHARUS);

                if (Creature * baltharus = ObjectAccessor::GetCreature(*player, instance->GetData64(DATA_BALTHARUS_THE_WARBORN)))
                    baltharus->AI()->DoAction(ACTION_INTRO_BALTHARUS);
            }
            return true;
        }
};

enum RubinsanktumTrashNPCs
{
    NPC_Herbeirufer_der_Schmorschuppen  = 40417,
    NPC_Angreifer_der_Schmorschuppen    = 40419,
    NPC_Elite_der_Schmorschuppen        = 40421,
    NPC_Kommandant_der_Schmorschuppen   = 40423
};

enum RubinsanktumTrashSpells
{
#define SPELL_Herbeirufer_der_Schmorschuppen_Versengen          RAID_MODE<uint32>(75412,75419,75412,75419) // Rnd - 30 Meter Reichweite
        SPELL_Herbeirufer_der_Schmorschuppen_Flammenwelle       = 75413, // AOE - selbst - 10 Meter Reichweite
        SPELL_Herbeirufer_der_Schmorschuppen_Zusammenruf        = 75416, // Selbst - Ruft verbündete herbei - jeder macht pro Einheit in 8 Metern 25% mehr Schaden - hält 10 Sek.
#define SPELL_Angreifer_der_Schmorschuppen_Schockwelle          RAID_MODE<uint32>(75417,75418,75417,75418) // Victim - 15 Meter Reichweite
        SPELL_Angreifer_der_Schmorschuppen_Spalten              = 15284, // Victim
        SPELL_Angreifer_der_Schmorschuppen_Zusammenruf          = 75416, // Selbst - Ruft verbündete herbei - jeder macht pro Einheit in 8 Metern 25% mehr Schaden - hält 10 Sek.
        SPELL_Elite_der_Schmorschuppen_Schaedelkracher          = 15621, // Victim
        SPELL_Kommandant_der_Schmorschuppen_Sammelruf           = 75414, // Selbst - Ruft verbündete herbei - jeder macht pro Einheit in 8 Metern 25% mehr Schaden - hält 50 Sek. - Dummy -> Script!
        SPELL_Kommandant_der_Schmorschuppen_Sammelruf_Effect    = 75415,
        SPELL_Kommandant_der_Schmorschuppen_Toedlicher_Stoss    = 13737  // Victim
};

enum RubinsanktumTrashEvents
{
    EVENT_Herbeirufer_der_Schmorschuppen_Versengen = 1,
    EVENT_Herbeirufer_der_Schmorschuppen_Flammenwelle,
    EVENT_Herbeirufer_der_Schmorschuppen_Zusammenruf,
    EVENT_Angreifer_der_Schmorschuppen_Schockwelle,
    EVENT_Angreifer_der_Schmorschuppen_Spalten,
    EVENT_Angreifer_der_Schmorschuppen_Zusammenruf,
    EVENT_Elite_der_Schmorschuppen_Schaedelkracher,
    EVENT_Kommandant_der_Schmorschuppen_Sammelruf,
    EVENT_Kommandant_der_Schmorschuppen_Toedlicher_Stoss
};

class mob_rubinsanktum_trash : public CreatureScript
{
public:
    mob_rubinsanktum_trash() : CreatureScript("mob_rubinsanktum_trash") { }

    struct mob_rubinsanktum_trashAI: public ScriptedAI
    {
        mob_rubinsanktum_trashAI(Creature * creature) : ScriptedAI(creature)
        {
            instance = me->GetInstanceScript();
        }

        void Reset()
        {
            events.Reset();
        }

        void EnterCombat(Unit * who)
        {
            if (!who || !who->isValid())
                return;

            events.ScheduleEvent(EVENT_Herbeirufer_der_Schmorschuppen_Versengen, urand(SEKUNDEN_05, SEKUNDEN_10));
            events.ScheduleEvent(EVENT_Herbeirufer_der_Schmorschuppen_Flammenwelle, urand(SEKUNDEN_10, SEKUNDEN_20));
            events.ScheduleEvent(EVENT_Herbeirufer_der_Schmorschuppen_Zusammenruf, SEKUNDEN_30);
            events.ScheduleEvent(EVENT_Angreifer_der_Schmorschuppen_Schockwelle, urand(SEKUNDEN_05, SEKUNDEN_10));
            events.ScheduleEvent(EVENT_Angreifer_der_Schmorschuppen_Spalten, urand(SEKUNDEN_10, SEKUNDEN_20));
            events.ScheduleEvent(EVENT_Angreifer_der_Schmorschuppen_Zusammenruf, SEKUNDEN_30);
            events.ScheduleEvent(EVENT_Elite_der_Schmorschuppen_Schaedelkracher, urand(SEKUNDEN_05, SEKUNDEN_10));
            events.ScheduleEvent(EVENT_Kommandant_der_Schmorschuppen_Sammelruf, SEKUNDEN_20);
            events.ScheduleEvent(EVENT_Kommandant_der_Schmorschuppen_Toedlicher_Stoss, urand(SEKUNDEN_05, SEKUNDEN_10));

            me->InterruptNonMeleeSpells(true);
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
                switch(me->GetEntry())
                {
                    case NPC_Herbeirufer_der_Schmorschuppen:
                        switch(eventId)
                        {
                            case EVENT_Herbeirufer_der_Schmorschuppen_Versengen:
                                if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                                    DoCast(target, SPELL_Herbeirufer_der_Schmorschuppen_Versengen);
                                events.RescheduleEvent(EVENT_Herbeirufer_der_Schmorschuppen_Versengen, urand(SEKUNDEN_05, SEKUNDEN_10));
                                break;
                            case EVENT_Herbeirufer_der_Schmorschuppen_Flammenwelle:
                                DoCastAOE(SPELL_Herbeirufer_der_Schmorschuppen_Flammenwelle);
                                events.RescheduleEvent(EVENT_Herbeirufer_der_Schmorschuppen_Flammenwelle, urand(SEKUNDEN_15, SEKUNDEN_30));
                                break;
                            case EVENT_Herbeirufer_der_Schmorschuppen_Zusammenruf:
                                DoCast(SPELL_Herbeirufer_der_Schmorschuppen_Zusammenruf);
                                events.RescheduleEvent(EVENT_Herbeirufer_der_Schmorschuppen_Zusammenruf, SEKUNDEN_30);
                                break;
                        }
                        break;
                    case NPC_Angreifer_der_Schmorschuppen:
                        switch(eventId)
                        {
                            case EVENT_Angreifer_der_Schmorschuppen_Schockwelle:
                                DoCastAOE(SPELL_Angreifer_der_Schmorschuppen_Schockwelle);
                                events.RescheduleEvent(EVENT_Angreifer_der_Schmorschuppen_Schockwelle, urand(SEKUNDEN_10, SEKUNDEN_20));
                                break;
                            case EVENT_Angreifer_der_Schmorschuppen_Spalten:
                                DoCastVictim(SPELL_Angreifer_der_Schmorschuppen_Spalten);
                                events.RescheduleEvent(EVENT_Angreifer_der_Schmorschuppen_Spalten, urand(SEKUNDEN_05, SEKUNDEN_10));
                                break;
                            case EVENT_Angreifer_der_Schmorschuppen_Zusammenruf:
                                DoCast(SPELL_Angreifer_der_Schmorschuppen_Zusammenruf);
                                events.RescheduleEvent(EVENT_Angreifer_der_Schmorschuppen_Zusammenruf, SEKUNDEN_30);
                                break;
                        }
                        break;
                    case NPC_Elite_der_Schmorschuppen:
                        switch(eventId)
                        {
                            case EVENT_Elite_der_Schmorschuppen_Schaedelkracher:
                                DoCastVictim(SPELL_Elite_der_Schmorschuppen_Schaedelkracher);
                                events.RescheduleEvent(EVENT_Elite_der_Schmorschuppen_Schaedelkracher, urand(SEKUNDEN_10, SEKUNDEN_20));
                                break;
                        }
                        break;
                    case NPC_Kommandant_der_Schmorschuppen:
                        switch(eventId)
                        {
                            case EVENT_Kommandant_der_Schmorschuppen_Sammelruf:
                                if (instance)
                                    instance->DoSendNotifyToInstance("%s ruft seine Truppen herbei!", me->GetCreatureInfo()->Name.c_str());
                                me->AddAura(SPELL_Kommandant_der_Schmorschuppen_Sammelruf, me);
                                Sammelruf(60.0f, SPELL_Kommandant_der_Schmorschuppen_Sammelruf);
                                events.RescheduleEvent(EVENT_Kommandant_der_Schmorschuppen_Sammelruf, urand(SEKUNDEN_60, SEKUNDEN_60+SEKUNDEN_30));
                                break;
                            case EVENT_Kommandant_der_Schmorschuppen_Toedlicher_Stoss:
                                DoCastVictim(SPELL_Kommandant_der_Schmorschuppen_Toedlicher_Stoss);
                                events.RescheduleEvent(EVENT_Kommandant_der_Schmorschuppen_Toedlicher_Stoss, urand(SEKUNDEN_10, SEKUNDEN_20));
                                break;
                        }
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
private:
        EventMap events;
        InstanceScript * instance;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new mob_rubinsanktum_trashAI(creature);
    }
};

void AddSC_ruby_sanctum()
{
    new npc_xerestrasza();
    new at_baltharus_plateau();
    new mob_rubinsanktum_trash();
}
