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
    SAY_INTRO                        = 0, // Meddlesome insects! You are too late. The Ruby Sanctum is lost!
    SAY_AGGRO                        = 1, // Your world teeters on the brink of annihilation. You will ALL bear witness to the coming of a new age of DESTRUCTION!
    SAY_METEOR_STRIKE                = 2, // The heavens burn!
    SAY_PHASE_TWO                    = 3, // You will find only suffering within the realm of twilight! Enter if you dare! (17507)
    SAY_DEATH                        = 4  // Relish this victory, mortals, for it will be your last! This world will burn with the master's return! (17503)
    // Beware the shadow! (17506)
    // I am the light and the darkness! Cower, mortals, before the herald of Deathwing!
    // Not good enough. 17504
    // Another "hero" falls. 17501
};

enum Spells
{
    // Halion
    SPELL_FLAME_BREATH                  = 74525,
    SPELL_CLEAVE                        = 74524,
    SPELL_METEOR_STRIKE                 = 74637,

    SPELL_COMBUSTION                    = 74562,    // Will each tick, apart from the damage, also add a stack to 74567
    SPELL_COMBUSTION_STACK              = 74567,    // If 74562 or 74567 is removed; this will trigger an explosion (74607) based on stackamount.
    SPELL_COMBUSTION_SCALE_AURA         = 70507,    // Aura created in spell_dbc since missing in client dbc. Value based on 74567 stackamount.

    SPELL_FIERY_COMBUSTION_EXPLOSION    = 74607,
    SPELL_FIERY_COMBUSTION_SUMMON       = 74610,
    SPELL_COMBUSTION_DAMAGE_AURA        = 74629,

    SPELL_CONSUMTION                    = 74792,
    SPELL_CONSUMTION_STACK              = 74795,
    SPELL_SOUL_CONSUMPTION_EXPLOSION    = 74799,
    SPELL_SOUL_CONSUMPTION_SUMMON       = 74800,
    SPELL_CONSUMPTION_DAMAGE_AURA       = 74803,

    // Living Inferno
    SPELL_BLAZING_AURA                  = 75885,
    // Halion Controller
    SPELL_COSMETIC_FIRE_PILLAR          = 76006,
    SPELL_FIERY_EXPLOSION               = 76010,
    // Meteor Strike
    SPELL_METEOR_STRIKE_COUNTDOWN       = 74641,
    SPELL_METEOR_STRIKE_AOE_DAMAGE      = 74648,
    SPELL_METEOR_STRIKE_FIRE_AURA_1     = 74713,
    SPELL_METEOR_STRIKE_FIRE_AURA_2     = 74718,
    SPELL_BIRTH_NO_VISUAL               = 40031
};

enum Events
{
    // Halion
    EVENT_ACTIVATE_FIREWALL = 1,
    EVENT_CLEAVE,
    EVENT_FLAME_BREATH,
    EVENT_METEOR_STRIKE,
    EVENT_FIERY_COMBUSTION,
    // Halion Controller
    EVENT_START_INTRO,
    EVENT_INTRO_PROGRESS_1,
    EVENT_INTRO_PROGRESS_2,
    EVENT_INTRO_PROGRESS_3,
    // Meteor Strike
    EVENT_SPAWN_METEOR_FLAME
};

enum Actions
{
    ACTION_METEOR_STRIKE_BURN = 1,
    ACTION_METEOR_STRIKE_AOE
};

enum Phases
{
    PHASE_ALL       = 0,
    PHASE_ONE       = 1,
    PHASE_TWO       = 2,
    PHASE_THREE     = 3,

    PHASE_ONE_MASK = 1 << PHASE_ONE,
    PHASE_TWO_MASK = 1 << PHASE_TWO,
    PHASE_THREE_MASK = 1 << PHASE_THREE
};

enum Misc
{
    TYPE_COMBUSTION_SUMMON = 1
};

const Position HalionSpawnPos = { 3156.67f,  533.8108f, 72.98822f, 3.159046f };

class boss_halion : public CreatureScript
{
    public:
        boss_halion() : CreatureScript("boss_halion") { }

        struct boss_halionAI : public BossAI
        {
            boss_halionAI(Creature* creature) : BossAI(creature, DATA_HALION)
            {
            }

