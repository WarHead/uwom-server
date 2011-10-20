/*
 * Copyright (C) 2008-2011 by WarHead - United Worlds of MaNGOS - http://www.uwom.de
 */

#include "ScriptMgr.h"
#include "Chat.h"

class reset_commandscript : public CommandScript
{
public:
    reset_commandscript() : CommandScript("reset_commandscript") { }

    ChatCommand * GetCommands() const
    {
        static ChatCommand ResetCommandTable[] =
        {
            { "honorworld",     SEC_ADMINISTRATOR,  true,  &HandleResetHonorWorldCommand,   "", NULL },
            { "arenaworld",     SEC_ADMINISTRATOR,  true,  &HandleResetArenaWorldCommand,   "", NULL },

            { "achievements",   SEC_ADMINISTRATOR,  true,  &HandleResetAchievementsCommand, "", NULL },
            { "honor",          SEC_ADMINISTRATOR,  true,  &HandleResetHonorCommand,        "", NULL },
            { "level",          SEC_ADMINISTRATOR,  true,  &HandleResetLevelCommand,        "", NULL },
            { "spells",         SEC_ADMINISTRATOR,  true,  &HandleResetSpellsCommand,       "", NULL },
            { "stats",          SEC_ADMINISTRATOR,  true,  &HandleResetStatsCommand,        "", NULL },
            { "talents",        SEC_ADMINISTRATOR,  true,  &HandleResetTalentsCommand,      "", NULL },
            { "all",            SEC_ADMINISTRATOR,  true,  &HandleResetAllCommand,          "", NULL },
            { NULL,             0,                  false, NULL,                            "", NULL }
        };

        static ChatCommand commandTable[] =
        {
            { "reset",  SEC_ADMINISTRATOR,  true,  NULL,    "", ResetCommandTable },
            { NULL,     0,                  false, NULL,    "", NULL }
        };

        return commandTable;
    }

    static bool HandleResetArenaWorldCommand(ChatHandler * handler, const char * args)
    {
        const SessionMap SMap = sWorld->GetAllSessions();
        for (SessionMap::const_iterator itr = SMap.begin(); itr != SMap.end(); ++itr)
            if (Player * plr = (*itr).second->GetPlayer())
            {
                if (plr->GetSession() != handler->GetSession()) // Den Ausführenden nicht ausloggen, da es sonst zu einem Crash kommt!!!
                    plr->GetSession()->LogoutPlayer(true); // Alle online Spieler ausloggen und speichern, damit die Änderungen sofort aktiv sind!
                else
                {
                    plr->SetArenaPoints(0);
                    plr->GetSession()->LogoutRequest(time(NULL));
                }
            }

        SQLTransaction trans = CharacterDatabase.BeginTransaction();

        trans->PAppend("UPDATE `characters` SET `arenaPoints`=%u", sWorld->getIntConfig(CONFIG_START_ARENA_POINTS));

        std::string param = args;

        if (*args && param == "delete")
        {
            trans->PAppend("TRUNCATE `arena_team`");
            trans->PAppend("TRUNCATE `arena_team_member`");
            trans->PAppend("TRUNCATE `character_arena_stats`");
        }
        else
        {
            trans->PAppend("UPDATE `arena_team` SET `rating`=%u,`seasonGames`=0,`seasonWins`=0,`weekGames`=0,`weekWins`=0,`rank`=0", sWorld->getIntConfig(CONFIG_ARENA_START_RATING));
            trans->PAppend("UPDATE `arena_team_member` SET `personalRating`=%u,`weekGames`=0,`weekWins`=0,`seasonGames`=0,`seasonWins`=0", sWorld->getIntConfig(CONFIG_ARENA_START_PERSONAL_RATING));
            trans->PAppend("UPDATE `character_arena_stats` SET `matchMakerRating`=%u", sWorld->getIntConfig(CONFIG_ARENA_START_MATCHMAKER_RATING));
        }
        CharacterDatabase.CommitTransaction(trans);

        return true;
    }

    static bool HandleResetHonorWorldCommand(ChatHandler * handler, const char * args)
    {
        const SessionMap SMap = sWorld->GetAllSessions();
        for (SessionMap::const_iterator itr = SMap.begin(); itr != SMap.end(); ++itr)
            if (Player * plr = (*itr).second->GetPlayer())
            {
                if (plr->GetSession() != handler->GetSession()) // Den Ausführenden nicht ausloggen, da es sonst zu einem Crash kommt!!!
                    plr->GetSession()->LogoutPlayer(true); // Alle online Spieler ausloggen und speichern, damit die Änderungen sofort aktiv sind!
                else
                {
                    plr->SetHonorPoints(0);
                    plr->GetSession()->LogoutRequest(time(NULL));
                }
            }

        SQLTransaction trans = CharacterDatabase.BeginTransaction();
        trans->PAppend("UPDATE `characters` SET `totalHonorPoints`=%u,`todayHonorPoints`=0,`yesterdayHonorPoints`=0", sWorld->getIntConfig(CONFIG_START_HONOR_POINTS));

        std::string param = args;

        if (*args && param == "kills")
            trans->PAppend("UPDATE `characters` SET `totalKills`=0,`todayKills`=0,`yesterdayKills`=0");

        CharacterDatabase.CommitTransaction(trans);

        return true;
    }

