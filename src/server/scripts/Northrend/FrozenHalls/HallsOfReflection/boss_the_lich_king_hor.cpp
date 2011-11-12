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

/* ScriptData
SDName: boss_Lich_king
SD%Complete: 0%
SDComment: new script for tc implementation.
SDCategory: Halls of Reflection
EndScriptData */

#include "ScriptPCH.h"
#include "halls_of_reflection.h"
#include "ScriptedEscortAI.h"

enum
{
   SPELL_WINTER                 = 69780,
   SPELL_FURY_OF_FROSTMOURNE    = 70063,
   SPELL_SOUL_REAPER            = 73797,
   SPELL_RAISE_DEAD             = 69818,
   SPELL_ICE_PRISON             = 69708,
   SPELL_DARK_ARROW             = 70194,
   SPELL_HARVEST_SOUL           = 70070,

   //Raging gnoul
   SPELL_EMERGE_VISUAL          = 50142,
   SPELL_GNOUL_JUMP             = 70150,

   //Witch Doctor
   SPELL_COURSE_OF_DOOM         = 70144,
   H_SPELL_COURSE_OF_DOOM       = 70183,
   SPELL_SHADOW_BOLT_VOLLEY     = 70145,
   H_SPELL_SHADOW_BOLT_VOLLEY   = 70184,
   SPELL_SHADOW_BOLT            = 70080,
   H_SPELL_SHADOW_BOLT          = 70182,

   //Lumbering Abomination
   SPELL_ABON_STRIKE            = 40505,
   SPELL_VOMIT_SPRAY            = 70176,
   H_SPELL_VOMIT_SPRAY          = 70181,

   SAY_LICH_KING_WALL_01        = -1594486,
   SAY_LICH_KING_WALL_02        = -1594491,
   SAY_LICH_KING_GNOUL          = -1594482,
   SAY_LICH_KING_ABON           = -1594483,
   SAY_LICH_KING_WINTER         = -1594481,
   SAY_LICH_KING_END_DUN        = -1594504,
   SAY_LICH_KING_WIN            = -1594485
};

class boss_lich_king_hor : public CreatureScript
{
public:
   boss_lich_king_hor() : CreatureScript("boss_lich_king_hor") { }

   struct boss_lich_king_horAI : public npc_escortAI
   {
       boss_lich_king_horAI(Creature * creature) : npc_escortAI(creature)
       {
           instance = creature->GetInstanceScript();
       }

       InstanceScript * instance;
       uint32 Step;
       uint32 StepTimer;
       uint32 wall;
       bool StartEscort;
       bool NonFight;

       void Reset()
       {
           NonFight = false;
           StartEscort = false;
           wall = 0;
       }

       void WaypointReached(uint32 i)
       {
           if (instance->GetData(DATA_ICE_WALL_1) == IN_PROGRESS)
           {
               wall = 1;
               SetEscortPaused(true);
           }

           if (instance->GetData(DATA_ICE_WALL_2) == IN_PROGRESS)
           {
               wall = 2;
               SetEscortPaused(true);
           }

           if (instance->GetData(DATA_ICE_WALL_3) == IN_PROGRESS)
           {
               wall = 3;
               SetEscortPaused(true);
           }

           if (instance->GetData(DATA_ICE_WALL_4) == IN_PROGRESS)
           {
               wall = 4;
               SetEscortPaused(true);
           }

           switch(i)
           {
               case 66:
                   SetEscortPaused(true);
                   instance->SetData(DATA_LICHKING_EVENT, SPECIAL);
                   DoScriptText(SAY_LICH_KING_END_DUN, me);
                   if (Creature * leader = ((Creature *)Unit::GetUnit((*me), instance->GetData64(DATA_ESCAPE_LEADER))))
                       me->CastSpell(leader, SPELL_HARVEST_SOUL, false);
                   me->setActive(false);
                   break;
           }

           if (!me->HasAura(SPELL_WINTER))
               DoCast(me, SPELL_WINTER);
       }

