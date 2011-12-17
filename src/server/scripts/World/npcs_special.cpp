/*
 * Copyright (C) 2008-2011 by WarHead - United Worlds of MaNGOS - http://www.uwom.de
 * Copyright (C) 2008-2011 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2006-2009 ScriptDev2 <https://scriptdev2.svn.sourceforge.net/>
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
SDName: Npcs_Special
SD%Complete: 100
SDComment: To be used for special NPCs that are located globally.
SDCategory: NPCs
EndScriptData
*/

/* ContentData
npc_air_force_bots       80%    support for misc (invisible) guard bots in areas where plr allowed to fly. Summon guards after a preset time if tagged by spell
npc_lunaclaw_spirit      80%    support for quests 6001/6002 (Body and Heart)
npc_chicken_cluck       100%    support for quest 3861 (Cluck!)
npc_dancing_flames      100%    midsummer event NPC
npc_garments_of_quests   80%    NPC's related to all Garments of-quests 5621, 5624, 5625, 5648, 565
npc_injured_patient     100%    patients for triage-quests (6622 and 6624)
npc_doctor              100%    Gustaf Vanhowzen and Gregory Victor, quest 6622 and 6624 (Triage)
npc_kingdom_of_dalaran_quests   Misc NPC's gossip option related to quests 12791, 12794 and 12796
npc_mount_vendor        100%    Regular mount vendors all over the world. Display gossip if plr doesn't meet the requirements to buy
npc_rogue_trainer        80%    Scripted trainers, so they are able to offer item 17126 for class quest 6681
npc_sayge               100%    Darkmoon event fortune teller, buff plr based on answers given
npc_snake_trap_serpents  80%    AI for snakes that summoned by Snake Trap
npc_shadowfiend         100%   restore 5% of owner's mana when shadowfiend die from damage
npc_locksmith            75%    list of keys needs to be confirmed
EndContentData */

#include "ScriptPCH.h"
#include "ScriptedEscortAI.h"
#include "ObjectMgr.h"
#include "ScriptMgr.h"
#include "World.h"
#include "ReputationMgr.h"
#include "Config.h"
#include <AccountMgr.h>

enum HATI_ENUM
{
    SOUND_AGGRO = 11176,
    SOUND_SLAY1 = 11177,
    SOUND_SLAY2 = 11178,
    SOUND_SLAY3 = 11183,

    SPELL_DURCHBOHREN = 58666
};

enum WEHRLOS_ENUM
{
    //SPELL_ABSTOSSENDE_WELLE = 74509, // 50% + 75% HP - selbst
    SPELL_SCHOCKWELLE       = 75417, // 25% HP - victim
    SPELL_VERSENGEN         = 75412, // victim (30m)
    SPELL_RUESTUNG_SPALTEN  = 74367, // victim (5m)

    ACHIEVE_Der_4_Geburtstag_von_WoW    = 2398,
    ACHIEVE_Jadetiger                   = 3636,
    ACHIEVE_Der_5_Geburtstag_von_WoW    = 4400,

    EVENT_VERSENGEN         = 1,
    EVENT_RUESTUNG_SPALTEN  = 2,

    MAP_NORDEND = 571,

    MAX_HEALTH = 200000,

    DRUNK_VALUE = 25600
};

// ------------------------------------------------------------------------------------------------------------
// Robotron
// ------------------------------------------------------------------------------------------------------------
class npc_robotron : public CreatureScript
{
public:
    npc_robotron() : CreatureScript("npc_robotron") { }

    struct npc_robotronAI : public ScriptedAI
    {
        npc_robotronAI(Creature * cr) : ScriptedAI(cr)
        {
        }

        void Reset()
        {
        }
    };

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        if (cr->isQuestGiver()) plr->PrepareQuestMenu(cr->GetGUID());
        if (cr->isTrainer())    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_TRAIN,          GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRAIN);
        if (cr->isVendor())     plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    GOSSIP_TEXT_BROWSE_GOODS,   GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        if (cr->isAuctioner())  plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, GOSSIP_TEXT_AUCTIONHOUSE,   GOSSIP_SENDER_MAIN, GOSSIP_ACTION_AUCTION);

        plr->SEND_GOSSIP_MENU(14134, cr->GetGUID());

        return true;
    }

    void SendDefaultMenu(Player * /*plr*/, Creature * /*cr*/, uint32 /*action*/)
    {
    }

    void SendActionMenu(Player * /*plr*/, Creature * /*cr*/, uint32 /*action*/)
    {
    }

    bool OnGossipSelect(Player * /*plr*/, Creature * /*cr*/, uint32 /*sender*/, uint32 /*action*/)
    {
        return true;
    }

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_robotronAI(cr);
    }
};

// ------------------------------------------------------------------------------------------------------------
// Skript für die Wehrlosen :-) (Eichhörnchen / Mäuse etc.)
// ------------------------------------------------------------------------------------------------------------
class npc_schutz_den_wehrlosen : public CreatureScript
{
public:
    npc_schutz_den_wehrlosen() : CreatureScript("npc_schutz_den_wehrlosen") { }

    struct npc_schutz_den_wehrlosenAI : public ScriptedAI
    {
        npc_schutz_den_wehrlosenAI(Creature * cr) : ScriptedAI(cr), summons(me)
        {
            enabled = me->GetMapId() == MAP_NORDEND ? true : false;

            OrgHP = me->GetMaxHealth();
            OrgLvl = me->getLevel();
            OrgScale = me->GetFloatValue(OBJECT_FIELD_SCALE_X);
            OrgSpeed = me->GetSpeed(MOVE_RUN);

            PlrGUID = 0;

            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
        }

        void Reset()
        {
            if (enabled)
            {
                FirstTime = true;

                done75 = false;
                done50 = false;
                done25 = false;

                events.Reset();
                summons.DespawnAll();

                me->SetReactState(REACT_DEFENSIVE);
                me->SetMaxHealth(OrgHP);
                me->SetLevel(OrgLvl);
                me->SetFloatValue(OBJECT_FIELD_SCALE_X, OrgScale);
                me->SetSpeed(MOVE_RUN, OrgSpeed);

                if (PlrGUID)
                    if (Player * plr = ObjectAccessor::GetPlayer(*me, PlrGUID))
                    {
                        if (!plr->isAlive())
                            plr->ResurrectPlayer(100.0f);

                        plr->SetDrunkValue(0);
                    }

                PlrGUID = 0;
            }
        }

        void IsSummonedBy(Unit * summoner)
        {
            if (enabled && summoner)
                enabled = false;
        }

        void JustSummoned(Creature * summon)
        {
            summons.Summon(summon);
        }

        void SummonedCreatureDespawn(Creature * summon)
        {
            summons.Despawn(summon);
        }

        void EnterCombat(Unit * who)
        {
            if (!who || !who->IsInWorld())
                return;

            if (enabled)
            {
                DoPlaySoundToSet(me, SOUND_AGGRO);

                me->SetReactState(REACT_AGGRESSIVE);

                me->SetLevel(DEFAULT_MAX_LEVEL+3);
                me->SetFloatValue(OBJECT_FIELD_SCALE_X, 5.0f);
                me->SetMaxHealth(MAX_HEALTH);
                me->SetFullHealth();
                me->SetSpeed(MOVE_RUN, 2.5f);

                me->SetStatFloatValue(UNIT_FIELD_MINDAMAGE, 222.22f);
                me->SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, 333.33f);
                me->SetStatInt32Value(UNIT_FIELD_ATTACK_POWER, 666);
                me->SetStatFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, 99.99f);

                events.ScheduleEvent(EVENT_VERSENGEN, SEKUNDEN_05);
                events.ScheduleEvent(EVENT_RUESTUNG_SPALTEN, SEKUNDEN_05);

                if (Player * plr = who->ToPlayer())
                {
                    plr->SetDrunkValue(DRUNK_VALUE);
                    PlrGUID = plr->GetGUID();
                }

                for (uint8 i=0; i<6; ++i)
                {
                    Position pos;
                    me->GetRandomNearPosition(pos, 10.0f);

                    if (TempSummon * ts = me->SummonCreature(me->GetEntry(), pos, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, SEKUNDEN_60))
                    {
                        ts->SetLevel(DEFAULT_MAX_LEVEL+3);
                        ts->SetFloatValue(OBJECT_FIELD_SCALE_X, 2.5f);
                        ts->SetMaxHealth(MAX_HEALTH / 2);
                        ts->SetFullHealth();
                        ts->SetSpeed(MOVE_RUN, 2.5f);

                        ts->SetStatFloatValue(UNIT_FIELD_MINDAMAGE, 99.99f);
                        ts->SetStatFloatValue(UNIT_FIELD_MAXDAMAGE, 199.99f);
                        ts->SetStatInt32Value(UNIT_FIELD_ATTACK_POWER, 333);
                        ts->SetStatFloatValue(UNIT_FIELD_ATTACK_POWER_MULTIPLIER, 66.66f);

                        ts->AI()->AttackStart(me->getVictim());
                    }
                }
            }
            ScriptedAI::EnterCombat(who);
        }

        void DamageTaken(Unit * /*victim*/, uint32 & dmg)
        {
            if (enabled)
            {
                if (FirstTime)
                    dmg = 0;
                else
                    dmg = dmg / 50;

                FirstTime = false;

                if (!done75 && !me->HasUnitState(UNIT_STAT_CASTING) && me->HealthBelowPct(75))
                {
                    DoCastAOE(SPELL_SCHOCKWELLE);
                    done75 = true;
                }

                if (!done50 && !me->HasUnitState(UNIT_STAT_CASTING) && me->HealthBelowPct(50))
                {
                    DoCastAOE(SPELL_SCHOCKWELLE);
                    done50 = true;
                }

                if (!done25 && !me->HasUnitState(UNIT_STAT_CASTING) && me->HealthBelowPct(25))
                {
                    DoCastAOE(SPELL_SCHOCKWELLE);
                    done25 = true;
                }
            }
        }

        void KilledUnit(Unit * /*victim*/)
        {
            if (enabled)
            {
                switch(urand(0,2))
                {
                    case 0: DoPlaySoundToSet(me, SOUND_SLAY1); break;
                    case 1: DoPlaySoundToSet(me, SOUND_SLAY2); break;
                    case 2: DoPlaySoundToSet(me, SOUND_SLAY3); break;
                }
            }
        }

        void JustDied(Unit * /*killer*/)
        {
            if (enabled && PlrGUID)
                if (Player * plr = ObjectAccessor::GetPlayer(*me, PlrGUID))
                {
                    if (plr->isAlive())
                    {
                        AchievementEntry const * AE[3] =
                        {
                            GetAchievementStore()->LookupEntry(ACHIEVE_Der_4_Geburtstag_von_WoW),
                            GetAchievementStore()->LookupEntry(ACHIEVE_Jadetiger),
                            GetAchievementStore()->LookupEntry(ACHIEVE_Der_5_Geburtstag_von_WoW)
                        };

                        for (uint8 i=0; i<3; ++i)
                            if (AE[i] && !plr->GetAchievementMgr().HasAchieved(AE[i]->ID))
                            {
                                plr->CompletedAchievement(AE[i]);
                                break;
                            }
                    }
                    plr->SetDrunkValue(0);
                }
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim() || me->HasUnitState(UNIT_STAT_CASTING))
                return;

            if (enabled)
            {
                events.Update(diff);

                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch(eventId)
                    {
                        case EVENT_VERSENGEN:
                            DoCastVictim(SPELL_VERSENGEN);
                            events.RescheduleEvent(EVENT_VERSENGEN, SEKUNDEN_05);
                            break;
                        case EVENT_RUESTUNG_SPALTEN:
                            DoCastVictim(SPELL_RUESTUNG_SPALTEN);
                            events.RescheduleEvent(EVENT_RUESTUNG_SPALTEN, urand(SEKUNDEN_10, SEKUNDEN_20));
                            break;
                        default:
                            break;
                    }
                }
            }
            DoMeleeAttackIfReady();
        }
    private:
        bool enabled;
        bool done75;
        bool done50;
        bool done25;
        EventMap events;
        uint64 PlrGUID;
        uint32 OrgHP;
        uint8 OrgLvl;
        float OrgScale;
        float OrgSpeed;
        SummonList summons;
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_schutz_den_wehrlosenAI(cr);
    }
};

// ------------------------------------------------------------------------------------------------------------
// Feuerrufer 60000
// ------------------------------------------------------------------------------------------------------------
#define SPELL_RAKETENBUENDEL_ZUENDER    26299   // Zünder für Raketenbündel herstellen - 2 Secs. cast - NUR EINMAL BEIM START CASTEN!!!
#define MAX_GESCHENKE                   4       // Maximale Anzahl der zu vergebenden Geschenke

#define FirecallerSpellsCnt 15
const uint32 FirecallerSpells[FirecallerSpellsCnt] =
{
    6668,   // Rotes Feuerwerk - 2.5 Secs. cooldown
    11540,  // Blaues Feuerwerk - 2.5 Secs. cooldown
    11541,  // Grünes Feuerwerk - 2.5 Secs. cooldown
    11542,  // Rotes Streifen-Feuerwerk - 2.5 Secs. cooldown
    11543,  // Rot - Weiß - Blau Feuerwerk - 2.5 Secs. cooldown
    11544,  // Gelbe Rosen - Feuerwerk - 2.5 Secs. cooldown
    19823,  // Feuernova - Visual - kein Schaden
    30161,  // Lila Feuerwerk - 30 Secs. cooldown
    30237,  // Astralflimmern - Visual
    34602,  // Explosion - kein Schaden
    42075,  // Headless Horseman - Visual - Large Fire
    45153,  // Liebesrakete
    46235,  // Schwarzes Loch 2 - Visual
    46829,  // Ribbon Pole Firework and Flame Patch
    55420   // Feuerwerk von Dalaran - 5 Secs. Duration
};

enum FirecallerSpellsIdx
{
    ROTES_FEUERWERK = 0,
    BLAUES_FEUERWERK,
    GRUENES_FEUERWERK,
    ROTE_STREIFEN_FEUERWERK,
    ROT_WEISS_BLAU_FEUERWERK,
    GELBE_ROSEN_FEUERWERK,
    FEUERNOVA,
    LILA_FEUERWERK,
    ASTRALFLIMMERN,
    EXPLOSION,
    GROSSES_FEUER,
    LIEBESRAKETE,
    SCHWARZES_LOCH,
    RIBBON_POLE,
    DALARAN_FEUERWERK
};

#define FirecallerTargetSpellsCnt 4
const uint32 FirecallerTargetSpells[FirecallerTargetSpellsCnt] =
{
    45729,  // Bodenblüte - Reichweite 20Y (needs target!)
    45971,  // Bodenraketen - Reichweite 150Y - self oder target
    49872,  // Raketenschuss - 10-70Y - needs target!
    75419   // Versengen - 30Y - NUR GEGEN GM ANWENDEN!!! (needs target!)
};

enum FirecallerTargetSpellsIdx
{
    BODENBLUETE = 0,
    BODENRAKETE,
    RAKETENSCHUSS,
    VERSENGEN
};

#define FirecallerClusterCnt 15
const uint32 FirecallerCluster[FirecallerClusterCnt] =
{
    26304,  // Blaues Raketenbündel - 10 Secs. Duration
    26325,  // Grünes Raketenbündel - 10 Secs. Duration
    26326,  // Lila Raketenbündel - 10 Secs. Duration
    26327,  // Rotes Raketenbündel - 10 Secs. Duration
    26328,  // Weißes Raketenbündel - 10 Secs. Duration
    26329,  // Gelbes Raketenbündel - 10 Secs. Duration
    26488,  // Großes blaues Raketenbündel - 10 Secs. Duration
    26490,  // Großes grünes Raketenbündel - 10 Secs. Duration
    26516,  // Großes lila Raketenbündel - 10 Secs. Duration
    26517,  // Großes rotes Raketenbündel - 10 Secs. Duration
    26518,  // Großes weißes Raketenbündel - 10 Secs. Duration
    26519,  // Großes gelbes Raketenbündel - 10 Secs. Duration
    42813,  // Blaue Rakete von Theramore - 5 Secs. Duration
    42815,  // Gelbe Rakete von Theramore - 5 Secs. Duration
    42816   // Lila Rakete von Theramore - 5 Secs. Duration
};

enum FirecallerClusterIdx
{
    BLAU_CLUSTER = 0,
    GRUEN_CLUSTER,
    LILA_CLUSTER,
    ROT_CLUSTER,
    WEISS_CLUSTER,
    GELB_CLUSTER,
    BLAU_GROSS,
    GRUEN_GROSS,
    LILA_GROSS,
    ROT_GROSS,
    WEISS_GROSS,
    GELB_GROSS,
    RAKETE_BLAU,
    RAKETE_GELB,
    RAKETE_LILA
};

#define FirecallerJokesCnt 17
const uint32 FirecallerJokes[FirecallerJokesCnt] =
{
    24708,  // Piratenkostüm - 100Y - m
    24709,  // Piratenkostüm - 100Y - f
    24710,  // Ninjakostüm - 100Y - m
    24711,  // Ninjakostüm - 100Y - f
    24712,  // Lepragnomkostüm - 100Y - m
    24713,  // Lepragnomkostüm - 100Y - f
    24723,  // Skelettkostüm - 100Y
    24735,  // Geistkostüm - 100Y - m
    24736,  // Geistkostüm - 100Y - f
    24740,  // Irrwischkostüm - 100Y
    26157,  // PX-238 Winterwundervolt - self
    26272,  // PX-238 Winterwundervolt - self
    26273,  // PX-238 Winterwundervolt - self
    26274,  // PX-238 Winterwundervolt - self
    43906,  // Frosch im Hals - 40Y
    45684,  // Verwandlung: Pfiffi Wackelspross - self
    61781   // Truthahnfedern - 40Y
};

enum FirecallerJokesIdx
{
    PIRAT_MANN = 0,
    PIRAT_FRAU,
    NINJA_MANN,
    NINJA_FRAU,
    LEPRAGNOM_MANN,
    LEPRAGNOM_FRAU,
    SKELETT,
    GEIST_MANN,
    GEIST_FRAU,
    IRRWISCH,
    PX_238_1,
    PX_238_2,
    PX_238_3,
    PX_238_4,
    FROSCH_IM_HALS,
    PFIFFI_WACKELSPROSS,
    TRUTHAHN
};

#define FirecallerSoundsCnt 3
const uint32 FirecallerSounds[FirecallerSoundsCnt][5] =
{
    // Welcome
    {11966, 0, 0, 0, 0},
    // Abschied
    {11968, 0, 0, 0, 0},
    // Random
    {11962, 11965, 11967, 11975, 11976}
};

enum FirecallerSoundsIdx
{
    WILLKOMMEN = 0,
    ABSCHIED,
    ZUFAELLIG
};

enum FirecallerEvents
{
    EVENT_START = 1,
    EVENT_STOP,
    EVENT_SOUND,
    EVENT_CAST,
    EVENT_TARGET,
    EVENT_CLUSTER,
    EVENT_PRESENT_1,
    EVENT_PRESENT_2,
    EVENT_PRESENT_3,
    EVENT_PRESENT_4,
    EVENT_JOKE
};

/*
DROP TABLE IF EXISTS `feuerrufer`;
CREATE TABLE `feuerrufer` (
  `num` int(11) unsigned NOT NULL AUTO_INCREMENT COMMENT 'Nummer des Geschenkes',
  `char` tinytext NOT NULL COMMENT 'Charaktername',
  `item` int(11) unsigned NOT NULL COMMENT 'Das verschenkte Item',
  `link` varchar(256) NOT NULL COMMENT 'Link zu Wowhead für das Item',
  `zeit` int(11) unsigned NOT NULL COMMENT 'Zeitpunkt',
  `guid` int(11) unsigned NOT NULL COMMENT 'NPC GUID des Feuerrufers',
  PRIMARY KEY (`num`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Tabelle für die Geschenke des Feuerrufers';
*/
class npc_uwom_firecaller : public CreatureScript
{
public:
    npc_uwom_firecaller() : CreatureScript("npc_uwom_firecaller") { }

    struct npc_uwom_firecallerAI : public ScriptedAI
    {
        npc_uwom_firecallerAI(Creature * cr) : ScriptedAI(cr)
        {
            events.Reset();
            events.ScheduleEvent(EVENT_START, SEKUNDEN_60); // 1 Minute warten, bis zum Start.
            logfile = sWorld->GetDataPath().c_str();
            logfile.append("log/feuerrufer.log");
        }

        void Reset()
        {
        }

        void MoveInLineOfSight(Unit * who)
        {
            if (!who || !who->IsInWorld() || who->GetTypeId() != TYPEID_PLAYER)
                return;

            Player * chr = who->ToPlayer();
            if (!chr || !chr->IsInWorld())
                return;

            if (SpielerGUIDSet.find(chr->GetGUID()) == SpielerGUIDSet.end())
                SpielerGUIDSet.insert(chr->GetGUID());
        }

        void LadePetListe()
        {
            QueryResult result = WorldDatabase.PQuery("SELECT `entry`,`spellid_2` FROM `item_template` WHERE `class`=15 AND `subclass`=2 AND `spellid_2`!=0");
            if (!result)
            {
                sLog->outErrorDb("FEUERRUFER: Kann die Pets nicht laden!");
                return;
            }
            uint32 cnt = 0;

            do
            {
                Field * fields = result->Fetch();
                if (fields[0].GetUInt32() && fields[1].GetUInt32())
                {
                    PetListe[fields[0].GetUInt32()] = fields[1].GetUInt32();
                    ++cnt;
                }
            } while (result->NextRow());

            if (!EintragVorhanden("gültige Minipets"))
                SchreibeBericht(fmtstring("Es stehen zur Zeit %u gültige Minipets zur Verfügung.\n\n", cnt));
        }

        // Bericht schreiben, damit wir wissen, wer welches Geschenk bekommen hat. ;)
        void SchreibeBericht(std::string str)
        {
            if (FILE * reportfile = fopen(logfile.c_str(), "a"))
            {
                sLog->outString("FEUERUFER: (NPC-GUID %u) %s", me->GetGUIDLow(), str.c_str());
                fputs(str.c_str(), reportfile);
                fclose(reportfile);
            }
            else
            {
                sLog->outError("FEUERUFER: KANN %s NICHT SCHREIBEN!", logfile.c_str());
                sLog->outError("FEUERUFER: %s", str.c_str());
            }
        }

        bool EintragVorhanden(const char * eintrag)
        {
            if (FILE * reportfile = fopen(logfile.c_str(), "r+t"))
            {
                std::string tmpstr;

                while (!feof(reportfile))
                {
                    char tmpchar[256];
                    if (fgets(tmpchar, sizeof(tmpchar), reportfile))
                    {
                        tmpstr = tmpchar;
                        if (tmpstr.find(eintrag) != std::string::npos)
                            return true;
                    }
                }
            }
            return false;
        }

        // Anzahl der bereits vergebenen Geschenke ermitteln.
        uint8 BereitsVergeben()
        {
            if (FILE * reportfile = fopen(logfile.c_str(), "r+t"))
            {
                uint8 cnt = 0;
                std::string tmpstr;

                while (!feof(reportfile))
                {
                    char tmpchar[256];
                    if (fgets(tmpchar, sizeof(tmpchar), reportfile))
                    {
                        tmpstr = tmpchar;
                        if (tmpstr.find("http://de.wowhead.com/item=") != std::string::npos)
                            ++cnt;
                    }
                }
                fclose(reportfile);
                return cnt;
            }
            else
                sLog->outError("FEUERUFER: KANN %s NICHT LESEN!", logfile.c_str());

            return 0;
        }

        void ErstelleSema(uint8 num = 0)
        {
            std::string tmpfile = sWorld->GetDataPath().c_str();

            if (!num)
                tmpfile.append("event.run");
            else
            {
                char buffer[2];
                sprintf(buffer, "%u", num);
                tmpfile.append("geschenk.").append(buffer);
            }
            if (FILE * semafile = fopen(tmpfile.c_str(), "a"))
                fclose(semafile);
        }

        void LoescheSema(uint8 num)
        {
            std::string tmpfile = sWorld->GetDataPath().c_str();

            if (!num)
                tmpfile.append("event.run");
            else
            {
                char buffer[2];
                sprintf(buffer, "%u", num);
                tmpfile.append("geschenk.").append(buffer);
            }
            remove(tmpfile.c_str());
        }

        bool SemaVorhanden(uint8 num)
        {
            std::string tmpfile = sWorld->GetDataPath().c_str();
            char buffer[2];

            sprintf(buffer, "%u", num);
            tmpfile.append("geschenk.").append(buffer);

            if (FILE * semafile = fopen(tmpfile.c_str(), "r"))
            {
                fclose(semafile);
                return true;
            }
            return false;
        }

