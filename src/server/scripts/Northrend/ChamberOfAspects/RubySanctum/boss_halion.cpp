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
#include "MapManager.h"
#include "ruby_sanctum.h"

enum Texts
{
    SAY_INTRO,      // Lästige Insekten! Ihr kommt zu spät! Das Rubinsanktum ist verloren! 17499
    SAY_AGGRO,      // Eure Weilt steht auf Messersschneide. Ihr ALLE werdet Zeuge des Anbegins eines neuen Zeitalters der ZERSTÖRUNG! 17500
    SAY_SLAY01,     // Ein weiterer "Held" geht dahin!' 17501
    SAY_SLAY02,     // Ha, ha, ha, ha, ha! 17502
    SAY_DEATH,      // Genießt euren Sieg sterbliche, denn es war euer Letzter! Bei der Rückkehr des Meisters, wird diese Welt brennen! 17503
    SAY_BERSERK,    // Nicht gut genug! 17504
    SAY_SPECIAL01,  // Die Himmel brennen! 17505 Meteor Strike
    SAY_SPECIAL02,  // Hütet euch vor dem Schatten! 17506 Twilight Pulse
    SAY_PHASE2,     // Ihr werdet im Reich des Zwielichts nur Leid finden! Tretet ein, wenn ihr es wagt! 17507
    SAY_PHASE3,     // Ich bin das Licht und die Dunkelheit! Zittert Sterbliche, vor dem Herold Todesschwinges! 17508
    SAY_ATTAK,      // 17509 Kampfgeräusche
    SAY_WOUND,      // 17510 Kampfgeräusche
    SAY_WOUNDCRIT   // 17511 Kampfgeräusche
};

enum Spells
{
        // Halion
#define SPELL_FLAME_BREATH              RAID_MODE<uint32>(74525,74526,74527,74528)
        SPELL_SUMMON_TWILIGHT_PORTAL    = 74809, // GO_HALION_PORTAL_1
        SPELL_TWILIGHT_DIVISION         = 75063, // Phases the caster, allowing him to exist and act simultaneously in both the material and Twilight realms.

        SPELL_METEOR_STRIKE             = 74637,
        SPELL_METEOR_STRIKE_COUNTDOWN   = 74641,
#define SPELL_METEOR_STRIKE_AOE_DAMAGE  RAID_MODE<uint32>(74648,75877,75878,75879)
        SPELL_METEOR_STRIKE_FIRE_AURA_1 = 74713,
        SPELL_METEOR_STRIKE_FIRE_AURA_2 = 74718,

        SPELL_COMBUSTION                = 74562, // Will each tick, apart from the damage, also add a stack to 74567
        SPELL_COMBUSTION_STACK          = 74567, // If 74562 or 74567 is removed; this will trigger an explosion (74607) based on stackamount.
        SPELL_COMBUSTION_SCALE_AURA     = 70507, // Aura created in spell_dbc since missing in client dbc. Value based on 74567 stackamount.
        SPELL_COMBUSTION_DAMAGE_AURA    = 74629,
        SPELL_COMBUSTION_EXPLOSION      = 74607,
        SPELL_COMBUSTION_SUMMON         = 74610,
        // Halion Twilight
#define SPELL_DARK_BREATH               RAID_MODE<uint32>(74806,75954,75956,75956)
        SPELL_DUSK_SHROUD               = 75476, // Inflicts 3,000 Shadow damage every 2 seconds to everyone in the Twilight Realm

        SPELL_CONSUMPTION_EXPLOSION     = 74799,
        SPELL_CONSUMPTION_SUMMON        = 74800,
        SPELL_CONSUMPTION               = 74792,
        SPELL_CONSUMPTION_STACK         = 74795,
        SPELL_CONSUMPTION_DAMAGE_AURA   = 74803,

        SPELL_TWILIGHT_CUTTER           = 74768, // Triggert SPELL_TWILIGHT_CUTTER_02
        SPELL_TWILIGHT_CUTTER_02        = 74769, // Fires a beam of concentrated twilight energy, dealing massive Shadow damage to any enemies that make contact with it. 13875 bis 16125
        SPELL_TWILIGHT_CUTTER_03        = 77844, // Fires a beam of concentrated twilight energy, dealing massive Shadow damage to any enemies that make contact with it. 13875 bis 16125
        SPELL_TWILIGHT_CUTTER_04        = 77845, // Fires a beam of concentrated twilight energy, dealing massive Shadow damage to any enemies that make contact with it. 41625 bis 48375
        SPELL_TWILIGHT_CUTTER_05        = 77846, // Fires a beam of concentrated twilight energy, dealing massive Shadow damage to any enemies that make contact with it. 41625 bis 48375
        SPELL_TWILIGHT_PULSE_PERIODIC   = 78861, // Triggert 78862 (Twilight Pulse) - Deals 27000 to 33000 Shadow damage to enemies within 5 yards.
        // Beide Halions
        SPELL_CLEAVE                    = 74524,
        SPELL_TAIL_LASH                 = 74531, // A sweeping tail strike hits all enemies behind the caster, inflicting 3063 to 3937 damage and stunning them for 2 sec.
        SPELL_BERSERK                   = 26662, // Increases the caster's attack and movement speeds by 150% and all damage it deals by 500% for 5 min.  Also grants immunity to Taunt effects.
        SPELL_TWILIGHT_PRECISION        = 78243, // Increases Halion's chance to hit by 5% and decreases all players' chance to dodge by 20%

        SPELL_CORPOREALITY_EVEN         = 74826, // Deals & receives normal damage
        SPELL_CORPOREALITY_20I          = 74827, // Damage dealt increased by 10% & Damage taken increased by 15%
        SPELL_CORPOREALITY_40I          = 74828, // Damage dealt increased by 30% & Damage taken increased by 50%
        SPELL_CORPOREALITY_60I          = 74829, // Damage dealt increased by 60% & Damage taken increased by 100%
        SPELL_CORPOREALITY_80I          = 74830, // Damage dealt increased by 100% & Damage taken increased by 200%
        SPELL_CORPOREALITY_100I         = 74831, // Damage dealt increased by 200% & Damage taken increased by 400%
        SPELL_CORPOREALITY_20D          = 74832, // Damage dealt reduced by 10% & Damage taken reduced by 15%
        SPELL_CORPOREALITY_40D          = 74833, // Damage dealt reduced by 30% & Damage taken reduced by 50%
        SPELL_CORPOREALITY_60D          = 74834, // Damage dealt reduced by 60% & Damage taken reduced by 100%
        SPELL_CORPOREALITY_80D          = 74835, // Damage dealt reduced by 100% & Damage taken reduced by 200%
        SPELL_CORPOREALITY_100D         = 74836, // Damage dealt reduced by 200% & Damage taken reduced by 400%
        // Zwielichtportale
        SPELL_TWILIGHT_ENTER            = 74807, // Phases the caster into the Twilight realm - phase 32
        SPELL_TWILIGHT_LEAVE            = 74812,
        // Living Inferno
        SPELL_BLAZING_AURA              = 75885,
        // Halion Controller
        SPELL_COSMETIC_FIRE_PILLAR      = 76006,
        SPELL_FIERY_EXPLOSION           = 76010,
        SPELL_BIRTH_NO_VISUAL           = 40031
};

