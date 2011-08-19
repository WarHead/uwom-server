/*
 * Copyright (c) 2008-2011 by WarHead - United Worlds of MaNGOS - http://www.uwom.de
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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

#include "UnitAI.h"
#include "Player.h"
#include "Creature.h"
#include "SpellAuras.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "SpellInfo.h"
#include "CreatureAIImpl.h"

// Angriffsart nach Art der Unit (Caster/Melee) starten - nutzen der Daten aus creature_template_caster
void UnitAI::AttackStart(Unit * victim, float dist)
{
    if (!victim)
        return;

    // AttackStartCaster() wurde aus einem Skript aufgerufen
    if (dist)
    {
        if (me->Attack(victim, false))
            me->GetMotionMaster()->MoveChase(victim, dist);
    }
    // Prüfen ob me ein Caster ist (creature_template_caster), und entsprechend handeln
    else if (me->ToCreature() && me->ToCreature()->isCaster())
    {
        if (me->Attack(victim, false))
            // Als Caster einfach stehen bleiben
            me->GetMotionMaster()->MoveIdle();
    }
    // AttackStart() wurde normal aufgerufen, und me ist nicht in creature_template_caster angegeben
    else if (me->Attack(victim, true))
        me->GetMotionMaster()->MoveChase(victim);
}

void UnitAI::DoMeleeAttackIfReady()
{
    if (me->HasUnitState(UNIT_STAT_CASTING))
        return;

    //Make sure our attack is ready and we aren't currently casting before checking distance
    if (me->isAttackReady())
    {
        //If we are within range melee the target
        if (me->IsWithinMeleeRange(me->getVictim()))
        {
            me->AttackerStateUpdate(me->getVictim());
            me->resetAttackTimer();
        }
    }
    if (me->haveOffhandWeapon() && me->isAttackReady(OFF_ATTACK))
    {
        //If we are within range melee the target
        if (me->IsWithinMeleeRange(me->getVictim()))
        {
            me->AttackerStateUpdate(me->getVictim(), OFF_ATTACK);
            me->resetAttackTimer(OFF_ATTACK);
        }
    }
}

bool UnitAI::DoSpellAttackIfReady(uint32 spell)
{
    if (me->HasUnitState(UNIT_STAT_CASTING))
        return true;

    if (me->isAttackReady())
    {
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spell))
        {
            if (me->IsWithinCombatRange(me->getVictim(), spellInfo->GetMaxRange(false)))
            {
                me->CastSpell(me->getVictim(), spell, false);
                me->resetAttackTimer();
            }
            else
                return false;
        }
        else
            return false;
    }
    return true;
}

Unit* UnitAI::SelectTarget(SelectAggroTarget targetType, uint32 position, float dist, bool playerOnly, int32 aura, bool alive)
{
    return SelectTarget(targetType, position, DefaultTargetSelector(me, dist, playerOnly, aura, alive));
}

void UnitAI::SelectTargetList(std::list<Unit*> &targetList, uint32 num, SelectAggroTarget targetType, float dist, bool playerOnly, int32 aura, bool alive)
{
    SelectTargetList(targetList, DefaultTargetSelector(me, dist, playerOnly, aura, alive), num, targetType);
}

float UnitAI::DoGetSpellMaxRange(uint32 spellId, bool positive)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    return spellInfo ? spellInfo->GetMaxRange(positive) : 0;
}

void UnitAI::DoAddAuraToAllHostilePlayers(uint32 spellid)
{
    if (me->isInCombat())
    {
        std::list<HostileReference*>& threatlist = me->getThreatManager().getThreatList();
        for (std::list<HostileReference*>::iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
        {
            if (Unit* unit = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
                if (unit->GetTypeId() == TYPEID_PLAYER)
                    me->AddAura(spellid, unit);
        }
    }else
        return;
}

void UnitAI::DoCastToAllHostilePlayers(uint32 spellid, bool triggered, bool alive)
{
    if (me->isInCombat())
    {
        std::list<HostileReference*>& threatlist = me->getThreatManager().getThreatList();
        for (std::list<HostileReference*>::iterator itr = threatlist.begin(); itr != threatlist.end(); ++itr)
        {
            if (Unit* unit = Unit::GetUnit(*me, (*itr)->getUnitGuid()))
                if (pTemp->GetTypeId() == TYPEID_PLAYER && pTemp->isAlive() == alive)
                    me->CastSpell(unit, spellid, triggered);
        }
    }else
        return;
}

void UnitAI::DoCast(uint32 spellId, bool alive)
{
    Unit* target = NULL;
    //sLog->outError("aggre %u %u", spellId, (uint32)AISpellInfo[spellId].target);
    switch(AISpellInfo[spellId].target)
    {
        default:
        case AITARGET_SELF:     target = me; break;
        case AITARGET_VICTIM:   target = me->getVictim(); break;
        case AITARGET_ENEMY:
        {
            const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            bool playerOnly = spellInfo->AttributesEx3 & SPELL_ATTR3_ONLY_TARGET_PLAYERS;
            //float range = GetSpellMaxRange(spellInfo, false);
            target = SelectTarget(SELECT_TARGET_RANDOM, 0, spellInfo->GetMaxRange(false), playerOnly);
            break;
        }
        case AITARGET_ALLY:     target = me; break;
        case AITARGET_BUFF:     target = me; break;
        case AITARGET_DEBUFF:
        {
            const SpellInfo* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            bool playerOnly = spellInfo->AttributesEx3 & SPELL_ATTR3_ONLY_TARGET_PLAYERS;
            float range = spellInfo->GetMaxRange(false);

            DefaultTargetSelector targetSelector(me, range, playerOnly, -(int32)spellId, alive);
            if (!(spellInfo->Attributes & SPELL_ATTR0_BREAKABLE_BY_DAMAGE)
                && !(spellInfo->AuraInterruptFlags & AURA_INTERRUPT_FLAG_NOT_VICTIM)
                && targetSelector(me->getVictim()))
                target = me->getVictim();
            else
                target = SelectTarget(SELECT_TARGET_RANDOM, 0, targetSelector);
            break;
        }
    }

    if (target)
        me->CastSpell(target, spellId, false);
}

#define UPDATE_TARGET(a) {if (AIInfo->target<a) AIInfo->target=a;}

void UnitAI::FillAISpellInfo()
{
    AISpellInfo = new AISpellInfoType[sSpellMgr->GetSpellInfoStoreSize()];

    AISpellInfoType* AIInfo = AISpellInfo;
    const SpellInfo* spellInfo;

    for (uint32 i = 0; i < sSpellMgr->GetSpellInfoStoreSize(); ++i, ++AIInfo)
    {
        spellInfo = sSpellMgr->GetSpellInfo(i);
        if (!spellInfo)
            continue;

        if (spellInfo->Attributes & SPELL_ATTR0_CASTABLE_WHILE_DEAD)
            AIInfo->condition = AICOND_DIE;
        else if (spellInfo->IsPassive() || spellInfo->GetDuration() == -1)
            AIInfo->condition = AICOND_AGGRO;
        else
            AIInfo->condition = AICOND_COMBAT;

        if (AIInfo->cooldown < spellInfo->RecoveryTime)
            AIInfo->cooldown = spellInfo->RecoveryTime;

        if (!spellInfo->GetMaxRange(false))
            UPDATE_TARGET(AITARGET_SELF)
        else
        {
            for (uint32 j = 0; j < MAX_SPELL_EFFECTS; ++j)
            {
                uint32 targetType = spellInfo->Effects[j].TargetA.GetTarget();

                if (targetType == TARGET_UNIT_TARGET_ENEMY
                    || targetType == TARGET_DEST_TARGET_ENEMY)
                    UPDATE_TARGET(AITARGET_VICTIM)
                else if (targetType == TARGET_UNIT_DEST_AREA_ENEMY)
                    UPDATE_TARGET(AITARGET_ENEMY)

                if (spellInfo->Effects[j].Effect == SPELL_EFFECT_APPLY_AURA)
                {
                    if (targetType == TARGET_UNIT_TARGET_ENEMY)
                        UPDATE_TARGET(AITARGET_DEBUFF)
                    else if (spellInfo->IsPositive())
                        UPDATE_TARGET(AITARGET_BUFF)
                }
            }
        }
        AIInfo->realCooldown = spellInfo->RecoveryTime + spellInfo->StartRecoveryTime;
        AIInfo->maxRange = spellInfo->GetMaxRange(false) * 3 / 4;
    }
}

//Enable PlayerAI when charmed
void PlayerAI::OnCharmed(bool apply) { me->IsAIEnabled = apply; }

void SimpleCharmedAI::UpdateAI(const uint32 /*diff*/)
{
  Creature* charmer = me->GetCharmer()->ToCreature();

    //kill self if charm aura has infinite duration
    if (charmer->IsInEvadeMode())
    {
        Unit::AuraEffectList const& auras = me->GetAuraEffectsByType(SPELL_AURA_MOD_CHARM);
        for (Unit::AuraEffectList::const_iterator iter = auras.begin(); iter != auras.end(); ++iter)
            if ((*iter)->GetCasterGUID() == charmer->GetGUID() && (*iter)->GetBase()->IsPermanent())
            {
                charmer->Kill(me);
                return;
            }
    }

    if (!charmer->isInCombat())
        me->GetMotionMaster()->MoveFollow(charmer, PET_FOLLOW_DIST, me->GetFollowAngle());

    Unit* target = me->getVictim();
    if (!target || !charmer->canAttack(target))
        AttackStart(charmer->SelectNearestTargetInAttackDistance());
}