        Player * FindeSpieler(float range = 50.0f)
        {
            if (SpielerGUIDSet.empty())
                return NULL;

            for (std::set<uint64>::iterator itr = SpielerGUIDSet.begin(); itr != SpielerGUIDSet.end(); ++itr)
            {
                for (uint32 i=0; i<=urand(0, SpielerGUIDSet.size()-1); ++i)
                    if (++itr == SpielerGUIDSet.end())
                        itr = SpielerGUIDSet.begin();

                Player * chr = sObjectAccessor->FindPlayer(*itr);
                if (!chr || !chr->IsInWorld())
                {
                    SpielerGUIDSet.erase(itr);
                    continue;
                }
                if (chr->isAlive() && !chr->IsMounted() && me->IsWithinDistInMap(chr, range) && chr->isMoving() && !chr->isAFK())
                    return chr;
            }
            return NULL;
        }

        bool BeschenkeZiel(Player * chr, uint8 num)
        {
            if (!chr || !chr->IsInWorld() || !AccountMgr::IsPlayerAccount(chr->GetSession()->GetSecurity()))
                return false;

            if (BereitsVergeben() >= MAX_GESCHENKE || SemaVorhanden(num))
                return true; // Es wurden bereits alle Geschenke, oder diese Geschenknummer vergeben!

            for (std::map<uint32, uint32>::const_iterator itr = PetListe.begin(); itr != PetListe.end(); ++itr)
            {
                if (!chr->HasItemCount(itr->first, 1, true) && !chr->HasSpell(itr->second)) // Wenn der Spieler das zu gebende Item schon hat/kennt, nicht doppelt geben!
                {
                    char buffer[6];
                    time_t localtime = time(NULL);
                    sprintf(buffer, "%u", itr->first);

                    if (addItem(chr, itr->first))
                    {
                        // Bericht erstellen, damit wir wissen, wer welches Geschenk bekommen hat. ;)
                        SchreibeBericht(fmtstring("%s hat das Item http://de.wowhead.com/item=%s am %s Uhr von NPC-GUID %u erhalten.\n",
                                                  chr->GetName(), buffer, TimeToTimestampStr(localtime, GERMAN).c_str(), me->GetGUIDLow()));

                        // Die Daten in die Datenbank eintragen.
                        CharacterDatabase.DirectPExecute("INSERT INTO `feuerrufer` (`char`,`item`,`link`,`zeit`,`guid`) VALUES ('%s',%u,'%s',%u,%u)",
                            chr->GetName(), itr->first, fmtstring("http://de.wowhead.com/item=%s", buffer), uint32(localtime), me->GetGUIDLow());

                        return true;
                    }
                }
            }
            return false;
        }

        void VerzauberZiel(Player * chr)
        {
            if (!chr)
                return;

            uint8 i = urand(0,FirecallerJokesCnt-1);

            switch(i)
            {
                case PIRAT_MANN:
                case PIRAT_FRAU:
                case NINJA_MANN:
                case NINJA_FRAU:
                case LEPRAGNOM_MANN:
                case LEPRAGNOM_FRAU:
                case SKELETT:
                case GEIST_MANN:
                case GEIST_FRAU:
                case IRRWISCH:
                    for (uint8 j=0; j<FirecallerJokesCnt; ++j)
                        if (chr->HasAura(FirecallerJokes[j]))
                            chr->RemoveAurasDueToSpell(FirecallerJokes[j]);

                    if (me->GetDistance(chr) <= 100.0f)
                        DoCast(chr, FirecallerJokes[i], true);
                    break;
                case PX_238_1:
                case PX_238_2:
                case PX_238_3:
                case PX_238_4:
                case PFIFFI_WACKELSPROSS:
                    for (uint8 j=0; j<FirecallerJokesCnt; ++j)
                        if (chr->HasAura(FirecallerJokes[j]))
                            chr->RemoveAurasDueToSpell(FirecallerJokes[j]);

                    chr->CastSpell(chr, FirecallerJokes[i], true);
                    break;
                case FROSCH_IM_HALS:
                case TRUTHAHN:
                    for (uint8 j=0; j<FirecallerJokesCnt; ++j)
                        if (chr->HasAura(FirecallerJokes[j]))
                            chr->RemoveAurasDueToSpell(FirecallerJokes[j]);

                    if (me->GetDistance(chr) <= 40.0f)
                        DoCast(chr, FirecallerJokes[i], true);
                    break;
            }
        }

        void StartEvent()
        {
            ErstelleSema();

            if (!EintragVorhanden("Feuerrufer-Bericht"))
                SchreibeBericht("Feuerrufer-Bericht über die verschenkten Items. ;)\n\n");

            LadePetListe();

            DoPlaySoundToSet(me, FirecallerSounds[WILLKOMMEN][0]);

            DoCast(me, SPELL_RAKETENBUENDEL_ZUENDER, true);
            DoCast(FirecallerSpells[FEUERNOVA]);

            me->GetMotionMaster()->MoveRandom(10.0f);

            events.ScheduleEvent(EVENT_STOP, 29 * SEKUNDEN_60); // 29 Minuten vom Event übrig (30-1 Minute vor dem Start).
            events.ScheduleEvent(EVENT_SOUND, urand(SEKUNDEN_10, SEKUNDEN_60));
            events.ScheduleEvent(EVENT_CAST, 3 * IN_MILLISECONDS);
            events.ScheduleEvent(EVENT_TARGET, urand(SEKUNDEN_10, SEKUNDEN_60));
            events.ScheduleEvent(EVENT_CLUSTER, urand(SEKUNDEN_10, SEKUNDEN_60));
            events.ScheduleEvent(EVENT_PRESENT_1, urand(5 * SEKUNDEN_60, MINUTEN_05));
            events.ScheduleEvent(EVENT_JOKE, urand(SEKUNDEN_10, SEKUNDEN_60));
        }

        void StopEvent()
        {
            DoPlaySoundToSet(me, FirecallerSounds[ABSCHIED][0]);
            DoCast(FirecallerSpells[EXPLOSION]);

            if (!BereitsVergeben() && !EintragVorhanden("Leider war diesmal"))
                SchreibeBericht("Leider war diesmal niemand zum Event erschienen! :-(");

            me->SetTimeUntilDisappear(8 * IN_MILLISECONDS);

            for (uint8 i=0; i<=MAX_GESCHENKE; ++i)
                LoescheSema(i);
        }

        uint32 ZufallsZauberHolen()
        {
            uint32 Zauber = urand(0,FirecallerSpellsCnt-1);

            if (Zauber == FirecallerSpells[GROSSES_FEUER] || Zauber == FirecallerSpells[SCHWARZES_LOCH])
                me->RemoveAllAuras();

            return Zauber;
        }

        void UpdateAI(const uint32 diff)
        {
            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_START:
                        StartEvent();
                        break;
                    case EVENT_PRESENT_1:
                        if (!BeschenkeZiel(FindeSpieler(), 1))
                            events.RescheduleEvent(EVENT_PRESENT_1, urand(SEKUNDEN_10, SEKUNDEN_20));
                        else
                        {
                            ErstelleSema(1);
                            events.CancelEvent(EVENT_PRESENT_1);
                            events.ScheduleEvent(EVENT_PRESENT_2, urand(MINUTEN_05, MINUTEN_10));
                        }
                        break;
                    case EVENT_PRESENT_2:
                        if (!BeschenkeZiel(FindeSpieler(), 2))
                            events.RescheduleEvent(EVENT_PRESENT_2, urand(SEKUNDEN_10, SEKUNDEN_20));
                        else
                        {
                            ErstelleSema(2);
                            events.CancelEvent(EVENT_PRESENT_2);
                            events.ScheduleEvent(EVENT_PRESENT_3, urand(MINUTEN_05, MINUTEN_10));
                        }
                        break;
                    case EVENT_PRESENT_3:
                        if (!BeschenkeZiel(FindeSpieler(), 3))
                            events.RescheduleEvent(EVENT_PRESENT_3, urand(SEKUNDEN_10, SEKUNDEN_20));
                        else
                        {
                            ErstelleSema(3);
                            events.CancelEvent(EVENT_PRESENT_3);
                            events.ScheduleEvent(EVENT_PRESENT_4, urand(MINUTEN_05, MINUTEN_10));
                        }
                        break;
                    case EVENT_PRESENT_4:
                        if (!BeschenkeZiel(FindeSpieler(), 4))
                            events.RescheduleEvent(EVENT_PRESENT_4, urand(SEKUNDEN_10, SEKUNDEN_20));
                        else
                        {
                            ErstelleSema(4);
                            events.CancelEvent(EVENT_PRESENT_4);
                        }
                        break;
                    case EVENT_SOUND:
                        DoPlaySoundToSet(me, FirecallerSounds[ZUFAELLIG][urand(0,4)]);
                        events.RescheduleEvent(EVENT_SOUND, urand(SEKUNDEN_60, 3 * SEKUNDEN_60));
                        break;
                    case EVENT_CAST:
                        DoCast(FirecallerSpells[ZufallsZauberHolen()]);
                        events.RescheduleEvent(EVENT_CAST, 3 * IN_MILLISECONDS);
                        break;
                    case EVENT_TARGET:
                        if (Player * chr = FindeSpieler())
                        {
                            me->setFaction(14);
                            if (!AccountMgr::IsPlayerAccount(chr->GetSession()->GetSecurity()))
                                DoCast(chr, FirecallerTargetSpells[VERSENGEN]);
                            else
                                DoCast(chr, FirecallerTargetSpells[urand(BODENBLUETE, RAKETENSCHUSS)]);
                            me->setFaction(35);
                        }
                        events.RescheduleEvent(EVENT_TARGET, urand(SEKUNDEN_30, SEKUNDEN_60));
                        break;
                    case EVENT_CLUSTER:
                        DoCast(FirecallerCluster[urand(0,FirecallerClusterCnt-1)]);
                        events.RescheduleEvent(EVENT_CLUSTER, urand(SEKUNDEN_10, SEKUNDEN_30));
                        break;
                    case EVENT_JOKE:
                        VerzauberZiel(FindeSpieler());
                        events.RescheduleEvent(EVENT_JOKE, urand(SEKUNDEN_30, SEKUNDEN_60));
                        break;
                    case EVENT_STOP:
                        StopEvent();
                        break;
                }
            }
        }
        private:
            EventMap events;
            std::set<uint64> SpielerGUIDSet;
            std::map<uint32, uint32> PetListe;
            std::string logfile;
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_uwom_firecallerAI(cr);
    }
};

// ------------------------------------------------------------------------------------------------------------
// Flugmeister sowie Add Skripte - http://de.wowhead.com/npcs?filter=cr=21;crs=1;crv=0#0+1
// ------------------------------------------------------------------------------------------------------------
#define SPELL_NET   38661 // Immobilizes an enemy for 8 sec.

class npc_flugmeister : public CreatureScript
{
public:
    npc_flugmeister() : CreatureScript("npc_flugmeister") { }

    struct npc_flugmeisterAI : public ScriptedAI
    {
        npc_flugmeisterAI(Creature * cr) : ScriptedAI(cr)
        {
            switch(me->GetEntry())
            {
                // Surristrasz
                case 24795:
                    add1 = 26088;
                    add2 = 26088;
                    break;
                // Drachenfalkenmeister
                case 26560:
                case 30269:
                    add1 = 25175;
                    add2 = 25175;
                    break;
                // Fledermausführer
                case 2226:
                case 2389:
                case 4551:
                case 12636:
                case 16189:
                case 16192:
                case 24155:
                case 26844:
                case 26845:
                case 27344:
                    add1 = 14965;
                    add2 = 14965;
                    break;
                // Hippogryphenmeister
                case 1233:
                case 3838:
                case 3841:
                case 4267:
                case 4319:
                case 4407:
                case 6706:
                case 8019:
                case 10897:
                case 11138:
                case 12577:
                case 12578:
                case 15177:
                case 17554:
                case 17555:
                case 18785:
                case 18788:
                case 18789:
                case 18937:
                case 22485:
                case 22935:
                case 26881:
                case 30271:
                    add1 = 9527;
                    add2 = 9527;
                    break;
                // Greifenmeister
                case 352:
                case 523:
                case 931:
                case 1571:
                case 1572:
                case 1573:
                case 1574:
                case 1575:
                case 2299:
                case 2409:
                case 2432:
                case 2835:
                case 2859:
                case 2941:
                case 4321:
                case 7823:
                case 8018:
                case 8609:
                case 12596:
                case 12617:
                case 18939:
                case 16822:
                case 18809:
                case 18931:
                case 20234:
                case 21107:
                case 23736:
                case 23859:
                case 24061:
                case 24366:
                case 26876:
                case 26877:
                case 26878:
                case 26879:
                case 26880:
                    add1 = 9526;
                    add2 = 9526;
                    break;
                // Windreitermeister
                case 1387:
                case 2851:
                case 2858:
                case 2861:
                case 2995:
                case 3305:
                case 3310:
                case 3615:
                case 4312:
                case 4314:
                case 4317:
                case 6026:
                case 6726:
                case 7824:
                case 8020:
                case 8610:
                case 10378:
                case 11139:
                case 11899:
                case 11900:
                case 11901:
                case 12616:
                case 12740:
                case 13177:
                case 14242:
                case 15178:
                case 16587:
                case 18791:
                case 18807:
                case 18808:
                case 18930:
                case 18942:
                case 18953:
                case 19317:
                case 19558:
                case 20762:
                case 24032:
                case 26566:
                case 26847:
                case 26850:
                case 26852:
                case 26853:
                case 29762:
                    add1 = 20489;
                    add2 = 20489;
                    break;
                // Flugmeister
                case 10583:
                case 16227:
                case 18938:
                case 18940:
                case 19581:
                case 19583:
                case 20515:
                case 21766:
                case 22216:
                case 22455:
                case 22931:
                case 23612:
                case 24851:
                case 26602:
                case 26851:
                case 27046:
                case 28195:
                case 28196:
                case 28197:
                case 28574:
                case 28615:
                case 28618:
                case 28623:
                case 28624:
                case 28674:
                case 29480:
                case 29721:
                case 29750:
                case 29757:
                case 29950:
                case 29951:
                case 30314:
                case 30433:
                case 30569:
                case 30869:
                case 30870:
                case 31069:
                case 31078:
                case 32571:
                case 33849:
                case 37888:
                    add1 = 9526;
                    add2 = 20489;
                    break;
            }
        }

        uint32 add1,
            add2,
            Net_Timer;

        bool done;

        void Reset()
        {
            Net_Timer = 1000;
            done = false;

            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISARM, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_INTERRUPT_CAST, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SILENCE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DAZE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SLEEP, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_BANISH, true);
            me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NATURE, true);
        }

        void SpawnHelper()
        {
            uint32 DSpwTime = 10000;
            float X=0.0f, Y=0.0f, Z=0.0f, A=0.0f;
            TempSummonType DSpwType = TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT;

            for (uint8 i=0; i<10; ++i)
            {
                Creature * Spawned = NULL;
                me->GetContactPoint(me, X, Y, Z, float(urand(5,15)));

                switch(i)
                {
                    case 0: Spawned = me->SummonCreature(add1, X, Y, Z+float(urand(1,10)), A, DSpwType, DSpwTime); break;
                    case 1: Spawned = me->SummonCreature(add2, X, Y, Z+float(urand(1,5)), A, DSpwType, DSpwTime); break;
                    case 2: Spawned = me->SummonCreature(add1, X, Y, Z+float(urand(1,10)), A, DSpwType, DSpwTime); break;
                    case 3: Spawned = me->SummonCreature(add2, X, Y, Z+float(urand(1,5)), A, DSpwType, DSpwTime); break;
                    case 4: Spawned = me->SummonCreature(add1, X, Y, Z+float(urand(1,10)), A, DSpwType, DSpwTime); break;
                    case 5: Spawned = me->SummonCreature(add2, X, Y, Z+float(urand(1,5)), A, DSpwType, DSpwTime); break;
                    case 6: Spawned = me->SummonCreature(add1, X, Y, Z+float(urand(1,10)), A, DSpwType, DSpwTime); break;
                    case 7: Spawned = me->SummonCreature(add2, X, Y, Z+float(urand(1,5)), A, DSpwType, DSpwTime); break;
                    case 8: Spawned = me->SummonCreature(add1, X, Y, Z+float(urand(1,10)), A, DSpwType, DSpwTime); break;
                    case 9: Spawned = me->SummonCreature(add2, X, Y, Z+float(urand(1,5)), A, DSpwType, DSpwTime); break;
                }

                if (Spawned)
                {
                    Spawned->setFaction(me->getFaction());
                    Spawned->AI()->AttackStart(me->getVictim());
                }
            }
            done = true;
        }

        void MoveInLineOfSight(Unit * /*who*/) { return; }

        void AttackStart(Unit * who, float /*dist*/ = 0)
        {
            if (!done)
                SpawnHelper();

            ScriptedAI::AttackStart(who);
        }

        void UpdateAI(const uint32 diff)
        {
            //Return since we have no target
            if (!UpdateVictim())
                return;

            if (Net_Timer < diff)
            {
                DoCast(me->getVictim(), SPELL_NET);
                Net_Timer = 8000;
            }
            else
                Net_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        if (cr->isQuestGiver())
            plr->PrepareQuestMenu(cr->GetGUID());

        if (cr->isTrainer())
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_TEXT_TRAIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRAIN);

        if (cr->isVendor())
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        if (cr->isAuctioner())
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, GOSSIP_TEXT_AUCTIONHOUSE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_AUCTION);

        if (cr->isTaxi())
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TAXI, "Ich benötige einen Flug.", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TAXI);

        plr->SEND_GOSSIP_MENU(68, cr->GetGUID());

        return true;
    }

    void SendDefaultMenu(Player * plr, Creature * cr, uint32 action)
    {
        switch (action)
        {
            // Standard-Aktionen
            case GOSSIP_ACTION_TRAIN:
                plr->GetSession()->SendTrainerList(cr->GetGUID());
                break;
            case GOSSIP_ACTION_TRADE:
                plr->GetSession()->SendListInventory(cr->GetGUID());
                break;
            case GOSSIP_ACTION_AUCTION:
                plr->GetSession()->SendAuctionHello(cr->GetGUID(), cr);
                break;
            case GOSSIP_ACTION_TAXI:
                plr->GetSession()->SendTaxiMenu(cr);
                break;
        }
    }

    bool OnGossipSelect(Player * plr, Creature * cr, uint32 sender, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();

        switch(sender)
        {
            case GOSSIP_SENDER_MAIN: SendDefaultMenu(plr, cr, action); break;
        }
        return true;
    }

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_flugmeisterAI(cr);
    }
};

#define SPELL_WING_BUFFET   31475

class npc_flugmeister_adds : public CreatureScript
{
public:
    npc_flugmeister_adds() : CreatureScript("npc_flugmeister_adds") { }

    struct npc_flugmeister_addsAI : public ScriptedAI
    {
        npc_flugmeister_addsAI(Creature * cr) : ScriptedAI(cr) { }

        uint32 WingTimer;

        void Reset()
        {
            WingTimer = 10000;

            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISARM, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_INTERRUPT, true);
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_INTERRUPT_CAST, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SILENCE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DAZE, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SLEEP, true);
            me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_BANISH, true);
            me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NATURE, true);

            me->SetHover(true);
            me->SetSpeed(MOVE_FLIGHT, 1.50f, true);
            me->SetSpeed(MOVE_FLIGHT_BACK, 1.50f, true);
        }

        void UpdateAI(const uint32 diff)
        {
            //Return since we have no target
            if (!UpdateVictim())
                return;

            if (WingTimer < diff)
            {
                DoCast(me, SPELL_WING_BUFFET);
                WingTimer = urand(10000,60000);
            }
            else
                WingTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_flugmeister_addsAI(cr);
    }
};

void LearnAllSkillRecipes(Player * plr, uint32 skill_id)
{
    uint32 classmask = plr->getClassMask();

    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const *skillLine = sSkillLineAbilityStore.LookupEntry(j);
        if (!skillLine)
            continue;

        // wrong skill
        if (skillLine->skillId != skill_id)
            continue;

        // not high rank
        if (skillLine->forward_spellid)
            continue;

        // skip racial skills
        if (skillLine->racemask != 0)
            continue;

        // skip wrong class skills
        if (skillLine->classmask && (skillLine->classmask & classmask) == 0)
            continue;

        SpellEntry const * spellInfo = sSpellStore.LookupEntry(skillLine->spellId);
        if (!spellInfo)
            continue;

        plr->learnSpell(skillLine->spellId, false);
    }
}

// ------------------------------------------------------------------------------------------------------------
// UWoM's GM-Pimper 60001
// ------------------------------------------------------------------------------------------------------------

// To find all mount items...
// SELECT `entry`,`spellid_2` FROM `item_template` WHERE `class`=15 AND `subclass`=5 AND `spellid_2`!=0;

#define HordeFactionCnt 16
uint32 HordeFaction[HordeFactionCnt] = {530,729,1052,76,911,510,1067,941,1124,1064,947,81,922,68,1085,889};

#define AllyFactionCnt  17
uint32 AllyFaction[AllyFactionCnt] = {1037,69,930,1068,54,946,47,978,890,730,72,1126,509,1094,1050,471,589};

#define WorldFactionCnt 43
uint32 WorldFaction[WorldFactionCnt] = {1106,529,1012,87,21,910,609,942,909,577,1104,369,92,749,989,1090,1098,1011,93,1015,1038,470,349,1031,1077,809,970,70,932,933,1073,1105,990,934,935,1119,967,1091,59,576,270,1156,1094};

#define GOSSIP_GM_PIMPER_01 "Merkt Ihr denn nicht, dass ich genervt bin!? Oder könnt Ihr einfach nur nicht lesen? Ich bin für GM zuständig, nicht für Truthähnchen!"

#define GOSSIP_GM_PIMPER_02 "Ey Mann, was hast du so unter dem Ladentisch!?"
#define GOSSIP_GM_PIMPER_03 "Ein paar Jobs würden mir ganz gut tun."
#define GOSSIP_GM_PIMPER_11 "Ich möchte sooooooooo gerne gross und stark sein! *siehpimperschmachtendan*"
#define GOSSIP_GM_PIMPER_12 "Kannst du mit deinen Käsemessern nur rumfuchteln, oder kannste mir auch was beibringen, Drache?!"

#define GOSSIP_GM_PIMPER_04 "Kannste mir n bisschen Gold pumpen, Alter?"
#define GOSSIP_GM_PIMPER_05 "Ich bin neu hier, und habe noch nicht einmal vernünftige Taschen."
#define GOSSIP_GM_PIMPER_06 "Rück sofort alle deine Mini-Pets raus, oder es knallt!"
#define GOSSIP_GM_PIMPER_07 "Ich könnte da so das eine oder andere Mount für meine Fraktion gebrauchen!"
#define GOSSIP_GM_PIMPER_08 "Ey Drachen, sag mal... wie sieht's denn mit Mounts für beide Seiten aus!?"
#define GOSSIP_GM_PIMPER_09 "Meine Truppe und ich brauchen unbedingt mal wieder n paar Schrottis!"
#define GOSSIP_GM_PIMPER_10 "Also mein Ruf hier in der Gegend lässt ein wenig zu wünschen übrig."

#define MAX_GM_GOLD_ADD 1000000000

enum PIMPER_SPELL_ENUM
{
    SPELL_ALCHEMY_GRAND_MASTER          = 51304,
    SPELL_BLACKSMITHING_GRAND_MASTER    = 51300,
    SPELL_COOKING_GRAND_MASTER          = 51296,
    SPELL_ENCHANTING_GRAND_MASTER       = 51313,
    SPELL_ENGINEERING_GRAND_MASTER      = 51306,
    SPELL_FIRAT_AID_GRAND_MASTER        = 45542,
    SPELL_HERB_GATHERING_GRAND_MASTER   = 50300,
    SPELL_INSCRIPTION_GRAND_MASTER      = 45363,
    SPELL_JEWELCRAFTING_GRAND_MASTER    = 51311,
    SPELL_LEATHERWORKING_GRAND_MASTER   = 51302,
    SPELL_TAILORING_GRAND_MASTER        = 51309,
    SPELL_MINING_GRAND_MASTER           = 50310,
    SPELL_FISHING_GRAND_MASTER          = 51294,
    SPELL_SKINNING_GRAND_MASTER         = 50305,
    SPELL_COLD_WEATHER_FLYING_PASSIVE   = 54197,
    SPELL_TURKEY_FEATHERS               = 61781
};

enum PIMPER_ITEM_ENUM
{
    ITEM_FORORS_CRATE               = 23162,
    ITEM_SCRAPBOT_CONSTRUCTION_KIT  = 40769
};

