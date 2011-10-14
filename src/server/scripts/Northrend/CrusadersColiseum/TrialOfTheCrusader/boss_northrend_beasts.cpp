/*
 * Copyright (C) 2008-2011 by WarHead - United Worlds of MaNGOS - http://www.uwom.de
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2010 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
/* ScriptData
SDName: northrend_beasts
SD%Complete: 90%
SDComment: based on /dev/rsa
SDCategory:
EndScriptData */

// Known bugs:
// Gormok - Not implemented as a vehicle
//        - Snobold Firebomb
//        - Snobolled (creature at back)
// Snakes - miss the 1-hitkill from emerging
//        - visual changes between mobile and stationary models seems not to work sometimes

#include "ScriptPCH.h"
#include "trial_of_the_crusader.h"

enum Yells
{
    //Gormok
    SAY_SNOBOLLED        = -1649000,
    //Acidmaw & Dreadscale
    SAY_SUBMERGE         = -1649010,
    SAY_EMERGE           = -1649011,
    SAY_BERSERK          = -1649012,
    //Icehowl
    SAY_TRAMPLE_STARE    = -1649020,
    SAY_TRAMPLE_FAIL     = -1649021,
    SAY_TRAMPLE_START    = -1649022
};

enum Equipment
{
    EQUIP_MAIN           = 50760,
    EQUIP_OFFHAND        = 48040,
    EQUIP_RANGED         = 47267,
    EQUIP_DONE           = EQUIP_NO_CHANGE
};

enum Model
{
    MODEL_ACIDMAW_STATIONARY     = 29815,
    MODEL_ACIDMAW_MOBILE         = 29816,
    MODEL_DREADSCALE_STATIONARY  = 26935,
    MODEL_DREADSCALE_MOBILE      = 24564
};

enum Summons
{
    NPC_SNOBOLD_VASSAL   = 34800,
    NPC_SLIME_POOL       = 35176
};

enum BossSpells
{
        // Gormok
#define SPELL_IMPALE            RAID_MODE<uint32>(66331,67477,67478,67479)
#define SPELL_STAGGERING_STOMP  RAID_MODE<uint32>(66330,67647,67648,67649)
        // Snobold
        SPELL_RISING_ANGER      = 66636,
        SPELL_SNOBOLLED         = 66406,
        SPELL_BATTER            = 66408,
        SPELL_FIRE_BOMB         = 66313,
        SPELL_FIRE_BOMB_1       = 66317,
        SPELL_FIRE_BOMB_DOT     = 66318,
        SPELL_HEAD_CRACK        = 66407,
        // Acidmaw & Dreadscale
        SPELL_ACID_SPEW         = 66819,
        SPELL_MOLTEN_SPEW       = 66821,
        SPELL_EMERGE_0          = 66947,
        SPELL_SUBMERGE_0        = 66948,
        SPELL_ENRAGE            = 68335,
        SPELL_SLIME_POOL_EFFECT = 66882, // In 60s it diameter grows from 10y to 40y (r=r+0.25 per second)
#define SPELL_ACID_SPIT         RAID_MODE<uint32>(66880,67606,67607,67608)
#define SPELL_PARALYTIC_SPRAY   RAID_MODE<uint32>(66901,67615,67616,67617)
#define SPELL_PARALYTIC_BITE    RAID_MODE<uint32>(66824,67612,67613,67614)
#define SPELL_PARALYTIC_TOXIN   RAID_MODE<uint32>(66823,67618,67619,67620)
#define SPELL_SWEEP             RAID_MODE<uint32>(66794,67644,67645,67646)
#define SPELL_SLIME_POOL_ACID   RAID_MODE<uint32>(67643,67641,67642,67643)
#define SPELL_SLIME_POOL_DREAD  RAID_MODE<uint32>(66883,67641,67642,67643)
#define SPELL_FIRE_SPIT         RAID_MODE<uint32>(66796,67632,67633,67634)
#define SPELL_BURNING_BITE      RAID_MODE<uint32>(66879,67624,67625,67626)
#define SPELL_BURNING_SPRAY     RAID_MODE<uint32>(66902,67627,67628,67629)
        // Icehowl
#define SPELL_FEROCIOUS_BUTT    RAID_MODE<uint32>(66770,67654,67655,67656)
#define SPELL_MASSIVE_CRASH     RAID_MODE<uint32>(66683,67660,67661,67662)
#define SPELL_WHIRL             RAID_MODE<uint32>(67345,67663,67664,67665)
#define SPELL_ARCTIC_BREATH     RAID_MODE<uint32>(66689,67650,67651,67652)
        SPELL_TRAMPLE           = 66734,
#define SPELL_FROTHING_RAGE     RAID_MODE<uint32>(66759,67657,67658,67659)
        SPELL_STAGGERED_DAZE    = 66758,
        SPELL_BERSERK           = 26662  // TODO: Wird noch nicht genutzt im Skript!!!
};

