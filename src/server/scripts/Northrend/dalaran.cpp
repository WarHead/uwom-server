/*
 * Copyright (C) 2008-2012 by WarHead - United Worlds of MaNGOS - http://www.uwom.de
 * Copyright (C) 2008-2012 TrinityCore <http://www.trinitycore.org/>
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
SDName: Dalaran
SDAuthor: WarHead, MaXiMiUS
SD%Complete: 99%
SDComment: For what is 63990+63991? Same function but don't work correct...
SDCategory: Dalaran
Script Data End */

#include "ScriptPCH.h"

// NPC IDs: 29254,29255

/*******************************************************
 * npc_mageguard_dalaran
 *******************************************************/

enum Spells
{
    SPELL_EINDRINGLING_A    = 54028,
    SPELL_EINDRINGLING_H    = 54029,

    SPELL_DETECTION = 18950,

    SPELL_SUNREAVER_DISGUISE_FEMALE        = 70973,
    SPELL_SUNREAVER_DISGUISE_MALE          = 70974,
    SPELL_SILVER_COVENANT_DISGUISE_FEMALE  = 70971,
    SPELL_SILVER_COVENANT_DISGUISE_MALE    = 70972
};

enum Texte
{
    TEXT_A  = -1010001,
    TEXT_H  = -1010000
};

enum NPCs
{
    ALLY_WACHE  = 29254,
    HORDE_WACHE = 29255
};

class npc_mageguard_dalaran : public CreatureScript
{
public:
    npc_mageguard_dalaran() : CreatureScript("npc_mageguard_dalaran") { }

    struct npc_mageguard_dalaranAI : public Scripted_NoMovementAI
    {
        npc_mageguard_dalaranAI(Creature* creature) : Scripted_NoMovementAI(creature)
        {
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_NORMAL, true);
            me->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_MAGIC, true);
        }

        void Reset()
        {
            me->CastSpell(me, SPELL_DETECTION, true);
        }

        void EnterCombat(Unit * /*who*/) { }
        void AttackStart(Unit * /*who*/, float /*dist*/) { }

        void MoveInLineOfSight(Unit * who)
        {
            if (!who)
                return;

            Player * player = who->GetCharmerOrOwnerPlayerOrPlayerItself();
            
            if (!player || !player->IsInWorld() || player->isGameMaster() || player->IsBeingTeleported() || me->GetDistance(player) > 12.0f ||
                // If player has Disguise aura for quest A Meeting With The Magister or An Audience With The Arcanist, do not teleport it away but let it pass
                player->HasAura(SPELL_SUNREAVER_DISGUISE_FEMALE) || player->HasAura(SPELL_SUNREAVER_DISGUISE_MALE) ||
                player->HasAura(SPELL_SILVER_COVENANT_DISGUISE_FEMALE) || player->HasAura(SPELL_SILVER_COVENANT_DISGUISE_MALE))
                return;

            switch(me->GetEntry())
            {
                case ALLY_WACHE:
                    if (!me->isInCombat() && player->GetTeam() == HORDE && !player->HasAura(SPELL_EINDRINGLING_A))
                    {
                        me->Yell(TEXT_A, LANG_UNIVERSAL, player->GetGUID());
                        DoCast(player, SPELL_EINDRINGLING_A);
                        EnterEvadeMode();
                        return;
                    }
                    break;
                case HORDE_WACHE:
                    if (!me->isInCombat() && player->GetTeam() == ALLIANCE && !player->HasAura(SPELL_EINDRINGLING_H))
                    {
                        me->Yell(TEXT_H, LANG_UNIVERSAL, player->GetGUID());
                        DoCast(player, SPELL_EINDRINGLING_H);
                        EnterEvadeMode();
                        return;
                    }
                    break;
            }
        }

        void UpdateAI(const uint32 /*diff*/) { }
    };

    CreatureAI *GetAI(Creature *creature) const
    {
        return new npc_mageguard_dalaranAI(creature);
    }
};

/*######
## npc_hira_snowdawn
######*/

enum eHiraSnowdawn
{
    SPELL_COLD_WEATHER_FLYING                   = 54197
};

#define GOSSIP_TEXT_TRAIN_HIRA "I seek training to ride a steed."

class npc_hira_snowdawn : public CreatureScript
{
public:
    npc_hira_snowdawn() : CreatureScript("npc_hira_snowdawn") { }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (!creature->isVendor() || !creature->isTrainer())
            return false;

        player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_TRAIN_HIRA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRAIN);

        if (player->getLevel() >= 80 && player->HasSpell(SPELL_COLD_WEATHER_FLYING))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
    {
        player->PlayerTalkClass->ClearMenus();
        if (uiAction == GOSSIP_ACTION_TRAIN)
            player->GetSession()->SendTrainerList(creature->GetGUID());

        if (uiAction == GOSSIP_ACTION_TRADE)
            player->GetSession()->SendListInventory(creature->GetGUID());

        return true;
    }
};

void AddSC_dalaran()
{
    new npc_mageguard_dalaran;
    new npc_hira_snowdawn;
}