enum Events
{
    // Halion
    EVENT_ACTIVATE_FIREWALL = 1,
    EVENT_FLAME_BREATH,
    EVENT_METEOR_STRIKE,
    EVENT_COMBUSTION,
    // Halion Zwielicht
    EVENT_DARK_BREATH,
    EVENT_CONSUMTION,
    EVENT_CUTTER,
    // Beide Halions
    EVENT_CLEAVE,
    EVENT_TAIL_LASH,
    EVENT_ENRAGE,
    // Halion Controller
    EVENT_START_INTRO,
    EVENT_INTRO_PROGRESS_1,
    EVENT_INTRO_PROGRESS_2,
    EVENT_INTRO_PROGRESS_3,
    EVENT_CORPOREALITY,
    EVENT_CHECK_ENCOUNTER,
    EVENT_ZWIELICHT_COMBAT_CHECK,
    EVENT_REAL_COMBAT_CHECK,
    // Meteor Strike
    EVENT_SPAWN_METEOR_FLAME
};

enum Actions
{
    // Halion
    ACTION_HALION_VERSCHWINDEN,
    ACTION_HALION_ERSCHEINEN,
    // Halion Zwielicht
    ACTION_PREPARE,
    // Meteor Strike
    ACTION_METEOR_STRIKE_BURN,
    ACTION_METEOR_STRIKE_AOE
};

enum Phases
{
    PHASE_ALL,
    PHASE_ONE,
    PHASE_TWO,
    PHASE_THREE
};

enum Misc
{
    TYPE_COMBUSTION_SUMMON  = 1,
    FR_RADIUS               = 45
};

struct HalionBuffLine
{
    float diff;         // Boss Health diff in percent
    uint32 real;        // Buff pair (real world)
    uint32 twilight;    // Buff pair (twilight world)
};

static HalionBuffLine Buff[] =
{
    { -10.0f,   SPELL_CORPOREALITY_100I,    SPELL_CORPOREALITY_100D },
    { -8.0f,    SPELL_CORPOREALITY_80I,     SPELL_CORPOREALITY_80D  },
    { -6.0f,    SPELL_CORPOREALITY_60I,     SPELL_CORPOREALITY_60D  },
    { -4.0f,    SPELL_CORPOREALITY_40I,     SPELL_CORPOREALITY_40D  },
    { -2.0f,    SPELL_CORPOREALITY_20I,     SPELL_CORPOREALITY_20D  },
    { -1.0f,    SPELL_CORPOREALITY_EVEN,    SPELL_CORPOREALITY_EVEN },
    { 1.0f,     SPELL_CORPOREALITY_EVEN,    SPELL_CORPOREALITY_EVEN },
    { 2.0f,     SPELL_CORPOREALITY_20D,     SPELL_CORPOREALITY_20I  },
    { 4.0f,     SPELL_CORPOREALITY_40D,     SPELL_CORPOREALITY_40I  },
    { 6.0f,     SPELL_CORPOREALITY_60D,     SPELL_CORPOREALITY_60I  },
    { 8.0f,     SPELL_CORPOREALITY_80D,     SPELL_CORPOREALITY_80I  },
    { 10.0f,    SPELL_CORPOREALITY_100D,    SPELL_CORPOREALITY_100I }
};

class boss_halion : public CreatureScript
{
public:
    boss_halion() : CreatureScript("boss_halion") { }

    struct boss_halionAI : public BossAI
    {
        boss_halionAI(Creature * creature) : BossAI(creature, DATA_HALION) { }

        void Reset()
        {
            if (!instance || (instance->GetData(DATA_PHASE) == PHASE_THREE && instance->GetBossState(DATA_HALION_TWILIGHT) == IN_PROGRESS))
                return;

            _Reset();

            instance->SendEncounterUnit(ENCOUNTER_FRAME_REMOVE, me);

            sLog->outError("halion: Reset()");
        }

        void KilledUnit(Unit * victim)
        {
            if (!victim || !victim->ToPlayer())
                return;

            Talk(urand(SAY_SLAY01, SAY_SLAY02));
        }

        void DoAction(const int32 action)
        {
            switch(action)
            {
                case ACTION_HALION_VERSCHWINDEN:    Verschwinden(); break;
                case ACTION_HALION_ERSCHEINEN:      Erscheinen();   break;
                default:                                            break;
            }
        }

        void Verschwinden()
        {
            if (!instance)
                return;

            sLog->outError("halion: Verschwinden()");

            Talk(SAY_PHASE2);

            DoCast(me, SPELL_SUMMON_TWILIGHT_PORTAL, true);

            instance->SetData(DATA_PHASE, PHASE_TWO);
            instance->SetData(DATA_HALION_HEALTH, me->GetHealth());

            if (Creature * Controller = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION_CONTROLLER)))
            {
                me->getThreatManager().resetAllAggro();
                me->getThreatManager().doAddThreat(Controller, 0.0f);
            }
            me->SetVisible(false);
            me->setActive(false);
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

            if (GameObject * ZwielichtRing = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_ZWIELICHT_RING)))
                instance->HandleGameObject(instance->GetData64(DATA_ZWIELICHT_RING), false, ZwielichtRing);
        }

        void Erscheinen()
        {
            if (!instance)
                return;

            sLog->outError("halion: Erscheinen()");

            Talk(SAY_PHASE3);

            me->SetVisible(true);
            me->setActive(true);
            me->SetReactState(REACT_AGGRESSIVE);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);

            if (Creature * Controller = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION_CONTROLLER)))
            {
                me->getThreatManager().resetAllAggro();
                me->getThreatManager().doAddThreat(Controller, 0.0f);
            }
            me->SetHealth(instance->GetData(DATA_HALION_HEALTH));
            // Bring Halion in eine Phase (31?) wo der Spieler ihn nicht sieht.
            // Zu dieser Phase muss anscheinend das 3. Portal führen.
            //me->CastSpell(me, SPELL_TWILIGHT_DIVISION, true);
        }

        void EnterCombat(Unit * /*who*/)
        {
            if (!instance)
                return;

            sLog->outError("halion: EnterCombat()");

            Talk(SAY_AGGRO);

            _EnterCombat();

            me->AddAura(SPELL_TWILIGHT_PRECISION, me);

            if (Sammelruf()) // Falls noch Trash in der Instanz steht -> herbei rufen!
                instance->DoSendNotifyToInstance("%s ruft seine Truppen herbei!", me->GetNameForLocaleIdx(LOCALE_deDE));

            instance->SendEncounterUnit(ENCOUNTER_FRAME_ADD, me);
            instance->SetData(DATA_PHASE, PHASE_ONE);

            events.ScheduleEvent(EVENT_ACTIVATE_FIREWALL, SEKUNDEN_10);
            events.ScheduleEvent(EVENT_CLEAVE, urand(8 * IN_MILLISECONDS, SEKUNDEN_10));
            events.ScheduleEvent(EVENT_FLAME_BREATH, urand(SEKUNDEN_10, SEKUNDEN_15));
            events.ScheduleEvent(EVENT_METEOR_STRIKE, urand(SEKUNDEN_20, 25 * IN_MILLISECONDS));
            events.ScheduleEvent(EVENT_COMBUSTION, urand(SEKUNDEN_15, SEKUNDEN_20));
            events.ScheduleEvent(EVENT_TAIL_LASH, urand(SEKUNDEN_10, SEKUNDEN_20));
            events.ScheduleEvent(EVENT_ENRAGE, 8 * SEKUNDEN_60);
        }

        void JustDied(Unit * /*killer*/)
        {
            if (!instance)
                return;

            sLog->outError("halion: JustDied()");

            Talk(SAY_DEATH);

            _JustDied();

            instance->SendEncounterUnit(ENCOUNTER_FRAME_REMOVE, me);
            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWILIGHT_ENTER);

            if (GameObject * FlammenRing = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_FLAME_RING)))
                instance->HandleGameObject(instance->GetData64(DATA_FLAME_RING), true, FlammenRing);
        }

        Position const * GetMeteorStrikePosition() const
        {
            return & meteorStrikePos;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!instance || !UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            while(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_ACTIVATE_FIREWALL:
                        if (GameObject * FlammenRing = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_FLAME_RING)))
                            instance->HandleGameObject(instance->GetData64(DATA_FLAME_RING), false, FlammenRing);
                        break;
                    case EVENT_FLAME_BREATH:
                        DoCast(me, SPELL_FLAME_BREATH);
                        events.RescheduleEvent(EVENT_FLAME_BREATH, SEKUNDEN_20);
                        break;
                    case EVENT_CLEAVE:
                        DoCastVictim(SPELL_CLEAVE);
                        events.RescheduleEvent(EVENT_CLEAVE, urand(8 * IN_MILLISECONDS, SEKUNDEN_10));
                        break;
                    case EVENT_METEOR_STRIKE:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        {
                            Talk(SAY_SPECIAL01);
                            target->GetPosition(&meteorStrikePos);
                            me->CastSpell(meteorStrikePos.GetPositionX(), meteorStrikePos.GetPositionY(), meteorStrikePos.GetPositionZ(), SPELL_METEOR_STRIKE, true, NULL, NULL, me->GetGUID());
                        }
                        events.RescheduleEvent(EVENT_METEOR_STRIKE, SEKUNDEN_40);
                        break;
                    case EVENT_COMBUSTION:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                            DoCast(target, SPELL_COMBUSTION);
                        else if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_COMBUSTION);
                        events.RescheduleEvent(EVENT_COMBUSTION, 25 * IN_MILLISECONDS);
                        break;
                    case EVENT_TAIL_LASH:
                        DoCastAOE(SPELL_TAIL_LASH);
                        events.RescheduleEvent(EVENT_TAIL_LASH, urand(SEKUNDEN_05, SEKUNDEN_10));
                        break;
                    case EVENT_ENRAGE:
                        Talk(SAY_BERSERK);
                        DoCast(SPELL_BERSERK);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        Position meteorStrikePos;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return GetRubySanctumAI<boss_halionAI>(creature);
    }
};