            void EnterCombat(Unit* /*who*/)
            {
                _EnterCombat();
                Talk(SAY_AGGRO);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_ADD, me);
                events.Reset();
                events.SetPhase(PHASE_ONE);
                events.ScheduleEvent(EVENT_ACTIVATE_FIREWALL, 10000);
                events.ScheduleEvent(EVENT_CLEAVE, urand(8000, 10000));
                events.ScheduleEvent(EVENT_FLAME_BREATH, urand(10000, 12000));
                events.ScheduleEvent(EVENT_METEOR_STRIKE, urand(20000, 25000));
                events.ScheduleEvent(EVENT_FIERY_COMBUSTION, urand(15000, 18000));
            }

            void JustDied(Unit* /*killer*/)
            {
                _JustDied();
                Talk(SAY_DEATH);
                instance->SendEncounterUnit(ENCOUNTER_FRAME_REMOVE, me);
            }

            void JustReachedHome()
            {
                _JustReachedHome();
                instance->SendEncounterUnit(ENCOUNTER_FRAME_REMOVE, me);
            }

            Position const* GetMeteorStrikePosition() const
            {
                return &_meteorStrikePos;
            }

            void UpdateAI(uint32 const diff)
            {
                if ((events.GetPhaseMask() & PHASE_ONE_MASK) && !UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STAT_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ACTIVATE_FIREWALL:
                            // Firewall is activated 10 seconds after starting encounter, DOOR_TYPE_ROOM is only instant.
                            if (GameObject* firewall = ObjectAccessor::GetGameObject(*me, instance->GetData64(DATA_FLAME_RING)))
                                instance->HandleGameObject(instance->GetData64(DATA_FLAME_RING), false, firewall);
                            break;
                        case EVENT_FLAME_BREATH:
                            DoCast(me, SPELL_FLAME_BREATH);
                            events.ScheduleEvent(EVENT_FLAME_BREATH, 25000);
                            break;
                        case EVENT_CLEAVE:
                            DoCastVictim(SPELL_CLEAVE);
                            events.ScheduleEvent(EVENT_CLEAVE, urand(8000, 10000));
                            break;
                        case EVENT_METEOR_STRIKE:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            {
                                target->GetPosition(&_meteorStrikePos);
                                me->CastSpell(_meteorStrikePos.GetPositionX(), _meteorStrikePos.GetPositionY(), _meteorStrikePos.GetPositionZ(), SPELL_METEOR_STRIKE, true, NULL, NULL, me->GetGUID());
                                Talk(SAY_METEOR_STRIKE);
                            }
                            events.ScheduleEvent(EVENT_METEOR_STRIKE, 40000);
                            break;
                        }
                        case EVENT_FIERY_COMBUSTION:
                        {
                            Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true);
                            if (!target)
                                target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true);
                            if (target)
                                DoCast(target, SPELL_COMBUSTION);
                            events.ScheduleEvent(EVENT_FIERY_COMBUSTION, 25000);
                            break;
                        }
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            Position _meteorStrikePos;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetRubySanctumAI<boss_halionAI>(creature);
        }
};

typedef boss_halion::boss_halionAI HalionAI;

class npc_halion_controller : public CreatureScript
{
    public:
        npc_halion_controller() : CreatureScript("npc_halion_controller") { }

        struct npc_halion_controllerAI : public ScriptedAI
        {
            npc_halion_controllerAI(Creature* creature) : ScriptedAI(creature),
                _instance(creature->GetInstanceScript())
            {
                me->SetPhaseMask(me->GetPhaseMask() | 0x20, true);
            }

            void Reset()
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_INTRO_HALION)
                {
                    me->setActive(true);
                    _events.Reset();
                    _events.ScheduleEvent(EVENT_START_INTRO, 2000);
                    _events.ScheduleEvent(EVENT_INTRO_PROGRESS_1, 6000);
                    _events.ScheduleEvent(EVENT_INTRO_PROGRESS_2, 10000);
                    _events.ScheduleEvent(EVENT_INTRO_PROGRESS_3, 14000);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                _events.Update(diff);

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_START_INTRO:
                            DoCast(me, SPELL_COSMETIC_FIRE_PILLAR, true);
                            break;
                        case EVENT_INTRO_PROGRESS_1:
                            for (uint8 i = 2; i < 4; ++i)
                                if (GameObject* tree = ObjectAccessor::GetGameObject(*me, _instance->GetData64(DATA_BURNING_TREE_1 + i)))
                                    _instance->HandleGameObject(_instance->GetData64(DATA_BURNING_TREE_1 + i), true, tree);
                            break;
                        case EVENT_INTRO_PROGRESS_2:
                            for (uint8 i = 0; i < 2; ++i)
                                if (GameObject* tree = ObjectAccessor::GetGameObject(*me, _instance->GetData64(DATA_BURNING_TREE_1 + i)))
                                    _instance->HandleGameObject(_instance->GetData64(DATA_BURNING_TREE_1 + i), true, tree);
                            break;
                        case EVENT_INTRO_PROGRESS_3:
                            DoCast(me, SPELL_FIERY_EXPLOSION);
                            if (Creature* halion = me->GetMap()->SummonCreature(NPC_HALION, HalionSpawnPos))
                                halion->AI()->Talk(SAY_INTRO);
                            me->setActive(false);
                            break;
                        default:
                            break;
                    }
                }
            }

        private:
            EventMap _events;
            InstanceScript* _instance;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetRubySanctumAI<npc_halion_controllerAI>(creature);
        }
};

