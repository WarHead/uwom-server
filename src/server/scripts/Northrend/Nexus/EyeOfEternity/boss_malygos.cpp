/*
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

/* Script Data Start
SDName: Boss malygos
Script Data End */

// TO-DOs:
// Implement a better pathing for Malygos.
// Find sniffed spawn position for chest
// Remove hack that re-adds targets to the aggro list after they enter to a vehicle when it works as expected
// Platform is susceptible to siege damage. sniff-conform sollution? e.g. not upping hitpoints or changing faction to 35
// Improve whatever can be improved :)

#include "ScriptPCH.h"
#include "eye_of_eternity.h"
#include "ScriptedEscortAI.h"
#include "Vehicle.h"

// not implemented
enum Achievements
{
    ACHIEV_TIMED_START_EVENT   = 20387,
};

enum Events
{
    // =========== PHASE ONE ===============
    EVENT_ARCANE_BREATH = 1,
    EVENT_ARCANE_STORM  = 2,
    EVENT_VORTEX        = 3,
    EVENT_POWER_SPARKS  = 4,

    // =========== PHASE TWO ===============
    EVENT_SURGE_POWER   = 5, // wowhead is wrong, Surge of Power is casted instead of Arcane Pulse (source sniffs!)
    EVENT_SUMMON_ARCANE = 6,

    // =========== PHASE TWO ===============
    EVENT_SURGE_POWER_PHASE_3 = 7,
    EVENT_STATIC_FIELD = 8,

    // =============== YELLS ===============
    EVENT_YELL_0 = 9,
    EVENT_YELL_1 = 10,
    EVENT_YELL_2 = 11,
    EVENT_YELL_3 = 12,
    EVENT_YELL_4 = 13,
};

enum Phases
{
    PHASE_ONE = 1,
    PHASE_TWO = 2,
    PHASE_THREE = 3
};

enum Spells
{
    // ============= PREFIGHT ==============
    SPELL_BERSERKER            = 60670,
    SPELL_PORTAL_VISUAL_CLOSED = 55949,
    SPELL_PORTAL_BEAM          = 56046,
    SPELL_IRIS_OPENED          = 61012,

    // =========== PHASE ONE ===============
    SPELL_ARCANE_BREATH_10     = 56272,
    SPELL_ARCANE_STORM_10      = 57459,
    SPELL_ARCANE_BREATH_25     = 60072,
    SPELL_ARCANE_STORM_25      = 61694,

    SPELL_VORTEX_1             = 56237,                     // seems that frezze object animation
    SPELL_VORTEX_2             = 55873,                     // visual effect
    SPELL_VORTEX_3             = 56105,                     // this spell must handle all the script - casted by the boss and to himself
    //SPELL_VORTEX_4           = 55853,                     // damage | used to enter to the vehicle - defined in eye_of_eternity.h
    //SPELL_VORTEX_5           = 56263,                     // damage | used to enter to the vehicle - defined in eye_of_eternity.h
    SPELL_VORTEX_6             = 73040,                     // teleport - (casted to all raid) | caster 30090 | target player

    SPELL_SUMMON_POWER_PARK    = 56142,
    SPELL_POWER_SPARK_DEATH    = 55852,
    SPELL_POWER_SPARK_MALYGOS  = 56152,

    // =========== PHASE TWO ===============
    SPELL_SURGE_POWER_P2       = 56505,                     // used in phase 2
    SPELL_ARCANE_BOMB_SUMMON   = 56429,
    SPELL_ARCANE_BOMB_MISSILE  = 56430,
    SPELL_ARCANE_BOMB_EFFECT   = 56431,
    SPELL_ARCANE_OVERLOAD      = 56432,
    SPELL_ARCANE_OVERLOAD_SIZE = 56435,

    SPELL_ARCANE_BARRAGE_TRIG  = 56397,
    SPELL_TELEPORT_VISUAL      = 51347,

    // ========== PHASE THREE ==============
    SPELL_PLATTFORM_CHANNEL    = 58842,
    SPELL_PLATTFORM_BOOM       = 59084,
    SPELL_SURGE_POWER_P3       = 60936,
    SPELL_STATIC_FIELD         = 57430,
    SPELL_ARCANE_PULSE         = 57432,

    // ========= ALEXSTRASZA OUTRO =========

    SPELL_GIFT_CHANNEL         = 61028,
    SPELL_GIFT_VISUAL          = 61023
};

enum Movements
{
    MOVE_VORTEX = 1,
    MOVE_PHASE_TWO,
    MOVE_DEEP_BREATH_ROTATION,
    MOVE_INIT_PHASE_ONE,
    MOVE_CENTER_PLATFORM
};

enum Seats
{
    SEAT_0 = 0,
};

enum Factions
{
    FACTION_FRIENDLY = 35,
    FACTION_HOSTILE = 14
};

enum Actions
{
    ACTION_HOVER_DISK_START_WP_1,
    ACTION_HOVER_DISK_START_WP_2
};

enum MalygosSays
{
    SAY_AGGRO_P_ONE,
    SAY_KILLED_PLAYER_P_ONE,
    SAY_END_P_ONE,
    SAY_AGGRO_P_TWO,
    SAY_ANTI_MAGIC_SHELL, // not sure when execute it
    SAY_MAGIC_BLAST,  // not sure when execute it
    SAY_KILLED_PLAYER_P_TWO,
    SAY_END_P_TWO,
    SAY_INTRO_P_THREE,
    SAY_AGGRO_P_THREE,
    SAY_SURGE_POWER,  // not sure when execute it
    SAY_BUFF_SPARK,
    SAY_KILLED_PLAYER_P_THREE,
    SAY_SPELL_CASTING_P_THREE,
    SAY_DEATH
};

#define MAX_HOVER_DISK_WAYPOINTS 18