       void AttackStart(Unit * who, float /*dist*/)
       {
           if (!instance || !who)
               return;

           if (NonFight)
               return;

           if (instance->GetData(DATA_LICHKING_EVENT) == IN_PROGRESS || who->GetTypeId() == TYPEID_PLAYER)
               return;

           npc_escortAI::AttackStart(who);
       }

       void JustSummoned(Creature * summoned)
       {
           if (!instance || !summoned)
               return;

           summoned->setActive(true);
           summoned->SetInCombatWithZone();

           instance->SetData(DATA_SUMMONS, 1);
           if (Unit * leader = Unit::GetUnit((*me), instance->GetData64(DATA_ESCAPE_LEADER)))
           {
               summoned->GetMotionMaster()->MoveChase(leader);
               summoned->AddThreat(leader, 100.0f);
           }
       }

       void CallGuard(uint32 GuardID)
       {
           me->SummonCreature(GuardID, (me->GetPositionX()-5)+rand()%10, (me->GetPositionY()-5)+rand()%10, me->GetPositionZ(), 4.17f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, MINUTEN_05);
       }

       void Wall01()
       {
           switch(Step)
           {
               case 0:
                   instance->SetData(DATA_SUMMONS, 3);
                   DoScriptText(SAY_LICH_KING_WALL_01, me);
                   StepTimer = 2000;
                   ++Step;
                   break;
               case 1:
                   DoCast(me, SPELL_RAISE_DEAD);
                   DoScriptText(SAY_LICH_KING_GNOUL, me);
                   StepTimer = 7000;
                   ++Step;
                   break;
               case 2:
                   DoCast(me, SPELL_WINTER);
                   DoScriptText(SAY_LICH_KING_WINTER, me);
                   StepTimer = 1000;
                   ++Step;
                   break;
               case 3:
                   StepTimer = 2000;
                   ++Step;
                   break;
               case 4:
                   CallGuard(NPC_RISEN_WITCH_DOCTOR);
                   instance->SetData(DATA_ICE_WALL_1, DONE);
                   StepTimer = 100;
                   Step = 0;
                   wall = 0;
                   SetEscortPaused(false);
                   break;
           }
       }

       void Wall02()
       {
           switch(Step)
           {
               case 0:
                   instance->SetData(DATA_SUMMONS, 3);
                   DoScriptText(SAY_LICH_KING_GNOUL, me);
                   DoCast(me, SPELL_RAISE_DEAD);
                   StepTimer = 6000;
                   ++Step;
                   break;
               case 1:
                   CallGuard(NPC_RISEN_WITCH_DOCTOR);
                   CallGuard(NPC_RISEN_WITCH_DOCTOR);
                   CallGuard(NPC_ABON);
                   instance->SetData(DATA_ICE_WALL_2, DONE);
                   StepTimer = 5000;
                   Step = 0;
                   wall = 0;
                   SetEscortPaused(false);
                   break;
           }
       }

       void Wall03()
       {
           switch(Step)
           {
               case 0:
                   instance->SetData(DATA_SUMMONS, 3);
                   DoCast(me, SPELL_RAISE_DEAD);
                   DoScriptText(SAY_LICH_KING_GNOUL, me);
                   StepTimer = 6000;
                   ++Step;
                   break;
               case 1:
                   DoScriptText(SAY_LICH_KING_ABON, me);
                   CallGuard(NPC_RISEN_WITCH_DOCTOR);
                   CallGuard(NPC_RISEN_WITCH_DOCTOR);
                   CallGuard(NPC_RISEN_WITCH_DOCTOR);
                   CallGuard(NPC_ABON);
                   CallGuard(NPC_ABON);
                   instance->SetData(DATA_ICE_WALL_3, DONE);
                   StepTimer = 5000;
                   Step = 0;
                   wall = 0;
                   SetEscortPaused(false);
                   break;
           }
       }