class npc_uwom_gm_pimper : public CreatureScript
{
public:
    npc_uwom_gm_pimper() : CreatureScript("npc_uwom_gm_pimper")
    {
        LadePetListe();
        LadeHordeMountListe();
        LadeAllyMountListe();
        LadeBothMountListe();
    }

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        if (AccountMgr::IsPlayerAccount(plr->GetSession()->GetSecurity()))
        {
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_GM_PIMPER_01, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            plr->SEND_GOSSIP_MENU(13674, cr->GetGUID());
        }
        else
        {
            if (cr->isQuestGiver())  plr->PrepareQuestMenu(cr->GetGUID());
            if (cr->isTrainer())     plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_TRAIN,          GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRAIN);
            if (cr->isVendor())      plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    GOSSIP_TEXT_BROWSE_GOODS,   GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
            if (cr->isAuctioner())   plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, GOSSIP_TEXT_AUCTIONHOUSE,   GOSSIP_SENDER_MAIN, GOSSIP_ACTION_AUCTION);

            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    GOSSIP_GM_PIMPER_02,    GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_GM_PIMPER_03,    GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_GM_PIMPER_11,    GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_GM_PIMPER_12,    GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

            plr->SEND_GOSSIP_MENU(14141, cr->GetGUID());
        }
        return true;
    }

    void SendDefaultMenu(Player * plr, Creature * cr, uint32 action)
    {
        switch (action)
        {
            // User
            case GOSSIP_ACTION_INFO_DEF + 1:
                plr->CLOSE_GOSSIP_MENU();
                break;

            // Standard-Aktionen
            case GOSSIP_ACTION_TRAIN:
                plr->GetSession()->SendTrainerList(cr->GetGUID());
                break;
            case GOSSIP_ACTION_TRADE:
                plr->GetSession()->SendListInventory(cr->GetGUID());
                break;
            case GOSSIP_ACTION_AUCTION:
                cr->setFaction(plr->getFaction());
                plr->GetSession()->SendAuctionHello(cr->GetGUID(), cr);
                cr->setFaction(35);
                break;

            // Unterm Ladentisch
            case GOSSIP_ACTION_INFO_DEF + 2:
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, GOSSIP_GM_PIMPER_04,    GOSSIP_SENDER_SEC_BANK, GOSSIP_ACTION_INFO_DEF + 4);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    GOSSIP_GM_PIMPER_05,    GOSSIP_SENDER_SEC_BANK, GOSSIP_ACTION_INFO_DEF + 5);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    GOSSIP_GM_PIMPER_06,    GOSSIP_SENDER_SEC_BANK, GOSSIP_ACTION_INFO_DEF + 6);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    GOSSIP_GM_PIMPER_07,    GOSSIP_SENDER_SEC_BANK, GOSSIP_ACTION_INFO_DEF + 7);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    GOSSIP_GM_PIMPER_08,    GOSSIP_SENDER_SEC_BANK, GOSSIP_ACTION_INFO_DEF + 8);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    GOSSIP_GM_PIMPER_09,    GOSSIP_SENDER_SEC_BANK, GOSSIP_ACTION_INFO_DEF + 9);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_GM_PIMPER_10,    GOSSIP_SENDER_SEC_BANK, GOSSIP_ACTION_INFO_DEF + 10);
                plr->SEND_GOSSIP_MENU(14208, cr->GetGUID());
                break;

            // Ein paar Jobs
            case GOSSIP_ACTION_INFO_DEF + 3:
                if ((plr->GetFreePrimaryProfessionPoints() == 0 && AccountMgr::IsHGMAccount(plr->GetSession()->GetSecurity())) || plr->GetFreePrimaryProfessionPoints() > 0)
                {
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_ALCHEMY,        GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 11);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_BLACKSMITHING,  GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 12);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_COOKING,        GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 13);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_ENCHANTING,     GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 14);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_ENGINEERING,    GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 15);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_FIRSTAID,       GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 16);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_HERBALISM,      GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 17);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_INSCRIPTION,    GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 18);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_JEWELCRAFTING,  GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 19);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_LEATHERWORKING, GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 20);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_TAILORING,      GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 21);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_MINING,         GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 22);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_FISHING,        GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 23);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_SKINNING,       GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 24);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_RIDING,         GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 25);

                    if (AccountMgr::IsOGMAccount(plr->GetSession()->GetSecurity()))
                        plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_ALL_PROFS,      GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 26);

                    plr->SEND_GOSSIP_MENU(9331, cr->GetGUID());
                }
                else if (plr->GetFreePrimaryProfessionPoints() == 0 && !AccountMgr::IsHGMAccount(plr->GetSession()->GetSecurity()))
                {
                    plr->GetSession()->SendNotification("Dein GM-Level erlaubt nicht mehr primäre Berufe.");

                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_COOKING,        GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 13);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_FIRSTAID,       GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 16);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_FISHING,        GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 23);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_RIDING,         GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 25);

                    plr->SEND_GOSSIP_MENU(9331, cr->GetGUID());
                }
                break;

            // Leveln
            case GOSSIP_ACTION_INFO_DEF + 4:
                plr->GiveLevel(DEFAULT_MAX_LEVEL);
                plr->CLOSE_GOSSIP_MENU();
                break;

            // Maxskill
            case GOSSIP_ACTION_INFO_DEF + 5:
                plr->UpdateSkillsToMaxSkillsForLevel();
                plr->CLOSE_GOSSIP_MENU();
            break;
        }
    }

    void LadePetListe()
    {
        QueryResult result = WorldDatabase.PQuery("SELECT `entry`,`spellid_2` FROM `item_template` WHERE `class`=15 AND `subclass`=2 AND `spellid_2`!=0");
        if (!result)
        {
            sLog->outErrorDb("GM-PIMPER: Kann die Minipets nicht laden!");
            return;
        }

        uint32 cnt = 0;

        do
        {
            Field * fields = result->Fetch();
            if (fields[0].GetUInt32() && fields[1].GetUInt32())
            {
                PetListe[fields[0].GetUInt32()] = fields[1].GetUInt32();
                ++cnt;
            }
        } while (result->NextRow());

        sLog->outDetail("GM-PIMPER: Habe %u gültige Minipets gefunden und geladen.", cnt);
    }

    void LadeHordeMountListe()
    {
        QueryResult result = WorldDatabase.PQuery("SELECT `entry`,`spellid_2` FROM `item_template` WHERE `class`=15 AND `subclass`=5 AND `spellid_2`!=0 AND `AllowableRace`=690");
        if (!result)
        {
            sLog->outErrorDb("GM-PIMPER: Kann die Hordemounts nicht laden!");
            return;
        }

        uint32 cnt = 0;

        do
        {
            Field * fields = result->Fetch();
            if (fields[0].GetUInt32() && fields[1].GetUInt32())
            {
                HordeMountListe[fields[0].GetUInt32()] = fields[1].GetUInt32();
                ++cnt;
            }
        } while (result->NextRow());

        sLog->outDetail("GM-PIMPER: Habe %u gültige Hordemounts gefunden und geladen.", cnt);
    }

    void LadeAllyMountListe()
    {
        QueryResult result = WorldDatabase.PQuery("SELECT `entry`,`spellid_2` FROM `item_template` WHERE `class`=15 AND `subclass`=5 AND `spellid_2`!=0 AND `AllowableRace`=1101");
        if (!result)
        {
            sLog->outErrorDb("GM-PIMPER: Kann die Allymounts nicht laden!");
            return;
        }

        uint32 cnt = 0;

        do
        {
            Field * fields = result->Fetch();
            if (fields[0].GetUInt32() && fields[1].GetUInt32())
            {
                AllyMountListe[fields[0].GetUInt32()] = fields[1].GetUInt32();
                ++cnt;
            }
        } while (result->NextRow());

        sLog->outDetail("GM-PIMPER: Habe %u gültige Allymounts gefunden und geladen.", cnt);
    }

    void LadeBothMountListe()
    {
        QueryResult result = WorldDatabase.PQuery("SELECT `entry`,`spellid_2` FROM `item_template` WHERE `class`=15 AND `subclass`=5 AND `spellid_2`!=0 AND `AllowableRace`=-1");
        if (!result)
        {
            sLog->outErrorDb("GM-PIMPER: Kann die Mounts für beide Seiten nicht laden!");
            return;
        }

        uint32 cnt = 0;

        do
        {
            Field * fields = result->Fetch();
            if (fields[0].GetUInt32() && fields[1].GetUInt32())
            {
                BothMountListe[fields[0].GetUInt32()] = fields[1].GetUInt32();
                ++cnt;
            }
        } while (result->NextRow());

        sLog->outDetail("GM-PIMPER: Habe %u gültige Mounts für beide Seiten gefunden und geladen.", cnt);
    }

    void SendActionMenu(Player * plr, Creature * cr, uint32 action)
    {
        switch(action)
        {
            // Gold
            case GOSSIP_ACTION_INFO_DEF + 4:
                if (!AccountMgr::IsHGMAccount(plr->GetSession()->GetSecurity()) && plr->GetMoney() >= MAX_GM_GOLD_ADD)
                    plr->GetSession()->SendNotification("Mit deinem GM-Level bekommst du nicht mehr Gold.");
                else
                    plr->ModifyMoney(+MAX_GM_GOLD_ADD); // 100K Gold
                plr->CLOSE_GOSSIP_MENU();
                break;

            // Koffer
            case GOSSIP_ACTION_INFO_DEF + 5:
                if (plr->HasItemCount(ITEM_FORORS_CRATE, 11, true))
                    plr->GetSession()->SendNotification("Du hast bereits 11 GM-Koffer!");
                else
                    ((ScriptedAI*)cr)->addItem(plr, ITEM_FORORS_CRATE, 11); // 11 Koffer
                plr->CLOSE_GOSSIP_MENU();
                break;

            // Mini-Pets
            case GOSSIP_ACTION_INFO_DEF + 6:
                for (std::map<uint32, uint32>::const_iterator itr = PetListe.begin(); itr != PetListe.end(); ++itr)
                {
                    if (!plr->HasSpell(itr->second))
                        plr->learnSpell(itr->second, false);
                }
                plr->CLOSE_GOSSIP_MENU();
                break;

            // Mounts (Ally/Horde)
            case GOSSIP_ACTION_INFO_DEF + 7:
                if (plr->GetTeam() == ALLIANCE)
                {
                    for (std::map<uint32, uint32>::const_iterator itr = AllyMountListe.begin(); itr != AllyMountListe.end(); ++itr)
                    {
                        if (!plr->HasSpell(itr->second))
                            plr->learnSpell(itr->second, false);
                    }
                }
                else
                {
                    for (std::map<uint32, uint32>::const_iterator itr = HordeMountListe.begin(); itr != HordeMountListe.end(); ++itr)
                    {
                        if (!plr->HasSpell(itr->second))
                            plr->learnSpell(itr->second, false);
                    }
                }
                plr->CLOSE_GOSSIP_MENU();
                break;

            // Mounts (Both Side)
            case GOSSIP_ACTION_INFO_DEF + 8:
                for (std::map<uint32, uint32>::const_iterator itr = BothMountListe.begin(); itr != BothMountListe.end(); ++itr)
                {
                    if (!plr->HasSpell(itr->second))
                        plr->learnSpell(itr->second, false);
                }
                plr->CLOSE_GOSSIP_MENU();
                break;

            // Schrottis
            case GOSSIP_ACTION_INFO_DEF + 9:
                if (plr->HasItemCount(ITEM_SCRAPBOT_CONSTRUCTION_KIT, 40, true))
                    plr->GetSession()->SendNotification("Mehr als 40 Schrottis gibbet nit.");
                else
                    ((ScriptedAI*)cr)->addItem(plr, ITEM_SCRAPBOT_CONSTRUCTION_KIT, 20); // 20 Schrottis
                plr->CLOSE_GOSSIP_MENU();
                break;

            // Ruf
            case GOSSIP_ACTION_INFO_DEF + 10:
                if (plr->GetTeam() == ALLIANCE)
                {
                    for (uint8 i=0; i<AllyFactionCnt; ++i)
                    {
                        FactionEntry const * fe = sFactionStore.LookupEntry(AllyFaction[i]);
                        plr->GetReputationMgr().ModifyReputation(fe, ReputationMgr::Reputation_Cap);
                    }
                }
                else
                {
                    for (uint8 i=0; i<HordeFactionCnt; ++i)
                    {
                        FactionEntry const * fe = sFactionStore.LookupEntry(HordeFaction[i]);
                        plr->GetReputationMgr().ModifyReputation(fe, ReputationMgr::Reputation_Cap);
                    }
                }
                for (uint8 i=0; i<WorldFactionCnt; ++i)
                {
                    FactionEntry const * fe = sFactionStore.LookupEntry(WorldFaction[i]);
                    plr->GetReputationMgr().ModifyReputation(fe, ReputationMgr::Reputation_Cap);
                }
                plr->CLOSE_GOSSIP_MENU();
                break;
            // -----------------------------------------------------------------------------------
            // -------------------------------------- BERUFE -------------------------------------
            // -----------------------------------------------------------------------------------
            // Alchimie
            case GOSSIP_ACTION_INFO_DEF + 11:
                if (!plr->HasSpell(SPELL_ALCHEMY_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_ALCHEMY_GRAND_MASTER);
                    plr->SetSkill(SKILL_ALCHEMY, 1, plr->GetPureMaxSkillValue(SKILL_ALCHEMY), plr->GetPureMaxSkillValue(SKILL_ALCHEMY));
                    LearnAllSkillRecipes(plr, SKILL_ALCHEMY);
                }
                else
                    if (plr->HasSpell(SPELL_ALCHEMY_GRAND_MASTER))
                        LearnAllSkillRecipes(plr, SKILL_ALCHEMY);
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Schmieden
            case GOSSIP_ACTION_INFO_DEF + 12:
                if (!plr->HasSpell(SPELL_BLACKSMITHING_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_BLACKSMITHING_GRAND_MASTER);
                    plr->SetSkill(SKILL_BLACKSMITHING, 1, plr->GetPureMaxSkillValue(SKILL_BLACKSMITHING), plr->GetPureMaxSkillValue(SKILL_BLACKSMITHING));
                    LearnAllSkillRecipes(plr, SKILL_BLACKSMITHING);
                }
                else
                    if (plr->HasSpell(SPELL_BLACKSMITHING_GRAND_MASTER))
                        LearnAllSkillRecipes(plr, SKILL_BLACKSMITHING);
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Kochen
            case GOSSIP_ACTION_INFO_DEF + 13:
                if (!plr->HasSpell(SPELL_COOKING_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_COOKING_GRAND_MASTER);
                    plr->SetSkill(SKILL_COOKING, 1, plr->GetPureMaxSkillValue(SKILL_COOKING), plr->GetPureMaxSkillValue(SKILL_COOKING));
                    LearnAllSkillRecipes(plr, SKILL_COOKING);
                }
                else
                    if (plr->HasSpell(SPELL_COOKING_GRAND_MASTER))
                        LearnAllSkillRecipes(plr, SKILL_COOKING);
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Verzaubern
            case GOSSIP_ACTION_INFO_DEF + 14:
                if (!plr->HasSpell(SPELL_ENCHANTING_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_ENCHANTING_GRAND_MASTER);
                    plr->SetSkill(SKILL_ENCHANTING, 1, plr->GetPureMaxSkillValue(SKILL_ENCHANTING), plr->GetPureMaxSkillValue(SKILL_ENCHANTING));
                    LearnAllSkillRecipes(plr, SKILL_ENCHANTING);
                }
                else
                    if (plr->HasSpell(SPELL_ENCHANTING_GRAND_MASTER))
                        LearnAllSkillRecipes(plr, SKILL_ENCHANTING);
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Ingenieurwesen
            case GOSSIP_ACTION_INFO_DEF + 15:
                if (!plr->HasSpell(SPELL_ENGINEERING_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_ENGINEERING_GRAND_MASTER);
                    plr->SetSkill(SKILL_ENGINEERING, 1, plr->GetPureMaxSkillValue(SKILL_ENGINEERING), plr->GetPureMaxSkillValue(SKILL_ENGINEERING));
                    LearnAllSkillRecipes(plr, SKILL_ENGINEERING);
                }
                else
                    if (plr->HasSpell(SPELL_ENGINEERING_GRAND_MASTER))
                        LearnAllSkillRecipes(plr, SKILL_ENGINEERING);
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Erste Hilfe
            case GOSSIP_ACTION_INFO_DEF + 16:
                if (!plr->HasSpell(SPELL_FIRAT_AID_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_FIRAT_AID_GRAND_MASTER);
                    plr->SetSkill(SKILL_FIRST_AID, 1, plr->GetPureMaxSkillValue(SKILL_FIRST_AID), plr->GetPureMaxSkillValue(SKILL_FIRST_AID));
                }
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Kräuterkunde
            case GOSSIP_ACTION_INFO_DEF + 17:
                if (!plr->HasSpell(SPELL_HERB_GATHERING_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_HERB_GATHERING_GRAND_MASTER);
                    plr->SetSkill(SKILL_HERBALISM, 1, plr->GetPureMaxSkillValue(SKILL_HERBALISM), plr->GetPureMaxSkillValue(SKILL_HERBALISM));
                }
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Inschriftenkunde
            case GOSSIP_ACTION_INFO_DEF + 18:
                if (!plr->HasSpell(SPELL_INSCRIPTION_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_INSCRIPTION_GRAND_MASTER);
                    plr->SetSkill(SKILL_INSCRIPTION, 1, plr->GetPureMaxSkillValue(SKILL_INSCRIPTION), plr->GetPureMaxSkillValue(SKILL_INSCRIPTION));
                    LearnAllSkillRecipes(plr, SKILL_INSCRIPTION);
                }
                else
                    if (plr->HasSpell(SPELL_INSCRIPTION_GRAND_MASTER))
                        LearnAllSkillRecipes(plr, SKILL_INSCRIPTION);
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Juwelenschleifen
            case GOSSIP_ACTION_INFO_DEF + 19:
                if (!plr->HasSpell(SPELL_JEWELCRAFTING_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_JEWELCRAFTING_GRAND_MASTER);
                    plr->SetSkill(SKILL_JEWELCRAFTING, 1, plr->GetPureMaxSkillValue(SKILL_JEWELCRAFTING), plr->GetPureMaxSkillValue(SKILL_JEWELCRAFTING));
                    LearnAllSkillRecipes(plr, SKILL_JEWELCRAFTING);
                }
                else
                    if (plr->HasSpell(SPELL_JEWELCRAFTING_GRAND_MASTER))
                        LearnAllSkillRecipes(plr, SKILL_JEWELCRAFTING);
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Lederverarbeitung
            case GOSSIP_ACTION_INFO_DEF + 20:
                if (!plr->HasSpell(SPELL_LEATHERWORKING_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_LEATHERWORKING_GRAND_MASTER);
                    plr->SetSkill(SKILL_LEATHERWORKING, 1, plr->GetPureMaxSkillValue(SKILL_LEATHERWORKING), plr->GetPureMaxSkillValue(SKILL_LEATHERWORKING));
                    LearnAllSkillRecipes(plr, SKILL_LEATHERWORKING);
                }
                else
                    if (plr->HasSpell(SPELL_LEATHERWORKING_GRAND_MASTER))
                        LearnAllSkillRecipes(plr, SKILL_LEATHERWORKING);
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Schneidern
            case GOSSIP_ACTION_INFO_DEF + 21:
                if (!plr->HasSpell(SPELL_TAILORING_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_TAILORING_GRAND_MASTER);
                    plr->SetSkill(SKILL_TAILORING, 1, plr->GetPureMaxSkillValue(SKILL_TAILORING), plr->GetPureMaxSkillValue(SKILL_TAILORING));
                    LearnAllSkillRecipes(plr, SKILL_TAILORING);
                }
                else
                    if (plr->HasSpell(SPELL_TAILORING_GRAND_MASTER))
                        LearnAllSkillRecipes(plr, SKILL_TAILORING);
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Bergbau
            case GOSSIP_ACTION_INFO_DEF + 22:
                if (!plr->HasSpell(SPELL_MINING_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_MINING_GRAND_MASTER);
                    plr->SetSkill(SKILL_MINING, 1, plr->GetPureMaxSkillValue(SKILL_MINING), plr->GetPureMaxSkillValue(SKILL_MINING));
                }
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Angeln
            case GOSSIP_ACTION_INFO_DEF + 23:
                if (!plr->HasSpell(SPELL_FISHING_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_FISHING_GRAND_MASTER);
                    plr->SetSkill(SKILL_FISHING, 1, plr->GetPureMaxSkillValue(SKILL_FISHING), plr->GetPureMaxSkillValue(SKILL_FISHING));
                }
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Kürschnern
            case GOSSIP_ACTION_INFO_DEF + 24:
                if (!plr->HasSpell(SPELL_SKINNING_GRAND_MASTER))
                {
                    plr->learnSpellHighRank(SPELL_SKINNING_GRAND_MASTER);
                    plr->SetSkill(SKILL_SKINNING, 1, plr->GetPureMaxSkillValue(SKILL_SKINNING), plr->GetPureMaxSkillValue(SKILL_SKINNING));
                }
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Reiten
            case GOSSIP_ACTION_INFO_DEF + 25:
                if (!plr->HasSpell(SPELL_COLD_WEATHER_FLYING_PASSIVE))
                {
                    plr->learnSpellHighRank(SPELL_COLD_WEATHER_FLYING_PASSIVE);
                    plr->SetSkill(SKILL_RIDING, 1, plr->GetPureMaxSkillValue(SKILL_RIDING), plr->GetPureMaxSkillValue(SKILL_RIDING));
                }
                plr->CLOSE_GOSSIP_MENU();
                break;
            // Alle Berufe
            case GOSSIP_ACTION_INFO_DEF + 26:
                if (AccountMgr::IsOGMAccount(plr->GetSession()->GetSecurity()))
                {
                    // Alchimie
                    if (!plr->HasSpell(SPELL_ALCHEMY_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_ALCHEMY_GRAND_MASTER);
                        plr->SetSkill(SKILL_ALCHEMY, 1, plr->GetPureMaxSkillValue(SKILL_ALCHEMY), plr->GetPureMaxSkillValue(SKILL_ALCHEMY));
                        LearnAllSkillRecipes(plr, SKILL_ALCHEMY);
                    }
                    else
                        if (plr->HasSpell(SPELL_ALCHEMY_GRAND_MASTER))
                            LearnAllSkillRecipes(plr, SKILL_ALCHEMY);
                    // Schmieden
                    if (!plr->HasSpell(SPELL_BLACKSMITHING_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_BLACKSMITHING_GRAND_MASTER);
                        plr->SetSkill(SKILL_BLACKSMITHING, 1, plr->GetPureMaxSkillValue(SKILL_BLACKSMITHING), plr->GetPureMaxSkillValue(SKILL_BLACKSMITHING));
                        LearnAllSkillRecipes(plr, SKILL_BLACKSMITHING);
                    }
                    else
                        if (plr->HasSpell(SPELL_BLACKSMITHING_GRAND_MASTER))
                            LearnAllSkillRecipes(plr, SKILL_BLACKSMITHING);
                    // Kochen
                    if (!plr->HasSpell(SPELL_COOKING_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_COOKING_GRAND_MASTER);
                        plr->SetSkill(SKILL_COOKING, 1, plr->GetPureMaxSkillValue(SKILL_COOKING), plr->GetPureMaxSkillValue(SKILL_COOKING));
                        LearnAllSkillRecipes(plr, SKILL_COOKING);
                    }
                    else
                        if (plr->HasSpell(SPELL_COOKING_GRAND_MASTER))
                            LearnAllSkillRecipes(plr, SKILL_COOKING);
                    // Verzaubern
                    if (!plr->HasSpell(SPELL_ENCHANTING_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_ENCHANTING_GRAND_MASTER);
                        plr->SetSkill(SKILL_ENCHANTING, 1, plr->GetPureMaxSkillValue(SKILL_ENCHANTING), plr->GetPureMaxSkillValue(SKILL_ENCHANTING));
                        LearnAllSkillRecipes(plr, SKILL_ENCHANTING);
                    }
                    else
                        if (plr->HasSpell(SPELL_ENCHANTING_GRAND_MASTER))
                            LearnAllSkillRecipes(plr, SKILL_ENCHANTING);
                    // Ingenieurwesen
                    if (!plr->HasSpell(SPELL_ENGINEERING_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_ENGINEERING_GRAND_MASTER);
                        plr->SetSkill(SKILL_ENGINEERING, 1, plr->GetPureMaxSkillValue(SKILL_ENGINEERING), plr->GetPureMaxSkillValue(SKILL_ENGINEERING));
                        LearnAllSkillRecipes(plr, SKILL_ENGINEERING);
                    }
                    else
                        if (plr->HasSpell(SPELL_ENGINEERING_GRAND_MASTER))
                            LearnAllSkillRecipes(plr, SKILL_ENGINEERING);
                    // Erste Hilfe
                    if (!plr->HasSpell(SPELL_FIRAT_AID_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_FIRAT_AID_GRAND_MASTER);
                        plr->SetSkill(SKILL_FIRST_AID, 1, plr->GetPureMaxSkillValue(SKILL_FIRST_AID), plr->GetPureMaxSkillValue(SKILL_FIRST_AID));
                    }
                    // Kräuterkunde
                    if (!plr->HasSpell(SPELL_HERB_GATHERING_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_HERB_GATHERING_GRAND_MASTER);
                        plr->SetSkill(SKILL_HERBALISM, 1, plr->GetPureMaxSkillValue(SKILL_HERBALISM), plr->GetPureMaxSkillValue(SKILL_HERBALISM));
                    }
                    // Inschriftenkunde
                    if (!plr->HasSpell(SPELL_INSCRIPTION_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_INSCRIPTION_GRAND_MASTER);
                        plr->SetSkill(SKILL_INSCRIPTION, 1, plr->GetPureMaxSkillValue(SKILL_INSCRIPTION), plr->GetPureMaxSkillValue(SKILL_INSCRIPTION));
                        LearnAllSkillRecipes(plr, SKILL_INSCRIPTION);
                    }
                    else
                        if (plr->HasSpell(SPELL_INSCRIPTION_GRAND_MASTER))
                            LearnAllSkillRecipes(plr, SKILL_INSCRIPTION);
                    // Juwelenschleifen
                    if (!plr->HasSpell(SPELL_JEWELCRAFTING_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_JEWELCRAFTING_GRAND_MASTER);
                        plr->SetSkill(SKILL_JEWELCRAFTING, 1, plr->GetPureMaxSkillValue(SKILL_JEWELCRAFTING), plr->GetPureMaxSkillValue(SKILL_JEWELCRAFTING));
                        LearnAllSkillRecipes(plr, SKILL_JEWELCRAFTING);
                    }
                    else
                        if (plr->HasSpell(SPELL_JEWELCRAFTING_GRAND_MASTER))
                            LearnAllSkillRecipes(plr, SKILL_JEWELCRAFTING);
                    // Lederverarbeitung
                    if (!plr->HasSpell(SPELL_LEATHERWORKING_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_LEATHERWORKING_GRAND_MASTER);
                        plr->SetSkill(SKILL_LEATHERWORKING, 1, plr->GetPureMaxSkillValue(SKILL_LEATHERWORKING), plr->GetPureMaxSkillValue(SKILL_LEATHERWORKING));
                        LearnAllSkillRecipes(plr, SKILL_LEATHERWORKING);
                    }
                    else
                        if (plr->HasSpell(SPELL_LEATHERWORKING_GRAND_MASTER))
                            LearnAllSkillRecipes(plr, SKILL_LEATHERWORKING);
                    // Schneidern
                    if (!plr->HasSpell(SPELL_TAILORING_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_TAILORING_GRAND_MASTER);
                        plr->SetSkill(SKILL_TAILORING, 1, plr->GetPureMaxSkillValue(SKILL_TAILORING), plr->GetPureMaxSkillValue(SKILL_TAILORING));
                        LearnAllSkillRecipes(plr, SKILL_TAILORING);
                    }
                    else
                        if (plr->HasSpell(SPELL_TAILORING_GRAND_MASTER))
                            LearnAllSkillRecipes(plr, SKILL_TAILORING);
                    // Bergbau
                    if (!plr->HasSpell(SPELL_MINING_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_MINING_GRAND_MASTER);
                        plr->SetSkill(SKILL_MINING, 1, plr->GetPureMaxSkillValue(SKILL_MINING), plr->GetPureMaxSkillValue(SKILL_MINING));
                    }
                    // Angeln
                    if (!plr->HasSpell(SPELL_FISHING_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_FISHING_GRAND_MASTER);
                        plr->SetSkill(SKILL_FISHING, 1, plr->GetPureMaxSkillValue(SKILL_FISHING), plr->GetPureMaxSkillValue(SKILL_FISHING));
                    }
                    // Kürschnern
                    if (!plr->HasSpell(SPELL_SKINNING_GRAND_MASTER))
                    {
                        plr->learnSpellHighRank(SPELL_SKINNING_GRAND_MASTER);
                        plr->SetSkill(SKILL_SKINNING, 1, plr->GetPureMaxSkillValue(SKILL_SKINNING), plr->GetPureMaxSkillValue(SKILL_SKINNING));
                    }
                    // Reiten
                    if (!plr->HasSpell(SPELL_COLD_WEATHER_FLYING_PASSIVE))
                    {
                        plr->learnSpellHighRank(SPELL_COLD_WEATHER_FLYING_PASSIVE);
                        plr->SetSkill(SKILL_RIDING, 1, plr->GetPureMaxSkillValue(SKILL_RIDING), plr->GetPureMaxSkillValue(SKILL_RIDING));
                    }
                }
                plr->CLOSE_GOSSIP_MENU();
                break;
        }
    }

    bool OnGossipSelect(Player * plr, Creature * cr, uint32 sender, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();

        switch(sender)
        {
            case GOSSIP_SENDER_MAIN:
                if (AccountMgr::IsPlayerAccount(plr->GetSession()->GetSecurity()))
                    cr->CastSpell(plr, SPELL_TURKEY_FEATHERS, true); // Truthahnfedern
                SendDefaultMenu(plr, cr, action);
                break;
            case GOSSIP_SENDER_SEC_BANK:
            case GOSSIP_SENDER_SEC_PROFTRAIN:
                SendActionMenu(plr, cr, action);
                break;
        }
        return true;
    }