typedef boss_halion::boss_halionAI HalionAI;

class boss_halion_twilight : public CreatureScript
{
public:
    boss_halion_twilight() : CreatureScript("boss_halion_twilight") { }

    struct boss_halion_twilightAI : public BossAI
    {
        boss_halion_twilightAI(Creature * creature) : BossAI(creature, DATA_HALION_TWILIGHT)
        {
            me->SetPhaseMask(me->GetPhaseMask() &~ 0x01, true); // Spawn-PhaseMask (1) entfernen, da er sonst auch in Phase 1 sichtbar ist!
        }

        void Reset()
        {
            if (!instance)
                return;

            sLog->outError("haliontwilight: Reset()");

            _Reset();

            if (instance->GetBossState(DATA_HALION) != IN_PROGRESS)
            {
                if (Creature * Halion = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION)))
                    if (HalionAI * halionAI = CAST_AI(HalionAI, Halion->AI()))
                    {
                        sLog->outError("haliontwilight: Reset() - Halion Berserk");
                        halionAI->DoCast(me, SPELL_BERSERK, true);
                    }
                sLog->outError("haliontwilight: Reset()->ForcedDespawn()");
                me->ForcedDespawn();
            }
        }

        void EnterCombat(Unit * /*victim*/)
        {
            if (!instance)
                return;

            sLog->outError("haliontwilight: EnterCombat()");

            _EnterCombat();

            events.ScheduleEvent(EVENT_CLEAVE, urand(8 * IN_MILLISECONDS, SEKUNDEN_10));
            events.ScheduleEvent(EVENT_TAIL_LASH, urand(SEKUNDEN_10, SEKUNDEN_20));
            events.ScheduleEvent(EVENT_CORPOREALITY, SEKUNDEN_05);
            events.ScheduleEvent(EVENT_ENRAGE, 8 * SEKUNDEN_60);
            events.ScheduleEvent(EVENT_DARK_BREATH, urand(SEKUNDEN_10, SEKUNDEN_15));
            events.ScheduleEvent(EVENT_CONSUMTION, urand(SEKUNDEN_15, SEKUNDEN_20));

            me->SummonCreature(NPC_KUGELROTATIONSFOKUS, HalionSpawnPos);
        }

        void DoAction(const int32 action)
        {
            switch(action)
            {
                case ACTION_PREPARE:
                    Prepare();
                    break;
                default:
                    break;
            }
        }

        void Prepare()
        {
            if (!instance)
                return;

            sLog->outError("haliontwilight: Prepare()");

            me->AddAura(SPELL_TWILIGHT_ENTER, me);
            me->AddAura(SPELL_DUSK_SHROUD, me);
            me->AddAura(SPELL_TWILIGHT_PRECISION, me);

            if (Creature * Controller = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION_CONTROLLER)))
                me->getThreatManager().doAddThreat(Controller, 0.0f);

            me->SetHealth(instance->GetData(DATA_HALION_HEALTH));
        }

        void KilledUnit(Unit * victim)
        {
            if (!victim || !victim->ToPlayer())
                return;

            Talk(urand(SAY_SLAY01, SAY_SLAY02));
        }

        void JustDied(Unit * /*killer*/)
        {
            _JustDied();
            me->RemoveCorpse(false);
            if (instance)
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWILIGHT_ENTER);
        }

        void UpdateAI(const uint32 diff)
        {
            if (instance && instance->GetBossState(DATA_HALION) != IN_PROGRESS)
            {
                me->ForcedDespawn();
                return;
            }

            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            while(uint32 eventId = events.ExecuteEvent())
            {
                switch(eventId)
                {
                    case EVENT_DARK_BREATH:
                        DoCast(me, SPELL_DARK_BREATH);
                        events.RescheduleEvent(EVENT_DARK_BREATH, SEKUNDEN_20);
                        break;
                    case EVENT_CONSUMTION:
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 1))
                            DoCast(target, SPELL_CONSUMPTION);
                        else if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_CONSUMPTION);
                        events.RescheduleEvent(EVENT_CONSUMTION, 25 * IN_MILLISECONDS);
                        break;
                    case EVENT_CLEAVE:
                        DoCastVictim(SPELL_CLEAVE);
                        events.RescheduleEvent(EVENT_CLEAVE, urand(8 * IN_MILLISECONDS, SEKUNDEN_10));
                        break;
                    case EVENT_TAIL_LASH:
                        DoCastAOE(SPELL_TAIL_LASH);
                        events.RescheduleEvent(EVENT_TAIL_LASH, urand(SEKUNDEN_05, SEKUNDEN_10));
                        break;
                    case EVENT_ENRAGE:
                        DoCast(SPELL_BERSERK);
                        break;
                    default:
                        break;
                }
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return GetRubySanctumAI<boss_halion_twilightAI>(creature);
    }
};

typedef boss_halion_twilight::boss_halion_twilightAI HalionTwilightAI;

class npc_halion_controller : public CreatureScript
{
public:
    npc_halion_controller() : CreatureScript("npc_halion_controller") { }

    struct npc_halion_controllerAI : public ScriptedAI
    {
        npc_halion_controllerAI(Creature * creature) : ScriptedAI(creature), instance(creature->GetInstanceScript())
        {
            me->SetPhaseMask(me->GetPhaseMask() | 0x20, true); // 1 + 32
            me->SetReactState(REACT_PASSIVE);
        }

