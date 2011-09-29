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
#include "oculus.h"
#include "Vehicle.h"

#define GOSSIP_ITEM_DRAKES         "So where do we go from here?"
#define GOSSIP_ITEM_BELGARISTRASZ1 "I want to fly on the wings of the Red Flight"
#define GOSSIP_ITEM_BELGARISTRASZ2 "What abilities do Ruby Drakes have?"
#define GOSSIP_ITEM_VERDISA1       "I want to fly on the wings of the Green Flight"
#define GOSSIP_ITEM_VERDISA2       "What abilities do Emerald Drakes have?"
#define GOSSIP_ITEM_ETERNOS1       "I want to fly on the wings of the Bronze Flight"
#define GOSSIP_ITEM_ETERNOS2       "What abilities do Amber Drakes have?"

#define HAS_ESSENCE(a) ((a)->HasItemCount(ITEM_Smaragdessenz, 1) || (a)->HasItemCount(ITEM_Bernsteinessenz, 1) || (a)->HasItemCount(ITEM_Rubinessenz, 1))

enum DrakeGossips
{
    GOSSIP_TEXTID_DRAKES            = 13267,
    GOSSIP_TEXTID_BELGARISTRASZ1    = 12916,
    GOSSIP_TEXTID_BELGARISTRASZ2    = 13466,
    GOSSIP_TEXTID_BELGARISTRASZ3    = 13254,
    GOSSIP_TEXTID_VERDISA1          = 1,
    GOSSIP_TEXTID_VERDISA2          = 1,
    GOSSIP_TEXTID_VERDISA3          = 1,
    GOSSIP_TEXTID_ETERNOS1          = 1,
    GOSSIP_TEXTID_ETERNOS2          = 1,
    GOSSIP_TEXTID_ETERNOS3          = 13256
};

enum VehicleItem
{
    ITEM_Smaragdessenz      = 37815, // Grün = Heiler
    ITEM_Bernsteinessenz    = 37859, // Bronze = DD
    ITEM_Rubinessenz        = 37860  // Rot = Tank
};

enum VehicleNPCs
{
    NPC_Smaragddrache   = 27692, // Grün = Heiler
    NPC_Bernsteindrache = 27755, // Bronze = DD
    NPC_Rubindrache     = 27756  // Rot = Tank
};

class npc_oculus_drake : public CreatureScript
{
public:
    npc_oculus_drake() : CreatureScript("npc_oculus_drake") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*uiSender*/, uint32 uiAction)
    {
        player->PlayerTalkClass->ClearMenus();
        switch (creature->GetEntry())
        {
        case NPC_VERDISA: //Verdisa
            switch (uiAction)
            {
            case GOSSIP_ACTION_INFO_DEF + 1:
                if (!HAS_ESSENCE(player))
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_VERDISA1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_VERDISA2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_VERDISA1, creature->GetGUID());
                }
                else
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_VERDISA2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_VERDISA2, creature->GetGUID());
                }
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
            {
                ItemPosCountVec dest;
                uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_Smaragdessenz, 1);
                if (msg == EQUIP_ERR_OK)
                    player->StoreNewItem(dest, ITEM_Smaragdessenz, true);
                player->CLOSE_GOSSIP_MENU();
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 3:
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_VERDISA3, creature->GetGUID());
                break;
            }
            break;
        case NPC_BELGARISTRASZ: //Belgaristrasz
            switch (uiAction)
            {
            case GOSSIP_ACTION_INFO_DEF + 1:
                if (!HAS_ESSENCE(player))
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_BELGARISTRASZ1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_BELGARISTRASZ2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_BELGARISTRASZ1, creature->GetGUID());
                }
                else
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_BELGARISTRASZ2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_BELGARISTRASZ2, creature->GetGUID());
                }
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
            {
                ItemPosCountVec dest;
                uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_Rubinessenz, 1);
                if (msg == EQUIP_ERR_OK)
                    player->StoreNewItem(dest, ITEM_Rubinessenz, true);
                player->CLOSE_GOSSIP_MENU();
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 3:
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_BELGARISTRASZ3, creature->GetGUID());
                break;
            }
            break;
        case NPC_ETERNOS: //Eternos
            switch (uiAction)
            {
            case GOSSIP_ACTION_INFO_DEF + 1:
                if (!HAS_ESSENCE(player))
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ETERNOS1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ETERNOS2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ETERNOS1, creature->GetGUID());
                }
                else
                {
                    player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_ETERNOS2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ETERNOS2, creature->GetGUID());
                }
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
            {
                ItemPosCountVec dest;
                uint8 msg = player->CanStoreNewItem(NULL_BAG, NULL_SLOT, dest, ITEM_Bernsteinessenz, 1);
                if (msg == EQUIP_ERR_OK)
                    player->StoreNewItem(dest, ITEM_Bernsteinessenz, true);
                player->CLOSE_GOSSIP_MENU();
                break;
            }
            case GOSSIP_ACTION_INFO_DEF + 3:
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_ETERNOS3, creature->GetGUID());
                break;
            }
            break;
        }

        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature)
    {
        if (creature->isQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (InstanceScript* instance = creature->GetInstanceScript())
        {
            if (instance->GetBossState(DATA_DRAKOS_EVENT) == DONE)
            {
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_DRAKES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                player->SEND_GOSSIP_MENU(GOSSIP_TEXTID_DRAKES, creature->GetGUID());
            }
        }

        return true;
    }

};

class npc_oculus_vehicle_drachen : public CreatureScript
{
public:
    npc_oculus_vehicle_drachen() : CreatureScript("npc_oculus_vehicle_drachen") { }

    struct npc_oculus_vehicle_drachenAI : public VehicleAI
    {
        npc_oculus_vehicle_drachenAI(Creature * creature) : VehicleAI(creature)
        {
            me->SetSpeed(MOVE_FLIGHT, 2.0f, true);
            me->SetReactState(REACT_PASSIVE);
        }

        void Reset()
        {
            checktimer = 15 * IN_MILLISECONDS; // Am Anfang dem Spieler 15 Sek. (CD vom Item) Zeit zum aufsteigen geben
        }

        void JustDied(Unit * /*killer*/)
        {
            me->GetVehicleKit()->Dismiss();
        }

        bool CheckRider()
        {
            if (me->GetVehicleKit()->GetAvailableSeatCount() == 0)
                return true;

            if (Unit * passenger = me->GetVehicleKit()->GetPassenger(0))
                if (passenger->ToPlayer() && passenger->ToPlayer()->isValid())
                    return true;

            return false;
        }

        void UpdateAI(const uint32 diff)
        {
            if (checktimer && checktimer <= diff)
            {
                if (!CheckRider())
                {
                    me->GetVehicleKit()->Dismiss();
                    checktimer = 0;
                }
                else
                    checktimer = 5 * IN_MILLISECONDS;
            }
            else
                checktimer -= diff;
        }
    private:
        uint32 checktimer;
    };

    CreatureAI * GetAI(Creature * creature) const
    {
        return new npc_oculus_vehicle_drachenAI(creature);
    }
};

void AddSC_oculus()
{
    new npc_oculus_drake();
    new npc_oculus_vehicle_drachen();
}