    static bool HandleResetAchievementsCommand(ChatHandler * handler, const char * args)
    {
        Player* target;
        uint64 target_guid;
        if (!handler->extractPlayerTarget((char*)args, &target, &target_guid))
            return false;

        if (target)
            target->GetAchievementMgr().Reset();
        else
            AchievementMgr::DeleteFromDB(GUID_LOPART(target_guid));

        return true;
    }

    static bool HandleResetHonorCommand(ChatHandler * handler, const char * args)
    {
        Player* target;
        if (!handler->extractPlayerTarget((char*)args, &target))
            return false;

        target->SetHonorPoints(0);
        target->SetUInt32Value(PLAYER_FIELD_KILLS, 0);
        target->SetUInt32Value(PLAYER_FIELD_LIFETIME_HONORABLE_KILLS, 0);
        target->SetUInt32Value(PLAYER_FIELD_TODAY_CONTRIBUTION, 0);
        target->SetUInt32Value(PLAYER_FIELD_YESTERDAY_CONTRIBUTION, 0);
        target->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_EARN_HONORABLE_KILL);

        return true;
    }

    static bool HandleResetStatsOrLevelHelper(Player* player)
    {
        ChrClassesEntry const* cEntry = sChrClassesStore.LookupEntry(player->getClass());
        if (!cEntry)
        {
            sLog->outError("Class %u not found in DBC (Wrong DBC files?)", player->getClass());
            return false;
        }

        uint8 powertype = cEntry->powerType;

        // reset m_form if no aura
        if (!player->HasAuraType(SPELL_AURA_MOD_SHAPESHIFT))
            player->SetShapeshiftForm(FORM_NONE);

        player->SetFloatValue(UNIT_FIELD_BOUNDINGRADIUS, DEFAULT_WORLD_OBJECT_SIZE);
        player->SetFloatValue(UNIT_FIELD_COMBATREACH, DEFAULT_COMBAT_REACH);
        player->setFactionForRace(player->getRace());
        player->SetUInt32Value(UNIT_FIELD_BYTES_0, ((player->getRace()) | (player->getClass() << 8) | (player->getGender() << 16) | (powertype << 24)));

        // reset only if player not in some form;
        if (player->GetShapeshiftForm() == FORM_NONE)
            player->InitDisplayIds();

        player->SetByteValue(UNIT_FIELD_BYTES_2, 1, UNIT_BYTE2_FLAG_PVP);
        player->SetUInt32Value(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);
        //-1 is default value
        player->SetUInt32Value(PLAYER_FIELD_WATCHED_FACTION_INDEX, uint32(-1));
        //player->SetUInt32Value(PLAYER_FIELD_BYTES, 0xEEE00000);
        return true;
    }

    static bool HandleResetLevelCommand(ChatHandler * handler, const char * args)
    {
        Player* target;
        if (!handler->extractPlayerTarget((char*)args, &target))
            return false;

        if (!HandleResetStatsOrLevelHelper(target))
            return false;

        uint8 oldLevel = target->getLevel();
        // set starting level
        uint32 start_level = target->getClass() != CLASS_DEATH_KNIGHT ? sWorld->getIntConfig(CONFIG_START_PLAYER_LEVEL) : sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL);

        target->_ApplyAllLevelScaleItemMods(false);
        target->SetLevel(start_level);
        target->InitRunes();
        target->InitStatsForLevel(true);
        target->InitTaxiNodesForLevel();
        target->InitGlyphsForLevel();
        target->InitTalentForLevel();
        target->SetUInt32Value(PLAYER_XP, 0);
        target->_ApplyAllLevelScaleItemMods(true);

        // reset level for pet
        if (Pet* pet = target->GetPet())
            pet->SynchronizeLevelWithOwner();

        sScriptMgr->OnPlayerLevelChanged(target, oldLevel);

        return true;
    }

    static bool HandleResetStatsCommand(ChatHandler * handler, const char * args)
    {
        Player* target;
        if (!handler->extractPlayerTarget((char*)args, &target))
            return false;

        if (!HandleResetStatsOrLevelHelper(target))
            return false;

        target->InitRunes();
        target->InitStatsForLevel(true);
        target->InitTaxiNodesForLevel();
        target->InitGlyphsForLevel();
        target->InitTalentForLevel();

        return true;
    }

    static bool HandleResetSpellsCommand(ChatHandler * handler, const char * args)
    {
        Player* target;
        uint64 target_guid;
        std::string target_name;
        if (!handler->extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
            return false;

        if (target)
        {
            target->resetSpells(/* bool myClassOnly */);

            ChatHandler(target).SendSysMessage(LANG_RESET_SPELLS);
            if (!handler->GetSession() || handler->GetSession()->GetPlayer() != target)
                handler->PSendSysMessage(LANG_RESET_SPELLS_ONLINE, handler->GetNameLink(target).c_str());
        }
        else
        {
            CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'", uint32(AT_LOGIN_RESET_SPELLS), GUID_LOPART(target_guid));
            handler->PSendSysMessage(LANG_RESET_SPELLS_OFFLINE, target_name.c_str());
        }

        return true;
    }

    static bool HandleResetTalentsCommand(ChatHandler * handler, const char * args)
    {
        Player* target;
        uint64 target_guid;
        std::string target_name;
        if (!handler->extractPlayerTarget((char*)args, &target, &target_guid, &target_name))
        {
            // Try reset talents as Hunter Pet
            Creature* creature = handler->getSelectedCreature();
            if (!*args && creature && creature->isPet())
            {
                Unit* owner = creature->GetOwner();
                if (owner && owner->GetTypeId() == TYPEID_PLAYER && creature->ToPet()->IsPermanentPetFor(owner->ToPlayer()))
                {
                    creature->ToPet()->resetTalents();
                    owner->ToPlayer()->SendTalentsInfoData(true);

                    ChatHandler(owner->ToPlayer()).SendSysMessage(LANG_RESET_PET_TALENTS);
                    if (!handler->GetSession() || handler->GetSession()->GetPlayer() != owner->ToPlayer())
                        handler->PSendSysMessage(LANG_RESET_PET_TALENTS_ONLINE, handler->GetNameLink(owner->ToPlayer()).c_str());
                }
                return true;
            }

            handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (target)
        {
            target->resetTalents(true);
            target->SendTalentsInfoData(false);
            ChatHandler(target).SendSysMessage(LANG_RESET_TALENTS);
            if (!handler->GetSession() || handler->GetSession()->GetPlayer() != target)
                handler->PSendSysMessage(LANG_RESET_TALENTS_ONLINE, handler->GetNameLink(target).c_str());

            Pet* pet = target->GetPet();
            Pet::resetTalentsForAllPetsOf(target, pet);
            if (pet)
                target->SendTalentsInfoData(true);
            return true;
        }
        else if (target_guid)
        {
            uint32 at_flags = AT_LOGIN_NONE | AT_LOGIN_RESET_PET_TALENTS;
            CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE guid = '%u'", at_flags, GUID_LOPART(target_guid));
            std::string nameLink = handler->playerLink(target_name);
            handler->PSendSysMessage(LANG_RESET_TALENTS_OFFLINE, nameLink.c_str());
            return true;
        }

        handler->SendSysMessage(LANG_NO_CHAR_SELECTED);
        handler->SetSentErrorMessage(true);
        return false;
    }

    static bool HandleResetAllCommand(ChatHandler * handler, const char * args)
    {
        if (!*args)
            return false;

        std::string casename = args;

        AtLoginFlags atLogin;

        // Command specially created as single command to prevent using short case names
        if (casename == "spells")
        {
            atLogin = AT_LOGIN_RESET_SPELLS;
            sWorld->SendWorldText(LANG_RESETALL_SPELLS);
            if (!handler->GetSession())
                handler->SendSysMessage(LANG_RESETALL_SPELLS);
        }
        else if (casename == "talents")
        {
            atLogin = AtLoginFlags(AT_LOGIN_RESET_TALENTS | AT_LOGIN_RESET_PET_TALENTS);
            sWorld->SendWorldText(LANG_RESETALL_TALENTS);
            if (!handler->GetSession())
                handler->SendSysMessage(LANG_RESETALL_TALENTS);
        }
        else
        {
            handler->PSendSysMessage(LANG_RESETALL_UNKNOWN_CASE, args);
            handler->SetSentErrorMessage(true);
            return false;
        }

        CharacterDatabase.PExecute("UPDATE characters SET at_login = at_login | '%u' WHERE (at_login & '%u') = '0'", atLogin, atLogin);

        ACE_GUARD_RETURN(ACE_Thread_Mutex, guard, *HashMapHolder<Player>::GetLock(), true);
        HashMapHolder<Player>::MapType const& plist = sObjectAccessor->GetPlayers();
        for (HashMapHolder<Player>::MapType::const_iterator itr = plist.begin(); itr != plist.end(); ++itr)
            itr->second->SetAtLoginFlag(atLogin);

        return true;
    }
};

void AddSC_reset_commandscript()
{
    new reset_commandscript();
}