private:
    std::map<uint32, uint32> PetListe;
    std::map<uint32, uint32> HordeMountListe;
    std::map<uint32, uint32> AllyMountListe;
    std::map<uint32, uint32> BothMountListe;
};

// ------------------------------------------------------------------------------------------------------------
// UWoM's User-Pimper 60002
// ------------------------------------------------------------------------------------------------------------

#define GOSSIP_USER_PIMPER_01 "Ey Mann, was hast du so unter dem Ladentisch!?"
#define GOSSIP_USER_PIMPER_02 "Ein paar Jobs würden mir ganz gut tun."
#define GOSSIP_USER_PIMPER_03 "Ich möchte gerne meine Skills maximieren."
#define GOSSIP_USER_PIMPER_04 "Rück sofort alle deine Mini-Pets raus, oder es knallt!"
#define GOSSIP_USER_PIMPER_05 "Also mein Ruf hier in der Gegend lässt ein wenig zu wünschen übrig."
#define GOSSIP_USER_PIMPER_06 "Sag mal, hast du auch Rezepte zu den Berufen!?"
#define GOSSIP_USER_PIMPER_07 "Ich habe keine Lust zu Leveln! Gib mir den max. Level!"
#define GOSSIP_USER_PIMPER_08 "Bring mich sofort nach Dalaran!"
#define GOSSIP_USER_PIMPER_09 "Was kosten mich eure Dienste (Preisliste)!?"

#define COST_STRING_USER_PIMPER_PETS    "Alle Minipets lernen: "
#define COST_STRING_USER_PIMPER_REP     "Alle Fraktionen ehrfürchtig: "
#define COST_STRING_USER_PIMPER_PROF    "1 Beruf lernen (inkl. max. Skill): "
#define COST_STRING_USER_PIMPER_RECIPES "Alle Rezepte für 1 Beruf lernen: "
#define COST_STRING_USER_PIMPER_SKILL   "Alle Fähigkeiten auf max. Skill: "
#define COST_STRING_USER_PIMPER_LEVEL   "Charakter auf max. Level setzen: 0"

#define PIMPER_NOT_ENOUGH   "Du hast leider nicht genug %s für diesen Dienst!"
#define PIMPER_PROF_MAX     "Du musst den Beruf auf Maximum haben!"
#define PIMPER_PROF_LIMIT   "Du hast bereits das Maximum an Berufen!"

struct UserPimper_PriceSetup
{
    uint8 Cost_Type;                // 0 = Money - 1 = Honor - 2 = Arena Points - 3 = Item

    uint32 Costs_Item;              // Used (must be set) if Cost_Type == 3
    uint32 Costs_AllMiniPets;       // Costs (amount) to learn all mini pets
    uint32 Costs_Reputation;        // Costs (amount) to max. all factions (Horde/Ally and world factions)
    uint32 Costs_Profession;        // Costs (amount) to learn a profession as grand master and get max. skill for it
    uint32 Costs_AllRecipes;        // Costs (amount) to learn all recipes for one profession
    //uint32 Costs_MaxSkill;          // Costs (amount) to set the max. skill for one profession

    bool PresentsAllowed;

    // Cost strings for the gossip
    std::string str_Item;
    std::string str_AllMiniPets;
    std::string str_Reputation;
    std::string str_Profession;
    std::string str_AllRecipes;
    std::string str_MaxSkill;
    std::string str_bottom;
};

class npc_uwom_user_pimper : public CreatureScript
{
public:
    npc_uwom_user_pimper() : CreatureScript("npc_uwom_user_pimper") { }

    struct npc_uwom_user_pimperAI : public ScriptedAI
    {
        npc_uwom_user_pimperAI(Creature * cr) : ScriptedAI(cr)
        {
            ps.PresentsAllowed      = sWorld->getBoolConfig(CONFIG_USER_PIMPER_PRESENTS_ALLOWED);
            ps.Cost_Type            = sWorld->getIntConfig(CONFIG_USER_PIMPER_CURRENCY);
            ps.Costs_Item           = sWorld->getIntConfig(CONFIG_USER_PIMPER_CURRENCY_ITEM);
            ps.Costs_AllMiniPets    = sWorld->getIntConfig(CONFIG_USER_PIMPER_COSTS_MINIPETS);
            ps.Costs_Reputation     = sWorld->getIntConfig(CONFIG_USER_PIMPER_COSTS_REPUTATION);
            ps.Costs_Profession     = sWorld->getIntConfig(CONFIG_USER_PIMPER_COSTS_PROFFESSION);
            ps.Costs_AllRecipes     = sWorld->getIntConfig(CONFIG_USER_PIMPER_COSTS_RECIPES);
            //ps.Costs_MaxSkill       = sWorld->getIntConfig(CONFIG_USER_PIMPER_COSTS_MAXSKILL);

            if (ps.Cost_Type == 3)
            {
                item = sObjectMgr->GetItemTemplate(ps.Costs_Item);
                if (item)
                    ps.str_Item = item->Name1;
            }
            else
                item = NULL;

            if (ps.Cost_Type == 0)
            {
                ps.Costs_AllMiniPets = ps.Costs_AllMiniPets*GOLD;
                ps.Costs_Reputation = ps.Costs_Reputation*GOLD;
                ps.Costs_Profession = ps.Costs_Profession*GOLD;
                ps.Costs_AllRecipes = ps.Costs_AllRecipes*GOLD;
                //ps.Costs_MaxSkill = ps.Costs_MaxSkill*GOLD;
            }
            InitStrings();
            LadePetListe();
        }

        UserPimper_PriceSetup ps;
        ItemTemplate const * item;
        std::map<uint32, uint32> PetListe;

        void Reset()
        {
        }

        void LadePetListe()
        {
            QueryResult result = WorldDatabase.PQuery("SELECT `entry`,`spellid_2` FROM `item_template` WHERE `class`=15 AND `subclass`=2 AND `spellid_2`!=0");
            if (!result)
            {
                sLog->outErrorDb("USER-PIMPER: Kann die Minipets nicht laden!");
                return;
            }

            uint32 cnt = 0;

            do
            {
                Field * fields = result->Fetch();
                if (fields[0].GetUInt32() && fields[1].GetUInt32())
                {
                    PetListe[fields[0].GetUInt32()] = fields[1].GetUInt32();
                    ++cnt;
                }
            } while (result->NextRow());

            sLog->outDetail("USER-PIMPER: Habe %u gültige Minipets gefunden und geladen.", cnt);
        }

        void InitStrings()
        {
            char* tmpchars = (char*)malloc(32);
            std::string tmpstr;

            if (ps.Cost_Type == 0)
            {
                sprintf(tmpchars, "%i", ps.Costs_AllMiniPets/GOLD);
                tmpstr.append(COST_STRING_USER_PIMPER_PETS).append(tmpchars);
                ps.str_AllMiniPets = tmpstr;
                tmpstr.clear();

                sprintf(tmpchars, "%i", ps.Costs_Reputation/GOLD);
                tmpstr.append(COST_STRING_USER_PIMPER_REP).append(tmpchars);
                ps.str_Reputation = tmpstr;
                tmpstr.clear();

                sprintf(tmpchars, "%i", ps.Costs_Profession/GOLD);
                tmpstr.append(COST_STRING_USER_PIMPER_PROF).append(tmpchars);
                ps.str_Profession = tmpstr;
                tmpstr.clear();

                sprintf(tmpchars, "%i", ps.Costs_AllRecipes/GOLD);
                tmpstr.append(COST_STRING_USER_PIMPER_RECIPES).append(tmpchars);
                ps.str_AllRecipes = tmpstr;
                tmpstr.clear();

                //sprintf(tmpchars, "%i", ps.Costs_MaxSkill/GOLD);
                tmpstr.append(COST_STRING_USER_PIMPER_SKILL).append("0")/*.append(tmpchars)*/;
                ps.str_MaxSkill = tmpstr;
                tmpstr.clear();
            }
            else
            {
                sprintf(tmpchars, "%i", ps.Costs_AllMiniPets);
                tmpstr.append(COST_STRING_USER_PIMPER_PETS).append(tmpchars);
                ps.str_AllMiniPets = tmpstr;
                tmpstr.clear();

                sprintf(tmpchars, "%i", ps.Costs_Reputation);
                tmpstr.append(COST_STRING_USER_PIMPER_REP).append(tmpchars);
                ps.str_Reputation = tmpstr;
                tmpstr.clear();

                sprintf(tmpchars, "%i", ps.Costs_Profession);
                tmpstr.append(COST_STRING_USER_PIMPER_PROF).append(tmpchars);
                ps.str_Profession = tmpstr;
                tmpstr.clear();

                sprintf(tmpchars, "%i", ps.Costs_AllRecipes);
                tmpstr.append(COST_STRING_USER_PIMPER_RECIPES).append(tmpchars);
                ps.str_AllRecipes = tmpstr;
                tmpstr.clear();

                //sprintf(tmpchars, "%i", ps.Costs_MaxSkill);
                tmpstr.append(COST_STRING_USER_PIMPER_SKILL).append("0")/*.append(tmpchars)*/;
                ps.str_MaxSkill = tmpstr;
                tmpstr.clear();
            }
            ps.str_bottom.append("Alle Preise sind in ");

            switch(ps.Cost_Type)
            {
                case 0: ps.str_bottom.append("Gold."); break;
                case 1: ps.str_bottom.append("Ehre."); break;
                case 2: ps.str_bottom.append("Arenapunkten."); break;
                case 3:
                    if (item)
                        ps.str_bottom.append(ps.str_Item).append(".");
                    else
                        ps.str_bottom.append("ITEM NOT FOUND.");
                    break;
            }
            free(tmpchars);
        }

        bool HasEnough(Player * pl, int32 amount)
        {
            if (!pl || !pl->IsInWorld())
                return false;

            switch(ps.Cost_Type)
            {
                case 0:
                    if (pl->GetMoney() < static_cast<uint32>(amount))
                        pl->GetSession()->SendNotification(PIMPER_NOT_ENOUGH, "Gold");
                    else
                        return true;
                    break;
                case 1:
                    if (pl->GetHonorPoints() < static_cast<uint32>(amount))
                        pl->GetSession()->SendNotification(PIMPER_NOT_ENOUGH, "Ehre");
                    else
                        return true;
                    break;
                case 2:
                    if (pl->GetArenaPoints() < static_cast<uint32>(amount))
                        pl->GetSession()->SendNotification(PIMPER_NOT_ENOUGH, "Arenapunkte");
                    else
                        return true;
                    break;
                case 3:
                    if (pl->GetItemCount(ps.Costs_Item) < static_cast<uint32>(amount))
                    {
                        if (item)
                            pl->GetSession()->SendNotification(PIMPER_NOT_ENOUGH, item->Name1.c_str());
                    }
                    else
                        return true;
                    break;
            }
            return false;
        }

        bool SubstructCurrency(Player * pl, int32 amount)
        {
            if (!pl || !pl->IsInWorld())
                return false;

            switch(ps.Cost_Type)
            {
                case 0:
                    if (HasEnough(pl, amount))
                        pl->ModifyMoney(-amount);
                    else
                        return false;
                    break;
                case 1:
                    if (HasEnough(pl, amount))
                        pl->ModifyHonorPoints(-amount);
                    else
                        return false;
                    break;
                case 2:
                    if (HasEnough(pl, amount))
                        pl->ModifyArenaPoints(-amount);
                    else
                        return false;
                    break;
                case 3:
                    if (HasEnough(pl, amount))
                        pl->DestroyItemCount(ps.Costs_Item, amount, true);
                    else
                        return false;
                    break;
            }
            return true;
        }

        bool CanGetPresent(Player * pl, uint32 item, uint32 spell)
        {
            if (!pl || !pl->IsInWorld())
                return false;

            return (!pl->HasItemCount(item, 1, true) && !pl->HasSpell(spell));
        }

        std::string GetPlaytimeString(uint32 total_playtime_secs)
        {
            std::ostringstream ss;

            ss << ("Deine Spielzeit: ");

            if (GetPlaytimeDays(total_playtime_secs) > 0)
                ss << uint32(GetPlaytimeDays(total_playtime_secs)) << (" Tage(e) ");
            if (GetPlaytimeHours(total_playtime_secs) > 0)
                ss << uint32(GetPlaytimeHours(total_playtime_secs)) << (" Stunde(n)");

            return ss.str();
        }

        uint32 GetPlaytimeHours(uint32 total_playtime_secs) { return total_playtime_secs%DAY/HOUR; }
        uint32 GetPlaytimeDays(uint32 total_playtime_secs) { return total_playtime_secs/DAY; }
        uint32 GetPlaytimeWeeks(uint32 total_playtime_secs) { return total_playtime_secs/WEEK; }
        uint32 GetPlaytimeMonth(uint32 total_playtime_secs) { return total_playtime_secs/MONTH; }
    };

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        if (!plr || !plr->IsInWorld())
            return false;

        npc_uwom_user_pimperAI *ai = CAST_AI(npc_uwom_user_pimperAI, cr->AI());

        if (!ai)
            return false;

        uint32 total_playtime_secs = plr->GetTotalPlayedTime(); // Sekunden!

        if (ai->ps.PresentsAllowed && ai->GetPlaytimeMonth(total_playtime_secs) >= 1) // Ab 1 Monat Spielzeit
        {
            if (ai->CanGetPresent(plr, 49665, 69541)) // Pandarenmönch
                ai->addItem(plr, 49665, 1, true, false, true);
            if (ai->CanGetPresent(plr, 45693, 63796)) // Mimirons Kopf
                ai->addItem(plr, 45693, 1, true, false, true);
        }

        if (ai->ps.PresentsAllowed && ai->GetPlaytimeMonth(total_playtime_secs) >= 2) // Ab 2 Monate Spielzeit
        {
            if (ai->CanGetPresent(plr, 49343, 68810)) // Spektraltigerjunges
                ai->addItem(plr, 49343, 1, true, false, true);
            if (ai->CanGetPresent(plr, 45802, 63963)) // Zügel des rostigen Protodrachen
                ai->addItem(plr, 45802, 1, true, false, true);
        }

        if (ai->ps.PresentsAllowed && ai->GetPlaytimeMonth(total_playtime_secs) >= 3) // Ab 3 Monate Spielzeit
        {
            if (ai->CanGetPresent(plr, 49283, 42776)) // Zügel des Spektraltigers
                ai->addItem(plr, 49283, 1, true, false, true);
            if (ai->CanGetPresent(plr, 45801, 63956)) // Zügel des eisenbeschlagenen Protodrachen
                ai->addItem(plr, 45801, 1, true, false, true);
        }

        if (ai->ps.PresentsAllowed && ai->GetPlaytimeMonth(total_playtime_secs) >= 5) // Ab 5 Monate Spielzeit
        {
            if (ai->CanGetPresent(plr, 33223, 42766)) // Angelstuhl
                ai->addItem(plr, 33223, 1, true, false, true);

            if (ai->CanGetPresent(plr, 33079, 42365)) // Murlockostüm
                ai->addItem(plr, 33079, 1, true, false, true);

            if (plr->GetTeamId() == TEAM_ALLIANCE)
            {
                if (ai->CanGetPresent(plr, 12302, 16056)) // Zügel des uralten Frostsäblers
                    ai->addItem(plr, 12302, 1, true, false, true);
            }
            else
            {
                if (ai->CanGetPresent(plr, 13317, 17450)) // Pfeife des elfenbeinfarbenen Raptors
                    ai->addItem(plr, 13317, 1, true, false, true);
            }

            if (AchievementEntry const * AE = GetAchievementStore()->LookupEntry(2186)) // Der Unsterbliche
                plr->CompletedAchievement(AE);
        }

