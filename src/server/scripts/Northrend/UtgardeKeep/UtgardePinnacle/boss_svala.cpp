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
#include "utgarde_pinnacle.h"

enum Spells
{
    SPELL_CALL_FLAMES           = 48258,
    SPELL_RITUAL_OF_THE_SWORD   = 48276, // Effect #1 Teleport,  Effect #2 Dummy
    SPELL_SINSTER_STRIKE        = 15667,
    H_SPELL_SINSTER_STRIKE      = 59409,
    SPELL_SVALA_TRANSFORMING1   = 54140,
    SPELL_SVALA_TRANSFORMING2   = 54205
};

//not in db
enum Yells
{
    SAY_DIALOG_WITH_ARTHAS_1    = -1575015,
    SAY_DIALOG_WITH_ARTHAS_2    = -1575016,
    SAY_DIALOG_WITH_ARTHAS_3    = -1575017,
    SAY_AGGRO                   = -1575018,
    SAY_SLAY_1                  = -1575019,
    SAY_SLAY_2                  = -1575020,
    SAY_SLAY_3                  = -1575021,
    SAY_DEATH                   = -1575022,
    SAY_SACRIFICE_PLAYER_1      = -1575023,
    SAY_SACRIFICE_PLAYER_2      = -1575024,
    SAY_SACRIFICE_PLAYER_3      = -1575025,
    SAY_SACRIFICE_PLAYER_4      = -1575026,
    SAY_SACRIFICE_PLAYER_5      = -1575027,
    SAY_DIALOG_OF_ARTHAS_1      = -1575003,
    SAY_DIALOG_OF_ARTHAS_2      = -1575014
};

enum Creatures
{
    CREATURE_ARTHAS             = 29280, // Image of Arthas
    CREATURE_SVALA_SORROWGRAVE  = 26668, // Svala after transformation
    CREATURE_SVALA              = 29281, // Svala before transformation
    CREATURE_RITUAL_CHANNELER   = 27281
};

enum ChannelerSpells
{
    //ritual channeler's spells
    SPELL_PARALYZE              = 48278,
    SPELL_SHADOWS_IN_THE_DARK   = 59407
};

enum Misc
{
    DATA_SVALA_DISPLAY_ID = 25944
};

enum IntroPhase
{
    IDLE,
    INTRO,
    FINISHED
};

enum CombatPhase
{
    NORMAL,
    SACRIFICING
};

static Position RitualChannelerPos[]=
{
    {296.42f, -355.01f, 90.94f, 0.0f},
    {302.36f, -352.01f, 90.54f, 0.0f},
    {291.39f, -350.89f, 90.54f, 0.0f}
};

static Position ArthasPos = { 295.81f, -366.16f, 92.57f, 1.58f };
static Position SvalaPos = { 296.632f, -346.075f, 90.6307f, 1.58f };

class boss_svala : public CreatureScript
{
public:
    boss_svala() : CreatureScript("boss_svala") { }

    struct boss_svalaAI : public ScriptedAI
    {
        boss_svalaAI(Creature * c) : ScriptedAI(c)
        {
            instance = c->GetInstanceScript();
        }

        uint64 ArthasGUID;
        uint32 IntroTimer;
        uint8 IntroPhase;
        uint8 Phase;
        TempSummon * Arthas;
        InstanceScript * instance;

        void Reset()
        {
            Phase = IDLE;
            IntroTimer = 1 * IN_MILLISECONDS;
            IntroPhase = 0;
            ArthasGUID = 0;

            if (instance)
                instance->SetData(DATA_SVALA_SORROWGRAVE_EVENT, NOT_STARTED);
        }

        void MoveInLineOfSight(Unit * who)
        {
            if (!who || Phase != IDLE)
                return;

            if (me->IsWithinDistInMap(who, 50))
            {
                Phase = INTRO;
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

                if (Creature * Arthas = me->SummonCreature(CREATURE_ARTHAS, ArthasPos))
                {
                    Arthas->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    ArthasGUID = Arthas->GetGUID();
                }
            }
        }

        void AttackStart(Unit * /*who*/, float /*dist*/) { }