// Sniffed data (x,y,z)
const Position HoverDiskWaypoints[MAX_HOVER_DISK_WAYPOINTS] =
{
   {782.9821f, 1296.652f, 282.1114f, 0.0f},
   {779.5459f, 1287.228f, 282.1393f, 0.0f},
   {773.0028f, 1279.52f, 282.4164f, 0.0f},
   {764.3626f, 1274.476f, 282.4731f, 0.0f},
   {754.3961f, 1272.639f, 282.4171f, 0.0f},
   {744.4422f, 1274.412f, 282.222f, 0.0f},
   {735.575f, 1279.742f, 281.9674f, 0.0f},
   {729.2788f, 1287.187f, 281.9943f, 0.0f},
   {726.1191f, 1296.688f, 282.2997f, 0.0f},
   {725.9396f, 1306.531f, 282.2448f, 0.0f},
   {729.3045f, 1316.122f, 281.9108f, 0.0f},
   {735.8322f, 1323.633f, 282.1887f, 0.0f},
   {744.4616f, 1328.999f, 281.9948f, 0.0f},
   {754.4739f, 1330.666f, 282.049f, 0.0f},
   {764.074f, 1329.053f, 281.9949f, 0.0f},
   {772.8409f, 1323.951f, 282.077f, 0.0f},
   {779.5085f, 1316.412f, 281.9145f, 0.0f},
   {782.8365f, 1306.778f, 282.3035f, 0.0f},
};

#define GROUND_Z 268

// Source: Sniffs (x,y,z)
#define MALYGOS_MAX_WAYPOINTS 16
const Position MalygosPhaseTwoWaypoints[MALYGOS_MAX_WAYPOINTS] =
{
    {812.7299f, 1391.672f, 283.2763f, 0.0f},
    {848.2912f, 1358.61f, 283.2763f, 0.0f},
    {853.9227f, 1307.911f, 283.2763f, 0.0f},
    {847.1437f, 1265.538f, 283.2763f, 0.0f},
    {839.9229f, 1245.245f, 283.2763f, 0.0f},
    {827.3463f, 1221.818f, 283.2763f, 0.0f},
    {803.2727f, 1203.851f, 283.2763f, 0.0f},
    {772.9372f, 1197.981f, 283.2763f, 0.0f},
    {732.1138f, 1200.647f, 283.2763f, 0.0f},
    {693.8761f, 1217.995f, 283.2763f, 0.0f},
    {664.5038f, 1256.539f, 283.2763f, 0.0f},
    {650.1497f, 1303.485f, 283.2763f, 0.0f},
    {662.9109f, 1350.291f, 283.2763f, 0.0f},
    {677.6391f, 1377.607f, 283.2763f, 0.0f},
    {704.8198f, 1401.162f, 283.2763f, 0.0f},
    {755.2642f, 1417.1f, 283.2763f, 0.0f},
};

#define MAX_SUMMONS_PHASE_TWO 4

#define MAX_MALYGOS_POS 2
const Position MalygosPositions[MAX_MALYGOS_POS] =
{
    {754.544f, 1301.71f, 320.0f, 0.0f},
    {754.39f, 1301.27f, 292.91f, 0.0f},
};