        if (cr->isQuestGiver())  plr->PrepareQuestMenu(cr->GetGUID());
        if (cr->isTrainer())     plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_TRAIN,          GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRAIN);
        if (cr->isVendor())      plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    GOSSIP_TEXT_BROWSE_GOODS,   GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
        if (cr->isAuctioner())   plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_MONEY_BAG, GOSSIP_TEXT_AUCTIONHOUSE,   GOSSIP_SENDER_MAIN, GOSSIP_ACTION_AUCTION);

        plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    GOSSIP_USER_PIMPER_01,    GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    GOSSIP_USER_PIMPER_09,    GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 9);

        plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ai->GetPlaytimeString(total_playtime_secs), GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 10);

        plr->SEND_GOSSIP_MENU(1100000, cr->GetGUID());

        switch(urand(0,1))
        {
            case 0: cr->CastSpell(plr, SPELL_TURKEY_FEATHERS, true); break; // Truthahnfedern
            case 1: plr->SetDisplayId(15369); break; // Murloc Baby
        }

        return true;
    }

    void SendDefaultMenu(Player * plr, Creature * cr, uint32 action)
    {
        npc_uwom_user_pimperAI *ai = CAST_AI(npc_uwom_user_pimperAI, cr->AI());

        if (!ai)
            return;

        switch (action)
        {   // Standard-Aktionen
            case GOSSIP_ACTION_TRAIN:
                plr->GetSession()->SendTrainerList(cr->GetGUID());
                break;
            case GOSSIP_ACTION_TRADE:
                plr->GetSession()->SendListInventory(cr->GetGUID());
                break;
            case GOSSIP_ACTION_AUCTION:
                cr->setFaction(plr->getFaction());
                plr->GetSession()->SendAuctionHello(cr->GetGUID(), cr);
                cr->setFaction(35);
                break;

            // Unterm Ladentisch
            case GOSSIP_ACTION_INFO_DEF + 1:
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_USER_PIMPER_02,    GOSSIP_SENDER_MAIN,             GOSSIP_ACTION_INFO_DEF + 2);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_USER_PIMPER_03,    GOSSIP_SENDER_MAIN,             GOSSIP_ACTION_INFO_DEF + 3);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR,    GOSSIP_USER_PIMPER_04,    GOSSIP_SENDER_SEC_BANK,         GOSSIP_ACTION_INFO_DEF + 4);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_USER_PIMPER_05,    GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 5);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_USER_PIMPER_06,    GOSSIP_SENDER_MAIN,             GOSSIP_ACTION_INFO_DEF + 6);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_USER_PIMPER_07,    GOSSIP_SENDER_MAIN,             GOSSIP_ACTION_INFO_DEF + 7);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_USER_PIMPER_08,    GOSSIP_SENDER_MAIN,             GOSSIP_ACTION_INFO_DEF + 8);
                plr->SEND_GOSSIP_MENU(1100001, cr->GetGUID());
                break;

            // Ein paar Jobs
            case GOSSIP_ACTION_INFO_DEF + 2:
                if (plr->GetFreePrimaryProfessionPoints() > 0)
                {
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_ALCHEMY,        GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 11);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_BLACKSMITHING,  GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 12);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_COOKING,        GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 13);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_ENCHANTING,     GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 14);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_ENGINEERING,    GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 15);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_FIRSTAID,       GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 16);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_HERBALISM,      GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 17);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_INSCRIPTION,    GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 18);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_JEWELCRAFTING,  GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 19);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_LEATHERWORKING, GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 20);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_TAILORING,      GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 21);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_MINING,         GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 22);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_FISHING,        GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 23);
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_SKINNING,       GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 24);

                    plr->SEND_GOSSIP_MENU(4328, cr->GetGUID());
                }
                else
                {
                    plr->GetSession()->SendNotification(PIMPER_PROF_LIMIT);
                    plr->CLOSE_GOSSIP_MENU();
                }
                break;

            // Maxskill
            case GOSSIP_ACTION_INFO_DEF + 3:
                //if (ai->SubstructCurrency(plr, ai->ps.Costs_MaxSkill))
                    plr->UpdateSkillsToMaxSkillsForLevel();
                plr->CLOSE_GOSSIP_MENU();
                break;

            // Rezepte für Jobs
            case GOSSIP_ACTION_INFO_DEF + 6:
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_ALCHEMY,        GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 30);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_BLACKSMITHING,  GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 31);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_COOKING,        GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 32);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_ENCHANTING,     GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 33);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_ENGINEERING,    GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 34);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_INSCRIPTION,    GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 35);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_JEWELCRAFTING,  GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 36);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_LEATHERWORKING, GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 37);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER,   GOSSIP_TEXT_TAILORING,      GOSSIP_SENDER_SEC_PROFTRAIN,    GOSSIP_ACTION_INFO_DEF + 38);

                plr->SEND_GOSSIP_MENU(1100002, cr->GetGUID());
                break;

            // Maxlevel
            case GOSSIP_ACTION_INFO_DEF + 7:
                if (plr->getLevel() < DEFAULT_MAX_LEVEL)
                {
                    plr->SetLevel(DEFAULT_MAX_LEVEL);
                    plr->ModifyMoney(100000000); // 10k Gold geben.
                    plr->AddItem(51809, 4); // 4 * Tragbares Loch geben.
                }
                plr->CLOSE_GOSSIP_MENU();
                break;

            // Nach Dalaran!
            case GOSSIP_ACTION_INFO_DEF + 8:
                plr->CastSpell(plr, 71512, true);
                plr->CLOSE_GOSSIP_MENU();
                break;

            // Preisliste
            case GOSSIP_ACTION_INFO_DEF + 9:
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ai->ps.str_AllMiniPets,          GOSSIP_SENDER_INFO, GOSSIP_ACTION_INFO_DEF + 39);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ai->ps.str_Reputation,           GOSSIP_SENDER_INFO, GOSSIP_ACTION_INFO_DEF + 40);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ai->ps.str_Profession,           GOSSIP_SENDER_INFO, GOSSIP_ACTION_INFO_DEF + 41);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ai->ps.str_AllRecipes,           GOSSIP_SENDER_INFO, GOSSIP_ACTION_INFO_DEF + 42);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ai->ps.str_MaxSkill,             GOSSIP_SENDER_INFO, GOSSIP_ACTION_INFO_DEF + 43);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, COST_STRING_USER_PIMPER_LEVEL,   GOSSIP_SENDER_INFO, GOSSIP_ACTION_INFO_DEF + 44);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "",                              GOSSIP_SENDER_INFO, GOSSIP_ACTION_INFO_DEF + 45);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, ai->ps.str_bottom,               GOSSIP_SENDER_INFO, GOSSIP_ACTION_INFO_DEF + 46);

                plr->SEND_GOSSIP_MENU(1100003, cr->GetGUID());
                break;
        }
    }

    void SendActionMenu(Player * plr, Creature * cr, uint32 action)
    {
        npc_uwom_user_pimperAI *ai = CAST_AI(npc_uwom_user_pimperAI, cr->AI());

        if (!ai)
            return;

        switch(action)
        {
            // Mini-Pets
            case GOSSIP_ACTION_INFO_DEF + 4:
                if (ai->SubstructCurrency(plr, ai->ps.Costs_AllMiniPets))
                {
                    for (std::map<uint32, uint32>::const_iterator itr = ai->PetListe.begin(); itr != ai->PetListe.end(); ++itr)
                    {
                        if (!plr->HasSpell(itr->second))
                            plr->learnSpell(itr->second, false);
                    }
                }
                break;

            // Ruf
            case GOSSIP_ACTION_INFO_DEF + 5:
                if (ai->SubstructCurrency(plr, ai->ps.Costs_Reputation))
                {
                    if (plr->GetTeam() == ALLIANCE)
                    {
                        for (uint8 i=0; i<AllyFactionCnt; ++i)
                        {
                            FactionEntry const * fe = sFactionStore.LookupEntry(AllyFaction[i]);
                            plr->GetReputationMgr().ModifyReputation(fe, ReputationMgr::Reputation_Cap);
                        }
                    }
                    else
                    {
                        for (uint8 i=0; i<HordeFactionCnt; ++i)
                        {
                            FactionEntry const * fe = sFactionStore.LookupEntry(HordeFaction[i]);
                            plr->GetReputationMgr().ModifyReputation(fe, ReputationMgr::Reputation_Cap);
                        }
                    }
                    for (uint8 i=0; i<WorldFactionCnt; ++i)
                    {
                        FactionEntry const * fe = sFactionStore.LookupEntry(WorldFaction[i]);
                        plr->GetReputationMgr().ModifyReputation(fe, ReputationMgr::Reputation_Cap);
                    }
                }
                break;

            // Preisliste
            case GOSSIP_ACTION_INFO_DEF + 39:
            case GOSSIP_ACTION_INFO_DEF + 40:
            case GOSSIP_ACTION_INFO_DEF + 41:
            case GOSSIP_ACTION_INFO_DEF + 42:
            case GOSSIP_ACTION_INFO_DEF + 43:
            case GOSSIP_ACTION_INFO_DEF + 44:
            case GOSSIP_ACTION_INFO_DEF + 45:
            case GOSSIP_ACTION_INFO_DEF + 46:
                break;
            // -----------------------------------------------------------------------------------
            // -------------------------------------- BERUFE -------------------------------------
            // -----------------------------------------------------------------------------------
            // Alchimie
            case GOSSIP_ACTION_INFO_DEF + 11:
                if (!plr->HasSpell(SPELL_ALCHEMY_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_ALCHEMY_GRAND_MASTER);
                    plr->SetSkill(SKILL_ALCHEMY, 1, plr->GetPureMaxSkillValue(SKILL_ALCHEMY), plr->GetPureMaxSkillValue(SKILL_ALCHEMY));
                }
                break;
            // Schmieden
            case GOSSIP_ACTION_INFO_DEF + 12:
                if (!plr->HasSpell(SPELL_BLACKSMITHING_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_BLACKSMITHING_GRAND_MASTER);
                    plr->SetSkill(SKILL_BLACKSMITHING, 1, plr->GetPureMaxSkillValue(SKILL_BLACKSMITHING), plr->GetPureMaxSkillValue(SKILL_BLACKSMITHING));
                }
                break;
            // Kochen
            case GOSSIP_ACTION_INFO_DEF + 13:
                if (!plr->HasSpell(SPELL_COOKING_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_COOKING_GRAND_MASTER);
                    plr->SetSkill(SKILL_COOKING, 1, plr->GetPureMaxSkillValue(SKILL_COOKING), plr->GetPureMaxSkillValue(SKILL_COOKING));
                }
                break;
            // Verzaubern
            case GOSSIP_ACTION_INFO_DEF + 14:
                if (!plr->HasSpell(SPELL_ENCHANTING_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_ENCHANTING_GRAND_MASTER);
                    plr->SetSkill(SKILL_ENCHANTING, 1, plr->GetPureMaxSkillValue(SKILL_ENCHANTING), plr->GetPureMaxSkillValue(SKILL_ENCHANTING));
                }
                break;
            // Ingenieurwesen
            case GOSSIP_ACTION_INFO_DEF + 15:
                if (!plr->HasSpell(SPELL_ENGINEERING_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_ENGINEERING_GRAND_MASTER);
                    plr->SetSkill(SKILL_ENGINEERING, 1, plr->GetPureMaxSkillValue(SKILL_ENGINEERING), plr->GetPureMaxSkillValue(SKILL_ENGINEERING));
                }
                break;
            // Erste Hilfe
            case GOSSIP_ACTION_INFO_DEF + 16:
                if (!plr->HasSpell(SPELL_FIRAT_AID_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_FIRAT_AID_GRAND_MASTER);
                    plr->SetSkill(SKILL_FIRST_AID, 1, plr->GetPureMaxSkillValue(SKILL_FIRST_AID), plr->GetPureMaxSkillValue(SKILL_FIRST_AID));
                }
                break;
            // Kräuterkunde
            case GOSSIP_ACTION_INFO_DEF + 17:
                if (!plr->HasSpell(SPELL_HERB_GATHERING_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_HERB_GATHERING_GRAND_MASTER);
                    plr->SetSkill(SKILL_HERBALISM, 1, plr->GetPureMaxSkillValue(SKILL_HERBALISM), plr->GetPureMaxSkillValue(SKILL_HERBALISM));
                }
                break;
            // Inschriftenkunde
            case GOSSIP_ACTION_INFO_DEF + 18:
                if (!plr->HasSpell(SPELL_INSCRIPTION_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_INSCRIPTION_GRAND_MASTER);
                    plr->SetSkill(SKILL_INSCRIPTION, 1, plr->GetPureMaxSkillValue(SKILL_INSCRIPTION), plr->GetPureMaxSkillValue(SKILL_INSCRIPTION));
                }
                break;
            // Juwelenschleifen
            case GOSSIP_ACTION_INFO_DEF + 19:
                if (!plr->HasSpell(SPELL_JEWELCRAFTING_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_JEWELCRAFTING_GRAND_MASTER);
                    plr->SetSkill(SKILL_JEWELCRAFTING, 1, plr->GetPureMaxSkillValue(SKILL_JEWELCRAFTING), plr->GetPureMaxSkillValue(SKILL_JEWELCRAFTING));
                }
                break;
            // Lederverarbeitung
            case GOSSIP_ACTION_INFO_DEF + 20:
                if (!plr->HasSpell(SPELL_LEATHERWORKING_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_LEATHERWORKING_GRAND_MASTER);
                    plr->SetSkill(SKILL_LEATHERWORKING, 1, plr->GetPureMaxSkillValue(SKILL_LEATHERWORKING), plr->GetPureMaxSkillValue(SKILL_LEATHERWORKING));
                }
                break;
            // Schneidern
            case GOSSIP_ACTION_INFO_DEF + 21:
                if (!plr->HasSpell(SPELL_TAILORING_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_TAILORING_GRAND_MASTER);
                    plr->SetSkill(SKILL_TAILORING, 1, plr->GetPureMaxSkillValue(SKILL_TAILORING), plr->GetPureMaxSkillValue(SKILL_TAILORING));
                }
                break;
            // Bergbau
            case GOSSIP_ACTION_INFO_DEF + 22:
                if (!plr->HasSpell(SPELL_MINING_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_MINING_GRAND_MASTER);
                    plr->SetSkill(SKILL_MINING, 1, plr->GetPureMaxSkillValue(SKILL_MINING), plr->GetPureMaxSkillValue(SKILL_MINING));
                }
                break;
            // Angeln
            case GOSSIP_ACTION_INFO_DEF + 23:
                if (!plr->HasSpell(SPELL_FISHING_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_FISHING_GRAND_MASTER);
                    plr->SetSkill(SKILL_FISHING, 1, plr->GetPureMaxSkillValue(SKILL_FISHING), plr->GetPureMaxSkillValue(SKILL_FISHING));
                }
                break;
            // Kürschnern
            case GOSSIP_ACTION_INFO_DEF + 24:
                if (!plr->HasSpell(SPELL_SKINNING_GRAND_MASTER) && ai->SubstructCurrency(plr, ai->ps.Costs_Profession))
                {
                    plr->learnSpellHighRank(SPELL_SKINNING_GRAND_MASTER);
                    plr->SetSkill(SKILL_SKINNING, 1, plr->GetPureMaxSkillValue(SKILL_SKINNING), plr->GetPureMaxSkillValue(SKILL_SKINNING));
                }
                break;
            // -----------------------------------------------------------------------------------
            // --------------------------------- REZEPTE FÜR BERUFE ------------------------------
            // -----------------------------------------------------------------------------------
            // Alchimie
            case GOSSIP_ACTION_INFO_DEF + 30:
                if (plr->HasSkill(SKILL_ALCHEMY) && plr->GetSkillValue(SKILL_ALCHEMY) >= plr->GetPureMaxSkillValue(SKILL_ALCHEMY) && ai->SubstructCurrency(plr, ai->ps.Costs_AllRecipes))
                    LearnAllSkillRecipes(plr, SKILL_ALCHEMY);
                else
                    plr->GetSession()->SendNotification(PIMPER_PROF_MAX);
                break;
            // Schmieden
            case GOSSIP_ACTION_INFO_DEF + 31:
                if (plr->HasSkill(SKILL_BLACKSMITHING) && plr->GetSkillValue(SKILL_BLACKSMITHING) >= plr->GetPureMaxSkillValue(SKILL_BLACKSMITHING) && ai->SubstructCurrency(plr, ai->ps.Costs_AllRecipes))
                    LearnAllSkillRecipes(plr, SKILL_BLACKSMITHING);
                else
                    plr->GetSession()->SendNotification(PIMPER_PROF_MAX);
                break;
            // Kochen
            case GOSSIP_ACTION_INFO_DEF + 32:
                if (plr->HasSkill(SKILL_COOKING) && plr->GetSkillValue(SKILL_COOKING) >= plr->GetPureMaxSkillValue(SKILL_COOKING) && ai->SubstructCurrency(plr, ai->ps.Costs_AllRecipes))
                    LearnAllSkillRecipes(plr, SKILL_COOKING);
                else
                    plr->GetSession()->SendNotification(PIMPER_PROF_MAX);
                break;
            // Verzaubern
            case GOSSIP_ACTION_INFO_DEF + 33:
                if (plr->HasSkill(SKILL_ENCHANTING) && plr->GetSkillValue(SKILL_ENCHANTING) >= plr->GetPureMaxSkillValue(SKILL_ENCHANTING) && ai->SubstructCurrency(plr, ai->ps.Costs_AllRecipes))
                    LearnAllSkillRecipes(plr, SKILL_ENCHANTING);
                else
                    plr->GetSession()->SendNotification(PIMPER_PROF_MAX);
                break;
            // Ingenieurwesen
            case GOSSIP_ACTION_INFO_DEF + 34:
                if (plr->HasSkill(SKILL_ENGINEERING) && plr->GetSkillValue(SKILL_ENGINEERING) >= plr->GetPureMaxSkillValue(SKILL_ENGINEERING) && ai->SubstructCurrency(plr, ai->ps.Costs_AllRecipes))
                    LearnAllSkillRecipes(plr, SKILL_ENGINEERING);
                else
                    plr->GetSession()->SendNotification(PIMPER_PROF_MAX);
                break;
            // Inschriftenkunde
            case GOSSIP_ACTION_INFO_DEF + 35:
                if (plr->HasSkill(SKILL_INSCRIPTION) && plr->GetSkillValue(SKILL_INSCRIPTION) >= plr->GetPureMaxSkillValue(SKILL_INSCRIPTION) && ai->SubstructCurrency(plr, ai->ps.Costs_AllRecipes))
                    LearnAllSkillRecipes(plr, SKILL_INSCRIPTION);
                else
                    plr->GetSession()->SendNotification(PIMPER_PROF_MAX);
                break;
            // Juwelenschleifen
            case GOSSIP_ACTION_INFO_DEF + 36:
                if (plr->HasSkill(SKILL_JEWELCRAFTING) && plr->GetSkillValue(SKILL_JEWELCRAFTING) >= plr->GetPureMaxSkillValue(SKILL_JEWELCRAFTING) && ai->SubstructCurrency(plr, ai->ps.Costs_AllRecipes))
                    LearnAllSkillRecipes(plr, SKILL_JEWELCRAFTING);
                else
                    plr->GetSession()->SendNotification(PIMPER_PROF_MAX);
                break;
            // Lederverarbeitung
            case GOSSIP_ACTION_INFO_DEF + 37:
                if (plr->HasSkill(SKILL_LEATHERWORKING) && plr->GetSkillValue(SKILL_LEATHERWORKING) >= plr->GetPureMaxSkillValue(SKILL_LEATHERWORKING) && ai->SubstructCurrency(plr, ai->ps.Costs_AllRecipes))
                    LearnAllSkillRecipes(plr, SKILL_LEATHERWORKING);
                else
                    plr->GetSession()->SendNotification(PIMPER_PROF_MAX);
                break;
            // Schneidern
            case GOSSIP_ACTION_INFO_DEF + 38:
                if (plr->HasSkill(SKILL_TAILORING) && plr->GetSkillValue(SKILL_TAILORING) >= plr->GetPureMaxSkillValue(SKILL_TAILORING) && ai->SubstructCurrency(plr, ai->ps.Costs_AllRecipes))
                    LearnAllSkillRecipes(plr, SKILL_TAILORING);
                else
                    plr->GetSession()->SendNotification(PIMPER_PROF_MAX);
                break;
        }
        plr->CLOSE_GOSSIP_MENU();
    }

    bool OnGossipSelect(Player * plr, Creature * cr, uint32 sender, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();

        switch(sender)
        {
            case GOSSIP_SENDER_MAIN: SendDefaultMenu(plr, cr, action); break;
            case GOSSIP_SENDER_SEC_BANK:
            case GOSSIP_SENDER_SEC_PROFTRAIN: SendActionMenu(plr, cr, action); break;
        }
        return true;
    }

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_uwom_user_pimperAI(cr);
    }
};

// ------------------------------------------------------------------------------------------------------------
// Hati 60003
// ------------------------------------------------------------------------------------------------------------
class npc_hati : public CreatureScript
{
public:
    npc_hati() : CreatureScript("npc_hati") { }

    struct npc_hatiAI : public ScriptedAI
    {
        npc_hatiAI(Creature * cr) : ScriptedAI(cr) {}

        uint32 bohrentimer;

        void Reset()
        {
            bohrentimer = urand(5000,10000);
        }

        void KilledUnit(Unit * /*victim*/)
        {
            switch(urand(0,2))
            {
                case 0: DoPlaySoundToSet(me, SOUND_SLAY1); break;
                case 1: DoPlaySoundToSet(me, SOUND_SLAY2); break;
                case 2: DoPlaySoundToSet(me, SOUND_SLAY3); break;
            }
        }

        void MoveInLineOfSight(Unit * who)
        {
            if (!who)
                return;

            Unit * target = who;

            // Keine NPCs angreifen, die nicht zu einem Spieler gehören!
            if (who->GetTypeId() == TYPEID_UNIT && !who->GetOwner())
                return;

            if (target->GetTypeId() == TYPEID_PLAYER)
                if (!AccountMgr::IsPlayerAccount(target->ToPlayer()->GetSession()->GetSecurity())) // Nur Spieler angreifen, die keine GMs sind!
                    return;

            ScriptedAI::MoveInLineOfSight(target);
        }

        void EnterCombat(Unit * /*who*/)
        {
            DoPlaySoundToSet(me, SOUND_AGGRO);
        }