        void UpdateAI(const uint32 diff)
        {
            if (Phase != INTRO)
                return;

            if (instance && IntroTimer <= diff)
            {
                Creature * Arthas = instance->instance->GetCreature(ArthasGUID);

                if (!Arthas)
                    return;

                switch (IntroPhase)
                {
                    case 0:
                        DoScriptText(SAY_DIALOG_WITH_ARTHAS_1, me);
                        /*++IntroPhase;
                        IntroTimer = 8100;
                        Bis die Texte vorhanden/gefixt sind, überspringen! */
                        IntroPhase = 4;
                        IntroTimer = 1;
                        break;
                    case 1:
                        DoScriptText(SAY_DIALOG_OF_ARTHAS_1, Arthas);
                        ++IntroPhase;
                        IntroTimer = 18200;
                        break;
                    case 2:
                        DoScriptText(SAY_DIALOG_WITH_ARTHAS_2, me);
                        ++IntroPhase;
                        IntroTimer = SEKUNDEN_10;
                        break;
                    case 3:
                        DoScriptText(SAY_DIALOG_OF_ARTHAS_2, Arthas);
                        ++IntroPhase;
                        IntroTimer = 7200;
                        break;
                    case 4:
                        //DoScriptText(SAY_DIALOG_WITH_ARTHAS_3, me);
                        DoCast(me, SPELL_SVALA_TRANSFORMING1);
                        ++IntroPhase;
                        IntroTimer = 2800;
                        break;
                    case 5:
                        DoCast(me, SPELL_SVALA_TRANSFORMING2);
                        ++IntroPhase;
                        IntroTimer = 200;
                        break;
                    case 6:
                        if (me->SummonCreature(CREATURE_SVALA_SORROWGRAVE, SvalaPos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, MINUTEN_01))
                        {
                            Arthas->DespawnOrUnsummon();
                            ArthasGUID = 0;
                            Phase = FINISHED;
                            me->DisappearAndDie();
                        }
                        else
                            Reset();
                        break;
                }
            } else IntroTimer -= diff;
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_svalaAI (creature);
    }
};

class boss_svala_sorrowgrave : public CreatureScript
{
public:
    boss_svala_sorrowgrave() : CreatureScript("boss_svala_sorrowgrave") { }

    struct boss_svala_sorrowgraveAI : public ScriptedAI
    {
        boss_svala_sorrowgraveAI(Creature * c) : ScriptedAI(c), summons(c)
        {
            instance = c->GetInstanceScript();
        }

        uint32 SinsterStrikeTimer;
        uint32 CallFlamesTimer;
        uint32 RitualOfSwordTimer;
        uint32 SacrificeTimer;

        CombatPhase Phase;

        SummonList summons;

        bool Sacrificed;

        InstanceScript * instance;

        void Reset()
        {
            SinsterStrikeTimer = 7 * IN_MILLISECONDS;
            CallFlamesTimer = SEKUNDEN_10;
            RitualOfSwordTimer = SEKUNDEN_20;
            SacrificeTimer = 8 * IN_MILLISECONDS;

            Sacrificed = false;

            Phase = NORMAL;

            DoTeleportTo(296.632f, -346.075f, 90.6307f);
            me->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);

            summons.DespawnAll();

            if (instance)
            {
                instance->SetData(DATA_SVALA_SORROWGRAVE_EVENT, NOT_STARTED);
                instance->SetData64(DATA_SACRIFICED_PLAYER, 0);
            }
        }

        void EnterCombat(Unit * /*who*/)
        {
            DoScriptText(SAY_AGGRO, me);

            if (instance)
                instance->SetData(DATA_SVALA_SORROWGRAVE_EVENT, IN_PROGRESS);
        }

        void JustSummoned(Creature * summon)
        {
            summons.Summon(summon);
        }

        void SummonedCreatureDespawn(Creature * summon)
        {
            summons.Despawn(summon);
        }

        void KilledUnit(Unit * /*victim*/)
        {
            DoScriptText(RAND(SAY_SLAY_1, SAY_SLAY_2, SAY_SLAY_3), me);
        }

        void JustDied(Unit * killer)
        {
            if (instance && killer)
            {
                Creature * Svala = Unit::GetCreature((*me), instance->GetData64(DATA_SVALA));
                if (Svala && Svala->isAlive())
                    killer->Kill(Svala);

                instance->SetData(DATA_SVALA_SORROWGRAVE_EVENT, DONE);
            }
            DoScriptText(SAY_DEATH, me);
        }