       void Wall04()
       {
           switch(Step)
           {
               case 0:
                   instance->SetData(DATA_SUMMONS, 3);
                   DoCast(me, SPELL_RAISE_DEAD);
                   DoScriptText(SAY_LICH_KING_GNOUL, me);
                   StepTimer = 6000;
                   ++Step;
                   break;
               case 1:
                   CallGuard(NPC_RISEN_WITCH_DOCTOR);
                   CallGuard(NPC_RISEN_WITCH_DOCTOR);
                   CallGuard(NPC_RISEN_WITCH_DOCTOR);
                   CallGuard(NPC_ABON);
                   CallGuard(NPC_ABON);
                   StepTimer = 15000;
                   ++Step;
                   break;
               case 2:
                   DoScriptText(SAY_LICH_KING_ABON, me);
                   CallGuard(NPC_RISEN_WITCH_DOCTOR);
                   CallGuard(NPC_RISEN_WITCH_DOCTOR);
                   instance->SetData(DATA_ICE_WALL_4, DONE);
                   wall = 0;
                   SetEscortPaused(false);
                   ++Step;
                   break;
           }
       }

       void UpdateEscortAI(const uint32 diff)
       {
           if (!instance)
               return;

           if (instance->GetData(DATA_LICHKING_EVENT) == NOT_STARTED || instance->GetData(DATA_LICHKING_EVENT) == FAIL)
           {
               if (!UpdateVictim())
                   return;

               DoMeleeAttackIfReady();
           }

           if (me->isInCombat() && instance->GetData(DATA_LICHKING_EVENT) == IN_PROGRESS)
           {
               npc_escortAI::EnterEvadeMode();
           }

           // Start chase for leader
           if (instance->GetData(DATA_LICHKING_EVENT) == IN_PROGRESS && StartEscort != true)
           {
               StartEscort = true;
               me->RemoveAurasDueToSpell(SPELL_ICE_PRISON);
               me->RemoveAurasDueToSpell(SPELL_DARK_ARROW);
               me->setActive(true);
               NonFight = true;
               me->AttackStop();
               Start(false, false);
               Step = 0;
               StepTimer = 100;
           }

           // Leader caught, wipe
           if (Creature * leader = ObjectAccessor::GetCreature(*me, instance->GetData64(DATA_ESCAPE_LEADER)))
           {
               if (leader->IsWithinDistInMap(me, 2.0f) && instance->GetData(DATA_LICHKING_EVENT) == IN_PROGRESS)
               {
                   me->setActive(false);
                   SetEscortPaused(false);
                   me->StopMoving();
                   DoScriptText(SAY_LICH_KING_WIN, me);
                   me->CastSpell(me, SPELL_FURY_OF_FROSTMOURNE, false);
                   me->DealDamage(leader, leader->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
               }
           }

           if (wall == 1)
           {
               if (StepTimer < diff)
                   Wall01();
               else
                   StepTimer -= diff;
           }

           if (wall == 2)
           {
               if (StepTimer < diff)
                   Wall02();
               else
                   StepTimer -= diff;
           }

           if (wall == 3)
           {
               if (StepTimer < diff)
                   Wall03();
               else
                   StepTimer -= diff;
           }

           if (wall == 4)
           {
               if (StepTimer < diff)
                   Wall04();
               else
                   StepTimer -= diff;
           }
           return;
       }
   };

   CreatureAI * GetAI(Creature * creature) const
   {
       return new boss_lich_king_horAI(creature);
   }
};

class npc_raging_gnoul : public CreatureScript
{
public:
   npc_raging_gnoul() : CreatureScript("npc_raging_gnoul") { }

   struct npc_raging_gnoulAI : public ScriptedAI
   {
       npc_raging_gnoulAI(Creature * creature) : ScriptedAI(creature)
       {
           instance = creature->GetInstanceScript();
           me->setActive(true);
       }

       InstanceScript * instance;
       uint32 EmergeTimer;
       bool Emerge;
       bool Jumped;
       uint64 LeaderGUID;

       void Reset()
       {
           DoCast(me, SPELL_EMERGE_VISUAL);
           EmergeTimer = 4 * IN_MILLISECONDS;
           Emerge = false;
           Jumped = false;
       }

       void JustDied(Unit * /*killer*/)
       {
           if (!instance)
               return;

           instance->SetData(DATA_SUMMONS, 0);
       }