class boss_malygos : public CreatureScript
{
public:
    boss_malygos() : CreatureScript("boss_malygos") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_malygosAI(creature);
    }

    struct boss_malygosAI : public BossAI
    {
        boss_malygosAI(Creature* creature) : BossAI(creature, DATA_MALYGOS_EVENT)
        {
            // If we enter in combat when MovePoint generator is active, it overrwrites our homeposition
            _homePosition = creature->GetHomePosition();
        }

        void Reset()
        {
            _Reset();

            _bersekerTimer = 0;
            _currentPos = 0;

            SetPhase(PHASE_ONE, true);

            _delayedMovementTimer = 8000;
            _delayedMovement = false;

            _summonDeaths = 0;

            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            _cannotMove = true;

            if (instance)
                instance->DoStopTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, ACHIEV_TIMED_START_EVENT);
        }

        uint32 GetData(uint32 data)
        {
            if (data == DATA_SUMMON_DEATHS)
                return _summonDeaths;
            else if (data == DATA_PHASE)
                return _phase;

            return 0;
        }

        void SetData(uint32 data, uint32 value)
        {
            if (data == DATA_SUMMON_DEATHS && _phase == PHASE_TWO)
            {
                _summonDeaths = value;

                if (_summonDeaths >= MAX_SUMMONS_PHASE_TWO)
                    StartPhaseThree();
            }
        }

        void EnterEvadeMode()
        {
            me->SetHomePosition(_homePosition);

            me->AddUnitMovementFlag(MOVEMENTFLAG_LEVITATING);
            HandleRedDrakes(false);

            BossAI::EnterEvadeMode();

            if (instance)
                instance->SetBossState(DATA_MALYGOS_EVENT, FAIL);
        }

        void SetPhase(uint8 phase, bool setEvents = false)
        {
            events.Reset();

            events.SetPhase(phase);
            _phase = phase;

            if (setEvents)
                SetPhaseEvents();
        }

        void StartPhaseThree()
        {
            if (!instance)
                return;

            SetPhase(PHASE_THREE, true);

            // this despawns Hover Disks
            summons.DespawnAll();
            // players that used Hover Disk are no in the aggro list
            me->SetInCombatWithZone();
            HandleRedDrakes(true);

            if (GameObject* go = GameObject::GetGameObject(*me, instance->GetData64(DATA_PLATFORM)))
                go->SetFlag(GAMEOBJECT_FLAGS, GO_FLAG_DESTROYED); // In sniffs it has this flag, but i don't know how is applied.

            // pos sniffed
            me->GetMotionMaster()->MoveIdle();
            me->GetMotionMaster()->MovePoint(MOVE_CENTER_PLATFORM, MalygosPositions[0].GetPositionX(), MalygosPositions[0].GetPositionY(), MalygosPositions[0].GetPositionZ());
        }

        void SetPhaseEvents()
        {
            switch (_phase)
            {
                case PHASE_ONE:
                    events.ScheduleEvent(EVENT_ARCANE_BREATH, urand(15, 20)*IN_MILLISECONDS, 0, _phase);
                    events.ScheduleEvent(EVENT_ARCANE_STORM, urand(5, 10)*IN_MILLISECONDS, 0, _phase);
                    events.ScheduleEvent(EVENT_VORTEX, urand(30, 40)*IN_MILLISECONDS, 0, _phase);
                    events.ScheduleEvent(EVENT_POWER_SPARKS, urand(30, 35)*IN_MILLISECONDS, 0, _phase);
                    break;
                case PHASE_TWO:
                    events.ScheduleEvent(EVENT_YELL_0, 0, 0, _phase);
                    events.ScheduleEvent(EVENT_YELL_1, 24*IN_MILLISECONDS, 0, _phase);
                    events.ScheduleEvent(EVENT_SURGE_POWER, urand(60, 70)*IN_MILLISECONDS, 0, _phase);
                    events.ScheduleEvent(EVENT_SUMMON_ARCANE, urand(2, 5)*IN_MILLISECONDS, 0, _phase);
                    break;
                case PHASE_THREE:
                    events.ScheduleEvent(EVENT_YELL_2, 0, 0, _phase);
                    events.ScheduleEvent(EVENT_YELL_3, 8*IN_MILLISECONDS, 0, _phase);
                    events.ScheduleEvent(EVENT_YELL_4, 16*IN_MILLISECONDS, 0, _phase);
                    events.ScheduleEvent(EVENT_SURGE_POWER_PHASE_3, urand(7, 16)*IN_MILLISECONDS, 0, _phase);
                    events.ScheduleEvent(EVENT_STATIC_FIELD, urand(20, 30)*IN_MILLISECONDS, 0, _phase);
                    break;
                default:
                    break;
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            _EnterCombat();

            me->RemoveUnitMovementFlag(MOVEMENTFLAG_LEVITATING);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            Talk(SAY_AGGRO_P_ONE);

            if (instance)
                instance->DoStartTimedAchievement(ACHIEVEMENT_TIMED_TYPE_EVENT, ACHIEV_TIMED_START_EVENT);

            DoCast(SPELL_BERSERKER);
        }

        void KilledUnit(Unit* who)
        {
            if (who->GetTypeId() != TYPEID_PLAYER)
                return;

            switch (_phase)
            {
                case PHASE_ONE:
                    Talk(SAY_KILLED_PLAYER_P_ONE);
                    break;
                case PHASE_TWO:
                    Talk(SAY_KILLED_PLAYER_P_TWO);
                    break;
                case PHASE_THREE:
                    Talk(SAY_KILLED_PLAYER_P_THREE);
                    break;
            }
        }

        void SpellHit(Unit* caster, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_POWER_SPARK_MALYGOS)
            {
                if (Creature* creature = caster->ToCreature())
                    creature->DespawnOrUnsummon();

                Talk(SAY_BUFF_SPARK);
            }
        }

        void MoveInLineOfSight(Unit* who)
        {
            if (!me->isInCombat())
                return;

            if (who->GetEntry() == NPC_POWER_SPARK)
            {
                // not sure about the distance | I think it is better check this here than in the UpdateAI function...
                if (who->GetDistance(me) <= 2.5f)
                    who->CastSpell(me, SPELL_POWER_SPARK_MALYGOS, true);
            }
        }

        void PrepareForVortex()
        {
            me->AddUnitMovementFlag(MOVEMENTFLAG_LEVITATING);

            me->GetMotionMaster()->MovementExpired();
            me->GetMotionMaster()->MovePoint(MOVE_VORTEX, MalygosPositions[1].GetPositionX(), MalygosPositions[1].GetPositionY(), MalygosPositions[1].GetPositionZ());
            // continues in MovementInform function.
        }

        void ExecuteVortex()
        {
            DoCast(me, SPELL_VORTEX_1, true);
            DoCast(me, SPELL_VORTEX_2, true);

            // the vortex execution continues in the dummy effect of this spell (see its script)
            DoCast(me, SPELL_VORTEX_3, true);
        }

        void MovementInform(uint32 type, uint32 id)
        {
            if (type != POINT_MOTION_TYPE)
                return;

            switch (id)
            {
                case MOVE_VORTEX:
                    me->GetMotionMaster()->MoveIdle();
                    ExecuteVortex();
                    break;
                case MOVE_DEEP_BREATH_ROTATION:
                    _currentPos = _currentPos == MALYGOS_MAX_WAYPOINTS - 1 ? 0 : _currentPos+1;
                    me->GetMotionMaster()->MovementExpired();
                    me->GetMotionMaster()->MovePoint(MOVE_DEEP_BREATH_ROTATION, MalygosPhaseTwoWaypoints[_currentPos]);
                    break;
                case MOVE_INIT_PHASE_ONE:
                    if (GameObject* iris = instance->instance->GetGameObject(instance->GetData64(DATA_IRIS)))
                        iris->SetPhaseMask(0x1000, true);   // random value, just be invisible
                    me->SetInCombatWithZone();
                    break;
                case MOVE_CENTER_PLATFORM:
                    _cannotMove = false;
                    // malygos will move into center of platform and then he does not chase dragons, he just turns to his current target.
                    me->GetMotionMaster()->MoveIdle();
                    break;
            }
        }

        void StartPhaseTwo()
        {
            SetPhase(PHASE_TWO, true);

            me->AddUnitMovementFlag(MOVEMENTFLAG_LEVITATING);

            me->GetMotionMaster()->MoveIdle();
            me->GetMotionMaster()->MovePoint(MOVE_DEEP_BREATH_ROTATION, MalygosPhaseTwoWaypoints[0]);

            Creature* summon = me->SummonCreature(NPC_HOVER_DISK_CASTER, HoverDiskWaypoints[MAX_HOVER_DISK_WAYPOINTS-1]);
            if (summon && summon->IsAIEnabled)
                summon->AI()->DoAction(ACTION_HOVER_DISK_START_WP_2);
            summon = me->SummonCreature(NPC_HOVER_DISK_CASTER, HoverDiskWaypoints[0]);
            if (summon && summon->IsAIEnabled)
                summon->AI()->DoAction(ACTION_HOVER_DISK_START_WP_1);

            for (uint8 i = 0; i < 2; i++)
            {
                // not sure about its position.
                summon = me->SummonCreature(NPC_HOVER_DISK_MELEE, HoverDiskWaypoints[0]);
                if (summon)
                    summon->SetInCombatWithZone();
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            events.Update(diff);

            if (_phase == PHASE_THREE)
            {
                if (!_cannotMove)
                {
                    // it can change if the player falls from the vehicle.
                    if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != IDLE_MOTION_TYPE)
                    {
                        me->GetMotionMaster()->MovementExpired();
                        me->GetMotionMaster()->MoveIdle();
                    }
                } else
                {
                    if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE)
                    {
                        me->GetMotionMaster()->MovementExpired();
                        me->GetMotionMaster()->MovePoint(MOVE_CENTER_PLATFORM, MalygosPositions[0].GetPositionX(), MalygosPositions[0].GetPositionY(), MalygosPositions[0].GetPositionZ());
                    }
                }
            }

            // we need a better way for pathing
            if (_delayedMovement)
            {
                if (_delayedMovementTimer <= diff)
                {
                    me->GetMotionMaster()->MovePoint(MOVE_DEEP_BREATH_ROTATION, MalygosPhaseTwoWaypoints[_currentPos]);
                    _delayedMovementTimer = 8000;
                    _delayedMovement = false;
                } _delayedMovementTimer -= diff;
            }

            // at 50 % health malygos switch to phase 2
            if (me->GetHealthPct() <= 50.0f && _phase == PHASE_ONE)
                StartPhaseTwo();

            // the boss is handling vortex
            if (me->HasAura(SPELL_VORTEX_2))
                return;

            // We can't cast if we are casting already.
            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            // must be declared outside of loop
            UnitList fieldTargets;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_YELL_2:
                        Talk(SAY_END_P_TWO);
                        break;
                    case EVENT_YELL_3:
                        Talk(SAY_INTRO_P_THREE);
                        break;
                    case EVENT_YELL_4:
                        Talk(SAY_AGGRO_P_THREE);
                        break;
                    case EVENT_YELL_0:
                        Talk(SAY_END_P_ONE);
                        break;
                    case EVENT_YELL_1:
                        Talk(SAY_AGGRO_P_TWO);
                        break;
                    case EVENT_ARCANE_BREATH:
                        DoCast(me->getVictim(), SPELL_ARCANE_BREATH_10);
                        events.ScheduleEvent(EVENT_ARCANE_BREATH, urand(35, 60)*IN_MILLISECONDS, 0, PHASE_ONE);
                        break;
                    case EVENT_ARCANE_STORM:
                        DoCast(me->getVictim(), SPELL_ARCANE_STORM_10);
                        events.ScheduleEvent(EVENT_ARCANE_STORM, urand(5, 10)*IN_MILLISECONDS, 0, PHASE_ONE);
                        break;
                    case EVENT_VORTEX:
                        PrepareForVortex();
                        events.ScheduleEvent(EVENT_VORTEX, urand(60, 80)*IN_MILLISECONDS, 0, PHASE_ONE);
                        break;
                    case EVENT_POWER_SPARKS:
                        instance->SetData(DATA_POWER_SPARKS_HANDLING, 0);
                        events.ScheduleEvent(EVENT_POWER_SPARKS, urand(30, 35)*IN_MILLISECONDS, 0, PHASE_ONE);
                        break;
                    case EVENT_SURGE_POWER:
                        me->GetMotionMaster()->MoveIdle();
                        _delayedMovement = true;
                        DoCast(SPELL_SURGE_POWER_P2);
                        events.ScheduleEvent(EVENT_SURGE_POWER, urand(60, 70)*IN_MILLISECONDS, 0, PHASE_TWO);
                        break;
                    case EVENT_SUMMON_ARCANE:
                        DoCast(SPELL_ARCANE_BOMB_SUMMON);
                        events.ScheduleEvent(EVENT_SUMMON_ARCANE, urand(12, 15)*IN_MILLISECONDS, 0, PHASE_TWO);
                        break;
                    case EVENT_SURGE_POWER_PHASE_3:
                        DoCast(SPELL_SURGE_POWER_P3);       // target selection and Boss-Warning in SpellScript
                        events.ScheduleEvent(EVENT_SURGE_POWER_PHASE_3, urand(7, 16)*IN_MILLISECONDS, 0, PHASE_THREE);
                        break;
                    case EVENT_STATIC_FIELD:
                        fieldTargets = SelectRandomTargets(RAID_MODE(1, 3), true);
                        for (UnitList::iterator itr = fieldTargets.begin(); itr != fieldTargets.end(); ++itr)
                            DoCast((*itr), SPELL_STATIC_FIELD, true);
                        events.ScheduleEvent(EVENT_STATIC_FIELD, urand(20, 30)*IN_MILLISECONDS, 0, PHASE_THREE);
                        break;
                    default:
                        break;
                }
            }

            DoMeleeAttackIfReady();
        }

        UnitList SelectRandomTargets(uint8 maxCount, bool targetVehicle = false)
        {
            UnitList temp;
            UnitList result;

            std::list<HostileReference*> &UnitList = me->getThreatManager().getThreatList();
            if (UnitList.empty())
                return result;

            for (std::list<HostileReference*>::iterator itr = UnitList.begin(); itr != UnitList.end(); ++itr)
            {
                if (Unit* target = (*itr)->getTarget())
                {
                    if (target->GetTypeId() == TYPEID_UNIT && !target->GetVehicleKit())
                        continue;

                    if (targetVehicle)
                    {
                        if (Unit* base = target->GetVehicleBase())
                            temp.push_back(base);
                        else if (target->IsVehicle())
                            temp.push_back(target);
                    }
                    else
                    {
                        if (!target->IsVehicle())
                            temp.push_back(target);
                        else if (Vehicle* kit = target->GetVehicleKit())
                            if (Unit* rider = kit->GetPassenger(SEAT_0))
                                temp.push_back(rider);

                    }
                }
            }

            UnitList::iterator j;
            for (uint8 i = 0; i < maxCount; i++)
            {
                if (temp.empty())
                    break;

                j = temp.begin();
                advance(j, rand()%temp.size());
                result.push_back(*j);
                temp.erase(j);
            }
            return result;
        }

        void HandleRedDrakes(bool apply)
        {
            Map::PlayerList const &PlayerList = instance->instance->GetPlayers();
            if (!PlayerList.isEmpty())
            {
                for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
                {
                    if (Player* player = i->getSource())
                    {
                        if (player->isAlive())
                        {
                            if (apply)
                                player->CastSpell(player, SPELL_SUMMOM_RED_DRAGON, true);
                            else
                                player->ExitVehicle();
                        }
                    }
                }
            }
         }

        void JustDied(Unit* /*killer*/)
        {
            Talk(SAY_DEATH);
            _JustDied();

            Map::PlayerList const &PlayerList = instance->instance->GetPlayers();
            if (PlayerList.isEmpty())
                return;

            for (Map::PlayerList::const_iterator i = PlayerList.begin(); i != PlayerList.end(); ++i)
            {
                if (Player* player = i->getSource())
                {
                    if (player->GetQuestStatus(RAID_MODE(QUEST_JUDGEMENT_10, QUEST_JUDGEMENT_25)) == QUEST_STATUS_INCOMPLETE)
                    {
                        GameObject* go = new GameObject;
                        if (go->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_GAMEOBJECT), RAID_MODE(GO_HEART_OF_MAGIC_10, GO_HEART_OF_MAGIC_25), instance->instance, PHASEMASK_NORMAL, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), 0, 0, 0, 0, 120, GO_STATE_READY))
                        {
                            go->SetRespawnTime(1*WEEK);
                            instance->instance->Add(go);
                        }
                        else
                            delete go;
                        break;
                    }
                }
            }
        }

    private:
        uint8 _phase;
        uint32 _bersekerTimer;
        uint8 _currentPos; // used for phase 2 rotation...
        bool _delayedMovement; // used in phase 2.
        uint32 _delayedMovementTimer; // used in phase 2
        uint8 _summonDeaths;
        Position _homePosition; // it can get bugged because core thinks we are pathing
        bool _mustTalk;
        bool _cannotMove;
    };
};