bool SpielerAmLeben(Unit * source)
{
    if (!source || !source->isAlive() || !source->IsInWorld())
        return false;

    Map::PlayerList const &PlayerList = source->GetMap()->GetPlayers();

    if (PlayerList.isEmpty())
        return false;

    for (Map::PlayerList::const_iterator itr = PlayerList.begin(); itr != PlayerList.end(); ++itr)
        if (itr->getSource() && itr->getSource()->IsInWorld() && itr->getSource()->isAlive())
            return true;

    return false;
}

class boss_gormok : public CreatureScript
{
public:
    boss_gormok() : CreatureScript("boss_gormok") { }

    struct boss_gormokAI : public ScriptedAI
    {
        boss_gormokAI(Creature * creature) : ScriptedAI(creature), Summons(me)
        {
            instance = creature->GetInstanceScript();
        }

        void Reset()
        {
            m_uiImpaleTimer = SEKUNDEN_10;
            m_uiStaggeringStompTimer = SEKUNDEN_15;
            m_uiSummonTimer = urand(SEKUNDEN_15, SEKUNDEN_30);;

            if (GetDifficulty() == RAID_DIFFICULTY_25MAN_NORMAL || GetDifficulty() == RAID_DIFFICULTY_25MAN_HEROIC)
                m_uiSummonCount = 6;
            else
                m_uiSummonCount = 4;

            Summons.DespawnAll();

            if (instance)
                instance->DoUseDoorOrButton(instance->GetData64(GO_MAIN_GATE_DOOR));

            checktimer = SEKUNDEN_10;
        }

        void MovementInform(uint32 uiType, uint32 uiId)
        {
            if (uiType != POINT_MOTION_TYPE) return;

            switch (uiId)
            {
                case 0:
                    instance->DoUseDoorOrButton(instance->GetData64(GO_MAIN_GATE_DOOR));
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_OOC_NOT_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetInCombatWithZone();
                    break;
            }
        }

        void JustDied(Unit * /*killer*/)
        {
            if (instance)
                instance->SetData(TYPE_NORTHREND_BEASTS, GORMOK_DONE);

            Summons.DespawnAll();
        }

        void JustReachedHome()
        {
            if (instance)
            {
                instance->DoUseDoorOrButton(instance->GetData64(GO_MAIN_GATE_DOOR));
                instance->SetData(TYPE_NORTHREND_BEASTS, FAIL);
            }
            me->DespawnOrUnsummon();
        }

        void EnterCombat(Unit * /*who*/)
        {
            me->SetInCombatWithZone();
            instance->SetData(TYPE_NORTHREND_BEASTS, GORMOK_IN_PROGRESS);
        }

        void JustSummoned(Creature * summon)
        {
            if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
            {
                if (summon->GetEntry() == NPC_SNOBOLD_VASSAL)
                {
                    summon->GetMotionMaster()->MoveJump(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 5.0f, 10.0f);
                    summon->AddAura(SPELL_RISING_ANGER, summon);
                    --m_uiSummonCount;
                }
                summon->AI()->AttackStart(target);
            }
            Summons.Summon(summon);
        }

        void SummonedCreatureDespawn(Creature * summon)
        {
            if (summon->GetEntry() == NPC_SNOBOLD_VASSAL)
                if (summon->isAlive())
                    ++m_uiSummonCount;
            Summons.Despawn(summon);
        }