        void UpdateAI(const uint32 diff)
        {
            //Return since we have no target
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            if (bohrentimer <= diff)
            {
                DoCast(me->getVictim(), SPELL_DURCHBOHREN);
                bohrentimer = urand(5000,10000);
            }
            else
                bohrentimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_hatiAI(cr);
    }
};

/*########
# npc_air_force_bots
#########*/

enum SpawnType
{
    SPAWNTYPE_TRIPWIRE_ROOFTOP,                             // no warning, summon Creature at smaller range
    SPAWNTYPE_ALARMBOT,                                     // cast guards mark and summon npc - if plr shows up with that buff duration < 5 seconds attack
};

struct SpawnAssociation
{
    uint32 thisCreatureEntry;
    uint32 spawnedCreatureEntry;
    SpawnType spawnType;
};

enum eEnums
{
    SPELL_GUARDS_MARK               = 38067,
    AURA_DURATION_TIME_LEFT         = 5000
};

float const RANGE_TRIPWIRE          = 15.0f;
float const RANGE_GUARDS_MARK       = 50.0f;

SpawnAssociation spawnAssociations[] =
{
    {2614,  15241, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Alliance)
    {2615,  15242, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Horde)
    {21974, 21976, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Area 52)
    {21993, 15242, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Horde - Bat Rider)
    {21996, 15241, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Alliance - Gryphon)
    {21997, 21976, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Goblin - Area 52 - Zeppelin)
    {21999, 15241, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Alliance)
    {22001, 15242, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Horde)
    {22002, 15242, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Ground (Horde)
    {22003, 15241, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Ground (Alliance)
    {22063, 21976, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Goblin - Area 52)
    {22065, 22064, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Ethereal - Stormspire)
    {22066, 22067, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Scryer - Dragonhawk)
    {22068, 22064, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Ethereal - Stormspire)
    {22069, 22064, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Stormspire)
    {22070, 22067, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Scryer)
    {22071, 22067, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Scryer)
    {22078, 22077, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Aldor)
    {22079, 22077, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Aldor - Gryphon)
    {22080, 22077, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Aldor)
    {22086, 22085, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Sporeggar)
    {22087, 22085, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Sporeggar - Spore Bat)
    {22088, 22085, SPAWNTYPE_TRIPWIRE_ROOFTOP},             //Air Force Trip Wire - Rooftop (Sporeggar)
    {22090, 22089, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Toshley's Station - Flying Machine)
    {22124, 22122, SPAWNTYPE_ALARMBOT},                     //Air Force Alarm Bot (Cenarion)
    {22125, 22122, SPAWNTYPE_ALARMBOT},                     //Air Force Guard Post (Cenarion - Stormcrow)
    {22126, 22122, SPAWNTYPE_ALARMBOT}                      //Air Force Trip Wire - Rooftop (Cenarion Expedition)
};

class npc_air_force_bots : public CreatureScript
{
public:
    npc_air_force_bots() : CreatureScript("npc_air_force_bots") { }

    struct npc_air_force_botsAI : public ScriptedAI
    {
        npc_air_force_botsAI(Creature * cr) : ScriptedAI(cr)
        {
            SpawnAssoc = NULL;
            SpawnedGUID = 0;

            // find the correct spawnhandling
            static uint32 entryCount = sizeof(spawnAssociations) / sizeof(SpawnAssociation);

            for (uint8 i = 0; i < entryCount; ++i)
            {
                if (spawnAssociations[i].thisCreatureEntry == cr->GetEntry())
                {
                    SpawnAssoc = &spawnAssociations[i];
                    break;
                }
            }

            if (!SpawnAssoc)
                sLog->outErrorDb("TCSR: Creature template entry %u has ScriptName npc_air_force_bots, but it's not handled by that script", cr->GetEntry());
            else
            {
                CreatureTemplate const * spawnedTemplate = sObjectMgr->GetCreatureTemplate(SpawnAssoc->spawnedCreatureEntry);

                if (!spawnedTemplate)
                {
                    SpawnAssoc = NULL;
                    sLog->outErrorDb("TCSR: Creature template entry %u does not exist in DB, which is required by npc_air_force_bots", SpawnAssoc->spawnedCreatureEntry);
                    return;
                }
            }
        }

        SpawnAssociation* SpawnAssoc;
        uint64 SpawnedGUID;

        void Reset() {}

        Creature * SummonGuard()
        {
            Creature * summoned = me->SummonCreature(SpawnAssoc->spawnedCreatureEntry, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 300000);

            if (summoned)
                SpawnedGUID = summoned->GetGUID();
            else
            {
                sLog->outErrorDb("TCSR: npc_air_force_bots: wasn't able to spawn Creature %u", SpawnAssoc->spawnedCreatureEntry);
                SpawnAssoc = NULL;
            }

            return summoned;
        }

        Creature * GetSummonedGuard()
        {
            Creature * cr = Unit::GetCreature(*me, SpawnedGUID);

            if (cr && cr->isAlive())
                return cr;

            return NULL;
        }

        void MoveInLineOfSight(Unit * who)
        {
            if (!SpawnAssoc)
                return;

            if (me->IsValidAttackTarget(who))
            {
                Player * playerTarget = who->ToPlayer();

                // airforce guards only spawn for players
                if (!playerTarget)
                    return;

                Creature * lastSpawnedGuard = SpawnedGUID == 0 ? NULL : GetSummonedGuard();

                // prevent calling Unit::GetUnit at next MoveInLineOfSight call - speedup
                if (!lastSpawnedGuard)
                    SpawnedGUID = 0;

                switch (SpawnAssoc->spawnType)
                {
                    case SPAWNTYPE_ALARMBOT:
                    {
                        if (!who->IsWithinDistInMap(me, RANGE_GUARDS_MARK))
                            return;

                        Aura* markAura = who->GetAura(SPELL_GUARDS_MARK);
                        if (markAura)
                        {
                            // the target wasn't able to move out of our range within 25 seconds
                            if (!lastSpawnedGuard)
                            {
                                lastSpawnedGuard = SummonGuard();

                                if (!lastSpawnedGuard)
                                    return;
                            }

                            if (markAura->GetDuration() < AURA_DURATION_TIME_LEFT)
                                if (!lastSpawnedGuard->getVictim())
                                    lastSpawnedGuard->AI()->AttackStart(who);
                        }
                        else
                        {
                            if (!lastSpawnedGuard)
                                lastSpawnedGuard = SummonGuard();

                            if (!lastSpawnedGuard)
                                return;

                            lastSpawnedGuard->CastSpell(who, SPELL_GUARDS_MARK, true);
                        }
                        break;
                    }
                    case SPAWNTYPE_TRIPWIRE_ROOFTOP:
                    {
                        if (!who->IsWithinDistInMap(me, RANGE_TRIPWIRE))
                            return;

                        if (!lastSpawnedGuard)
                            lastSpawnedGuard = SummonGuard();

                        if (!lastSpawnedGuard)
                            return;

                        // ROOFTOP only triggers if the plr is on the ground
                        if (!playerTarget->IsFlying() && !lastSpawnedGuard->getVictim())
                            lastSpawnedGuard->AI()->AttackStart(who);

                        break;
                    }
                }
            }
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_air_force_botsAI(cr);
    }
};

/*######
## npc_lunaclaw_spirit
######*/

enum
{
    QUEST_BODY_HEART_A      = 6001,
    QUEST_BODY_HEART_H      = 6002,

    TEXT_ID_DEFAULT         = 4714,
    TEXT_ID_PROGRESS        = 4715
};

#define GOSSIP_ITEM_GRANT   "You have thought well, spirit. I ask you to grant me the strength of your body and the strength of your heart."

class npc_lunaclaw_spirit : public CreatureScript
{
public:
    npc_lunaclaw_spirit() : CreatureScript("npc_lunaclaw_spirit") { }

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        if (plr->GetQuestStatus(QUEST_BODY_HEART_A) == QUEST_STATUS_INCOMPLETE || plr->GetQuestStatus(QUEST_BODY_HEART_H) == QUEST_STATUS_INCOMPLETE)
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_GRANT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        plr->SEND_GOSSIP_MENU(TEXT_ID_DEFAULT, cr->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player * plr, Creature * cr, uint32 /*sender*/, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            plr->SEND_GOSSIP_MENU(TEXT_ID_PROGRESS, cr->GetGUID());
            plr->AreaExploredOrEventHappens(plr->GetTeam() == ALLIANCE ? QUEST_BODY_HEART_A : QUEST_BODY_HEART_H);
        }
        return true;
    }
};

/*########
# npc_chicken_cluck
#########*/

#define EMOTE_HELLO         -1070004
#define EMOTE_CLUCK_TEXT    -1070006

#define QUEST_CLUCK         3861
#define FACTION_FRIENDLY    35
#define FACTION_CHICKEN     31

class npc_chicken_cluck : public CreatureScript
{
public:
    npc_chicken_cluck() : CreatureScript("npc_chicken_cluck") { }

    struct npc_chicken_cluckAI : public ScriptedAI
    {
        npc_chicken_cluckAI(Creature * cr) : ScriptedAI(cr) {}

        uint32 ResetFlagTimer;

        void Reset()
        {
            ResetFlagTimer = 120000;
            me->setFaction(FACTION_CHICKEN);
            me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        }

        void EnterCombat(Unit * /*who*/) {}

        void UpdateAI(uint32 const diff)
        {
            // Reset flags after a certain time has passed so that the next plr has to start the 'event' again
            if (me->HasFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
            {
                if (ResetFlagTimer <= diff)
                {
                    EnterEvadeMode();
                    return;
                }
                else
                    ResetFlagTimer -= diff;
            }

            if (UpdateVictim())
                DoMeleeAttackIfReady();
        }

        void ReceiveEmote(Player * plr, uint32 emote)
        {
            switch (emote)
            {
                case TEXT_EMOTE_CHICKEN:
                    if (plr->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_NONE && rand() % 30 == 1)
                    {
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        me->setFaction(FACTION_FRIENDLY);
                        DoScriptText(EMOTE_HELLO, me);
                    }
                    break;
                case TEXT_EMOTE_CHEER:
                    if (plr->GetQuestStatus(QUEST_CLUCK) == QUEST_STATUS_COMPLETE)
                    {
                        me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
                        me->setFaction(FACTION_FRIENDLY);
                        DoScriptText(EMOTE_CLUCK_TEXT, me);
                    }
                    break;
            }
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_chicken_cluckAI(cr);
    }

    bool OnQuestAccept(Player * /*plr*/, Creature * cr, Quest const * quest)
    {
        if (quest->GetQuestId() == QUEST_CLUCK)
            CAST_AI(npc_chicken_cluck::npc_chicken_cluckAI, cr->AI())->Reset();

        return true;
    }

    bool OnQuestComplete(Player * /*plr*/, Creature * cr, Quest const * quest)
    {
        if (quest->GetQuestId() == QUEST_CLUCK)
            CAST_AI(npc_chicken_cluck::npc_chicken_cluckAI, cr->AI())->Reset();

        return true;
    }
};

/*######
## npc_dancing_flames
######*/

#define SPELL_BRAZIER       45423
#define SPELL_SEDUCTION     47057
#define SPELL_FIERY_AURA    45427

class npc_dancing_flames : public CreatureScript
{
public:
    npc_dancing_flames() : CreatureScript("npc_dancing_flames") { }

    struct npc_dancing_flamesAI : public ScriptedAI
    {
        npc_dancing_flamesAI(Creature * cr) : ScriptedAI(cr) {}

        bool Active;
        uint32 CanIteract;

        void Reset()
        {
            Active = true;
            CanIteract = 3500;
            DoCast(me, SPELL_BRAZIER, true);
            DoCast(me, SPELL_FIERY_AURA, false);
            float x, y, z;
            me->GetPosition(x, y, z);
            me->Relocate(x, y, z + 0.94f);
            me->AddUnitMovementFlag(MOVEMENTFLAG_LEVITATING);
            me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
            WorldPacket data;                       //send update position to client
            me->BuildHeartBeatMsg(&data);
            me->SendMessageToSet(&data, true);
        }

        void UpdateAI(uint32 const diff)
        {
            if (!Active)
            {
                if (CanIteract <= diff)
                {
                    Active = true;
                    CanIteract = 3500;
                    me->HandleEmoteCommand(EMOTE_ONESHOT_DANCE);
                }
                else
                    CanIteract -= diff;
            }
        }

        void EnterCombat(Unit * /*who*/){}

        void ReceiveEmote(Player * plr, uint32 emote)
        {
            if (me->IsWithinLOS(plr->GetPositionX(), plr->GetPositionY(), plr->GetPositionZ()) && me->IsWithinDistInMap(plr, 30.0f))
            {
                me->SetInFront(plr);
                Active = false;

                WorldPacket data;
                me->BuildHeartBeatMsg(&data);
                me->SendMessageToSet(&data, true);
                switch (emote)
                {
                    case TEXT_EMOTE_KISS:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_SHY);
                        break;
                    case TEXT_EMOTE_WAVE:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_WAVE);
                        break;
                    case TEXT_EMOTE_BOW:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_BOW);
                        break;
                    case TEXT_EMOTE_JOKE:
                        me->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                        break;
                    case TEXT_EMOTE_DANCE:
                        if (!plr->HasAura(SPELL_SEDUCTION))
                            DoCast(plr, SPELL_SEDUCTION, true);
                        break;
                }
            }
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_dancing_flamesAI(cr);
    }
};

/*######
## Triage quest
######*/

//signed for 9623
#define SAY_DOC1    -1000201
#define SAY_DOC2    -1000202
#define SAY_DOC3    -1000203

#define DOCTOR_ALLIANCE     12939
#define DOCTOR_HORDE        12920
#define ALLIANCE_COORDS     7
#define HORDE_COORDS        6

struct Location
{
    float x, y, z, o;
};

static Location AllianceCoords[]=
{
    {-3757.38f, -4533.05f, 14.16f, 3.62f},                      // Top-far-right bunk as seen from entrance
    {-3754.36f, -4539.13f, 14.16f, 5.13f},                      // Top-far-left bunk
    {-3749.54f, -4540.25f, 14.28f, 3.34f},                      // Far-right bunk
    {-3742.10f, -4536.85f, 14.28f, 3.64f},                      // Right bunk near entrance
    {-3755.89f, -4529.07f, 14.05f, 0.57f},                      // Far-left bunk
    {-3749.51f, -4527.08f, 14.07f, 5.26f},                      // Mid-left bunk
    {-3746.37f, -4525.35f, 14.16f, 5.22f},                      // Left bunk near entrance
};

//alliance run to where
#define A_RUNTOX -3742.96f
#define A_RUNTOY -4531.52f
#define A_RUNTOZ 11.91f

static Location HordeCoords[]=
{
    {-1013.75f, -3492.59f, 62.62f, 4.34f},                      // Left, Behind
    {-1017.72f, -3490.92f, 62.62f, 4.34f},                      // Right, Behind
    {-1015.77f, -3497.15f, 62.82f, 4.34f},                      // Left, Mid
    {-1019.51f, -3495.49f, 62.82f, 4.34f},                      // Right, Mid
    {-1017.25f, -3500.85f, 62.98f, 4.34f},                      // Left, front
    {-1020.95f, -3499.21f, 62.98f, 4.34f}                       // Right, Front
};

//horde run to where
#define H_RUNTOX -1016.44f
#define H_RUNTOY -3508.48f
#define H_RUNTOZ 62.96f

uint32 const AllianceSoldierId[3] =
{
    12938,                                                  // 12938 Injured Alliance Soldier
    12936,                                                  // 12936 Badly injured Alliance Soldier
    12937                                                   // 12937 Critically injured Alliance Soldier
};

uint32 const HordeSoldierId[3] =
{
    12923,                                                  //12923 Injured Soldier
    12924,                                                  //12924 Badly injured Soldier
    12925                                                   //12925 Critically injured Soldier
};

/*######
## npc_doctor (handles both Gustaf Vanhowzen and Gregory Victor)
######*/
class npc_doctor : public CreatureScript
{
public:
    npc_doctor() : CreatureScript("npc_doctor") {}

    struct npc_doctorAI : public ScriptedAI
    {
        npc_doctorAI(Creature * cr) : ScriptedAI(cr) {}

        uint64 PlayerGUID;

        uint32 SummonPatientTimer;
        uint32 SummonPatientCount;
        uint32 PatientDiedCount;
        uint32 PatientSavedCount;

        bool Event;

        std::list<uint64> Patients;
        std::vector<Location *> Coordinates;

        void Reset()
        {
            PlayerGUID = 0;

            SummonPatientTimer = 10000;
            SummonPatientCount = 0;
            PatientDiedCount = 0;
            PatientSavedCount = 0;

            Patients.clear();
            Coordinates.clear();

            Event = false;

            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void BeginEvent(Player * plr)
        {
            PlayerGUID = plr->GetGUID();

            SummonPatientTimer = 10000;
            SummonPatientCount = 0;
            PatientDiedCount = 0;
            PatientSavedCount = 0;

            switch (me->GetEntry())
            {
                case DOCTOR_ALLIANCE:
                    for (uint8 i = 0; i < ALLIANCE_COORDS; ++i)
                        Coordinates.push_back(&AllianceCoords[i]);
                    break;
                case DOCTOR_HORDE:
                    for (uint8 i = 0; i < HORDE_COORDS; ++i)
                        Coordinates.push_back(&HordeCoords[i]);
                    break;
            }

            Event = true;
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void PatientDied(Location * point)
        {
            Player * plr = Unit::GetPlayer(*me, PlayerGUID);
            if (plr && ((plr->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (plr->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)))
            {
                ++PatientDiedCount;

                if (PatientDiedCount > 5 && Event)
                {
                    if (plr->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE)
                        plr->FailQuest(6624);
                    else if (plr->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)
                        plr->FailQuest(6622);

                    Reset();
                    return;
                }

                Coordinates.push_back(point);
            }
            else
                // If no plr or plr abandon quest in progress
                Reset();
        }

        void PatientSaved(Creature * /*soldier*/, Player * plr, Location * point)
        {
            if (plr && PlayerGUID == plr->GetGUID())
            {
                if ((plr->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (plr->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
                {
                    ++PatientSavedCount;

                    if (PatientSavedCount == 15)
                    {
                        if (!Patients.empty())
                        {
                            std::list<uint64>::const_iterator itr;
                            for (itr = Patients.begin(); itr != Patients.end(); ++itr)
                            {
                                if (Creature * patient = Unit::GetCreature((*me), *itr))
                                    patient->setDeathState(JUST_DIED);
                            }
                        }

                        if (plr->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE)
                            plr->AreaExploredOrEventHappens(6624);
                        else if (plr->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE)
                            plr->AreaExploredOrEventHappens(6622);

                        Reset();
                        return;
                    }

                    Coordinates.push_back(point);
                }
            }
        }

        void UpdateAI(uint32 const diff);

        void EnterCombat(Unit * /*who*/){}
    };

    bool OnQuestAccept(Player * plr, Creature * cr, Quest const * quest)
    {
        if ((quest->GetQuestId() == 6624) || (quest->GetQuestId() == 6622))
            CAST_AI(npc_doctor::npc_doctorAI, cr->AI())->BeginEvent(plr);

        return true;
    }

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_doctorAI(cr);
    }
};

/*#####
## npc_injured_patient (handles all the patients, no matter Horde or Alliance)
#####*/

class npc_injured_patient : public CreatureScript
{
public:
    npc_injured_patient() : CreatureScript("npc_injured_patient") { }

    struct npc_injured_patientAI : public ScriptedAI
    {
        npc_injured_patientAI(Creature * cr) : ScriptedAI(cr) {}

        uint64 DoctorGUID;
        Location * Coord;

        void Reset()
        {
            DoctorGUID = 0;
            Coord = NULL;

            //no select
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

            //no regen health
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

            //to make them lay with face down
            me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_DEAD);

            uint32 mobId = me->GetEntry();

            switch (mobId)
            {                                                   //lower max health
                case 12923:
                case 12938:                                     //Injured Soldier
                    me->SetHealth(me->CountPctFromMaxHealth(75));
                    break;
                case 12924:
                case 12936:                                     //Badly injured Soldier
                    me->SetHealth(me->CountPctFromMaxHealth(50));
                    break;
                case 12925:
                case 12937:                                     //Critically injured Soldier
                    me->SetHealth(me->CountPctFromMaxHealth(25));
                    break;
            }
        }

        void EnterCombat(Unit * /*who*/){}

        void SpellHit(Unit * caster, SpellInfo const * spell)
        {
            if (caster->GetTypeId() == TYPEID_PLAYER && me->isAlive() && spell->Id == 20804)
            {
                if ((CAST_PLR(caster)->GetQuestStatus(6624) == QUEST_STATUS_INCOMPLETE) || (CAST_PLR(caster)->GetQuestStatus(6622) == QUEST_STATUS_INCOMPLETE))
                    if (DoctorGUID)
                        if (Creature * doctor = Unit::GetCreature(*me, DoctorGUID))
                            CAST_AI(npc_doctor::npc_doctorAI, doctor->AI())->PatientSaved(me, CAST_PLR(caster), Coord);

                //make not selectable
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);

                //regen health
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);

                //stand up
                me->SetUInt32Value(UNIT_FIELD_BYTES_1, UNIT_STAND_STATE_STAND);

                DoScriptText(RAND(SAY_DOC1, SAY_DOC2, SAY_DOC3), me);

                uint32 mobId = me->GetEntry();
                me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);

                switch (mobId)
                {
                    case 12923:
                    case 12924:
                    case 12925:
                        me->GetMotionMaster()->MovePoint(0, H_RUNTOX, H_RUNTOY, H_RUNTOZ);
                        break;
                    case 12936:
                    case 12937:
                    case 12938:
                        me->GetMotionMaster()->MovePoint(0, A_RUNTOX, A_RUNTOY, A_RUNTOZ);
                        break;
                }
            }
        }

        void UpdateAI(uint32 const /*diff*/)
        {
            //lower HP on every world tick makes it a useful counter, not officlone though
            if (me->isAlive() && me->GetHealth() > 6)
                me->ModifyHealth(-5);

            if (me->isAlive() && me->GetHealth() <= 6)
            {
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IN_COMBAT);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->setDeathState(JUST_DIED);
                me->SetFlag(UNIT_DYNAMIC_FLAGS, 32);

                if (DoctorGUID)
                    if (Creature * doctor = Unit::GetCreature((*me), DoctorGUID))
                        CAST_AI(npc_doctor::npc_doctorAI, doctor->AI())->PatientDied(Coord);
            }
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_injured_patientAI(cr);
    }
};

void npc_doctor::npc_doctorAI::UpdateAI(uint32 const diff)
{
    if (Event && SummonPatientCount >= 20)
    {
        Reset();
        return;
    }

    if (Event)
    {
        if (SummonPatientTimer <= diff)
        {
            if (Coordinates.empty())
                return;

            std::vector<Location *>::iterator itr = Coordinates.begin() + rand() % Coordinates.size();
            uint32 patientEntry = 0;

            switch (me->GetEntry())
            {
                case DOCTOR_ALLIANCE:
                    patientEntry = AllianceSoldierId[rand() % 3];
                    break;
                case DOCTOR_HORDE:
                    patientEntry = HordeSoldierId[rand() % 3];
                    break;
                default:
                    sLog->outError("TSCR: Invalid entry for Triage doctor. Please check your database");
                    return;
            }

            if (Location * point = *itr)
            {
                if (Creature * Patient = me->SummonCreature(patientEntry, point->x, point->y, point->z, point->o, TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 5000))
                {
                    //303, this flag appear to be required for client side item->spell to work (TARGET_SINGLE_FRIEND)
                    Patient->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PVP_ATTACKABLE);

                    Patients.push_back(Patient->GetGUID());
                    CAST_AI(npc_injured_patient::npc_injured_patientAI, Patient->AI())->DoctorGUID = me->GetGUID();
                    CAST_AI(npc_injured_patient::npc_injured_patientAI, Patient->AI())->Coord = point;

                    Coordinates.erase(itr);
                }
            }
            SummonPatientTimer = 10000;
            ++SummonPatientCount;
        }
        else
            SummonPatientTimer -= diff;
    }
}

/*######
## npc_garments_of_quests
######*/

//TODO: get text for each NPC

enum eGarments
{
    SPELL_LESSER_HEAL_R2    = 2052,
    SPELL_FORTITUDE_R1      = 1243,

    QUEST_MOON              = 5621,
    QUEST_LIGHT_1           = 5624,
    QUEST_LIGHT_2           = 5625,
    QUEST_SPIRIT            = 5648,
    QUEST_DARKNESS          = 5650,

    ENTRY_SHAYA             = 12429,
    ENTRY_ROBERTS           = 12423,
    ENTRY_DOLF              = 12427,
    ENTRY_KORJA             = 12430,
    ENTRY_DG_KEL            = 12428,

    //used by 12429, 12423, 12427, 12430, 12428, but signed for 12429
    SAY_COMMON_HEALED       = -1000164,
    SAY_DG_KEL_THANKS       = -1000165,
    SAY_DG_KEL_GOODBYE      = -1000166,
    SAY_ROBERTS_THANKS      = -1000167,
    SAY_ROBERTS_GOODBYE     = -1000168,
    SAY_KORJA_THANKS        = -1000169,
    SAY_KORJA_GOODBYE       = -1000170,
    SAY_DOLF_THANKS         = -1000171,
    SAY_DOLF_GOODBYE        = -1000172,
    SAY_SHAYA_THANKS        = -1000173,
    SAY_SHAYA_GOODBYE       = -1000174, //signed for 21469
};

class npc_garments_of_quests : public CreatureScript
{
public:
    npc_garments_of_quests() : CreatureScript("npc_garments_of_quests") { }

    struct npc_garments_of_questsAI : public npc_escortAI
    {
        npc_garments_of_questsAI(Creature * cr) : npc_escortAI(cr) {Reset();}

        uint64 CasterGUID;

        bool IsHealed;
        bool CanRun;

        uint32 RunAwayTimer;

        void Reset()
        {
            CasterGUID = 0;

            IsHealed = false;
            CanRun = false;

            RunAwayTimer = 5000;

            me->SetStandState(UNIT_STAND_STATE_KNEEL);
            //expect database to have RegenHealth=0
            me->SetHealth(me->CountPctFromMaxHealth(70));
        }

        void EnterCombat(Unit * /*who*/) {}

        void SpellHit(Unit * caster, SpellInfo const * Spell)
        {
            if (Spell->Id == SPELL_LESSER_HEAL_R2 || Spell->Id == SPELL_FORTITUDE_R1)
            {
                //not while in combat
                if (me->isInCombat())
                    return;

                //nothing to be done now
                if (IsHealed && CanRun)
                    return;

                if (Player * plr = caster->ToPlayer())
                {
                    switch (me->GetEntry())
                    {
                        case ENTRY_SHAYA:
                            if (plr->GetQuestStatus(QUEST_MOON) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_SHAYA_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                        case ENTRY_ROBERTS:
                            if (plr->GetQuestStatus(QUEST_LIGHT_1) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_ROBERTS_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                        case ENTRY_DOLF:
                            if (plr->GetQuestStatus(QUEST_LIGHT_2) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_DOLF_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                        case ENTRY_KORJA:
                            if (plr->GetQuestStatus(QUEST_SPIRIT) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_KORJA_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                        case ENTRY_DG_KEL:
                            if (plr->GetQuestStatus(QUEST_DARKNESS) == QUEST_STATUS_INCOMPLETE)
                            {
                                if (IsHealed && !CanRun && Spell->Id == SPELL_FORTITUDE_R1)
                                {
                                    DoScriptText(SAY_DG_KEL_THANKS, me, caster);
                                    CanRun = true;
                                }
                                else if (!IsHealed && Spell->Id == SPELL_LESSER_HEAL_R2)
                                {
                                    CasterGUID = caster->GetGUID();
                                    me->SetStandState(UNIT_STAND_STATE_STAND);
                                    DoScriptText(SAY_COMMON_HEALED, me, caster);
                                    IsHealed = true;
                                }
                            }
                            break;
                    }

                    //give quest credit, not expect any special quest objectives
                    if (CanRun)
                        plr->TalkedToCreature(me->GetEntry(), me->GetGUID());
                }
            }
        }

        void WaypointReached(uint32 /*point*/)
        {
        }

        void UpdateAI(uint32 const diff)
        {
            if (CanRun && !me->isInCombat())
            {
                if (RunAwayTimer <= diff)
                {
                    if (Unit * unit = Unit::GetUnit(*me, CasterGUID))
                    {
                        switch (me->GetEntry())
                        {
                            case ENTRY_SHAYA:
                                DoScriptText(SAY_SHAYA_GOODBYE, me, unit);
                                break;
                            case ENTRY_ROBERTS:
                                DoScriptText(SAY_ROBERTS_GOODBYE, me, unit);
                                break;
                            case ENTRY_DOLF:
                                DoScriptText(SAY_DOLF_GOODBYE, me, unit);
                                break;
                            case ENTRY_KORJA:
                                DoScriptText(SAY_KORJA_GOODBYE, me, unit);
                                break;
                            case ENTRY_DG_KEL:
                                DoScriptText(SAY_DG_KEL_GOODBYE, me, unit);
                                break;
                        }

                        Start(false, true, true);
                    }
                    else
                        EnterEvadeMode();                       //something went wrong

                    RunAwayTimer = 30000;
                }
                else
                    RunAwayTimer -= diff;
            }

            npc_escortAI::UpdateAI(diff);
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_garments_of_questsAI(cr);
    }
};

/*######
## npc_kingdom_of_dalaran_quests
######*/

enum eKingdomDalaran
{
    SPELL_TELEPORT_DALARAN  = 53360,
    ITEM_KT_SIGNET          = 39740,
    QUEST_MAGICAL_KINGDOM_A = 12794,
    QUEST_MAGICAL_KINGDOM_H = 12791,
    QUEST_MAGICAL_KINGDOM_N = 12796
};

#define GOSSIP_ITEM_TELEPORT_TO "I am ready to be teleported to Dalaran."

class npc_kingdom_of_dalaran_quests : public CreatureScript
{
public:
    npc_kingdom_of_dalaran_quests() : CreatureScript("npc_kingdom_of_dalaran_quests") { }
    bool OnGossipHello(Player * plr, Creature * cr)
    {
        if (cr->isQuestGiver())
            plr->PrepareQuestMenu(cr->GetGUID());

        if (plr->HasItemCount(ITEM_KT_SIGNET, 1) && (!plr->GetQuestRewardStatus(QUEST_MAGICAL_KINGDOM_A) ||
            !plr->GetQuestRewardStatus(QUEST_MAGICAL_KINGDOM_H) || !plr->GetQuestRewardStatus(QUEST_MAGICAL_KINGDOM_N)))
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ITEM_TELEPORT_TO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        plr->SEND_GOSSIP_MENU(plr->GetGossipTextId(cr), cr->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player * plr, Creature * /* cr*/, uint32 /*sender*/, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_INFO_DEF + 1)
        {
            plr->CLOSE_GOSSIP_MENU();
            plr->CastSpell(plr, SPELL_TELEPORT_DALARAN, false);
        }
        return true;
    }
};

/*######
## npc_mount_vendor
######*/

class npc_mount_vendor : public CreatureScript
{
public:
    npc_mount_vendor() : CreatureScript("npc_mount_vendor") { }

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        if (cr->isQuestGiver())
            plr->PrepareQuestMenu(cr->GetGUID());

        bool canBuy = false;
        uint32 vendor = cr->GetEntry();
        uint8 race = plr->getRace();

        switch (vendor)
        {
            case 384:                                           //Katie Hunter
            case 1460:                                          //Unger Statforth
            case 2357:                                          //Merideth Carlson
            case 4885:                                          //Gregor MacVince
                if (plr->GetReputationRank(72) != REP_EXALTED && race != RACE_HUMAN)
                    plr->SEND_GOSSIP_MENU(5855, cr->GetGUID());
                else canBuy = true;
                break;
            case 1261:                                          //Veron Amberstill
                if (plr->GetReputationRank(47) != REP_EXALTED && race != RACE_DWARF)
                    plr->SEND_GOSSIP_MENU(5856, cr->GetGUID());
                else canBuy = true;
                break;
            case 3362:                                          //Ogunaro Wolfrunner
                if (plr->GetReputationRank(76) != REP_EXALTED && race != RACE_ORC)
                    plr->SEND_GOSSIP_MENU(5841, cr->GetGUID());
                else canBuy = true;
                break;
            case 3685:                                          //Harb Clawhoof
                if (plr->GetReputationRank(81) != REP_EXALTED && race != RACE_TAUREN)
                    plr->SEND_GOSSIP_MENU(5843, cr->GetGUID());
                else canBuy = true;
                break;
            case 4730:                                          //Lelanai
                if (plr->GetReputationRank(69) != REP_EXALTED && race != RACE_NIGHTELF)
                    plr->SEND_GOSSIP_MENU(5844, cr->GetGUID());
                else canBuy = true;
                break;
            case 4731:                                          //Zachariah Post
                if (plr->GetReputationRank(68) != REP_EXALTED && race != RACE_UNDEAD_PLAYER)
                    plr->SEND_GOSSIP_MENU(5840, cr->GetGUID());
                else canBuy = true;
                break;
            case 7952:                                          //Zjolnir
                if (plr->GetReputationRank(530) != REP_EXALTED && race != RACE_TROLL)
                    plr->SEND_GOSSIP_MENU(5842, cr->GetGUID());
                else canBuy = true;
                break;
            case 7955:                                          //Milli Featherwhistle
                if (plr->GetReputationRank(54) != REP_EXALTED && race != RACE_GNOME)
                    plr->SEND_GOSSIP_MENU(5857, cr->GetGUID());
                else canBuy = true;
                break;
            case 16264:                                         //Winaestra
                if (plr->GetReputationRank(911) != REP_EXALTED && race != RACE_BLOODELF)
                    plr->SEND_GOSSIP_MENU(10305, cr->GetGUID());
                else canBuy = true;
                break;
            case 17584:                                         //Torallius the Pack Handler
                if (plr->GetReputationRank(930) != REP_EXALTED && race != RACE_DRAENEI)
                    plr->SEND_GOSSIP_MENU(10239, cr->GetGUID());
                else canBuy = true;
                break;
        }

        if (canBuy)
        {
            if (cr->isVendor())
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);
            plr->SEND_GOSSIP_MENU(plr->GetGossipTextId(cr), cr->GetGUID());
        }
        return true;
    }

    bool OnGossipSelect(Player * plr, Creature * cr, uint32 /*sender*/, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_TRADE)
            plr->GetSession()->SendListInventory(cr->GetGUID());

        return true;
    }
};

/*######
## npc_rogue_trainer
######*/

#define GOSSIP_HELLO_ROGUE1 "I wish to unlearn my talents"
#define GOSSIP_HELLO_ROGUE2 "<Take the letter>"
#define GOSSIP_HELLO_ROGUE3 "Purchase a Dual Talent Specialization."

class npc_rogue_trainer : public CreatureScript
{
public:
    npc_rogue_trainer() : CreatureScript("npc_rogue_trainer") { }

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        if (cr->isQuestGiver())
            plr->PrepareQuestMenu(cr->GetGUID());

        if (cr->isTrainer())
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_TEXT_TRAIN, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRAIN);

        if (cr->isCanTrainingAndResetTalentsOf(plr))
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_HELLO_ROGUE1, GOSSIP_SENDER_MAIN, GOSSIP_OPTION_UNLEARNTALENTS);

        if (plr->GetSpecsCount() == 1 && cr->isCanTrainingAndResetTalentsOf(plr) && plr->getLevel() >= sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL))
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_TRAINER, GOSSIP_HELLO_ROGUE3, GOSSIP_SENDER_MAIN, GOSSIP_OPTION_LEARNDUALSPEC);

        if (plr->getClass() == CLASS_ROGUE && plr->getLevel() >= 24 && !plr->HasItemCount(17126, 1) && !plr->GetQuestRewardStatus(6681))
        {
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_ROGUE2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            plr->SEND_GOSSIP_MENU(5996, cr->GetGUID());
        } else
            plr->SEND_GOSSIP_MENU(plr->GetGossipTextId(cr), cr->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player * plr, Creature * cr, uint32 /*sender*/, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, 21100, false);
                break;
            case GOSSIP_ACTION_TRAIN:
                plr->GetSession()->SendTrainerList(cr->GetGUID());
                break;
            case GOSSIP_OPTION_UNLEARNTALENTS:
                plr->CLOSE_GOSSIP_MENU();
                plr->SendTalentWipeConfirm(cr->GetGUID());
                break;
            case GOSSIP_OPTION_LEARNDUALSPEC:
                if (plr->GetSpecsCount() == 1 && !(plr->getLevel() < sWorld->getIntConfig(CONFIG_MIN_DUALSPEC_LEVEL)))
                {
                    if (!plr->HasEnoughMoney(10000000))
                    {
                        plr->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
                        plr->PlayerTalkClass->SendCloseGossip();
                        break;
                    }
                    else
                    {
                        plr->ModifyMoney(-10000000);

                        // Cast spells that teach dual spec
                        // Both are also ImplicitTarget self and must be cast by plr
                        plr->CastSpell(plr, 63680, true, NULL, NULL, plr->GetGUID());
                        plr->CastSpell(plr, 63624, true, NULL, NULL, plr->GetGUID());

                        // Should show another Gossip text with "Congratulations..."
                        plr->PlayerTalkClass->SendCloseGossip();
                    }
                }
                break;
        }
        return true;
    }
};