class spell_malygos_vortex_dummy : public SpellScriptLoader
{
public:
    spell_malygos_vortex_dummy() : SpellScriptLoader("spell_malygos_vortex_dummy") {}

    class spell_malygos_vortex_dummy_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_malygos_vortex_dummy_SpellScript)

        void HandleScript(SpellEffIndex /*effIndex*/)
        {
            Unit* caster = GetCaster();

            if (!caster)
                return;

            // each player will enter to the trigger vehicle (entry 30090) already spawned (each one can hold up to 5 players, it has 5 seats)
            // the players enter to the vehicles casting SPELL_VORTEX_4 OR SPELL_VORTEX_5
            if (InstanceScript* instance = caster->GetInstanceScript())
                instance->SetData(DATA_VORTEX_HANDLING, 0);

            // the rest of the vortex execution continues when SPELL_VORTEX_2 is removed.
        }

        void Register()
        {
            OnEffectHitTarget += SpellEffectFn(spell_malygos_vortex_dummy_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_malygos_vortex_dummy_SpellScript();
    }
};

class spell_malygos_vortex_visual : public SpellScriptLoader
{
    public:
        spell_malygos_vortex_visual() : SpellScriptLoader("spell_malygos_vortex_visual") { }

        class spell_malygos_vortex_visual_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_malygos_vortex_visual_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* caster = GetCaster())
                {
                    std::list<HostileReference*> &m_threatlist = caster->getThreatManager().getThreatList();
                    for (std::list<HostileReference*>::const_iterator itr = m_threatlist.begin(); itr!= m_threatlist.end(); ++itr)
                    {
                        if (Unit* target = (*itr)->getTarget())
                        {
                            Player* targetPlayer = target->ToPlayer();

                            if (!targetPlayer || targetPlayer->isGameMaster())
                                continue;

                            if (InstanceScript* instance = caster->GetInstanceScript())
                            {
                                // teleport spell - i am not sure but might be it must be casted by each vehicle when its passenger leaves it
                                if (Creature* trigger = caster->GetMap()->GetCreature(instance->GetData64(DATA_VORTEX)))
                                    trigger->CastSpell(targetPlayer, SPELL_VORTEX_6, true);
                            }
                        }
                    }

                    if (Creature* malygos = caster->ToCreature())
                    {
                        // This is a hack, we have to re add players to the threat list because when they enter to the vehicles they are removed.
                        // Anyway even with this issue, the boss does not enter in evade mode - this prevents iterate an empty list in the next vortex execution.
                        malygos->SetInCombatWithZone();

                        malygos->RemoveUnitMovementFlag(MOVEMENTFLAG_LEVITATING);

                        malygos->GetMotionMaster()->MoveChase(caster->getVictim());
                        malygos->RemoveAura(SPELL_VORTEX_1);
                    }
                }

            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_malygos_vortex_visual_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
        {
            return new spell_malygos_vortex_visual_AuraScript();
        }
};

