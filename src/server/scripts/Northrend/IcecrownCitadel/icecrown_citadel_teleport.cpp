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

#include "ScriptedGossip.h"
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "icecrown_citadel.h"
#include "Spell.h"
#include <AccountMgr.h>

#define GOSSIP_SENDER_ICC_PORT  631
#define GOSSIP_SENDER_ADMIN     1000

class icecrown_citadel_teleport : public GameObjectScript
{
public:
    icecrown_citadel_teleport() : GameObjectScript("icecrown_citadel_teleport") { }

    bool OnGossipHello(Player * player, GameObject * go)
    {
        if (InstanceScript * instance = go->GetInstanceScript())
        {
            if (AccountMgr::IsAdminAccount(player->GetSession()->GetSecurity()))
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "Schalte den Lich König frei!",             GOSSIP_SENDER_ADMIN, GOSSIP_ACTION_INFO_DEF+100);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "Schalte Sindragosa frei!",                 GOSSIP_SENDER_ADMIN, GOSSIP_ACTION_INFO_DEF+101);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "Schalte Blutkönigin Lana'thel frei!",      GOSSIP_SENDER_ADMIN, GOSSIP_ACTION_INFO_DEF+102);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "Schalte Professor Seuchenmord frei!",      GOSSIP_SENDER_ADMIN, GOSSIP_ACTION_INFO_DEF+103);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "Schalte Todesbringer Saurfang frei!",      GOSSIP_SENDER_ADMIN, GOSSIP_ACTION_INFO_DEF+104);
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_BATTLE, "Setze alle Encounter auf NOT_STARTED!!",   GOSSIP_SENDER_ADMIN, GOSSIP_ACTION_INFO_DEF+105);
            }

            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Der Hammer des Lichts.", GOSSIP_SENDER_ICC_PORT, LIGHT_S_HAMMER_TELEPORT);

            if (instance->GetBossState(DATA_LORD_MARROWGAR) == DONE)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Oratorium der Verdammten.", GOSSIP_SENDER_ICC_PORT, ORATORY_OF_THE_DAMNED_TELEPORT);
            if (instance->GetBossState(DATA_LADY_DEATHWHISPER) == DONE)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Das Schädelbollwerk.", GOSSIP_SENDER_ICC_PORT, RAMPART_OF_SKULLS_TELEPORT);
            if (instance->GetBossState(DATA_GUNSHIP_EVENT) == DONE)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Dom des Todesbringers.", GOSSIP_SENDER_ICC_PORT, DEATHBRINGER_S_RISE_TELEPORT);
            if (instance->GetData(DATA_COLDFLAME_JETS) == DONE)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Die obere Spitze.", GOSSIP_SENDER_ICC_PORT, UPPER_SPIRE_TELEPORT);
            // TODO: Gauntlet event before Sindragosa
            if (instance->GetBossState(DATA_VALITHRIA_DREAMWALKER) == DONE)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Der Hort der Frostkönigin.", GOSSIP_SENDER_ICC_PORT, SINDRAGOSA_S_LAIR_TELEPORT);
            if (instance->GetBossState(DATA_DEATHBRINGER_SAURFANG) == DONE &&
                instance->GetBossState(DATA_PROFESSOR_PUTRICIDE) == DONE &&
                instance->GetBossState(DATA_BLOOD_QUEEN_LANA_THEL) == DONE &&
                instance->GetBossState(DATA_SINDRAGOSA) == DONE)
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "Der Frostthron", GOSSIP_SENDER_ICC_PORT, FROZEN_THRONE_TELEPORT);
        }
        player->SEND_GOSSIP_MENU(player->GetGossipTextId(go), go->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player * player, GameObject * go, uint32 sender, uint32 action)
    {
        player->PlayerTalkClass->ClearMenus();
        player->CLOSE_GOSSIP_MENU();

        switch(sender)
        {
            case GOSSIP_SENDER_ADMIN:
                if (InstanceScript * instance = go->GetInstanceScript())
                {
                    switch(action)
                    {
                        case GOSSIP_ACTION_INFO_DEF+100:
                            for (uint8 i=0; i<12; ++i) // Bis einschließlich Sindragosa setzen
                                instance->SetBossState(i, DONE);
                            break;
                        case GOSSIP_ACTION_INFO_DEF+101:
                            for (uint8 i=0; i<4; ++i) // Bis einschließlich Saurfang setzen
                                instance->SetBossState(i, DONE);
                            instance->SetBossState(DATA_VALITHRIA_DREAMWALKER, DONE);
                            break;
                        case GOSSIP_ACTION_INFO_DEF+102:
                            for (uint8 i=0; i<4; ++i) // Bis einschließlich Saurfang setzen
                                instance->SetBossState(i, DONE);
                            instance->SetBossState(DATA_BLOOD_PRINCE_COUNCIL, DONE);
                            break;
                        case GOSSIP_ACTION_INFO_DEF+103:
                            for (uint8 i=0; i<6; ++i) // Bis einschließlich Modermiene setzen
                                instance->SetBossState(i, DONE);
                            break;
                        case GOSSIP_ACTION_INFO_DEF+104:
                            for (uint8 i=0; i<4; ++i) // Bis einschließlich Saurfang setzen
                                instance->SetBossState(i, DONE);
                            break;
                        case GOSSIP_ACTION_INFO_DEF+105:
                            for (uint8 i=0; i<=12; ++i) // Bis einschließlich dem Lich König setzen
                                instance->SetBossState(i, NOT_STARTED);
                            break;
                        default:
                            break;
                    }
                    return true;
                }
                break;
            case GOSSIP_SENDER_ICC_PORT:
                if (const SpellInfo * spell = sSpellMgr->GetSpellInfo(action))
                {
                    if (player->isInCombat())
                    {
                        Spell::SendCastResult(player, spell, 0, SPELL_FAILED_AFFECTING_COMBAT);
                        return true;
                    }
                    player->CastSpell(player, spell, true);
                    return true;
                }
                else
                    return false;
                break;
            default:
                break;
        }
        return false;
    }
};

class at_frozen_throne_teleport : public AreaTriggerScript
{
    public:
        at_frozen_throne_teleport() : AreaTriggerScript("at_frozen_throne_teleport") { }

        bool OnTrigger(Player* player, AreaTriggerEntry const* /*areaTrigger*/)
        {
            if (player->isInCombat())
            {
                if (SpellInfo const* spell = sSpellMgr->GetSpellInfo(FROZEN_THRONE_TELEPORT))
                    Spell::SendCastResult(player, spell, 0, SPELL_FAILED_AFFECTING_COMBAT);
                return true;
            }
            
            if (InstanceScript* instance = player->GetInstanceScript())
                if (instance->GetBossState(DATA_PROFESSOR_PUTRICIDE) == DONE &&
                    instance->GetBossState(DATA_BLOOD_QUEEN_LANA_THEL) == DONE &&
                    instance->GetBossState(DATA_SINDRAGOSA) == DONE &&
                    instance->GetBossState(DATA_THE_LICH_KING) != IN_PROGRESS)
                    player->CastSpell(player, FROZEN_THRONE_TELEPORT, true);

            return true;
        }
};

void AddSC_icecrown_citadel_teleport()
{
    new icecrown_citadel_teleport();
    new at_frozen_throne_teleport();
}