/*######
## npc_sayge
######*/

#define SPELL_DMG       23768                               //dmg
#define SPELL_RES       23769                               //res
#define SPELL_ARM       23767                               //arm
#define SPELL_SPI       23738                               //spi
#define SPELL_INT       23766                               //int
#define SPELL_STM       23737                               //stm
#define SPELL_STR       23735                               //str
#define SPELL_AGI       23736                               //agi
#define SPELL_FORTUNE   23765                               //faire fortune

#define GOSSIP_HELLO_SAYGE  "Yes"
#define GOSSIP_SENDACTION_SAYGE1    "Slay the Man"
#define GOSSIP_SENDACTION_SAYGE2    "Turn him over to liege"
#define GOSSIP_SENDACTION_SAYGE3    "Confiscate the corn"
#define GOSSIP_SENDACTION_SAYGE4    "Let him go and have the corn"
#define GOSSIP_SENDACTION_SAYGE5    "Execute your friend painfully"
#define GOSSIP_SENDACTION_SAYGE6    "Execute your friend painlessly"
#define GOSSIP_SENDACTION_SAYGE7    "Let your friend go"
#define GOSSIP_SENDACTION_SAYGE8    "Confront the diplomat"
#define GOSSIP_SENDACTION_SAYGE9    "Show not so quiet defiance"
#define GOSSIP_SENDACTION_SAYGE10   "Remain quiet"
#define GOSSIP_SENDACTION_SAYGE11   "Speak against your brother openly"
#define GOSSIP_SENDACTION_SAYGE12   "Help your brother in"
#define GOSSIP_SENDACTION_SAYGE13   "Keep your brother out without letting him know"
#define GOSSIP_SENDACTION_SAYGE14   "Take credit, keep gold"
#define GOSSIP_SENDACTION_SAYGE15   "Take credit, share the gold"
#define GOSSIP_SENDACTION_SAYGE16   "Let the knight take credit"
#define GOSSIP_SENDACTION_SAYGE17   "Thanks"

class npc_sayge : public CreatureScript
{
public:
    npc_sayge() : CreatureScript("npc_sayge") { }

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        if (cr->isQuestGiver())
            plr->PrepareQuestMenu(cr->GetGUID());

        if (plr->HasSpellCooldown(SPELL_INT) ||
            plr->HasSpellCooldown(SPELL_ARM) ||
            plr->HasSpellCooldown(SPELL_DMG) ||
            plr->HasSpellCooldown(SPELL_RES) ||
            plr->HasSpellCooldown(SPELL_STR) ||
            plr->HasSpellCooldown(SPELL_AGI) ||
            plr->HasSpellCooldown(SPELL_STM) ||
            plr->HasSpellCooldown(SPELL_SPI))
            plr->SEND_GOSSIP_MENU(7393, cr->GetGUID());
        else
        {
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO_SAYGE, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            plr->SEND_GOSSIP_MENU(7339, cr->GetGUID());
        }

        return true;
    }

    void SendAction(Player * plr, Creature * cr, uint32 action)
    {
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE1,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE2,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE3,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE4,            GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
                plr->SEND_GOSSIP_MENU(7340, cr->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE5,            GOSSIP_SENDER_MAIN + 1, GOSSIP_ACTION_INFO_DEF);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE6,            GOSSIP_SENDER_MAIN + 2, GOSSIP_ACTION_INFO_DEF);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE7,            GOSSIP_SENDER_MAIN + 3, GOSSIP_ACTION_INFO_DEF);
                plr->SEND_GOSSIP_MENU(7341, cr->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE8,            GOSSIP_SENDER_MAIN + 4, GOSSIP_ACTION_INFO_DEF);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE9,            GOSSIP_SENDER_MAIN + 5, GOSSIP_ACTION_INFO_DEF);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE10,           GOSSIP_SENDER_MAIN + 2, GOSSIP_ACTION_INFO_DEF);
                plr->SEND_GOSSIP_MENU(7361, cr->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 4:
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE11,           GOSSIP_SENDER_MAIN + 6, GOSSIP_ACTION_INFO_DEF);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE12,           GOSSIP_SENDER_MAIN + 7, GOSSIP_ACTION_INFO_DEF);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE13,           GOSSIP_SENDER_MAIN + 8, GOSSIP_ACTION_INFO_DEF);
                plr->SEND_GOSSIP_MENU(7362, cr->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 5:
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE14,           GOSSIP_SENDER_MAIN + 5, GOSSIP_ACTION_INFO_DEF);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE15,           GOSSIP_SENDER_MAIN + 4, GOSSIP_ACTION_INFO_DEF);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE16,           GOSSIP_SENDER_MAIN + 3, GOSSIP_ACTION_INFO_DEF);
                plr->SEND_GOSSIP_MENU(7363, cr->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF:
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SENDACTION_SAYGE17,           GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
                plr->SEND_GOSSIP_MENU(7364, cr->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 6:
                cr->CastSpell(plr, SPELL_FORTUNE, false);
                plr->SEND_GOSSIP_MENU(7365, cr->GetGUID());
                break;
        }
    }

    bool OnGossipSelect(Player * plr, Creature * cr, uint32 sender, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();
        switch (sender)
        {
            case GOSSIP_SENDER_MAIN:
                SendAction(plr, cr, action);
                break;
            case GOSSIP_SENDER_MAIN + 1:
                cr->CastSpell(plr, SPELL_DMG, false);
                plr->AddSpellCooldown(SPELL_DMG, 0, time(NULL) + 7200);
                SendAction(plr, cr, action);
                break;
            case GOSSIP_SENDER_MAIN + 2:
                cr->CastSpell(plr, SPELL_RES, false);
                plr->AddSpellCooldown(SPELL_RES, 0, time(NULL) + 7200);
                SendAction(plr, cr, action);
                break;
            case GOSSIP_SENDER_MAIN + 3:
                cr->CastSpell(plr, SPELL_ARM, false);
                plr->AddSpellCooldown(SPELL_ARM, 0, time(NULL) + 7200);
                SendAction(plr, cr, action);
                break;
            case GOSSIP_SENDER_MAIN + 4:
                cr->CastSpell(plr, SPELL_SPI, false);
                plr->AddSpellCooldown(SPELL_SPI, 0, time(NULL) + 7200);
                SendAction(plr, cr, action);
                break;
            case GOSSIP_SENDER_MAIN + 5:
                cr->CastSpell(plr, SPELL_INT, false);
                plr->AddSpellCooldown(SPELL_INT, 0, time(NULL) + 7200);
                SendAction(plr, cr, action);
                break;
            case GOSSIP_SENDER_MAIN + 6:
                cr->CastSpell(plr, SPELL_STM, false);
                plr->AddSpellCooldown(SPELL_STM, 0, time(NULL) + 7200);
                SendAction(plr, cr, action);
                break;
            case GOSSIP_SENDER_MAIN + 7:
                cr->CastSpell(plr, SPELL_STR, false);
                plr->AddSpellCooldown(SPELL_STR, 0, time(NULL) + 7200);
                SendAction(plr, cr, action);
                break;
            case GOSSIP_SENDER_MAIN + 8:
                cr->CastSpell(plr, SPELL_AGI, false);
                plr->AddSpellCooldown(SPELL_AGI, 0, time(NULL) + 7200);
                SendAction(plr, cr, action);
                break;
        }
        return true;
    }
};

class npc_steam_tonk : public CreatureScript
{
public:
    npc_steam_tonk() : CreatureScript("npc_steam_tonk") { }

    struct npc_steam_tonkAI : public ScriptedAI
    {
        npc_steam_tonkAI(Creature * cr) : ScriptedAI(cr) {}

        void Reset() {}
        void EnterCombat(Unit * /*who*/) {}

        void OnPossess(bool apply)
        {
            if (apply)
            {
                // Initialize the action bar without the melee attack command
                me->InitCharmInfo();
                me->GetCharmInfo()->InitEmptyActionBar(false);

                me->SetReactState(REACT_PASSIVE);
            }
            else
                me->SetReactState(REACT_AGGRESSIVE);
        }

    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_steam_tonkAI(cr);
    }
};

#define SPELL_TONK_MINE_DETONATE 25099

class npc_tonk_mine : public CreatureScript
{
public:
    npc_tonk_mine() : CreatureScript("npc_tonk_mine") { }

    struct npc_tonk_mineAI : public ScriptedAI
    {
        npc_tonk_mineAI(Creature * cr) : ScriptedAI(cr)
        {
            me->SetReactState(REACT_PASSIVE);
        }

        uint32 ExplosionTimer;

        void Reset()
        {
            ExplosionTimer = 3000;
        }

        void EnterCombat(Unit * /*who*/) {}
        void AttackStart(Unit * /*who*/, float /*dist*/ = 0) {}
        void MoveInLineOfSight(Unit * /*who*/) {}

        void UpdateAI(uint32 const diff)
        {
            if (ExplosionTimer <= diff)
            {
                DoCast(me, SPELL_TONK_MINE_DETONATE, true);
                me->setDeathState(DEAD); // unsummon it
            }
            else
                ExplosionTimer -= diff;
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_tonk_mineAI(cr);
    }
};

/*####
## npc_brewfest_reveler
####*/

class npc_brewfest_reveler : public CreatureScript
{
public:
    npc_brewfest_reveler() : CreatureScript("npc_brewfest_reveler") { }

    struct npc_brewfest_revelerAI : public ScriptedAI
    {
        npc_brewfest_revelerAI(Creature * cr) : ScriptedAI(cr) {}
        void ReceiveEmote(Player * plr, uint32 emote)
        {
            if (!IsHolidayActive(HOLIDAY_BREWFEST))
                return;

            if (emote == TEXT_EMOTE_DANCE)
                me->CastSpell(plr, 41586, false);
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_brewfest_revelerAI(cr);
    }
};

/*####
## npc_winter_reveler
####*/

enum WinterReveler
{
    SPELL_MISTLETOE_DEBUFF       = 26218,
    SPELL_CREATE_MISTLETOE       = 26206,
    SPELL_CREATE_HOLLY           = 26207,
    SPELL_CREATE_SNOWFLAKES      = 45036,
};

class npc_winter_reveler : public CreatureScript
{
    public:
        npc_winter_reveler() : CreatureScript("npc_winter_reveler") { }

        struct npc_winter_revelerAI : public ScriptedAI
        {
            npc_winter_revelerAI(Creature* c) : ScriptedAI(c) {}

            void ReceiveEmote(Player* player, uint32 emote)
            {
                if (player->HasAura(SPELL_MISTLETOE_DEBUFF))
                    return;

                if (!IsHolidayActive(HOLIDAY_FEAST_OF_WINTER_VEIL))
                    return;

                if (emote == TEXT_EMOTE_KISS)
                {
                    uint32 spellId = RAND<uint32>(SPELL_CREATE_MISTLETOE, SPELL_CREATE_HOLLY, SPELL_CREATE_SNOWFLAKES);
                    me->CastSpell(player, spellId, false);
                    me->CastSpell(player, SPELL_MISTLETOE_DEBUFF, false);
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_winter_revelerAI(creature);
        }
};

/*####
## npc_snake_trap_serpents
####*/

#define SPELL_MIND_NUMBING_POISON    25810   //Viper
#define SPELL_DEADLY_POISON          34655   //Venomous Snake
#define SPELL_CRIPPLING_POISON       30981   //Viper

#define VENOMOUS_SNAKE_TIMER 1500
#define VIPER_TIMER 3000

#define C_VIPER 19921

class npc_snake_trap : public CreatureScript
{
public:
    npc_snake_trap() : CreatureScript("npc_snake_trap_serpents") { }

    struct npc_snake_trap_serpentsAI : public ScriptedAI
    {
        npc_snake_trap_serpentsAI(Creature * cr) : ScriptedAI(cr) {}

        uint32 SpellTimer;
        bool IsViper;

        void EnterCombat(Unit * /*who*/) {}

        void Reset()
        {
            SpellTimer = 0;

            CreatureTemplate const * Info = me->GetCreatureInfo();

            IsViper = Info->Entry == C_VIPER ? true : false;

            me->SetMaxHealth(uint32(107 * (me->getLevel() - 40) * 0.025f));
            //Add delta to make them not all hit the same time
            uint32 delta = (rand() % 7) * 100;
            me->SetStatFloatValue(UNIT_FIELD_BASEATTACKTIME, float(Info->baseattacktime + delta));
            me->SetStatFloatValue(UNIT_FIELD_RANGED_ATTACK_POWER, float(Info->attackpower));

            // Start attacking attacker of owner on first ai update after spawn - move in line of sight may choose better target
            if (!me->getVictim() && me->isSummon())
                if (Unit * Owner = me->ToTempSummon()->GetSummoner())
                    if (Owner->getAttackerForHelper())
                        AttackStart(Owner->getAttackerForHelper());
        }

        //Redefined for random target selection:
        void MoveInLineOfSight(Unit *who)
        {
            if (!me->getVictim() && me->canCreatureAttack(who))
            {
                if (me->GetDistanceZ(who) > CREATURE_Z_ATTACK_RANGE)
                    return;

                float attackRadius = me->GetAttackDistance(who);
                if (me->IsWithinDistInMap(who, attackRadius) && me->IsWithinLOSInMap(who))
                {
                    if (!(rand() % 5))
                    {
                        me->setAttackTimer(BASE_ATTACK, (rand() % 10) * 100);
                        SpellTimer = (rand() % 10) * 100;
                        AttackStart(who);
                    }
                }
            }
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (SpellTimer <= diff)
            {
                if (IsViper) //Viper
                {
                    if (urand(0, 2) == 0) //33% chance to cast
                    {
                        uint32 spell;
                        if (urand(0, 1) == 0)
                            spell = SPELL_MIND_NUMBING_POISON;
                        else
                            spell = SPELL_CRIPPLING_POISON;

                        DoCast(me->getVictim(), spell);
                    }

                    SpellTimer = VIPER_TIMER;
                }
                else //Venomous Snake
                {
                    if (urand(0, 2) == 0) //33% chance to cast
                        DoCast(me->getVictim(), SPELL_DEADLY_POISON);
                    SpellTimer = VENOMOUS_SNAKE_TIMER + (rand() % 5) * 100;
                }
            }
            else
                SpellTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_snake_trap_serpentsAI(cr);
    }
};

#define SAY_RANDOM_MOJO0    "Now that's what I call froggy-style!"
#define SAY_RANDOM_MOJO1    "Your lily pad or mine?"
#define SAY_RANDOM_MOJO2    "This won't take long, did it?"
#define SAY_RANDOM_MOJO3    "I thought you'd never ask!"
#define SAY_RANDOM_MOJO4    "I promise not to give you warts..."
#define SAY_RANDOM_MOJO5    "Feelin' a little froggy, are ya?"
#define SAY_RANDOM_MOJO6a   "Listen, "
#define SAY_RANDOM_MOJO6b   ", I know of a little swamp not too far from here...."
#define SAY_RANDOM_MOJO7    "There's just never enough Mojo to go around..."

class mob_mojo : public CreatureScript
{
public:
    mob_mojo() : CreatureScript("mob_mojo") { }

    struct mob_mojoAI : public ScriptedAI
    {
        mob_mojoAI(Creature * cr) : ScriptedAI(cr) {Reset();}

        uint32 Hearts;
        uint64 VictimGUID;

        void Reset()
        {
            VictimGUID = 0;
            Hearts = 15000;
            if (Unit * own = me->GetOwner())
                me->GetMotionMaster()->MoveFollow(own, 0, 0);
        }

        void EnterCombat(Unit * /*who*/){}

        void UpdateAI(uint32 const diff)
        {
            if (me->HasAura(20372))
            {
                if (Hearts <= diff)
                {
                    me->RemoveAurasDueToSpell(20372);
                    Hearts = 15000;
                }
                Hearts -= diff;
            }
        }

        void ReceiveEmote(Player * plr, uint32 emote)
        {
            me->HandleEmoteCommand(emote);
            Unit * own = me->GetOwner();
            if (!own || own->GetTypeId() != TYPEID_PLAYER || CAST_PLR(own)->GetTeam() != plr->GetTeam())
                return;
            if (emote == TEXT_EMOTE_KISS)
            {
                std::string whisp = "";
                switch (rand() % 8)
                {
                    case 0:
                        whisp.append(SAY_RANDOM_MOJO0);
                        break;
                    case 1:
                        whisp.append(SAY_RANDOM_MOJO1);
                        break;
                    case 2:
                        whisp.append(SAY_RANDOM_MOJO2);
                        break;
                    case 3:
                        whisp.append(SAY_RANDOM_MOJO3);
                        break;
                    case 4:
                        whisp.append(SAY_RANDOM_MOJO4);
                        break;
                    case 5:
                        whisp.append(SAY_RANDOM_MOJO5);
                        break;
                    case 6:
                        whisp.append(SAY_RANDOM_MOJO6a);
                        whisp.append(plr->GetName());
                        whisp.append(SAY_RANDOM_MOJO6b);
                        break;
                    case 7:
                        whisp.append(SAY_RANDOM_MOJO7);
                        break;
                }

                me->MonsterWhisper(whisp.c_str(), plr->GetGUID());
                if (VictimGUID)
                    if (Player * victim = Unit::GetPlayer(*me, VictimGUID))
                        victim->RemoveAura(43906);//remove polymorph frog thing
                me->AddAura(43906, plr);//add polymorph frog thing
                VictimGUID = plr->GetGUID();
                DoCast(me, 20372, true);//tag.hearts
                me->GetMotionMaster()->MoveFollow(plr, 0, 0);
                Hearts = 15000;
            }
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new mob_mojoAI(cr);
    }
};

class npc_mirror_image : public CreatureScript
{
public:
    npc_mirror_image() : CreatureScript("npc_mirror_image") { }

    struct npc_mirror_imageAI : CasterAI
    {
        npc_mirror_imageAI(Creature * cr) : CasterAI(cr) {}

        void InitializeAI()
        {
            CasterAI::InitializeAI();
            Unit * owner = me->GetOwner();
            if (!owner)
                return;
            // Inherit Master's Threat List (not yet implemented)
            owner->CastSpell((Unit *)NULL, 58838, true);
            // here mirror image casts on summoner spell (not present in client dbc) 49866
            // here should be auras (not present in client dbc): 35657, 35658, 35659, 35660 selfcasted by mirror images (stats related?)
            // Clone Me!
            owner->CastSpell(me, 45204, false);
        }

        // Do not reload Creature templates on evade mode enter - prevent visual lost
        void EnterEvadeMode()
        {
            if (me->IsInEvadeMode() || !me->isAlive())
                return;

            Unit *owner = me->GetCharmerOrOwner();

            me->CombatStop(true);
            if (owner && !me->HasUnitState(UNIT_STAT_FOLLOW))
            {
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle(), MOTION_SLOT_ACTIVE);
            }
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_mirror_imageAI(cr);
    }
};

class npc_ebon_gargoyle : public CreatureScript
{
public:
    npc_ebon_gargoyle() : CreatureScript("npc_ebon_gargoyle") { }

    struct npc_ebon_gargoyleAI : CasterAI
    {
        npc_ebon_gargoyleAI(Creature * cr) : CasterAI(cr) {}

        uint32 DespawnTimer;

        void InitializeAI()
        {
            CasterAI::InitializeAI();
            uint64 ownerGuid = me->GetOwnerGUID();
            if (!ownerGuid)
                return;
            // Not needed to be despawned now
            DespawnTimer = 0;
            // Find victim of Summon Gargoyle spell
            std::list<Unit *> targets;
            Trinity::AnyUnfriendlyUnitInObjectRangeCheck u_check(me, me, 30);
            Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(me, targets, u_check);
            me->VisitNearbyObject(30, searcher);
            for (std::list<Unit *>::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
                if ((*iter)->GetAura(49206, ownerGuid))
                {
                    me->Attack((*iter), false);
                    break;
                }
        }

        void JustDied(Unit * /*killer*/)
        {
            // Stop Feeding Gargoyle when it dies
            if (Unit *owner = me->GetOwner())
                owner->RemoveAurasDueToSpell(50514);
        }

        // Fly away when dismissed
        void SpellHit(Unit * source, SpellInfo const * spell)
        {
            if (spell->Id != 50515 || !me->isAlive())
                return;

            Unit *owner = me->GetOwner();

            if (!owner || owner != source)
                return;

            // Stop Fighting
            me->ApplyModFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE, true);
            // Sanctuary
            me->CastSpell(me, 54661, true);
            me->SetReactState(REACT_PASSIVE);

            // Fly Away
            me->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY|MOVEMENTFLAG_ASCENDING|MOVEMENTFLAG_FLYING);
            me->SetSpeed(MOVE_FLIGHT, 0.75f, true);
            me->SetSpeed(MOVE_RUN, 0.75f, true);
            float x = me->GetPositionX() + 20 * cos(me->GetOrientation());
            float y = me->GetPositionY() + 20 * sin(me->GetOrientation());
            float z = me->GetPositionZ() + 40;
            me->GetMotionMaster()->Clear(false);
            me->GetMotionMaster()->MovePoint(0, x, y, z);