        void UpdateAI(const uint32 diff)
        {
            if (checktimer <= diff)
            {
                if (!SpielerAmLeben(me))
                    EnterEvadeMode();
                checktimer = SEKUNDEN_10;
            }
            else
                checktimer -= diff;

            if (!UpdateVictim())
                return;

            if (m_uiImpaleTimer <= diff)
            {
                DoCastVictim(SPELL_IMPALE);
                m_uiImpaleTimer = urand(SEKUNDEN_20, 29 * IN_MILLISECONDS);
            } else m_uiImpaleTimer -= diff;

            if (m_uiStaggeringStompTimer <= diff)
            {
                DoCastVictim(SPELL_STAGGERING_STOMP);
                m_uiStaggeringStompTimer = urand(SEKUNDEN_20, 25 * IN_MILLISECONDS);
            } else m_uiStaggeringStompTimer -= diff;

            if (m_uiSummonTimer <= diff)
            {
                if (m_uiSummonCount > 0)
                {
                    me->SummonCreature(NPC_SNOBOLD_VASSAL, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), 0, TEMPSUMMON_CORPSE_DESPAWN);
                    DoScriptText(SAY_SNOBOLLED, me);
                }
                m_uiSummonTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
            } else m_uiSummonTimer -= diff;

            DoMeleeAttackIfReady();
        }
    private:
        InstanceScript * instance;
        uint32 m_uiImpaleTimer;
        uint32 m_uiStaggeringStompTimer;
        SummonList Summons;
        uint32 m_uiSummonTimer;
        uint32 m_uiSummonCount;
        uint32 checktimer;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_gormokAI(creature);
    }
};

class mob_snobold_vassal : public CreatureScript
{
public:
    mob_snobold_vassal() : CreatureScript("mob_snobold_vassal") { }

    struct mob_snobold_vassalAI : public ScriptedAI
    {
        mob_snobold_vassalAI(Creature * creature) : ScriptedAI(creature)
        {
            instance = creature->GetInstanceScript();
            if (instance)
                instance->SetData(DATA_SNOBOLD_COUNT, INCREASE);
        }

        void Reset()
        {
            m_uiFireBombTimer = urand(SEKUNDEN_20, SEKUNDEN_30);
            m_uiBatterTimer = SEKUNDEN_05;
            m_uiHeadCrackTimer = urand(SEKUNDEN_10, SEKUNDEN_20);
            m_uiTargetGUID = 0;
            //Workaround for Snobold
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_OOC_NOT_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            checktimer = SEKUNDEN_10;
        }

        void EnterCombat(Unit * who)
        {
            if (!who || !who->IsInWorld() || !who->ToPlayer())
                return;

            Player * pl = who->ToPlayer();
            m_uiTargetGUID = pl->GetGUID();
            me->TauntApply(pl);
            pl->AddAura(SPELL_SNOBOLLED, pl);
            me->AddThreat(pl, 100000000.0f);
        }

        void DamageTaken(Unit * pDoneBy, uint32 & uiDamage)
        {
            if (pDoneBy && pDoneBy->GetGUID() == m_uiTargetGUID)
                uiDamage = 0;
        }

        void JustDied(Unit * /*killer*/)
        {
            if (Unit * target = Unit::GetPlayer(*me, m_uiTargetGUID))
                target->RemoveAurasDueToSpell(SPELL_SNOBOLLED);
            if (instance)
                instance->SetData(DATA_SNOBOLD_COUNT, DECREASE);
        }