        void UpdateAI(const uint32 diff)
        {
            if (Phase == NORMAL)
            {
                if (!UpdateVictim())
                    return;

                if (SinsterStrikeTimer <= diff)
                {
                    DoCast(me->getVictim(), SPELL_SINSTER_STRIKE);
                    SinsterStrikeTimer = urand(5 * IN_MILLISECONDS, 9 * IN_MILLISECONDS);
                } else SinsterStrikeTimer -= diff;

                if (CallFlamesTimer <= diff)
                {
                    if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                    {
                        DoCast(target, SPELL_CALL_FLAMES);
                        CallFlamesTimer = urand(8 * IN_MILLISECONDS, 12 * IN_MILLISECONDS);
                    }
                } else CallFlamesTimer -= diff;

                if (!Sacrificed)
                {
                    if (RitualOfSwordTimer <= diff)
                    {
                        if (Unit * SacrificeTarget = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            DoScriptText(RAND(SAY_SACRIFICE_PLAYER_1, SAY_SACRIFICE_PLAYER_2, SAY_SACRIFICE_PLAYER_3, SAY_SACRIFICE_PLAYER_4, SAY_SACRIFICE_PLAYER_5), me);
                            DoCast(SacrificeTarget, SPELL_RITUAL_OF_THE_SWORD);
                            // Spell doesn't teleport
                            DoTeleportPlayer(SacrificeTarget, 296.632f, -346.075f, 90.63f, 4.6f);
                            me->SetUnitMovementFlags(MOVEMENTFLAG_CAN_FLY);
                            DoTeleportTo(296.632f, -346.075f, 120.85f);
                            Phase = SACRIFICING;
                            if (instance)
                            {
                                instance->SetData64(DATA_SACRIFICED_PLAYER, SacrificeTarget->GetGUID());

                                for (uint8 i=0; i<3; ++i)
                                    if (Creature * summon = me->SummonCreature(CREATURE_RITUAL_CHANNELER, RitualChannelerPos[i], TEMPSUMMON_TIMED_OR_CORPSE_DESPAWN, 360000))
                                        summon->AI()->DoAction(0);
                            }
                            Sacrificed = true;
                        }
                    } else RitualOfSwordTimer -= diff;
                }
                DoMeleeAttackIfReady();
            }
            else  //SACRIFICING
            {
                if (SacrificeTimer <= diff)
                {
                    Unit * SacrificeTarget = instance ? Unit::GetUnit(*me, instance->GetData64(DATA_SACRIFICED_PLAYER)) : NULL;
                    if (instance && !summons.empty() && SacrificeTarget && SacrificeTarget->isAlive())
                        me->Kill(SacrificeTarget, false); // durability damage?
                    // go down
                    Phase = NORMAL;
                    SacrificeTarget = NULL;
                    me->SetUnitMovementFlags(MOVEMENTFLAG_WALKING);
                    if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        me->GetMotionMaster()->MoveChase(target);

                    SacrificeTimer = 8 * IN_MILLISECONDS;
                }
                else SacrificeTimer -= diff;
            }
        }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new boss_svala_sorrowgraveAI(creature);
    }
};

class mob_ritual_channeler : public CreatureScript
{
public:
    mob_ritual_channeler() : CreatureScript("mob_ritual_channeler") { }

    struct mob_ritual_channelerAI : public Scripted_NoMovementAI
    {
        mob_ritual_channelerAI(Creature * c) :Scripted_NoMovementAI(c)
        {
            instance = c->GetInstanceScript();
        }

        InstanceScript * instance;

        void Reset()
        {
            DoCast(me, SPELL_SHADOWS_IN_THE_DARK);
        }

        // called by svala sorrowgrave to set guid of victim
        void DoAction(const int32 /*action*/)
        {
            if (instance)
                if (Unit * victim = me->GetUnit(*me, instance->GetData64(DATA_SACRIFICED_PLAYER)))
                    DoCast(victim, SPELL_PARALYZE);
        }

        void EnterCombat(Unit * /*who*/) { }
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new mob_ritual_channelerAI(creature);
    }
};

void AddSC_boss_svala()
{
    new boss_svala();
    new mob_ritual_channeler();
    new boss_svala_sorrowgrave();
}