        void Reset()
        {
            sLog->outError("halioncontroller: Reset()");
            events.Reset();

            lastBuffReal = 0;
            lastBuffTwilight = 0;

            if (GameObject * GOPortal = me->FindNearestGameObject(GO_HALION_PORTAL_1, 100.0f))
                GOPortal->RemoveFromWorld();
            if (GameObject * GOPortal = me->FindNearestGameObject(GO_HALION_PORTAL_2, 100.0f))
                GOPortal->RemoveFromWorld();
            if (GameObject * GOPortal = me->FindNearestGameObject(GO_HALION_PORTAL_EXIT, 100.0f))
                GOPortal->RemoveFromWorld();

            if (instance)
            {
                instance->SetBossState(DATA_HALION, NOT_STARTED);
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_TWILIGHT_ENTER);

                if (GameObject * FlammenRing = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_FLAME_RING)))
                    instance->HandleGameObject(instance->GetData64(DATA_FLAME_RING), true, FlammenRing);

                if (GameObject * ZwielichtRing = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_ZWIELICHT_RING)))
                    instance->HandleGameObject(instance->GetData64(DATA_ZWIELICHT_RING), true, ZwielichtRing);
            }
        }

        void EnterCombat(Unit * /*attacker*/)
        {
            if (!instance)
                return;

            sLog->outError("halioncontroller: EnterCombat()");

            me->setActive(true);
            DoZoneInCombat();
            instance->SetBossState(DATA_HALION, IN_PROGRESS);
            events.ScheduleEvent(EVENT_CHECK_ENCOUNTER, SEKUNDEN_60);
        }

        void DoAction(const int32 action)
        {
            switch(action)
            {
                case ACTION_INTRO_HALION:
                    me->setActive(true);
                    events.ScheduleEvent(EVENT_START_INTRO, 2 * IN_MILLISECONDS);
                    events.ScheduleEvent(EVENT_INTRO_PROGRESS_1, 6 * IN_MILLISECONDS);
                    events.ScheduleEvent(EVENT_INTRO_PROGRESS_2, SEKUNDEN_10);
                    events.ScheduleEvent(EVENT_INTRO_PROGRESS_3, 14 * IN_MILLISECONDS);
                    break;
                case ACTION_SPAWN_HALION:
                    me->GetMap()->SummonCreature(NPC_HALION, HalionSpawnPos);
                    break;
                default:
                    break;
            }
        }

        bool GegnerVorhanden(bool real) // Überprüfen, ob Halion / HalionZwielicht einen Spieler in seiner Threatlist hat!
        {
            if (!instance)
                return false;

            Creature * boss = NULL;

            if (real)
                boss = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION));
            else
                boss = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION_TWILIGHT));

            if (boss)
            {
                std::list<HostileReference *> threatlist = boss->getThreatManager().getThreatList();
                for (std::list<HostileReference *>::const_iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
                    if ((*itr) && (*itr)->getSource())
                        if (Unit * unit = ObjectAccessor::GetUnit(*me, (*itr)->getUnitGuid()))
                            if (unit->ToPlayer())
                                return true; // Spieler gefunden - kein Berserk vom Boss nötig. ;)
            }
            return false;
        }

        void CheckEncounter()
        {
            if (!instance)
                return;

            if (!me->FindNearestPlayer(65.0f/*MAX_VISIBLE_DIST*/)) // Kein lebender Spieler mehr im Ring -> Wipe!
            {
                sLog->outError("halioncontroller: CheckEncounter()-> Kein Spieler mehr da!");
                if (Creature * Halion = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION)))
                    if (HalionAI * halionAI = CAST_AI(HalionAI, Halion->AI()))
                        if (Halion->isInCombat())
                            halionAI->EnterEvadeMode();

                if (me->isInCombat())
                {
                    events.CancelEvent(EVENT_CHECK_ENCOUNTER);
                    EnterEvadeMode();
                }
                return;
            }

            if (me->isInCombat() && instance->GetBossState(DATA_HALION) == DONE)
            {
                sLog->outError("halioncontroller: UpdateAI()->ForcedDespawn()");
                me->ForcedDespawn();
                return;
            }
            else if (me->isInCombat() && instance->GetBossState(DATA_HALION) != IN_PROGRESS)
            {
                sLog->outError("halioncontroller: UpdateAI()->EnterEvadeMode()");
                EnterEvadeMode();
                return;
            }

            switch(instance->GetData(DATA_PHASE))
            {
                case PHASE_ONE:
                    if (Creature * Halion = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION)))
                        if (Halion->HealthBelowPct(75))
                        {
                            sLog->outError("halioncontroller: Wechsel in Phase 2");
                            if (HalionAI * halionAI = CAST_AI(HalionAI, Halion->AI()))
                                halionAI->DoAction(ACTION_HALION_VERSCHWINDEN);

                            if (Creature * HalionZwielicht = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION_TWILIGHT)))
                            {
                                if (!HalionZwielicht->isAlive())
                                    HalionZwielicht->Respawn(true);
                            }
                            else if (Creature * HalionZwielicht = me->SummonCreature(NPC_HALION_TWILIGHT, HalionSpawnPos))
                            {
                                if (HalionTwilightAI * halionZwielichtAI = CAST_AI(HalionTwilightAI, HalionZwielicht->AI()))
                                    halionZwielichtAI->DoAction(ACTION_PREPARE);
                            }
                            // Den Spielern max. 15 Sek. geben, damit sie HalionZwielicht angreifen. Sonst geht Halion in den Berserk!
                            events.ScheduleEvent(EVENT_ZWIELICHT_COMBAT_CHECK, SEKUNDEN_15);
                        }
                    break;
                case PHASE_TWO:
                    if (Creature * HalionTwilight = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION_TWILIGHT)))
                        if (HalionTwilight->HealthBelowPct(50))
                        {
                            sLog->outError("halioncontroller: Wechsel in Phase 3");
                            if (GameObject * GOPortal = me->SummonGameObject(GO_HALION_PORTAL_EXIT, HalionSpawnPos.m_positionX, HalionSpawnPos.m_positionY, HalionSpawnPos.m_positionZ,
                                4.47206f, 0, 0, 0.786772f, -0.617243f, 99999999))
                            {
                                GOPortal->SetPhaseMask((me->GetPhaseMask() | 0x20) &~ 0x01, true); // 32
                                GOPortal->SetRespawnTime(9999999);
                                GOPortal->SetOwnerGUID(NULL);
                            }
                            instance->SetData(DATA_HALION_HEALTH, HalionTwilight->GetHealth());
                            instance->SetData(DATA_PHASE, PHASE_THREE);

                            if (Creature * Halion = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION)))
                                if (HalionAI * halionAI = CAST_AI(HalionAI, Halion->AI()))
                                    halionAI->DoAction(ACTION_HALION_ERSCHEINEN);

                            events.ScheduleEvent(EVENT_CORPOREALITY, SEKUNDEN_10);
                            events.ScheduleEvent(EVENT_REAL_COMBAT_CHECK, SEKUNDEN_10);
                        }
                    break;
                default:
                    break;
            }
        }

        void Corporeality()
        {
            if (!instance)
                return;

            if (Creature * Halion = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_HALION)))
                if (Creature * HalionTwilight = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_HALION_TWILIGHT)))
                {
                    sLog->outError("halioncontroller: Corporeality");
                    float HalionHP = (Halion && Halion->isAlive()) ? Halion->GetHealthPct() : 100.0f;
                    float HalionTwilightHP = (HalionTwilight && HalionTwilight->isAlive()) ? HalionTwilight->GetHealthPct() : 100.0f;
                    float m_diff = HalionHP - HalionTwilightHP;
                    uint8 buffnum;

                    if (m_diff <= Buff[0].diff)
                        buffnum = 0;
                    else
                    {
                        for (uint8 i=0; i<11; ++i)
                        {
                            if (m_diff >= Buff[i].diff)
                                buffnum = i+1;
                            else
                                break;
                        }
                    }

                    if (!lastBuffReal || lastBuffReal != Buff[buffnum].real)
                    {
                        if (Halion && Halion->isAlive())
                        {
                            if (lastBuffReal)
                                Halion->RemoveAurasDueToSpell(lastBuffReal);

                            Halion->CastSpell(Halion, Buff[buffnum].real, true);
                            lastBuffReal = Buff[buffnum].real;
                        }
                    }

                    if (!lastBuffTwilight || lastBuffTwilight != Buff[buffnum].twilight)
                    {
                        if (HalionTwilight && HalionTwilight->isAlive())
                        {
                            if (lastBuffTwilight)
                                HalionTwilight->RemoveAurasDueToSpell(lastBuffTwilight);

                            HalionTwilight->CastSpell(HalionTwilight, Buff[buffnum].twilight, true);
                            lastBuffTwilight = Buff[buffnum].twilight;
                        }
                    }
                    instance->SetData(DATA_COUNTER, uint32(50 + Buff[buffnum].diff));
                }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!instance)
                return;

            /*if (!me->isInCombat() && instance->GetBossState(DATA_HALION) == IN_PROGRESS)
            {
                sLog->outError("halioncontroller: UpdateAI()->EnterCombat()");
                if (Creature * Halion = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION)))
                    EnterCombat(Halion->getVictim());
            }*/

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START_INTRO:
                        DoCast(me, SPELL_COSMETIC_FIRE_PILLAR, true);
                        break;
                    case EVENT_INTRO_PROGRESS_1:
                        for (uint8 i=2; i<4; ++i)
                            if (GameObject * tree = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_BURNING_TREE_1 + i)))
                                instance->HandleGameObject(instance->GetData64(DATA_BURNING_TREE_1 + i), true, tree);
                        break;
                    case EVENT_INTRO_PROGRESS_2:
                        for (uint8 i=0; i<2; ++i)
                            if (GameObject * tree = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_BURNING_TREE_1 + i)))
                                instance->HandleGameObject(instance->GetData64(DATA_BURNING_TREE_1 + i), true, tree);
                        break;
                    case EVENT_INTRO_PROGRESS_3:
                        DoCast(me, SPELL_FIERY_EXPLOSION);
                        if (Creature * halion = me->GetMap()->SummonCreature(NPC_HALION, HalionSpawnPos))
                            halion->AI()->Talk(SAY_INTRO);
                        me->setActive(false);
                        break;
                    case EVENT_CORPOREALITY:
                        if (instance->GetData(DATA_PHASE) == PHASE_THREE)
                            Corporeality();
                        events.RescheduleEvent(EVENT_CORPOREALITY, SEKUNDEN_05);
                        break;
                    case EVENT_CHECK_ENCOUNTER:
                        CheckEncounter();
                        events.RescheduleEvent(EVENT_CHECK_ENCOUNTER, 2 * IN_MILLISECONDS);
                        break;
                    case EVENT_ZWIELICHT_COMBAT_CHECK:
                        if (!GegnerVorhanden(false))
                            if (Creature * Halion = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION)))
                                if (HalionAI * halionAI = CAST_AI(HalionAI, Halion->AI()))
                                    halionAI->DoCast(me, SPELL_BERSERK, true);
                        events.RescheduleEvent(EVENT_ZWIELICHT_COMBAT_CHECK, SEKUNDEN_10);
                        break;
                    case EVENT_REAL_COMBAT_CHECK:
                        if (!GegnerVorhanden(true))
                            if (Creature * HalionZwielicht = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_HALION_TWILIGHT)))
                                if (HalionTwilightAI * halionZwielichtAI = CAST_AI(HalionTwilightAI, HalionZwielicht->AI()))
                                    halionZwielichtAI->DoCast(me, SPELL_BERSERK, true);
                        events.RescheduleEvent(EVENT_REAL_COMBAT_CHECK, SEKUNDEN_10);
                        break;
                    default:
                        break;
                }
            }
        }
    private:
        EventMap events;
        InstanceScript * instance;
        uint32 lastBuffReal;
        uint32 lastBuffTwilight;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return GetRubySanctumAI<npc_halion_controllerAI>(creature);
    }
};