        void UpdateAI(const uint32 diff)
        {
            if (checktimer <= diff)
            {
                if (!SpielerAmLeben(me))
                    EnterEvadeMode();
                checktimer = SEKUNDEN_10;
            }
            else
                checktimer -= diff;

            if (!UpdateVictim())
                return;

            if (Unit * target = Unit::GetPlayer(*me, m_uiTargetGUID))
            {
                if (!target->isAlive())
                {
                    target->RemoveAurasDueToSpell(SPELL_SNOBOLLED);

                    if (Unit * target = me->SelectNearestTarget())
                        if (target->ToPlayer() && target->isAlive())
                        {
                            m_uiTargetGUID = target->GetGUID();
                            me->GetMotionMaster()->MoveJump(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 15.0f, 15.0f);
                            EnterCombat(target);
                        }
                }
            }
            else
            {
                if (Unit * target = me->SelectNearestTarget())
                    if (target->ToPlayer() && target->isAlive())
                    {
                        m_uiTargetGUID = target->GetGUID();
                        me->GetMotionMaster()->MoveJump(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 15.0f, 15.0f);
                        EnterCombat(target);
                    }
            }

            if (m_uiFireBombTimer < diff)
            {
                if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                    DoCast(target, SPELL_FIRE_BOMB);
                m_uiFireBombTimer = urand(SEKUNDEN_20, SEKUNDEN_30);
            }
            else m_uiFireBombTimer -= diff;

            if (m_uiBatterTimer < diff)
            {
                if (Unit * target = Unit::GetPlayer(*me, m_uiTargetGUID))
                    DoCast(target, SPELL_BATTER);
                m_uiBatterTimer = urand(SEKUNDEN_10, SEKUNDEN_20);
            }
            else m_uiBatterTimer -= diff;

            if (m_uiHeadCrackTimer < diff)
            {
                if (Unit * target = Unit::GetPlayer(*me, m_uiTargetGUID))
                    DoCast(target, SPELL_HEAD_CRACK);
                m_uiHeadCrackTimer = urand(SEKUNDEN_20, SEKUNDEN_30);
            }
            else m_uiHeadCrackTimer -= diff;

            DoMeleeAttackIfReady();
        }
    private:
        InstanceScript * instance;
        uint32 m_uiFireBombTimer;
        uint32 m_uiBatterTimer;
        uint32 m_uiHeadCrackTimer;
        uint32 checktimer;
        uint64 m_uiTargetGUID;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new mob_snobold_vassalAI(creature);
    }
};

struct boss_jormungarAI : public ScriptedAI
{
    boss_jormungarAI(Creature * creature) : ScriptedAI(creature)
    {
        instanceScript = creature->GetInstanceScript();
    }