class spell_malygos_surge_of_power_p3 : public SpellScriptLoader
{
public:
    spell_malygos_surge_of_power_p3() : SpellScriptLoader("spell_malygos_surge_of_power_p3") {}

    class spell_malygos_surge_of_power_p3_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_malygos_surge_of_power_p3_SpellScript)

        void FilterTargets(std::list<Unit*>& unitList)
        {
            Unit* caster = GetCaster();
            if (!caster || !caster->IsAIEnabled || caster->ToCreature()->GetEntry() != NPC_MALYGOS)
                return;

            uint8 numTargets = caster->GetMap()->GetDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL ? 3 : 1;
            unitList = CAST_AI(boss_malygos::boss_malygosAI, (caster->ToCreature()->AI()))->SelectRandomTargets(numTargets, true);
            /* uncomment after reordering the text
            for (UnitList::iterator itr = unitList.begin(); itr != unitList.end(); ++itr)
                caster->ToCreature()->AI()->Talk(EMOTE_SURGE_OF_POWER, (*itr)->GetVehicleKit()->GetPassenger(SEAT_0)->GetGUID());
            */
        }

        void Register()
        {
            OnUnitTargetSelect += SpellUnitTargetFn(spell_malygos_surge_of_power_p3_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_malygos_surge_of_power_p3_SpellScript();
    }
};

class spell_malygos_arcane_storm : public SpellScriptLoader
{
public:
    spell_malygos_arcane_storm() : SpellScriptLoader("spell_malygos_arcane_storm") {}