class mob_orb_rotation_focus : public CreatureScript
{
public:
    mob_orb_rotation_focus() : CreatureScript("mob_orb_rotation_focus") { }

    struct mob_orb_rotation_focusAI : public ScriptedAI
    {
        mob_orb_rotation_focusAI(Creature * creature) : ScriptedAI(creature)
        {
            sLog->outError("focus: gespawned");
            instance = creature->GetInstanceScript();
            //me->SetPhaseMask((me->GetPhaseMask() | 0x20) &~ 0x01, true); // 32
        }

        void Reset()
        {
            m_direction = 0.0f;
            m_nextdirection = 0.0f;
            m_timer = SEKUNDEN_30;
            m_warning = false;

            if (instance)
            {
                Creature * orb1 = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_SCHATTENKUGEL_N));
                if (!orb1)
                {
                    float x,y;
                    me->GetNearPoint2D(x, y, FR_RADIUS, m_direction);
                    orb1 = me->SummonCreature(NPC_SCHATTENKUGEL_N, x, y, me->GetPositionZ(), 0);
                }
                else if (!orb1->isAlive())
                    orb1->Respawn();

                Creature * orb2 = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_SCHATTENKUGEL_S));
                if (!orb2)
                {
                    float x,y;
                    me->GetNearPoint2D(x, y, FR_RADIUS, m_direction + M_PI);
                    orb2 = me->SummonCreature(NPC_SCHATTENKUGEL_S, x, y, me->GetPositionZ(), 0);
                }
                else if (!orb2->isAlive())
                    orb2->Respawn();

                if (IsHeroic())
                {
                    Creature * orb3 = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_SCHATTENKUGEL_O));
                    if (!orb3)
                    {
                        float x,y;
                        me->GetNearPoint2D(x, y, FR_RADIUS, m_direction + (M_PI / 2));
                        orb3 = me->SummonCreature(NPC_SCHATTENKUGEL_O, x, y, me->GetPositionZ(), 0);
                    }
                    else if (!orb3->isAlive())
                        orb3->Respawn();

                    Creature * orb4 = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_SCHATTENKUGEL_W));
                    if (!orb4)
                    {
                        float x,y;
                        me->GetNearPoint2D(x, y, FR_RADIUS, m_direction - (M_PI / 2));
                        orb4 = me->SummonCreature(NPC_SCHATTENKUGEL_W, x, y, me->GetPositionZ(), 0);
                    }
                    else if (!orb4->isAlive())
                        orb4->Respawn();
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!instance)
                return;

            /*if (instance->GetBossState(DATA_HALION) != IN_PROGRESS || instance->GetBossState(DATA_HALION_TWILIGHT) == DONE)
                me->ForcedDespawn();*/

            if (instance->GetData(DATA_SCHATTENKUGEL_N) == DONE && instance->GetData(DATA_SCHATTENKUGEL_S) == DONE)
            {
                m_direction = m_nextdirection;
                m_nextdirection = (m_direction - M_PI / 64.0f);

                if (m_nextdirection < 0.0f )
                    m_nextdirection = m_nextdirection + 2.0f * M_PI;

                instance->SetData(DATA_KUGEL_RICHTUNG, uint32(m_nextdirection*1000));
                instance->SetData(DATA_SCHATTENKUGEL_N, SPECIAL);
                instance->SetData(DATA_SCHATTENKUGEL_S, SPECIAL);
            }

            if (IsHeroic() && instance->GetData(DATA_SCHATTENKUGEL_O) == DONE && instance->GetData(DATA_SCHATTENKUGEL_W) == DONE)
            {
                //instance->SetData(DATA_KUGEL_RICHTUNG, uint32(m_nextdirection*1000));
                instance->SetData(DATA_SCHATTENKUGEL_O, SPECIAL);
                instance->SetData(DATA_SCHATTENKUGEL_W, SPECIAL);
            }

            if (m_timer - 6000 <= diff && !m_warning)
            {
                if (Creature * HalionTwilight = ObjectAccessor::GetCreature(*me, instance->GetData64(NPC_HALION_TWILIGHT)))
                    if (HalionTwilightAI * halionZwielichtAI = CAST_AI(HalionTwilightAI, HalionTwilight->AI()))
                        halionZwielichtAI->Talk(SAY_SPECIAL02);
                m_warning = true;
            }
            else
                m_timer -= diff;
        }
    private:
        InstanceScript * instance;
        uint32 m_timer;
        float m_direction;
        float m_nextdirection;
        bool m_warning;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new mob_orb_rotation_focusAI(creature);
    }
};