    void Reset()
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_OOC_NOT_ATTACKABLE);

        enraged = false;
        biteTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
        spewTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
        slimePoolTimer = SEKUNDEN_15;
        spitTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
        sprayTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
        sweepTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
        checktimer = SEKUNDEN_10;
    }

    void JustDied(Unit * /*killer*/)
    {
        if (instanceScript)
        {
            if (Creature * otherWorm = Unit::GetCreature(*me, instanceScript->GetData64(otherWormEntry)))
            {
                if (!otherWorm->isAlive())
                {
                    instanceScript->SetData(TYPE_NORTHREND_BEASTS, SNAKES_DONE);

                    me->DespawnOrUnsummon();
                    otherWorm->DespawnOrUnsummon();
                }
                else
                    instanceScript->SetData(TYPE_NORTHREND_BEASTS, SNAKES_SPECIAL);
            }
        }
    }

    void JustReachedHome()
    {
        if (instanceScript && instanceScript->GetData(TYPE_NORTHREND_BEASTS) != FAIL)
        {
            instanceScript->SetData(TYPE_NORTHREND_BEASTS, FAIL);
        }

        me->DespawnOrUnsummon();
    }

    void KilledUnit(Unit * who)
    {
        if (who->GetTypeId() == TYPEID_PLAYER)
        {
            if (instanceScript)
                instanceScript->SetData(DATA_TRIBUTE_TO_IMMORTALITY_ELEGIBLE, 0);
        }
    }

    void EnterCombat(Unit * /*who*/)
    {
        me->SetInCombatWithZone();
        if (instanceScript)
            instanceScript->SetData(TYPE_NORTHREND_BEASTS, SNAKES_IN_PROGRESS);
    }

    void UpdateAI(const uint32 diff)
    {
        if (checktimer <= diff)
        {
            if (!SpielerAmLeben(me))
                EnterEvadeMode();
            checktimer = SEKUNDEN_10;
        }
        else
            checktimer -= diff;

        if (!UpdateVictim())
            return;

        if (instanceScript && instanceScript->GetData(TYPE_NORTHREND_BEASTS) == SNAKES_SPECIAL && !enraged)
        {
            DoScriptText(SAY_EMERGE, me);
            me->RemoveAurasDueToSpell(SPELL_SUBMERGE_0);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            DoCast(SPELL_ENRAGE);
            enraged = true;
            DoScriptText(SAY_BERSERK, me);
            switch (stage)
            {
                case 0:
                    break;
                case 4:
                    stage = 5;
                    submergeTimer = SEKUNDEN_05;
                    break;
                default:
                    stage = 7;
            }
        }

        switch (stage)
        {
            case 0: // Mobile
                if (biteTimer <= diff)
                {
                    DoCastVictim(biteSpell);
                    biteTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
                } else biteTimer -= diff;

                if (spewTimer <= diff)
                {
                    DoCastAOE(spewSpell);
                    spewTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
                } else spewTimer -= diff;

                if (slimePoolTimer <= diff)
                {
                    DoCast(me, SlimePoolSpell);
                    slimePoolTimer = SEKUNDEN_30;
                } else slimePoolTimer -= diff;

                if (submergeTimer <= diff && !enraged)
                {
                    stage = 1;
                    submergeTimer = SEKUNDEN_05;
                } else submergeTimer -= diff;

                DoMeleeAttackIfReady();
                break;
            case 1: // Submerge
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                DoCast(me, SPELL_SUBMERGE_0);
                DoScriptText(SAY_SUBMERGE, me);
                me->GetMotionMaster()->MovePoint(0, ToCCommonLoc[1].GetPositionX()+urand(0, 80)-40, ToCCommonLoc[1].GetPositionY()+urand(0, 80)-40, ToCCommonLoc[1].GetPositionZ());
                stage = 2;
            case 2: // Wait til emerge
                if (submergeTimer <= diff)
                {
                    stage = 3;
                    submergeTimer = SEKUNDEN_50;
                } else submergeTimer -= diff;
                break;
            case 3: // Emerge
                me->SetDisplayId(modelStationary);
                DoScriptText(SAY_EMERGE, me);
                me->RemoveAurasDueToSpell(SPELL_SUBMERGE_0);
                DoCast(me, SPELL_EMERGE_0);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                SetCombatMovement(false);
                me->GetMotionMaster()->MoveIdle();
                stage = 4;
                break;
            case 4: // Stationary
                if (sprayTimer <= diff)
                {
                    if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                        DoCast(target, spraySpell);
                    sprayTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
                } else sprayTimer -= diff;

                if (sweepTimer <= diff)
                {
                    DoCastAOE(SPELL_SWEEP);
                    sweepTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
                } else sweepTimer -= diff;

                if (submergeTimer <= diff)
                {
                    stage = 5;
                    submergeTimer = SEKUNDEN_10;
                } else submergeTimer -= diff;

                DoSpellAttackIfReady(spitSpell);
                break;
            case 5: // Submerge
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                DoCast(me, SPELL_SUBMERGE_0);
                DoScriptText(SAY_SUBMERGE, me);
                me->GetMotionMaster()->MovePoint(0, ToCCommonLoc[1].GetPositionX()+urand(0, 80)-40, ToCCommonLoc[1].GetPositionY()+urand(0, 80)-40, ToCCommonLoc[1].GetPositionZ());
                stage = 6;
            case 6: // Wait til emerge
                if (submergeTimer <= diff)
                {
                    stage = 7;
                    submergeTimer = 45 * IN_MILLISECONDS;
                } else submergeTimer -= diff;
                break;
            case 7: // Emerge
                me->SetDisplayId(modelMobile);
                DoScriptText(SAY_EMERGE, me);
                me->RemoveAurasDueToSpell(SPELL_SUBMERGE_0);
                DoCast(me, SPELL_EMERGE_0);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_DISABLE_MOVE);
                SetCombatMovement(true);
                me->GetMotionMaster()->MoveChase(me->getVictim());
                stage = 0;
                break;
        }
    }

    InstanceScript * instanceScript;

    uint32 otherWormEntry;

    uint32 modelStationary;
    uint32 modelMobile;

    uint32 biteSpell;
    uint32 spewSpell;
    uint32 spitSpell;
    uint32 spraySpell;
    uint32 SlimePoolSpell;

    uint32 biteTimer;
    uint32 spewTimer;
    uint32 slimePoolTimer;
    uint32 spitTimer;
    uint32 sprayTimer;
    uint32 sweepTimer;
    uint32 submergeTimer;
    uint32 checktimer;
    uint8 stage;
    bool enraged;
};

class boss_acidmaw : public CreatureScript
{
    public:
    boss_acidmaw() : CreatureScript("boss_acidmaw") { }

    struct boss_acidmawAI : public boss_jormungarAI
    {
        boss_acidmawAI(Creature * creature) : boss_jormungarAI(creature) { }