    class spell_malygos_arcane_storm_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_malygos_arcane_storm_SpellScript)

        void FilterTargets(std::list<Unit*>& unitList)
        {
            Unit* caster = GetCaster();
            if (!caster || !caster->IsAIEnabled || caster->ToCreature()->GetEntry() != NPC_MALYGOS)
                return;

            uint8 numTargets = caster->GetMap()->GetDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL ? urand(7, 9) : urand(1, 3);
            unitList = CAST_AI(boss_malygos::boss_malygosAI, (caster->ToCreature()->AI()))->SelectRandomTargets(numTargets);
        }

        void Register()
        {
            OnUnitTargetSelect += SpellUnitTargetFn(spell_malygos_arcane_storm_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript *GetSpellScript() const
    {
        return new spell_malygos_arcane_storm_SpellScript();
    }
};

class spell_malygos_arcane_barrage : public SpellScriptLoader
{
public:
    spell_malygos_arcane_barrage() : SpellScriptLoader("spell_malygos_arcane_barrage") {}

    class spell_malygos_arcane_barrage_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_malygos_arcane_barrage_SpellScript)

        bool heroic;

        bool Load()
        {
            heroic = false;
            if (Unit* caster = GetCaster())
                heroic = caster->GetMap()->GetDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL;

            return true;
        }

        void FilterTargets(std::list<Unit*>& unitList)
        {
            if (GetSpellInfo()->Id != SPELL_ARCANE_BARRAGE_TRIG)
                return;

            if (Unit* caster = GetCaster())
                if (InstanceScript* iScript = caster->GetInstanceScript())
                    if (Creature* malygos = Unit::GetCreature(*caster, iScript->GetData64(DATA_MALYGOS)))
                        unitList = CAST_AI(boss_malygos::boss_malygosAI, (malygos->AI()))->SelectRandomTargets(heroic ? 3 : 1);
        }

        void DoHeroicDamage()
        {
            if (heroic && GetSpellInfo()->Id != SPELL_ARCANE_BARRAGE_TRIG)
                SetHitDamage(int32(GetHitDamage() * 1.2f));
        }

        void Register()
        {
            OnHit += SpellHitFn(spell_malygos_arcane_barrage_SpellScript::DoHeroicDamage);
            OnUnitTargetSelect += SpellUnitTargetFn(spell_malygos_arcane_barrage_SpellScript::FilterTargets, EFFECT_0, TARGET_SRC_CASTER);
        }
     };

    SpellScript *GetSpellScript() const
    {
        return new spell_malygos_arcane_barrage_SpellScript();
    }
};

class spell_malygos_arcane_overload : public SpellScriptLoader
{
public:
    spell_malygos_arcane_overload() : SpellScriptLoader("spell_malygos_arcane_overload") {}

    class spell_malygos_arcane_overload_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_malygos_arcane_overload_SpellScript)

        void FilterTargets(std::list<Unit*>& targetList)
        {
            uint8 stack = 50;
            Unit* caster = GetCaster();

            if (!caster)
                return;

            if (Aura* aur = caster->GetAura(SPELL_ARCANE_OVERLOAD_SIZE))
                stack -= aur->GetStackAmount();

            for (std::list<Unit*>::iterator itr = targetList.begin(); itr != targetList.end();)
            {
                if (caster->GetExactDist((*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ()) > ((stack / 50.0f) * 18.0f))
                    itr = targetList.erase(itr);
                else
                    itr++;
            }
        }

        void Register()
        {
            OnUnitTargetSelect += SpellUnitTargetFn(spell_malygos_arcane_overload_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_malygos_arcane_overload_SpellScript();
    }
};

class spell_malygos_surge_of_power : public SpellScriptLoader
{
public:
    spell_malygos_surge_of_power() : SpellScriptLoader("spell_malygos_surge_of_power") {}

    class spell_malygos_surge_of_power_SpellScript : public SpellScript
    {
        PrepareSpellScript(spell_malygos_surge_of_power_SpellScript)

        void FilterTargets(std::list<Unit*>& targetList)
        {
            for (std::list<Unit*>::iterator itr = targetList.begin(); itr != targetList.end();)
            {
                if ((*itr)->ToCreature() || (*itr)->ToPlayer()->GetVehicle())
                    itr = targetList.erase(itr);
                else
                    itr++;
            }
        }

        void Register()
        {
            OnUnitTargetSelect += SpellUnitTargetFn(spell_malygos_surge_of_power_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
        }
    };

    SpellScript* GetSpellScript() const
    {
        return new spell_malygos_surge_of_power_SpellScript();
    }
};

class npc_portal_eoe: public CreatureScript
{
public:
    npc_portal_eoe() : CreatureScript("npc_portal_eoe") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_portal_eoeAI(creature);
    }

    struct npc_portal_eoeAI : public ScriptedAI
    {
        npc_portal_eoeAI(Creature* creature) : ScriptedAI(creature)
        {
            _instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            _summonTimer = urand(5, 7)*IN_MILLISECONDS;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!me->HasAura(SPELL_PORTAL_VISUAL_CLOSED) &&
                !me->HasAura(SPELL_PORTAL_OPENED))
                DoCast(me, SPELL_PORTAL_VISUAL_CLOSED, true);

            if (_instance)
            {
                if (Creature* malygos = Unit::GetCreature(*me, _instance->GetData64(DATA_MALYGOS)))
                {
                    if (malygos->AI()->GetData(DATA_PHASE) != PHASE_ONE)
                    {
                        me->RemoveAura(SPELL_PORTAL_OPENED);
                        DoCast(me, SPELL_PORTAL_VISUAL_CLOSED, true);
                    }
                }
            }

            if (!me->HasAura(SPELL_PORTAL_OPENED))
                return;

            if (_summonTimer <= diff)
            {
                DoCast(SPELL_SUMMON_POWER_PARK);
                _summonTimer = urand(5, 7)*IN_MILLISECONDS;
            } else
                _summonTimer -= diff;
        }

        void JustSummoned(Creature* summon)
        {
            summon->SetInCombatWithZone();
        }

    private:
        uint32 _summonTimer;
        InstanceScript* _instance;
    };
};


class npc_power_spark: public CreatureScript
{
public:
    npc_power_spark() : CreatureScript("npc_power_spark") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_power_sparkAI(creature);
    }

    struct npc_power_sparkAI : public ScriptedAI
    {
        npc_power_sparkAI(Creature* creature) : ScriptedAI(creature)
        {
            _instance = creature->GetInstanceScript();

            MoveToMalygos();
        }

        void EnterEvadeMode()
        {
            me->DespawnOrUnsummon();
        }

        void MoveToMalygos()
        {
            me->GetMotionMaster()->MoveIdle();

            if (_instance)
            {
                if (Creature* malygos = Unit::GetCreature(*me, _instance->GetData64(DATA_MALYGOS)))
                    me->GetMotionMaster()->MoveFollow(malygos, 0.0f, 0.0f);
            }
        }

        void UpdateAI(uint32 const /*diff*/)
        {
            if (!_instance)
                return;

            if (Creature* malygos = Unit::GetCreature(*me, _instance->GetData64(DATA_MALYGOS)))
            {
                if (malygos->AI()->GetData(DATA_PHASE) != PHASE_ONE)
                {
                    me->DespawnOrUnsummon();
                    return;
                }

                if (malygos->HasAura(SPELL_VORTEX_1))
                {
                    me->GetMotionMaster()->MoveIdle();
                    return;
                }

                if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != TARGETED_MOTION_TYPE)
                    me->GetMotionMaster()->MoveFollow(malygos, 0.0f, 0.0f);
            }
        }

        void DamageTaken(Unit* /*done_by*/, uint32& damage)
        {
            if (damage > me->GetMaxHealth())
            {
                damage = 0;
                DoCast(me, SPELL_POWER_SPARK_DEATH, true);
                me->DespawnOrUnsummon(1000);
            }
        }

    private:
        InstanceScript* _instance;
    };
};