class npc_meteor_strike_initial : public CreatureScript
{
    public:
        npc_meteor_strike_initial() : CreatureScript("npc_meteor_strike_initial") { }

        struct npc_meteor_strike_initialAI : public Scripted_NoMovementAI
        {
            npc_meteor_strike_initialAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_METEOR_STRIKE_AOE)
                {
                    DoCast(me, SPELL_METEOR_STRIKE_AOE_DAMAGE, true);
                    DoCast(me, SPELL_METEOR_STRIKE_FIRE_AURA_1, true);
                    for (std::list<Creature*>::iterator itr = _meteorList.begin(); itr != _meteorList.end(); ++itr)
                        (*itr)->AI()->DoAction(ACTION_METEOR_STRIKE_BURN);
                }
            }

            void IsSummonedBy(Unit* summoner)
            {
                _owner = summoner->ToCreature();
                if (!_owner)
                    return;

                DoCast(me, SPELL_METEOR_STRIKE_COUNTDOWN);
                DoCast(me, SPELL_BIRTH_NO_VISUAL); // Unknown purpose

                if (HalionAI* halionAI = CAST_AI(HalionAI, _owner->AI()))
                {
                    Position const* ownerPos = halionAI->GetMeteorStrikePosition();
                    Position newpos;
                    float angle[4];
                    angle[0] = me->GetAngle(ownerPos);
                    angle[1] = me->GetAngle(ownerPos) - static_cast<float>(M_PI/2);
                    angle[2] = me->GetAngle(ownerPos) - static_cast<float>(-M_PI/2);
                    angle[3] = me->GetAngle(ownerPos) - static_cast<float>(M_PI);

                    _meteorList.clear();
                    for (uint8 i = 0; i < 4; i++)
                    {
                        MapManager::NormalizeOrientation(angle[i]);
                        me->SetOrientation(angle[i]);
                        me->GetNearPosition(newpos, 10.0f, 0.0f); // Exact distance
                        if (Creature* meteor = me->SummonCreature(NPC_METEOR_STRIKE_NORTH + i, newpos, TEMPSUMMON_TIMED_DESPAWN, 30000))
                            _meteorList.push_back(meteor);
                    }
                }
            }

            void UpdateAI(uint32 const /*diff*/) {}
            void EnterEvadeMode() {}

        private:
            Creature* _owner;
            std::list<Creature*> _meteorList;
        };

        CreatureAI* GetAI(Creature* creature) const
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
            npc_meteor_strikeAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
                _range = 5.0f;
                _spawnCount = 0;
            }

            void DoAction(int32 const action)
            {
                if (action == ACTION_METEOR_STRIKE_BURN)
                {
                    DoCast(me, SPELL_METEOR_STRIKE_FIRE_AURA_2, true);
                    me->setActive(true);
                    _events.ScheduleEvent(EVENT_SPAWN_METEOR_FLAME, 500);
                }
            }

            void UpdateAI(uint32 const diff)
            {
                if (_spawnCount > 5)
                    return;

                _events.Update(diff);

                if (_events.ExecuteEvent() == EVENT_SPAWN_METEOR_FLAME)
                {
                    Position pos;
                    me->GetNearPosition(pos, _range, 0.0f);

                    if (Creature* flame = me->SummonCreature(NPC_METEOR_STRIKE_FLAME, pos, TEMPSUMMON_TIMED_DESPAWN, 25000))
                    {
                        flame->CastSpell(flame, SPELL_METEOR_STRIKE_FIRE_AURA_2, true);
                        _spawnCount++;
                    }
                    _range += 5.0f;
                    _events.ScheduleEvent(EVENT_SPAWN_METEOR_FLAME, 800);
                }
            }

        private:
            EventMap _events;
            float _range;
            uint8 _spawnCount;
        };

        CreatureAI* GetAI(Creature* creature) const
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
            npc_combustionAI(Creature* creature) : Scripted_NoMovementAI(creature)
            {
            }

            void SetData(uint32 type, uint32 data)
            {
                if (type == TYPE_COMBUSTION_SUMMON)
                {
                    //if (Unit* owner = me->GetSummoner())
                    int32 damage = 1200 + (data * 1290); // Hardcoded values from guessing. Need some more research.
                    me->CastCustomSpell(SPELL_FIERY_COMBUSTION_EXPLOSION, SPELLVALUE_BASE_POINT0, damage, me, true);
                    // Scaling aura
                    me->CastCustomSpell(SPELL_COMBUSTION_SCALE_AURA, SPELLVALUE_AURA_STACK, data, me, false);
                    DoCast(me, SPELL_COMBUSTION_DAMAGE_AURA);
                }
            }

            void UpdateAI(uint32 const /*diff*/) {}
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return GetRubySanctumAI<npc_combustionAI>(creature);
        }
};