        void Reset()
        {
            boss_jormungarAI::Reset();
            biteSpell = SPELL_PARALYTIC_BITE;
            spewSpell = SPELL_ACID_SPEW;
            spitSpell = SPELL_ACID_SPIT;
            spraySpell = SPELL_PARALYTIC_SPRAY;
            modelStationary = MODEL_ACIDMAW_STATIONARY;
            modelMobile = MODEL_ACIDMAW_MOBILE;
            otherWormEntry = NPC_DREADSCALE;
            SlimePoolSpell = SPELL_SLIME_POOL_ACID;

            submergeTimer = 500;
            DoCast(me, SPELL_SUBMERGE_0);
            stage = 2;
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_acidmawAI(creature);
    }
};

class boss_dreadscale : public CreatureScript
{
public:
    boss_dreadscale() : CreatureScript("boss_dreadscale") { }

    struct boss_dreadscaleAI : public boss_jormungarAI
    {
        boss_dreadscaleAI(Creature * creature) : boss_jormungarAI(creature)
        {
            instanceScript = creature->GetInstanceScript();
        }

        InstanceScript * instanceScript;

        void Reset()
        {
            boss_jormungarAI::Reset();
            biteSpell = SPELL_BURNING_BITE;
            spewSpell = SPELL_MOLTEN_SPEW;
            spitSpell = SPELL_FIRE_SPIT;
            spraySpell = SPELL_BURNING_SPRAY;
            modelStationary = MODEL_DREADSCALE_STATIONARY;
            modelMobile = MODEL_DREADSCALE_MOBILE;
            otherWormEntry = NPC_ACIDMAW;
            SlimePoolSpell = SPELL_SLIME_POOL_DREAD;

            submergeTimer = 45 * IN_MILLISECONDS;
            stage = 0;
        }

        void MovementInform(uint32 uiType, uint32 uiId)
        {
            if (uiType != POINT_MOTION_TYPE) return;

            switch (uiId)
            {
                case 0:
                    instanceScript->DoUseDoorOrButton(instanceScript->GetData64(GO_MAIN_GATE_DOOR));
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_OOC_NOT_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetInCombatWithZone();
                    if (Creature * otherWorm = Unit::GetCreature(*me, instanceScript->GetData64(otherWormEntry)))
                    {
                        otherWorm->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_OOC_NOT_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        otherWorm->SetReactState(REACT_AGGRESSIVE);
                        otherWorm->SetVisible(true);
                        otherWorm->SetInCombatWithZone();
                    }
                    break;
            }
        }

        void EnterEvadeMode()
        {
            instanceScript->DoUseDoorOrButton(instanceScript->GetData64(GO_MAIN_GATE_DOOR));
            boss_jormungarAI::EnterEvadeMode();
        }

        void JustReachedHome()
        {
            if (instanceScript)
                instanceScript->DoUseDoorOrButton(instanceScript->GetData64(GO_MAIN_GATE_DOOR));

            boss_jormungarAI::JustReachedHome();
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_dreadscaleAI(creature);
    }
};

class mob_slime_pool : public CreatureScript
{
public:
    mob_slime_pool() : CreatureScript("mob_slime_pool") { }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new mob_slime_poolAI(creature);
    }

    struct mob_slime_poolAI : public ScriptedAI
    {
        mob_slime_poolAI(Creature * creature) : ScriptedAI(creature)
        {
        }

        bool casted;
        void Reset()
        {
            casted = false;
            me->SetReactState(REACT_PASSIVE);
        }

        void UpdateAI(const uint32 /*diff*/)
        {
            if (!casted)
            {
                casted = true;
                DoCast(me, SPELL_SLIME_POOL_EFFECT);
            }
        }
    };

};

class boss_icehowl : public CreatureScript
{
public:
    boss_icehowl() : CreatureScript("boss_icehowl") { }

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_icehowlAI(creature);
    }

    struct boss_icehowlAI : public ScriptedAI
    {
        boss_icehowlAI(Creature * creature) : ScriptedAI(creature)
        {
            instance = (InstanceScript *)creature->GetInstanceScript();
        }

        InstanceScript * instance;

        uint32 m_uiFerociousButtTimer;
        uint32 m_uiArticBreathTimer;
        uint32 m_uiWhirlTimer;
        uint32 m_uiMassiveCrashTimer;
        uint32 m_uiTrampleTimer;
        uint32 checktimer;
        float  m_fTrampleTargetX, m_fTrampleTargetY, m_fTrampleTargetZ;
        uint64 m_uiTrampleTargetGUID;
        bool   m_bMovementStarted;
        bool   m_bMovementFinish;
        bool   m_bTrampleCasted;
        uint8  m_uiStage;
        Unit *  target;