class npc_hover_disk : public CreatureScript
{
public:
    npc_hover_disk() : CreatureScript("npc_hover_disk") { }

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_hover_diskAI(creature);
    }

    struct npc_hover_diskAI : public npc_escortAI
    {
        npc_hover_diskAI(Creature* creature) : npc_escortAI(creature)
        {
            if (me->GetEntry() == NPC_HOVER_DISK_CASTER)
                me->SetReactState(REACT_PASSIVE);
             else
                me->SetInCombatWithZone();

            _instance = creature->GetInstanceScript();
        }

        void PassengerBoarded(Unit* unit, int8 /*seat*/, bool apply)
        {
            if (apply)
            {
                if (unit->GetTypeId() == TYPEID_UNIT)
                {
                    me->setFaction(FACTION_HOSTILE);
                    unit->ToCreature()->SetInCombatWithZone();
                }
            }
            else
            {
                // Error found: This is not called if the passenger is a player

                if (unit->GetTypeId() == TYPEID_UNIT)
                {
                    // This will only be called if the passenger dies
                    if (_instance)
                    {
                        if (Creature* malygos = Unit::GetCreature(*me, _instance->GetData64(DATA_MALYGOS)))
                            malygos->AI()->SetData(DATA_SUMMON_DEATHS, malygos->AI()->GetData(DATA_SUMMON_DEATHS)+1);
                    }

                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                }

                me->GetMotionMaster()->MoveIdle();

                if (me->GetEntry() == NPC_HOVER_DISK_MELEE || me->GetEntry() == NPC_HOVER_DISK_CASTER)
                {
                    // Hack: Fall ground function can fail (remember the platform is a gameobject), we will teleport the disk to the ground
                    if (me->GetPositionZ() > GROUND_Z)
                        me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), GROUND_Z, 0);
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    me->setFaction(FACTION_FRIENDLY);
                    me->AI()->EnterEvadeMode();
                }
            }
        }

        void EnterEvadeMode()
        {
            // we dont evade
        }

        void DoAction(int32 const action)
        {
            if (me->GetEntry() != NPC_HOVER_DISK_CASTER)
                return;

            switch (action)
            {
                case ACTION_HOVER_DISK_START_WP_1:
                    for (uint8 i = 0; i < MAX_HOVER_DISK_WAYPOINTS; i++)
                        AddWaypoint(i, HoverDiskWaypoints[i].GetPositionX(), HoverDiskWaypoints[i].GetPositionY(), HoverDiskWaypoints[i].GetPositionZ());
                    break;
                case ACTION_HOVER_DISK_START_WP_2:
                    {
                        uint8 count = 0;
                        for (uint8 i = MAX_HOVER_DISK_WAYPOINTS-1; i > 0; i--)
                        {
                            AddWaypoint(count, HoverDiskWaypoints[i].GetPositionX(), HoverDiskWaypoints[i].GetPositionY(), HoverDiskWaypoints[i].GetPositionZ());
                            count++;
                        }
                        break;
                    }
                default:
                    return;
            }

            Start(true, false, 0, 0, false, true);
        }

        void UpdateEscortAI(const uint32 /*diff*/)
        {
            // we dont do melee damage!
        }

        void WaypointReached(uint32 /*i*/)
        {
        }

    private:
        InstanceScript* _instance;
    };
};


// The reason of this AI is to make the creature able to enter in combat otherwise the spell casting of SPELL_ARCANE_OVERLOAD fails.
class npc_arcane_overload : public CreatureScript
{
public:
    npc_arcane_overload() : CreatureScript("npc_arcane_overload") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_arcane_overloadAI (creature);
    }

    struct npc_arcane_overloadAI : public ScriptedAI
    {
        npc_arcane_overloadAI(Creature* creature) : ScriptedAI(creature) {}

        void AttackStart(Unit * who, float /*dist*/ = 0)
        {
            DoStartNoMovement(who);
        }

        void Reset()
        {
            _delay = 1000;                                  // cast animation is not played if the model still 'animates in'
            me->DespawnOrUnsummon(50*IN_MILLISECONDS);
        }

        void UpdateAI(uint32 const diff)
        {
            if (_delay <= diff && _delay > 0)
            {
                _delay = 0;
                if (InstanceScript* instance = me->GetInstanceScript())
                    if (Creature* malygos = Unit::GetCreature(*me, instance->GetData64(DATA_MALYGOS)))
                        malygos->CastSpell(me, SPELL_ARCANE_BOMB_MISSILE, true);
            }
            else
                _delay -=diff;

            // we dont do melee damage!
        }

        void SpellHit(Unit* /*caster*/, SpellEntry const* spell)
        {
            if (spell->Id == SPELL_ARCANE_BOMB_MISSILE)
            {
                me->CastSpell(me, SPELL_ARCANE_BOMB_EFFECT, true);
                me->CastSpell(me, SPELL_ARCANE_OVERLOAD, true);
            }
        }

    private:
        uint16 _delay;
    };
};