class mob_halion_orb : public CreatureScript
{
public:
    mob_halion_orb() : CreatureScript("mob_halion_orb") { }

    struct mob_halion_orbAI : public ScriptedAI
    {
        mob_halion_orbAI(Creature * creature) : ScriptedAI(creature)
        {
            sLog->outError("orb: gespawned");
            instance = creature->GetInstanceScript();
            //me->SetPhaseMask((me->GetPhaseMask() | 0x20) &~ 0x01, true); // 32
        }

        void Reset()
        {
            switch(me->GetEntry())
            {
                case NPC_SCHATTENKUGEL_N:
                    m_flag = DATA_SCHATTENKUGEL_N;
                    m_delta = 0.0f;
                    break;
                case NPC_SCHATTENKUGEL_S:
                    m_flag = DATA_SCHATTENKUGEL_S;
                    m_delta = M_PI;
                    break;
                case NPC_SCHATTENKUGEL_O:
                    m_flag = DATA_SCHATTENKUGEL_O;
                    m_delta = M_PI / 2;
                    break;
                case NPC_SCHATTENKUGEL_W:
                    m_flag = DATA_SCHATTENKUGEL_W;
                    m_delta -= M_PI / 2;
                    break;
            }

            if (instance)
                instance->SetData(m_flag, DONE);

            SetCombatMovement(false);
            m_direction = 0.0f;
            nextPoint = 0;
            MovementStarted = false;
            m_delta = 0.0f;
            SpawnCastTimer = SEKUNDEN_30;
        }

        void AttackStart(Unit * /*who*/, float /*dist*/)
        {
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE || !MovementStarted)
                return;

            if (id == nextPoint)
            {
                me->GetMotionMaster()->MovementExpired();
                MovementStarted = false;
                if (instance)
                    instance->SetData(m_flag, DONE);
            }
        }

        void StartMovement(uint32 id)
        {
            nextPoint = id;
            float x,y;
            MovementStarted = true;

            if (instance)
            {
                instance->SetData(m_flag, IN_PROGRESS);
                m_direction = float(instance->GetData(DATA_KUGEL_RICHTUNG) / 1000 + m_delta);

                if (m_direction > 2.0f * M_PI)
                    m_direction = m_direction - 2.0f * M_PI;
                if (Creature * focus = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_KUGELROTATIONSFOKUS)))
                    focus->GetNearPoint2D(x, y, FR_RADIUS, m_direction);
                else
                    me->ForcedDespawn();
            }
            me->GetMotionMaster()->Clear();
            me->GetMotionMaster()->InitDefault();
            me->GetMotionMaster()->MovePoint(id, x, y,  me->GetPositionZ());
        }

        void UpdateAI(const uint32 diff)
        {
            if (!instance)
                return;

            /*if (instance->GetBossState(DATA_HALION) != IN_PROGRESS || instance->GetBossState(DATA_HALION_TWILIGHT) == DONE)
                me->ForcedDespawn();*/

            if (SpawnCastTimer <= diff)
            {
                if (Creature * focus = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_KUGELROTATIONSFOKUS)))
                {
                    DoCast(focus, SPELL_TWILIGHT_CUTTER);
                    me->SummonCreature(NPC_KUGELTRAEGER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0);
                }
                SpawnCastTimer = SEKUNDEN_30;
            }
            else
                SpawnCastTimer -= diff;

            if (!MovementStarted && instance->GetData(m_flag) == SPECIAL)
                StartMovement(1);
        }
    private:
        InstanceScript * instance;
        float m_direction,m_delta;
        uint32 m_flag;
        uint32 m_flag1;
        bool MovementStarted;
        uint32 nextPoint;
        uint32 SpawnCastTimer;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new mob_halion_orbAI(creature);
    }
};

class mob_orb_carrier : public CreatureScript
{
public:
    mob_orb_carrier() : CreatureScript("mob_orb_carrier") { }

    struct mob_orb_carrierAI : public ScriptedAI
    {
        mob_orb_carrierAI(Creature * creature) : ScriptedAI(creature)
        {
            sLog->outError("carrier: gespawned");
            instance = creature->GetInstanceScript();
            //me->SetPhaseMask((me->GetPhaseMask() | 0x20) &~ 0x01, true); // 32
            me->SetDisplayId(11686);
        }

        void Reset()
        {
            Ziel = DATA_KUGELROTATIONSFOKUS;
            DoCast(SPELL_TWILIGHT_PULSE_PERIODIC);
            timer = SEKUNDEN_10;
            MeineKugel = 0;
            SetCombatMovement(false);
            MovementStarted = false;
        }

        void AttackStart(Unit * /*who*/, float /*dist*/)
        {
        }

        void IsSummonedBy(Unit * summoner)
        {
            if (summoner && summoner->IsInWorld())
                switch(summoner->GetEntry())
                {
                    case NPC_SCHATTENKUGEL_N: MeineKugel = DATA_SCHATTENKUGEL_N; break;
                    case NPC_SCHATTENKUGEL_S: MeineKugel = DATA_SCHATTENKUGEL_S; break;
                    case NPC_SCHATTENKUGEL_O: MeineKugel = DATA_SCHATTENKUGEL_O; break;
                    case NPC_SCHATTENKUGEL_W: MeineKugel = DATA_SCHATTENKUGEL_W; break;
                }
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE || !MovementStarted)
                return;

            switch(id)
            {
                case DATA_KUGELROTATIONSFOKUS:
                    Ziel = MeineKugel;
                    break;
                case DATA_SCHATTENKUGEL_N:
                case DATA_SCHATTENKUGEL_S:
                case DATA_SCHATTENKUGEL_O:
                case DATA_SCHATTENKUGEL_W:
                    Ziel = DATA_KUGELROTATIONSFOKUS;
                    break;
            }
            MovementStarted = false;
        }

        void LaufKleinesMaedchenLauf()
        {
            float x,y;
            float m_direction = float(instance->GetData(DATA_KUGEL_RICHTUNG) / 1000.0f + M_PI - M_PI / 32.0f);

            if (m_direction > 2.0f * M_PI)
                m_direction = m_direction - 2.0f * M_PI;

            if (Creature * focus = ObjectAccessor::GetCreature(*me, instance->GetData64(Ziel)))
                focus->GetPosition(x, y);
            else
                me->ForcedDespawn();

            me->GetMotionMaster()->MovePoint(Ziel, x, y,  me->GetPositionZ());

            MovementStarted = true;
        }