        void Reset()
        {
            m_uiFerociousButtTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
            m_uiArticBreathTimer = urand(25 * IN_MILLISECONDS, SEKUNDEN_40);
            m_uiWhirlTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
            m_uiMassiveCrashTimer = SEKUNDEN_30;
            m_uiTrampleTimer = IN_MILLISECONDS;
            m_bMovementStarted = false;
            m_bMovementFinish = false;
            m_bTrampleCasted = false;
            m_uiTrampleTargetGUID = 0;
            m_fTrampleTargetX = 0;
            m_fTrampleTargetY = 0;
            m_fTrampleTargetZ = 0;
            m_uiStage = 0;
            checktimer = SEKUNDEN_10;
        }

        void JustDied(Unit * /*killer*/)
        {
            if (instance)
                instance->SetData(TYPE_NORTHREND_BEASTS, ICEHOWL_DONE);
        }

        void MovementInform(uint32 uiType, uint32 uiId)
        {
            if (uiType != POINT_MOTION_TYPE) return;

            switch (uiId)
            {
                case 0:
                    if (me->GetDistance2d(ToCCommonLoc[1].GetPositionX(), ToCCommonLoc[1].GetPositionY()) < 6.0f)
                    {
                        // Middle of the room
                        m_uiStage = 1;
                    }
                    else
                    {
                        // Landed from Hop backwards (start trample)
                        if (Unit::GetPlayer(*me, m_uiTrampleTargetGUID))
                        {
                            m_uiStage = 4;
                        } else m_uiStage = 6;
                    }
                    break;
                case 1: // Finish trample
                    m_bMovementFinish = true;
                    break;
                case 2:
                    instance->DoUseDoorOrButton(instance->GetData64(GO_MAIN_GATE_DOOR));
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_OOC_NOT_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_AGGRESSIVE);
                    me->SetInCombatWithZone();
                    break;
            }
        }

        void EnterEvadeMode()
        {
            instance->DoUseDoorOrButton(instance->GetData64(GO_MAIN_GATE_DOOR));
            ScriptedAI::EnterEvadeMode();
        }

        void JustReachedHome()
        {
            if (instance)
            {
                instance->DoUseDoorOrButton(instance->GetData64(GO_MAIN_GATE_DOOR));
                instance->SetData(TYPE_NORTHREND_BEASTS, FAIL);
            }
            me->DespawnOrUnsummon();
        }