// SmartAI does not work correctly for this (vehicles)
class npc_wyrmrest_skytalon : public CreatureScript
{
public:
    npc_wyrmrest_skytalon() : CreatureScript("npc_wyrmrest_skytalon") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_wyrmrest_skytalonAI (creature);
    }

    struct npc_wyrmrest_skytalonAI : public NullCreatureAI
    {
        npc_wyrmrest_skytalonAI(Creature* creature) : NullCreatureAI(creature)
        {
            _instance = creature->GetInstanceScript();

            _timer = 1000;
            _entered = false;
        }

        void IsSummonedBy(Unit* summoner)
        {
            if (InstanceScript* script = me->GetInstanceScript())
                if (script->GetBossState(0) == IN_PROGRESS) // DATA_MALYGOS_EVENT: 0
                    me->SetPosition(summoner->GetPositionX(), summoner->GetPositionY(), summoner->GetPositionZ()-40.0f, summoner->GetOrientation());
        }

        void PassengerBoarded(Unit* /*unit*/, int8 /*seat*/, bool apply)
        {
            if (!apply)
                me->DespawnOrUnsummon();
        }

        // we can't call this in reset function, it fails.
        void MakePlayerEnter()
        {
            if (!_instance)
                return;

            if (Unit* summoner = me->ToTempSummon()->GetSummoner())
            {
                if (Creature* malygos = Unit::GetCreature(*me, _instance->GetData64(DATA_MALYGOS)))
                {
                    summoner->CastSpell(me, SPELL_RIDE_RED_DRAGON, true);
                    float victimThreat = malygos->getThreatManager().getThreat(summoner);
                    malygos->AI()->AttackStart(me);
                    malygos->AddThreat(me, victimThreat);
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!_entered)
            {
                if (_timer <= diff)
                {
                    MakePlayerEnter();
                    _entered = true;
                } else
                    _timer -= diff;
            }
        }

    private:
        InstanceScript* _instance;
        uint32 _timer;
        bool _entered;
    };
};

enum AlexstraszaYells
{
    SAY_ONE,
    SAY_TWO,
    SAY_THREE,
    SAY_FOUR
};

enum AlexstraszaEvents
{
    EVENT_MOVE_IN     = 1,
    EVENT_CREATE_GIFT = 2,
    EVENT_SAY_ONE     = 3,
    EVENT_SAY_TWO     = 4,
    EVENT_SAY_THREE   = 5,
    EVENT_SAY_FOUR    = 6
};

class npc_alexstrasza_eoe : public CreatureScript
{
public:
    npc_alexstrasza_eoe() : CreatureScript("npc_alexstrasza_eoe") {}

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_alexstrasza_eoeAI (creature);
    }

    struct npc_alexstrasza_eoeAI : public ScriptedAI
    {
        npc_alexstrasza_eoeAI(Creature* creature) : ScriptedAI(creature) {}

        void Reset()
        {
            me->SetFlying(true);
            me->SetSpeed(MOVE_FLIGHT, 2.0f);
            _events.Reset();
            _events.ScheduleEvent(EVENT_MOVE_IN, 0);
            _gift = me->FindNearestCreature(NPC_ALEXSTRASZA_S_GIFT, 200.0f);

            // stop gift from visually hopping around or falling to void
            _gift->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
            _gift->SetFlying(true);
            _gift->GetMotionMaster()->MoveIdle(MOTION_SLOT_IDLE);
        }

        void UpdateAI(uint32 const diff)
        {
            _events.Update(diff);
            while (uint32 eventId = _events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_MOVE_IN:
                        // arbitrary point - ToDo: get sniffed position
                        me->GetMotionMaster()->MovePoint(0, 788.254761f, 1326.236938f, 227.795822f);
                        _events.ScheduleEvent(EVENT_CREATE_GIFT, 8*IN_MILLISECONDS);
                        break;
                    case EVENT_CREATE_GIFT:
                        me->SetFacing(0, _gift);
                        DoCast(_gift, SPELL_GIFT_VISUAL, true);
                        DoCast(_gift, SPELL_GIFT_CHANNEL);
                        _events.ScheduleEvent(EVENT_SAY_ONE, 2*IN_MILLISECONDS);
                        break;
                    case EVENT_SAY_ONE:
                        if (_gift)
                        {
                            if (InstanceScript* script = me->GetInstanceScript())
                            {
                                GameObject* go = new GameObject;
                                if (go->Create(sObjectMgr->GenerateLowGuid(HIGHGUID_GAMEOBJECT), RAID_MODE(GO_ALEXSTRASZA_S_GIFT_10, GO_ALEXSTRASZA_S_GIFT_25), script->instance, PHASEMASK_NORMAL, _gift->GetPositionX(), _gift->GetPositionY(), _gift->GetPositionZ(), _gift->GetOrientation(), 0, 0, 0, 0, 120, GO_STATE_READY))
                                {
                                    go->SetRespawnTime(1*WEEK);
                                    script->instance->Add(go);
                                }
                                else
                                    delete go;
                            }
                        }
                        Talk(SAY_ONE);
                        _events.ScheduleEvent(EVENT_SAY_TWO, 4*IN_MILLISECONDS);
                        break;
                    case EVENT_SAY_TWO:
                        Talk(SAY_TWO);
                        _events.ScheduleEvent(EVENT_SAY_THREE, 4*IN_MILLISECONDS);
                        break;
                    case EVENT_SAY_THREE:
                        Talk(SAY_THREE);
                        _events.ScheduleEvent(EVENT_SAY_FOUR, 7*IN_MILLISECONDS);
                        break;
                    case EVENT_SAY_FOUR:
                        Talk(SAY_FOUR);
                        break;
                }
            }
        }
    private:
        EventMap _events;
        Creature* _gift;
    };
};

class achievement_denyin_the_scion : public AchievementCriteriaScript
{
    public:
        achievement_denyin_the_scion() : AchievementCriteriaScript("achievement_denyin_the_scion") {}

        bool OnCheck(Player* source, Unit* /*target*/)
        {
            if (Unit* disk = source->GetVehicleBase())
                if (disk->GetEntry() == NPC_HOVER_DISK_CASTER || disk->GetEntry() == NPC_HOVER_DISK_MELEE)
                    return true;
            return false;
        }
};

void AddSC_boss_malygos()
{
    new boss_malygos();
    new npc_portal_eoe();
    new npc_power_spark();
    new npc_hover_disk();
    new npc_arcane_overload();
    new npc_wyrmrest_skytalon();
    new npc_alexstrasza_eoe();
    new spell_malygos_vortex_dummy();
    new spell_malygos_vortex_visual();
    new spell_malygos_arcane_barrage();
    new spell_malygos_arcane_storm();
    new spell_malygos_surge_of_power_p3();
    new spell_malygos_arcane_overload();
    new spell_malygos_surge_of_power();
    new achievement_denyin_the_scion();
}