        void UpdateAI(const uint32 diff)
        {
            if (!instance)
                return;

            if (timer <= diff)
                me->ForcedDespawn();
            else
                timer -= diff;

            /*if (instance->GetBossState(DATA_HALION) != IN_PROGRESS || instance->GetBossState(DATA_HALION_TWILIGHT) == DONE)
                me->ForcedDespawn();*/

            if (!MovementStarted)
                LaufKleinesMaedchenLauf();
        }
    private:
        InstanceScript * instance;
        bool MovementStarted;
        uint32 MeineKugel;
        uint32 Ziel;
        uint32 timer;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new mob_orb_carrierAI(creature);
    }
};

class npc_meteor_strike_initial : public CreatureScript
{
public:
    npc_meteor_strike_initial() : CreatureScript("npc_meteor_strike_initial") { }

    struct npc_meteor_strike_initialAI : public Scripted_NoMovementAI
    {
        npc_meteor_strike_initialAI(Creature * creature) : Scripted_NoMovementAI(creature)
        {
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_METEOR_STRIKE_AOE)
            {
                DoCast(me, SPELL_METEOR_STRIKE_AOE_DAMAGE, true);
                DoCast(me, SPELL_METEOR_STRIKE_FIRE_AURA_1, true);

                for (std::list<Creature *>::iterator itr = meteorList.begin(); itr != meteorList.end(); ++itr)
                    (*itr)->AI()->DoAction(ACTION_METEOR_STRIKE_BURN);

                if (SPELL_METEOR_STRIKE_AOE_DAMAGE == 75879) // Spawnt im 25H NPC_LIVING_INFERNO
                    if (HalionAI * halionAI = CAST_AI(HalionAI, owner->AI()))
                        for (uint8 i=0; i<10; ++i)
                            if (Unit * temp = me->SummonCreature(NPC_LIVING_EMBER, *halionAI->GetMeteorStrikePosition(), TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, SEKUNDEN_60)) // Spawnen beim Aufschlag im 25H!
                                temp->GetMotionMaster()->MoveRandom(10.0f);
            }
        }

        void IsSummonedBy(Unit * summoner)
        {
            owner = summoner->ToCreature();
            if (!owner)
                return;

            DoCast(me, SPELL_METEOR_STRIKE_COUNTDOWN);
            DoCast(me, SPELL_BIRTH_NO_VISUAL); // Unknown purpose

            if (HalionAI * halionAI = CAST_AI(HalionAI, owner->AI()))
            {
                Position const * ownerPos = halionAI->GetMeteorStrikePosition();
                Position newpos;
                float angle[4];
                angle[0] = me->GetAngle(ownerPos);
                angle[1] = me->GetAngle(ownerPos) - static_cast<float>(M_PI/2);
                angle[2] = me->GetAngle(ownerPos) - static_cast<float>(-M_PI/2);
                angle[3] = me->GetAngle(ownerPos) - static_cast<float>(M_PI);

                meteorList.clear();
                for (uint8 i=0; i<4; ++i)
                {
                    MapManager::NormalizeOrientation(angle[i]);
                    me->SetOrientation(angle[i]);
                    me->GetNearPosition(newpos, 10.0f, 0.0f); // Exact distance
                    if (Creature * meteor = me->SummonCreature(NPC_METEOR_STRIKE_NORTH + i, newpos, TEMPSUMMON_TIMED_DESPAWN, SEKUNDEN_30))
                        meteorList.push_back(meteor);
                }
            }
        }

        void UpdateAI(uint32 const /*diff*/) { }
        void EnterEvadeMode() { }

    private:
        Creature * owner;
        std::list<Creature *> meteorList;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return GetRubySanctumAI<npc_meteor_strike_initialAI>(creature);
    }
};

class npc_meteor_strike : public CreatureScript
{
public:
    npc_meteor_strike() : CreatureScript("npc_meteor_strike") { }

    struct npc_meteor_strikeAI : public Scripted_NoMovementAI
    {
        npc_meteor_strikeAI(Creature * creature) : Scripted_NoMovementAI(creature)
        {
            range = 5.0f;
            spawnCount = 0;
        }

        void DoAction(int32 const action)
        {
            if (action == ACTION_METEOR_STRIKE_BURN)
            {
                DoCast(me, SPELL_METEOR_STRIKE_FIRE_AURA_2, true);
                me->setActive(true);
                events.ScheduleEvent(EVENT_SPAWN_METEOR_FLAME, 500);
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (spawnCount > 5)
                return;

            events.Update(diff);

            if (events.ExecuteEvent() == EVENT_SPAWN_METEOR_FLAME)
            {
                Position pos;
                me->GetNearPosition(pos, range, 0.0f);

                if (Creature * flame = me->SummonCreature(NPC_METEOR_STRIKE_FLAME, pos, TEMPSUMMON_TIMED_DESPAWN, 25 * IN_MILLISECONDS))
                {
                    flame->CastSpell(flame, SPELL_METEOR_STRIKE_FIRE_AURA_2, true);
                    spawnCount++;
                }
                range += 5.0f;
                events.ScheduleEvent(EVENT_SPAWN_METEOR_FLAME, 800);
            }
        }
    private:
        EventMap events;
        float range;
        uint8 spawnCount;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return GetRubySanctumAI<npc_meteor_strikeAI>(creature);
    }
};

class npc_combustion : public CreatureScript
{
public:
    npc_combustion() : CreatureScript("npc_combustion") { }

    struct npc_combustionAI : public Scripted_NoMovementAI
    {
        npc_combustionAI(Creature * creature) : Scripted_NoMovementAI(creature)
        {
        }

        void SetData(uint32 type, uint32 data)
        {
            if (type == TYPE_COMBUSTION_SUMMON)
            {
                int32 damage = 1200 + (data * 1290); // Hardcoded values from guessing. Need some more research.
                me->CastCustomSpell(SPELL_COMBUSTION_EXPLOSION, SPELLVALUE_BASE_POINT0, damage, me, true);
                // Scaling aura
                me->CastCustomSpell(SPELL_COMBUSTION_SCALE_AURA, SPELLVALUE_AURA_STACK, data, me, false);
                DoCast(me, SPELL_COMBUSTION_DAMAGE_AURA);
            }
        }

        void UpdateAI(uint32 const /*diff*/) { }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return GetRubySanctumAI<npc_combustionAI>(creature);
    }
};

class mob_soul_consumption : public CreatureScript
{
public:
    mob_soul_consumption() : CreatureScript("mob_soul_consumption") { }

    struct mob_soul_consumptionAI : public ScriptedAI
    {
        mob_soul_consumptionAI(Creature * creature) : ScriptedAI(creature)
        {
            sLog->outError("consumption: gespawned");
            instance = creature->GetInstanceScript();
        }

        InstanceScript * instance;
        float m_Size0;
        float m_Size;
        uint32 m_uiConsumptTimer;

        void Reset()
        {
            if (!IsHeroic())
                me->SetPhaseMask((me->GetPhaseMask() | 0x20) &~ 0x01, true); // 32
            else
                me->SetPhaseMask(me->GetPhaseMask() | 0x20, true); // 1 + 32

            m_uiConsumptTimer = SEKUNDEN_60;
            DoCast(SPELL_CONSUMPTION_DAMAGE_AURA);
            m_Size0 = me->GetFloatValue(OBJECT_FIELD_SCALE_X);
            m_Size = m_Size0;
        }