       void AttackStart(Unit * who, float /*dist*/)
       {
           if (!who)
               return;

           if (Emerge == false)
               return;

           ScriptedAI::AttackStart(who);
       }

       void UpdateAI(const uint32 diff)
       {
           if (!instance || me->HasUnitState(UNIT_STAT_CASTING))
               return;

           if (instance->GetData(DATA_LICHKING_EVENT) == IN_PROGRESS)
           {
               if (Emerge != true && EmergeTimer < diff)
               {
                   Emerge = true;
                   if (Unit * target = me->FindNearestPlayer(MAX_VISIBLE_DIST))
                       me->AI()->AttackStart(target);

                   DoZoneInCombat(me, MAX_VISIBLE_DIST);
               }
               else
                   EmergeTimer -= diff;

               if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f))
               {
                   if (Emerge && !Jumped && me->IsWithinDistInMap(target, 30.0f) && !me->IsWithinDistInMap(target, 5.0f))
                   {
                       Jumped = true;
                       DoCast(target, SPELL_GNOUL_JUMP);
                   }
               }
           }
           else if (instance->GetData(DATA_LICHKING_EVENT) == FAIL || instance->GetData(DATA_LICHKING_EVENT) == NOT_STARTED)
               me->DespawnOrUnsummon();

           DoMeleeAttackIfReady();
       }
   };

   CreatureAI * GetAI(Creature * creature) const
   {
       return new npc_raging_gnoulAI(creature);
   }

};

class npc_risen_witch_doctor : public CreatureScript
{
public:
   npc_risen_witch_doctor() : CreatureScript("npc_risen_witch_doctor") { }

   struct npc_risen_witch_doctorAI : public ScriptedAI
   {
       npc_risen_witch_doctorAI(Creature * creature) : ScriptedAI(creature)
       {
           instance = creature->GetInstanceScript();
           me->setActive(true);
       }

       InstanceScript * instance;
       uint32 EmergeTimer;
       bool Emerge;
       uint64 LeaderGUID;
       uint32 BoltTimer;
       uint32 BoltVolleyTimer;
       uint32 CurseTimer;

       void Reset()
       {
           DoCast(me, SPELL_EMERGE_VISUAL);
           EmergeTimer = SEKUNDEN_05;
           BoltTimer = 6 * IN_MILLISECONDS;
           BoltVolleyTimer = SEKUNDEN_15;
           CurseTimer = 7 * IN_MILLISECONDS;
           Emerge = false;
       }

       void JustDied(Unit * /*killer*/)
       {
           if (!instance)
               return;

           instance->SetData(DATA_SUMMONS, 0);

       }

       void AttackStart(Unit * who, float /*dist*/)
       {
           if (!who)
               return;

           if (Emerge == false)
               return;

           ScriptedAI::AttackStart(who);
       }

       void UpdateAI(const uint32 diff)
       {
           if (!instance || me->HasUnitState(UNIT_STAT_CASTING))
               return;

           if (instance->GetData(DATA_LICHKING_EVENT) == IN_PROGRESS)
           {
               if (Emerge != true && EmergeTimer < diff)
               {
                   Emerge = true;
                   if (Unit * target = me->FindNearestPlayer(MAX_VISIBLE_DIST))
                       me->AI()->AttackStart(target);

                   DoZoneInCombat(me, MAX_VISIBLE_DIST);
               }
               else
                   EmergeTimer -= diff;

               if (Emerge && CurseTimer < diff)
               {
                   if (Unit * target = SelectTarget(SELECT_TARGET_RANDOM))
                   {
                       if (me->GetDistance(target) <= 30.0f)
                       {
                           DoCast(target, SPELL_COURSE_OF_DOOM);
                           CurseTimer = urand(SEKUNDEN_10, SEKUNDEN_15);
                       }
                       else
                           CurseTimer = 2 * IN_MILLISECONDS;
                   }
               } else CurseTimer -= diff;

               if (Emerge && BoltTimer < diff)
               {
                   if (Unit * target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                   {
                       if (me->GetDistance(target) <= 30.0f)
                       {
                           DoCast(target, SPELL_SHADOW_BOLT);
                           BoltTimer = urand(2 * IN_MILLISECONDS, 3 * IN_MILLISECONDS);
                       }
                       else
                           BoltTimer = 2 * IN_MILLISECONDS;
                   }
               } else BoltTimer -= diff;

               if (Emerge && BoltVolleyTimer < diff)
               {
                   if (Unit * target = SelectTarget(SELECT_TARGET_TOPAGGRO, 0, 30.0f))
                   {
                       DoCast(target, SPELL_SHADOW_BOLT_VOLLEY);
                       BoltVolleyTimer = urand(SEKUNDEN_10, SEKUNDEN_20);
                   }
                   else
                       BoltVolleyTimer = 1 * IN_MILLISECONDS;
               } else BoltVolleyTimer -= diff;

           }
           else if (instance->GetData(DATA_LICHKING_EVENT) == FAIL || instance->GetData(DATA_LICHKING_EVENT) == NOT_STARTED)
               me->DespawnOrUnsummon();

           DoMeleeAttackIfReady();
       }
   };