class spell_halion_meteor_strike_marker : public SpellScriptLoader
{
    public:
        spell_halion_meteor_strike_marker() : SpellScriptLoader("spell_halion_meteor_strike_marker") { }

        class spell_halion_meteor_strike_marker_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_halion_meteor_strike_marker_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_EXPIRE)
                {
                    if (Creature* creCaster = GetCaster()->ToCreature())
                        creCaster->AI()->DoAction(ACTION_METEOR_STRIKE_AOE);
                }
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_halion_meteor_strike_marker_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
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

            bool Validate(SpellEntry const* /*spell*/)
            {
                if (!sSpellStore.LookupEntry(SPELL_COMBUSTION_STACK))
                    return false;
                return true;
            }

            void HandleExtraEffect(AuraEffect const* /*aurEff*/)
            {
                GetTarget()->CastSpell(GetTarget(), SPELL_COMBUSTION_STACK, true);
            }

            void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                GetTarget()->CastSpell(GetTarget(), SPELL_COMBUSTION_STACK, true);
            }

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                sLog->outError("spell_halion_combustion: OnRemove");
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

        AuraScript* GetAuraScript() const
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

            bool Validate(SpellEntry const* /*spell*/)
            {
                if (!sSpellStore.LookupEntry(SPELL_FIERY_COMBUSTION_SUMMON))
                    return false;
                if (!sSpellStore.LookupEntry(SPELL_FIERY_COMBUSTION_EXPLOSION))
                    return false;
                return true;
            }

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                sLog->outError("spell_halion_combustion_stack: OnRemove");
                GetTarget()->CastCustomSpell(SPELL_FIERY_COMBUSTION_SUMMON, SPELLVALUE_BASE_POINT0, aurEff->GetBase()->GetStackAmount(), GetTarget(), true);
            }

            void Register()
            {
                AfterEffectRemove += AuraEffectRemoveFn(spell_halion_combustion_stack_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const
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
                Unit* caster = GetCaster();
                uint32 entry = uint32(GetSpellInfo()->Effects[effIndex].MiscValue);
                SummonPropertiesEntry const* properties = sSummonPropertiesStore.LookupEntry(uint32(GetSpellInfo()->Effects[effIndex].MiscValueB));
                uint32 duration = uint32(GetSpellInfo()->GetDuration());

                Position pos;
                caster->GetPosition(&pos);
                if (TempSummon* summon = caster->GetMap()->SummonCreature(entry, pos, properties, duration, caster, GetSpellInfo()->Id))
                    summon->AI()->SetData(TYPE_COMBUSTION_SUMMON, uint32(GetSpellInfo()->Effects[effIndex].BasePoints));
            }

            void Register()
            {
                OnEffect += SpellEffectFn(spell_combustion_consumption_summon_SpellScript::HandleSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_combustion_consumption_summon_SpellScript();
        }
};

void AddSC_boss_halion()
{
    new boss_halion();
    new npc_halion_controller();
    new npc_meteor_strike_initial();
    new npc_meteor_strike();
    new npc_combustion();
    new spell_halion_meteor_strike_marker();
    new spell_halion_combustion();
    new spell_halion_combustion_stack();
    new spell_combustion_consumption_summon();
}