        void KilledUnit(Unit * who)
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                if (instance)
                    instance->SetData(DATA_TRIBUTE_TO_IMMORTALITY_ELEGIBLE, 0);
            }
        }

        void EnterCombat(Unit * /*who*/)
        {
            if (instance)
                instance->SetData(TYPE_NORTHREND_BEASTS, ICEHOWL_IN_PROGRESS);
            me->SetInCombatWithZone();
        }

        void SpellHitTarget(Unit * target, const SpellInfo* spell)
        {
            if (spell->Id == SPELL_TRAMPLE && target->GetTypeId() == TYPEID_PLAYER)
            {
                if (!m_bTrampleCasted)
                {
                    DoCast(me, SPELL_FROTHING_RAGE, true);
                    m_bTrampleCasted = true;
                }
            }
        }

        void UpdateAI(const uint32 diff)
        {
            if (checktimer <= diff)
            {
                if (!SpielerAmLeben(me))
                    EnterEvadeMode();
                checktimer = SEKUNDEN_10;
            }
            else
                checktimer -= diff;

            if (!UpdateVictim())
                return;

            switch (m_uiStage)
            {
                case 0:
                    if (m_uiFerociousButtTimer <= diff)
                    {
                        DoCastVictim(SPELL_FEROCIOUS_BUTT);
                        m_uiFerociousButtTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
                    } else m_uiFerociousButtTimer -= diff;

                    if (m_uiArticBreathTimer <= diff)
                    {
                        if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0))
                            DoCast(target, SPELL_ARCTIC_BREATH);
                        m_uiArticBreathTimer = urand(25 * IN_MILLISECONDS, SEKUNDEN_40);
                    } else m_uiArticBreathTimer -= diff;

                    if (m_uiWhirlTimer <= diff)
                    {
                        DoCastAOE(SPELL_WHIRL);
                        m_uiWhirlTimer = urand(SEKUNDEN_15, SEKUNDEN_30);
                    } else m_uiWhirlTimer -= diff;

                    if (m_uiMassiveCrashTimer <= diff)
                    {
                        me->GetMotionMaster()->MoveJump(ToCCommonLoc[1].GetPositionX(), ToCCommonLoc[1].GetPositionY(), ToCCommonLoc[1].GetPositionZ(), 10.0f, 20.0f); // 1: Middle of the room
                        m_uiStage = 7; //Invalid (Do nothing more than move)
                        m_uiMassiveCrashTimer = SEKUNDEN_30;
                    } else m_uiMassiveCrashTimer -= diff;

                    DoMeleeAttackIfReady();
                    break;
                case 1:
                    DoCastAOE(SPELL_MASSIVE_CRASH);
                    m_uiStage = 2;
                    break;
                case 2:
                    if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0, true))
                    {
                        m_uiTrampleTargetGUID = target->GetGUID();
                        me->SetTarget(m_uiTrampleTargetGUID);
                        DoScriptText(SAY_TRAMPLE_STARE, me, target);
                        m_bTrampleCasted = false;
                        SetCombatMovement(false);
                        me->GetMotionMaster()->MoveIdle();
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        m_uiTrampleTimer = 4 * IN_MILLISECONDS;
                        m_uiStage = 3;
                    } else m_uiStage = 6;
                    break;
                case 3:
                    if (m_uiTrampleTimer <= diff)
                    {
                        if (Unit * target = Unit::GetPlayer(*me, m_uiTrampleTargetGUID))
                        {
                            m_bTrampleCasted = false;
                            m_bMovementStarted = true;
                            m_fTrampleTargetX = target->GetPositionX();
                            m_fTrampleTargetY = target->GetPositionY();
                            m_fTrampleTargetZ = target->GetPositionZ();
                            me->GetMotionMaster()->MoveJump(2*me->GetPositionX()-m_fTrampleTargetX,
                                2*me->GetPositionY()-m_fTrampleTargetY,
                                me->GetPositionZ(),
                                10.0f, 20.0f); // 2: Hop Backwards
                            m_uiStage = 7; //Invalid (Do nothing more than move)
                        } else m_uiStage = 6;
                    } else m_uiTrampleTimer -= diff;
                    break;
                case 4:
                    DoScriptText(SAY_TRAMPLE_START, me);
                    me->GetMotionMaster()->MoveCharge(m_fTrampleTargetX, m_fTrampleTargetY, m_fTrampleTargetZ+2, 42, 1);
                    me->SetTarget(0);
                    m_uiStage = 5;
                    break;
                case 5:
                    if (m_bMovementFinish)
                    {
                        if (m_uiTrampleTimer <= diff) DoCastAOE(SPELL_TRAMPLE);
                        m_bMovementFinish = false;
                        m_uiStage = 6;
                        return;
                    }
                    if (m_uiTrampleTimer <= diff)
                    {
                        Map::PlayerList const &lPlayers = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator itr = lPlayers.begin(); itr != lPlayers.end(); ++itr)
                        {
                            if (Unit * player = itr->getSource())
                                if (player->isAlive() && player->IsWithinDistInMap(me, 6.0f))
                                {
                                    DoCastAOE(SPELL_TRAMPLE);
                                    m_uiTrampleTimer = IN_MILLISECONDS;
                                    break;
                                }
                        }
                    } else m_uiTrampleTimer -= diff;
                    break;
                case 6:
                    if (!m_bTrampleCasted)
                    {
                        DoCast(me, SPELL_STAGGERED_DAZE);
                        DoScriptText(SAY_TRAMPLE_FAIL, me);
                    }
                    m_bMovementStarted = false;
                    me->GetMotionMaster()->MovementExpired();
                    me->GetMotionMaster()->MoveChase(me->getVictim());
                    SetCombatMovement(true);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    m_uiStage = 0;
                    break;
            }
        }
    };

};

void AddSC_boss_northrend_beasts()
{
    new boss_gormok();
    new mob_snobold_vassal();
    new boss_acidmaw();
    new boss_dreadscale();
    new mob_slime_pool();
    new boss_icehowl();
}