   CreatureAI * GetAI(Creature * creature) const
   {
       return new npc_risen_witch_doctorAI(creature);
   }

};

class npc_abon : public CreatureScript
{
public:
   npc_abon() : CreatureScript("npc_abon") { }

   struct npc_abonAI : public ScriptedAI
   {
       npc_abonAI(Creature * creature) : ScriptedAI(creature)
       {
           instance = creature->GetInstanceScript();
           me->setActive(true);
       }

       InstanceScript * instance;
       uint64 LeaderGUID;
       bool Walk;
       uint32 StrikeTimer;
       uint32 VomitTimer;

       void Reset()
       {
           Walk = false;
           VomitTimer = SEKUNDEN_15;
           StrikeTimer = 6 * IN_MILLISECONDS;
       }

       void UpdateAI(const uint32 diff)
       {
           if (!instance || me->HasUnitState(UNIT_STAT_CASTING))
               return;

           if (instance->GetData(DATA_LICHKING_EVENT) == IN_PROGRESS)
           {
               if (Walk != true)
               {
                   Walk = true;
                   if (Unit * target = me->FindNearestPlayer(MAX_VISIBLE_DIST))
                       me->AI()->AttackStart(target);

                   DoZoneInCombat(me, MAX_VISIBLE_DIST);
               }

               if (StrikeTimer < diff)
               {
                   if (Unit * target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                   {
                       if (me->GetDistance(target) <= 5.0f)
                       {
                           DoCast(target, SPELL_ABON_STRIKE);
                           StrikeTimer = urand(7 * IN_MILLISECONDS, 9 * IN_MILLISECONDS);
                       }
                       else
                           StrikeTimer = 2 * IN_MILLISECONDS;
                   }
               } else StrikeTimer -= diff;

               if (VomitTimer < diff)
               {
                   if (Unit * target = SelectTarget(SELECT_TARGET_TOPAGGRO))
                   {
                       if (me->GetDistance(target) <= 10.0f)
                       {
                           DoCast(target, SPELL_VOMIT_SPRAY);
                           VomitTimer = urand(SEKUNDEN_10, SEKUNDEN_20);
                       }
                       else
                           VomitTimer = 2 * IN_MILLISECONDS;
                   }
               } else VomitTimer -= diff;
           }
           else if (instance->GetData(DATA_LICHKING_EVENT) == FAIL || instance->GetData(DATA_LICHKING_EVENT) == NOT_STARTED)
               me->DespawnOrUnsummon();

           DoMeleeAttackIfReady();
       }

       void JustDied(Unit * /*killer*/)
       {
           if (!instance)
               return;

           instance->SetData(DATA_SUMMONS, 0);
       }
   };

   CreatureAI * GetAI(Creature * creature) const
   {
       return new npc_abonAI(creature);
   }
};

void AddSC_boss_lich_king_hr()
{
   new boss_lich_king_hor();
   new npc_raging_gnoul();
   new npc_risen_witch_doctor();
   new npc_abon();
}