        void AttackStart(Unit * /*who*/, float /*dist*/)
        {
        }

        void UpdateAI(const uint32 diff)
        {
            if (instance && instance->GetData(DATA_HALION) != IN_PROGRESS)
                me->ForcedDespawn();

            if (m_uiConsumptTimer <= diff)
                me->ForcedDespawn();
            else
                m_uiConsumptTimer -= diff;

            if (SelectTarget(SELECT_TARGET_RANDOM, 1, m_Size * 3.0f, true))
            {
                m_Size = float(m_Size * 1.01);
                me->SetFloatValue(OBJECT_FIELD_SCALE_X, m_Size);
            }
            else if (SelectTarget(SELECT_TARGET_RANDOM, 0, m_Size * 3.0f, true))
            {
                m_Size = float(m_Size * 1.01);
                me->SetFloatValue(OBJECT_FIELD_SCALE_X, m_Size);
            }
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new mob_soul_consumptionAI(creature);
    }
};

class go_halion_portal_twilight : public GameObjectScript
{
public:
    go_halion_portal_twilight() : GameObjectScript("go_halion_portal_twilight") { }

    bool OnGossipHello(Player * player, GameObject * /*go*/)
    {
        player->AddAura(SPELL_TWILIGHT_ENTER, player);
        return true;
    }
};

class go_halion_portal_real : public GameObjectScript
{
public:
    go_halion_portal_real() : GameObjectScript("go_halion_portal_real") { }

    bool OnGossipHello(Player * player, GameObject * /*go*/)
    {
        player->AddAura(SPELL_TWILIGHT_LEAVE, player);
        player->RemoveAurasDueToSpell(SPELL_TWILIGHT_ENTER);
        return true;
    }
};

class go_halion_portal_exit : public GameObjectScript
{
public:
    go_halion_portal_exit() : GameObjectScript("go_halion_portal_exit") { }

    bool OnGossipHello(Player * player, GameObject * /*go*/)
    {
        player->AddAura(SPELL_TWILIGHT_LEAVE, player);
        player->RemoveAurasDueToSpell(SPELL_TWILIGHT_ENTER);
        return true;
    }
};

class spell_halion_meteor_strike_marker : public SpellScriptLoader
{
public:
    spell_halion_meteor_strike_marker() : SpellScriptLoader("spell_halion_meteor_strike_marker") { }

    class spell_halion_meteor_strike_marker_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_halion_meteor_strike_marker_AuraScript);

        void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                if (Creature * creCaster = GetCaster()->ToCreature())
                    creCaster->AI()->DoAction(ACTION_METEOR_STRIKE_AOE);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_halion_meteor_strike_marker_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript * GetAuraScript() const
    {
        return new spell_halion_meteor_strike_marker_AuraScript();
    }
};

class spell_halion_combustion : public SpellScriptLoader
{
public:
    spell_halion_combustion() : SpellScriptLoader("spell_halion_combustion") { }

    class spell_halion_combustion_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_halion_combustion_AuraScript);

        bool Validate(SpellEntry const * /*spell*/)
        {
            if (!sSpellStore.LookupEntry(SPELL_COMBUSTION_STACK))
                return false;
            return true;
        }

        void HandleExtraEffect(AuraEffect const * /*aurEff*/)
        {
            GetTarget()->CastSpell(GetTarget(), SPELL_COMBUSTION_STACK, true);
        }

        void OnApply(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->CastSpell(GetTarget(), SPELL_COMBUSTION_STACK, true);
        }

        void OnRemove(AuraEffect const * /*aurEff*/, AuraEffectHandleModes /*mode*/)
        {
            if (GetTarget()->HasAura(SPELL_COMBUSTION_STACK))
                GetTarget()->RemoveAurasDueToSpell(SPELL_COMBUSTION_STACK, 0, 0, AURA_REMOVE_BY_ENEMY_SPELL);
        }

        void Register()
        {
            OnEffectPeriodic += AuraEffectPeriodicFn(spell_halion_combustion_AuraScript::HandleExtraEffect, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE);
            AfterEffectApply += AuraEffectApplyFn(spell_halion_combustion_AuraScript::OnApply, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
            AfterEffectRemove += AuraEffectRemoveFn(spell_halion_combustion_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript * GetAuraScript() const
    {
        return new spell_halion_combustion_AuraScript();
    }
};

class spell_halion_combustion_stack : public SpellScriptLoader
{
public:
    spell_halion_combustion_stack() : SpellScriptLoader("spell_halion_combustion_stack") { }

    class spell_halion_combustion_stack_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_halion_combustion_stack_AuraScript);

        bool Validate(SpellEntry const * /*spell*/)
        {
            if (!sSpellStore.LookupEntry(SPELL_COMBUSTION_SUMMON))
                return false;
            if (!sSpellStore.LookupEntry(SPELL_COMBUSTION_EXPLOSION))
                return false;
            return true;
        }

        void OnRemove(AuraEffect const * aurEff, AuraEffectHandleModes /*mode*/)
        {
            GetTarget()->CastCustomSpell(SPELL_COMBUSTION_SUMMON, SPELLVALUE_BASE_POINT0, aurEff->GetBase()->GetStackAmount(), GetTarget(), true);
        }

        void Register()
        {
            AfterEffectRemove += AuraEffectRemoveFn(spell_halion_combustion_stack_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        }
    };

    AuraScript * GetAuraScript() const
    {
        return new spell_halion_combustion_stack_AuraScript();
    }
};

class spell_combustion_consumption_summon : public SpellScriptLoader
{
public:
    spell_combustion_consumption_summon() : SpellScriptLoader("spell_combustion_consumption_summon") { }

    class spell_combustion_consumption_summon_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_combustion_consumption_summon_SpellScript);

        void HandleSummon(SpellEffIndex effIndex)
        {
            PreventHitDefaultEffect(effIndex);

            Unit * caster = GetCaster();
            if (!caster)
                return;

            uint32 entry = uint32(GetSpellInfo()->Effects[effIndex].MiscValue);
            SummonPropertiesEntry const * properties = sSummonPropertiesStore.LookupEntry(uint32(GetSpellInfo()->Effects[effIndex].MiscValueB));
            uint32 duration = uint32(GetSpellInfo()->GetDuration());

            Position pos;
            caster->GetPosition(&pos);
            if (TempSummon * summon = caster->GetMap()->SummonCreature(entry, pos, properties, duration, caster, GetSpellInfo()->Id))
                summon->AI()->SetData(TYPE_COMBUSTION_SUMMON, uint32(GetSpellInfo()->Effects[effIndex].BasePoints));
        }

        void Register()
        {
            OnEffectHit += SpellEffectFn(spell_combustion_consumption_summon_SpellScript::HandleSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
        }
    };

    SpellScript * GetSpellScript() const
    {
        return new spell_combustion_consumption_summon_SpellScript();
    }
};

void AddSC_boss_halion()
{
    new boss_halion();
    new boss_halion_twilight();

    new npc_halion_controller();
    new npc_meteor_strike_initial();
    new npc_meteor_strike();
    new npc_combustion();

    new mob_halion_orb();
    new mob_orb_rotation_focus();
    new mob_orb_carrier();
    new mob_soul_consumption();

    new go_halion_portal_twilight();
    new go_halion_portal_real();
    new go_halion_portal_exit();

    new spell_halion_meteor_strike_marker();
    new spell_halion_combustion();
    new spell_halion_combustion_stack();
    new spell_combustion_consumption_summon();
}