            // Despawn as soon as possible
            DespawnTimer = 4 * IN_MILLISECONDS;
        }

        void UpdateAI(const uint32 diff)
        {
            if (DespawnTimer > 0)
            {
                if (DespawnTimer > diff)
                    DespawnTimer -= diff;
                else
                    me->DespawnOrUnsummon();
                return;
            }
            CasterAI::UpdateAI(diff);
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_ebon_gargoyleAI(cr);
    }
};

class npc_lightwell : public CreatureScript
{
public:
    npc_lightwell() : CreatureScript("npc_lightwell") { }

    struct npc_lightwellAI : public PassiveAI
    {
        npc_lightwellAI(Creature * cr) : PassiveAI(cr) {}

        void Reset()
        {
            DoCast(me, 59907, false); // Spell for Lightwell Charges
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_lightwellAI(cr);
    }
};

enum eTrainingDummy
{
    NPC_ADVANCED_TARGET_DUMMY                  = 2674,
    NPC_TARGET_DUMMY                           = 2673
};

class npc_training_dummy : public CreatureScript
{
public:
    npc_training_dummy() : CreatureScript("npc_training_dummy") { }

    struct npc_training_dummyAI : Scripted_NoMovementAI
    {
        npc_training_dummyAI(Creature * cr) : Scripted_NoMovementAI(cr)
        {
            Entry = cr->GetEntry();
        }

        uint32 Entry;
        uint32 ResetTimer;
        uint32 DespawnTimer;

        void Reset()
        {
            me->SetControlled(true, UNIT_STAT_STUNNED);//disable rotate
            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);//imune to knock aways like blast wave

            ResetTimer = 5000;
            DespawnTimer = 15000;
        }

        void EnterEvadeMode()
        {
            if (!_EnterEvadeMode())
                return;

            Reset();
        }

        void DamageTaken(Unit * /*doneBy*/, uint32& damage)
        {
            ResetTimer = 5000;
            damage = 0;
        }

        void EnterCombat(Unit * /*who*/)
        {
            if (Entry != NPC_ADVANCED_TARGET_DUMMY && Entry != NPC_TARGET_DUMMY)
                return;
        }

        void UpdateAI(uint32 const diff)
        {
            if (!UpdateVictim())
                return;

            if (!me->HasUnitState(UNIT_STAT_STUNNED))
                me->SetControlled(true, UNIT_STAT_STUNNED);//disable rotate

            if (Entry != NPC_ADVANCED_TARGET_DUMMY && Entry != NPC_TARGET_DUMMY)
            {
                if (ResetTimer <= diff)
                {
                    EnterEvadeMode();
                    ResetTimer = 5000;
                }
                else
                    ResetTimer -= diff;
                return;
            }
            else
            {
                if (DespawnTimer <= diff)
                    me->DespawnOrUnsummon();
                else
                    DespawnTimer -= diff;
            }
        }
        void MoveInLineOfSight(Unit * /*who*/){return;}
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_training_dummyAI(cr);
    }
};

/*######
# npc_shadowfiend
######*/
#define GLYPH_OF_SHADOWFIEND_MANA         58227
#define GLYPH_OF_SHADOWFIEND              58228

class npc_shadowfiend : public CreatureScript
{
public:
    npc_shadowfiend() : CreatureScript("npc_shadowfiend") { }

    struct npc_shadowfiendAI : public ScriptedAI
    {
        npc_shadowfiendAI(Creature * cr) : ScriptedAI(cr) {}

        void DamageTaken(Unit * /*killer*/, uint32& damage)
        {
            if (me->isSummon())
                if (Unit * owner = me->ToTempSummon()->GetSummoner())
                    if (owner->HasAura(GLYPH_OF_SHADOWFIEND) && damage >= me->GetHealth())
                        owner->CastSpell(owner, GLYPH_OF_SHADOWFIEND_MANA, true);
        }

        void UpdateAI(uint32 const /*diff*/)
        {
            if (!UpdateVictim())
                return;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI * GetAI(Creature * cr) const
    {
        return new npc_shadowfiendAI(cr);
    }
};

/*######
# npc_fire_elemental
######*/
#define SPELL_FIRENOVA        12470
#define SPELL_FIRESHIELD      13376
#define SPELL_FIREBLAST       57984

class npc_fire_elemental : public CreatureScript
{
public:
    npc_fire_elemental() : CreatureScript("npc_fire_elemental") { }

    struct npc_fire_elementalAI : public ScriptedAI
    {
        npc_fire_elementalAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 FireNova_Timer;
        uint32 FireShield_Timer;
        uint32 FireBlast_Timer;

        void Reset()
        {
            FireNova_Timer = 5000 + rand() % 15000; // 5-20 sec cd
            FireBlast_Timer = 5000 + rand() % 15000; // 5-20 sec cd
            FireShield_Timer = 0;
            me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_FIRE, true);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (me->HasUnitState(UNIT_STAT_CASTING))
                return;

            if (FireShield_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_FIRESHIELD);
                FireShield_Timer = 2 * IN_MILLISECONDS;
            }
            else
                FireShield_Timer -= diff;

            if (FireBlast_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_FIREBLAST);
                FireBlast_Timer = 5000 + rand() % 15000; // 5-20 sec cd
            }
            else
                FireBlast_Timer -= diff;

            if (FireNova_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_FIRENOVA);
                FireNova_Timer = 5000 + rand() % 15000; // 5-20 sec cd
            }
            else
                FireNova_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI *GetAI(Creature* creature) const
    {
        return new npc_fire_elementalAI(creature);
    }
};

/*######
# npc_earth_elemental
######*/
#define SPELL_ANGEREDEARTH        36213

class npc_earth_elemental : public CreatureScript
{
public:
    npc_earth_elemental() : CreatureScript("npc_earth_elemental") { }

    struct npc_earth_elementalAI : public ScriptedAI
    {
        npc_earth_elementalAI(Creature* creature) : ScriptedAI(creature) {}

        uint32 AngeredEarth_Timer;

        void Reset()
        {
            AngeredEarth_Timer = 0;
            me->ApplySpellImmune(0, IMMUNITY_SCHOOL, SPELL_SCHOOL_MASK_NATURE, true);
        }

        void UpdateAI(const uint32 diff)
        {
            if (!UpdateVictim())
                return;

            if (AngeredEarth_Timer <= diff)
            {
                DoCast(me->getVictim(), SPELL_ANGEREDEARTH);
                AngeredEarth_Timer = 5000 + rand() % 15000; // 5-20 sec cd
            }
            else
                AngeredEarth_Timer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI *GetAI(Creature* creature) const
    {
        return new npc_earth_elementalAI(creature);
    }
};

/*######
# npc_wormhole
######*/

#define GOSSIP_ENGINEERING1   "Borean Tundra."
#define GOSSIP_ENGINEERING2   "Howling Fjord."
#define GOSSIP_ENGINEERING3   "Sholazar Basin."
#define GOSSIP_ENGINEERING4   "Icecrown."
#define GOSSIP_ENGINEERING5   "Storm Peaks."

enum eWormhole
{
    SPELL_HOWLING_FJORD         = 67838,
    SPELL_SHOLAZAR_BASIN        = 67835,
    SPELL_ICECROWN              = 67836,
    SPELL_STORM_PEAKS           = 67837,

    TEXT_WORMHOLE               = 907
};

class npc_wormhole : public CreatureScript
{
public:
    npc_wormhole() : CreatureScript("npc_wormhole") { }

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        if (cr->isSummon())
        {
            if (plr == cr->ToTempSummon()->GetSummoner())
            {
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING4, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_ENGINEERING5, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

                plr->PlayerTalkClass->SendGossipMenu(TEXT_WORMHOLE, cr->GetGUID());
            }
        }
        return true;
    }

    bool OnGossipSelect(Player * plr, Creature * /* cr*/, uint32 /*sender*/, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();
        bool roll = urand(0, 1);

        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1: //Borean Tundra
                plr->CLOSE_GOSSIP_MENU();
                if (roll) //At the moment we don't have chance on spell_target_position table so we hack this
                    plr->TeleportTo(571, 4305.505859f, 5450.839844f, 63.005806f, 0.627286f);
                else
                    plr->TeleportTo(571, 3201.936279f, 5630.123535f, 133.658798f, 3.855272f);
                break;
            case GOSSIP_ACTION_INFO_DEF + 2: //Howling Fjord
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_HOWLING_FJORD, true);
                break;
            case GOSSIP_ACTION_INFO_DEF + 3: //Sholazar Basin
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_SHOLAZAR_BASIN, true);
                break;
            case GOSSIP_ACTION_INFO_DEF + 4: //Icecrown
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_ICECROWN, true);
                break;
            case GOSSIP_ACTION_INFO_DEF + 5: //Storm peaks
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_STORM_PEAKS, true);
                break;
        }
        return true;
    }
};

/*######
## npc_pet_trainer
######*/

enum ePetTrainer
{
    TEXT_ISHUNTER               = 5838,
    TEXT_NOTHUNTER              = 5839,
    TEXT_PETINFO                = 13474,
    TEXT_CONFIRM                = 7722
};

#define GOSSIP_PET1             "How do I train my pet?"
#define GOSSIP_PET2             "I wish to untrain my pet."
#define GOSSIP_PET_CONFIRM      "Yes, please do."

class npc_pet_trainer : public CreatureScript
{
public:
    npc_pet_trainer() : CreatureScript("npc_pet_trainer") { }

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        if (cr->isQuestGiver())
            plr->PrepareQuestMenu(cr->GetGUID());

        if (plr->getClass() == CLASS_HUNTER)
        {
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_PET1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            if (plr->GetPet() && plr->GetPet()->getPetType() == HUNTER_PET)
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_PET2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

            plr->PlayerTalkClass->SendGossipMenu(TEXT_ISHUNTER, cr->GetGUID());
            return true;
        }
        plr->PlayerTalkClass->SendGossipMenu(TEXT_NOTHUNTER, cr->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player * plr, Creature * cr, uint32 /*sender*/, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                plr->PlayerTalkClass->SendGossipMenu(TEXT_PETINFO, cr->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                {
                    plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_PET_CONFIRM, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                    plr->PlayerTalkClass->SendGossipMenu(TEXT_CONFIRM, cr->GetGUID());
                }
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                {
                    plr->ResetPetTalents();
                    plr->CLOSE_GOSSIP_MENU();
                }
                break;
        }
        return true;
    }
};

/*######
## npc_locksmith
######*/

enum eLockSmith
{
    QUEST_HOW_TO_BRAKE_IN_TO_THE_ARCATRAZ = 10704,
    QUEST_DARK_IRON_LEGACY                = 3802,
    QUEST_THE_KEY_TO_SCHOLOMANCE_A        = 5505,
    QUEST_THE_KEY_TO_SCHOLOMANCE_H        = 5511,
    QUEST_HOTTER_THAN_HELL_A              = 10758,
    QUEST_HOTTER_THAN_HELL_H              = 10764,
    QUEST_RETURN_TO_KHAGDAR               = 9837,
    QUEST_CONTAINMENT                     = 13159,

    ITEM_ARCATRAZ_KEY                     = 31084,
    ITEM_SHADOWFORGE_KEY                  = 11000,
    ITEM_SKELETON_KEY                     = 13704,
    ITEM_SHATTERED_HALLS_KEY              = 28395,
    ITEM_THE_MASTERS_KEY                  = 24490,
    ITEM_VIOLET_HOLD_KEY                  = 42482,

    SPELL_ARCATRAZ_KEY                    = 54881,
    SPELL_SHADOWFORGE_KEY                 = 54882,
    SPELL_SKELETON_KEY                    = 54883,
    SPELL_SHATTERED_HALLS_KEY             = 54884,
    SPELL_THE_MASTERS_KEY                 = 54885,
    SPELL_VIOLET_HOLD_KEY                 = 67253
};

#define GOSSIP_LOST_ARCATRAZ_KEY         "I've lost my key to the Arcatraz."
#define GOSSIP_LOST_SHADOWFORGE_KEY      "I've lost my key to the Blackrock Depths."
#define GOSSIP_LOST_SKELETON_KEY         "I've lost my key to the Scholomance."
#define GOSSIP_LOST_SHATTERED_HALLS_KEY  "I've lost my key to the Shattered Halls."
#define GOSSIP_LOST_THE_MASTERS_KEY      "I've lost my key to the Karazhan."
#define GOSSIP_LOST_VIOLET_HOLD_KEY      "I've lost my key to the Violet Hold."

class npc_locksmith : public CreatureScript
{
public:
    npc_locksmith() : CreatureScript("npc_locksmith") { }

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        // Arcatraz Key
        if (plr->GetQuestRewardStatus(QUEST_HOW_TO_BRAKE_IN_TO_THE_ARCATRAZ) && !plr->HasItemCount(ITEM_ARCATRAZ_KEY, 1, true))
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_ARCATRAZ_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

        // Shadowforge Key
        if (plr->GetQuestRewardStatus(QUEST_DARK_IRON_LEGACY) && !plr->HasItemCount(ITEM_SHADOWFORGE_KEY, 1, true))
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SHADOWFORGE_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

        // Skeleton Key
        if ((plr->GetQuestRewardStatus(QUEST_THE_KEY_TO_SCHOLOMANCE_A) || plr->GetQuestRewardStatus(QUEST_THE_KEY_TO_SCHOLOMANCE_H)) &&
            !plr->HasItemCount(ITEM_SKELETON_KEY, 1, true))
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SKELETON_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

        // Shatered Halls Key
        if ((plr->GetQuestRewardStatus(QUEST_HOTTER_THAN_HELL_A) || plr->GetQuestRewardStatus(QUEST_HOTTER_THAN_HELL_H)) &&
            !plr->HasItemCount(ITEM_SHATTERED_HALLS_KEY, 1, true))
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_SHATTERED_HALLS_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);

        // Master's Key
        if (plr->GetQuestRewardStatus(QUEST_RETURN_TO_KHAGDAR) && !plr->HasItemCount(ITEM_THE_MASTERS_KEY, 1, true))
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_THE_MASTERS_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);

        // Violet Hold Key
        if (plr->GetQuestRewardStatus(QUEST_CONTAINMENT) && !plr->HasItemCount(ITEM_VIOLET_HOLD_KEY, 1, true))
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_VIOLET_HOLD_KEY, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);

        plr->SEND_GOSSIP_MENU(plr->GetGossipTextId(cr), cr->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player * plr, Creature * /* cr*/, uint32 /*sender*/, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_ARCATRAZ_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_SHADOWFORGE_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_SKELETON_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 4:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_SHATTERED_HALLS_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 5:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_THE_MASTERS_KEY, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 6:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_VIOLET_HOLD_KEY, false);
                break;
        }
        return true;
    }
};

/*######
## npc_tabard_vendor
######*/

enum
{
    QUEST_TRUE_MASTERS_OF_LIGHT = 9737,
    QUEST_THE_UNWRITTEN_PROPHECY = 9762,
    QUEST_INTO_THE_BREACH = 10259,
    QUEST_BATTLE_OF_THE_CRIMSON_WATCH = 10781,
    QUEST_SHARDS_OF_AHUNE = 11972,

    ACHIEVEMENT_EXPLORE_NORTHREND = 45,
    ACHIEVEMENT_TWENTYFIVE_TABARDS = 1021,
    ACHIEVEMENT_THE_LOREMASTER_A = 1681,
    ACHIEVEMENT_THE_LOREMASTER_H = 1682,

    ITEM_TABARD_OF_THE_HAND = 24344,
    ITEM_TABARD_OF_THE_BLOOD_KNIGHT = 25549,
    ITEM_TABARD_OF_THE_PROTECTOR = 28788,
    ITEM_OFFERING_OF_THE_SHATAR = 31408,
    ITEM_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI = 31404,
    ITEM_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI = 31405,
    ITEM_TABARD_OF_THE_SUMMER_SKIES = 35279,
    ITEM_TABARD_OF_THE_SUMMER_FLAMES = 35280,
    ITEM_TABARD_OF_THE_ACHIEVER = 40643,
    ITEM_LOREMASTERS_COLORS = 43300,
    ITEM_TABARD_OF_THE_EXPLORER = 43348,

    SPELL_TABARD_OF_THE_BLOOD_KNIGHT = 54974,
    SPELL_TABARD_OF_THE_HAND = 54976,
    SPELL_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI = 54977,
    SPELL_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI = 54982,
    SPELL_TABARD_OF_THE_ACHIEVER = 55006,
    SPELL_TABARD_OF_THE_PROTECTOR = 55008,
    SPELL_LOREMASTERS_COLORS = 58194,
    SPELL_TABARD_OF_THE_EXPLORER = 58224,
    SPELL_TABARD_OF_SUMMER_SKIES = 62768,
    SPELL_TABARD_OF_SUMMER_FLAMES = 62769
};

#define GOSSIP_LOST_TABARD_OF_BLOOD_KNIGHT "I've lost my Tabard of Blood Knight."
#define GOSSIP_LOST_TABARD_OF_THE_HAND "I've lost my Tabard of the Hand."
#define GOSSIP_LOST_TABARD_OF_THE_PROTECTOR "I've lost my Tabard of the Protector."
#define GOSSIP_LOST_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI "I've lost my Green Trophy Tabard of the Illidari."
#define GOSSIP_LOST_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI "I've lost my Purple Trophy Tabard of the Illidari."
#define GOSSIP_LOST_TABARD_OF_SUMMER_SKIES "I've lost my Tabard of Summer Skies."
#define GOSSIP_LOST_TABARD_OF_SUMMER_FLAMES "I've lost my Tabard of Summer Flames."
#define GOSSIP_LOST_LOREMASTERS_COLORS "I've lost my Loremaster's Colors."
#define GOSSIP_LOST_TABARD_OF_THE_EXPLORER "I've lost my Tabard of the Explorer."
#define GOSSIP_LOST_TABARD_OF_THE_ACHIEVER "I've lost my Tabard of the Achiever."

class npc_tabard_vendor : public CreatureScript
{
public:
    npc_tabard_vendor() : CreatureScript("npc_tabard_vendor") { }

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        bool lostBloodKnight = false;
        bool lostHand = false;
        bool lostProtector = false;
        bool lostIllidari = false;
        bool lostSummer = false;

        //Tabard of the Blood Knight
        if (plr->GetQuestRewardStatus(QUEST_TRUE_MASTERS_OF_LIGHT) && !plr->HasItemCount(ITEM_TABARD_OF_THE_BLOOD_KNIGHT, 1, true))
            lostBloodKnight = true;

        //Tabard of the Hand
        if (plr->GetQuestRewardStatus(QUEST_THE_UNWRITTEN_PROPHECY) && !plr->HasItemCount(ITEM_TABARD_OF_THE_HAND, 1, true))
            lostHand = true;

        //Tabard of the Protector
        if (plr->GetQuestRewardStatus(QUEST_INTO_THE_BREACH) && !plr->HasItemCount(ITEM_TABARD_OF_THE_PROTECTOR, 1, true))
            lostProtector = true;

        //Green Trophy Tabard of the Illidari
        //Purple Trophy Tabard of the Illidari
        if (plr->GetQuestRewardStatus(QUEST_BATTLE_OF_THE_CRIMSON_WATCH) &&
            (!plr->HasItemCount(ITEM_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI, 1, true) &&
            !plr->HasItemCount(ITEM_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI, 1, true) &&
            !plr->HasItemCount(ITEM_OFFERING_OF_THE_SHATAR, 1, true)))
            lostIllidari = true;

        //Tabard of Summer Skies
        //Tabard of Summer Flames
        if (plr->GetQuestRewardStatus(QUEST_SHARDS_OF_AHUNE) &&
            !plr->HasItemCount(ITEM_TABARD_OF_THE_SUMMER_SKIES, 1, true) &&
            !plr->HasItemCount(ITEM_TABARD_OF_THE_SUMMER_FLAMES, 1, true))
            lostSummer = true;

        if (lostBloodKnight || lostHand || lostProtector || lostIllidari || lostSummer)
        {
            plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

            if (lostBloodKnight)
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_BLOOD_KNIGHT, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);

            if (lostHand)
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_THE_HAND, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);

            if (lostProtector)
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_THE_PROTECTOR, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);

            if (lostIllidari)
            {
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 5);
            }

            if (lostSummer)
            {
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_SUMMER_SKIES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 6);
                plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_LOST_TABARD_OF_SUMMER_FLAMES, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 7);
            }

            plr->SEND_GOSSIP_MENU(13583, cr->GetGUID());
        }
        else
            plr->GetSession()->SendListInventory(cr->GetGUID());

        return true;
    }

    bool OnGossipSelect(Player * plr, Creature * cr, uint32 /*sender*/, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_TRADE:
                plr->GetSession()->SendListInventory(cr->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF + 1:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_TABARD_OF_THE_BLOOD_KNIGHT, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 2:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_TABARD_OF_THE_HAND, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 3:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_TABARD_OF_THE_PROTECTOR, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 4:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_GREEN_TROPHY_TABARD_OF_THE_ILLIDARI, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 5:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_PURPLE_TROPHY_TABARD_OF_THE_ILLIDARI, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 6:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_TABARD_OF_SUMMER_SKIES, false);
                break;
            case GOSSIP_ACTION_INFO_DEF + 7:
                plr->CLOSE_GOSSIP_MENU();
                plr->CastSpell(plr, SPELL_TABARD_OF_SUMMER_FLAMES, false);
                break;
        }
        return true;
    }
};

/*######
## npc_experience
######*/

#define EXP_COST                100000 //10 00 00 copper (10golds)
#define GOSSIP_TEXT_EXP         14736
#define GOSSIP_XP_OFF           "I no longer wish to gain experience."
#define GOSSIP_XP_ON            "I wish to start gaining experience again."

class npc_experience : public CreatureScript
{
public:
    npc_experience() : CreatureScript("npc_experience") { }

    bool OnGossipHello(Player * plr, Creature * cr)
    {
        plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_XP_OFF, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
        plr->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_XP_ON, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
        plr->PlayerTalkClass->SendGossipMenu(GOSSIP_TEXT_EXP, cr->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player * plr, Creature * /* cr*/, uint32 /*sender*/, uint32 action)
    {
        plr->PlayerTalkClass->ClearMenus();
        bool noXPGain = plr->HasFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
        bool doSwitch = false;

        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF + 1://xp off
                {
                    if (!noXPGain)//does gain xp
                        doSwitch = true;//switch to don't gain xp
                }
                break;
            case GOSSIP_ACTION_INFO_DEF + 2://xp on
                {
                    if (noXPGain)//doesn't gain xp
                        doSwitch = true;//switch to gain xp
                }
                break;
        }
        if (doSwitch)
        {
            if (!plr->HasEnoughMoney(EXP_COST))
                plr->SendBuyError(BUY_ERR_NOT_ENOUGHT_MONEY, 0, 0, 0);
            else if (noXPGain)
            {
                plr->ModifyMoney(-EXP_COST);
                plr->RemoveFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
            }
            else if (!noXPGain)
            {
                plr->ModifyMoney(-EXP_COST);
                plr->SetFlag(PLAYER_FLAGS, PLAYER_FLAGS_NO_XP_GAIN);
            }
        }
        plr->PlayerTalkClass->SendCloseGossip();
        return true;
    }
};

void AddSC_npcs_special()
{
    // Eigene
    new npc_schutz_den_wehrlosen;
    new npc_flugmeister;
    new npc_flugmeister_adds;
    new npc_hati;
    new npc_uwom_firecaller;
    new npc_uwom_gm_pimper;
    new npc_uwom_user_pimper;

    // Trinity
    new npc_air_force_bots;
    new npc_lunaclaw_spirit;
    new npc_chicken_cluck;
    new npc_dancing_flames;
    new npc_doctor;
    new npc_injured_patient;
    new npc_garments_of_quests;
    new npc_kingdom_of_dalaran_quests;
    new npc_mount_vendor;
    new npc_rogue_trainer;
    new npc_sayge;
    new npc_steam_tonk;
    new npc_tonk_mine;
    new npc_winter_reveler;
    new npc_brewfest_reveler;
    new npc_snake_trap;
    new npc_mirror_image;
    new npc_ebon_gargoyle;
    new npc_lightwell;
    new mob_mojo;
    new npc_training_dummy;
    new npc_shadowfiend;
    new npc_wormhole;
    new npc_pet_trainer;
    new npc_locksmith;
    new npc_tabard_vendor;
    new npc_experience;
    new npc_fire_elemental;
    new npc_earth_elemental;
}

