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

#ifndef DEF_HALLS_OF_REFLECTION_H
#define DEF_HALLS_OF_REFLECTION_H

enum Data
{
    MAX_ENCOUNTER          = 4,

    DATA_FALRIC_EVENT      = 1,
    DATA_MARWYN_EVENT      = 2,
    DATA_FROSWORN_EVENT    = 3,
    DATA_LICHKING_EVENT    = 4,

    DATA_INTRO_EVENT       = 5,
    DATA_TEAM_IN_INSTANCE  = 6,
    DATA_WAVE_COUNT        = 7,

    DATA_SUMMONS           = 8,
    DATA_ICE_WALL_1        = 9,
    DATA_ICE_WALL_2        = 10,
    DATA_ICE_WALL_3        = 11,
    DATA_ICE_WALL_4        = 12,

    DATA_PHASE             = 13
};

enum Data64
{
    DATA_FALRIC             = 101,
    DATA_MARWYN             = 102,
    DATA_LICHKING           = 103,
    DATA_ESCAPE_LEADER      = 104,
    DATA_FROSTMOURNE        = 105,
    DATA_FRONT_DOOR         = 106,
    DATA_FROSTWORN_DOOR     = 107,
    DATA_ARTHAS_DOOR        = 108,
    DATA_QUELDELAR_TRIGGER  = 109,
    DATA_QUELDELAR          = 110,
    DATA_QUELDELAR_PLR      = 111
};

enum Creatures
{
    NPC_FALRIC              = 38112,
    NPC_MARWYN              = 38113,
    NPC_FROSTWORN_GENERAL   = 36723,
    NPC_REFLECTION          = 37107,
    NPC_LICH_KING_EVENT     = 36954,
    NPC_QUELDELAR_TRIGGER   = 37745,
    NPC_QUELDELAR           = 37158,
    BOSS_LICH_KING          = 37226,

    NPC_UTHER               = 37225,
    NPC_JAINA_PART1         = 37221,
    NPC_JAINA_OUTRO         = 36955,
    NPC_SYLVANAS_PART1      = 37223,
    NPC_SYLVANA_OUTRO       = 37554,
    NPC_BARTLETT            = 37182,
    NPC_KORM                = 37833,

    NPC_WAVE_MERCENARY      = 38177,
    NPC_WAVE_FOOTMAN        = 38173,
    NPC_WAVE_RIFLEMAN       = 38176,
    NPC_WAVE_PRIEST         = 38175,
    NPC_WAVE_MAGE           = 38172,
    NPC_WAVE_LOOT_DUMMY     = 37906,

    NPC_ICE_WALL            = 37014,
    NPC_RAGING_GNOUL        = 36940,
    NPC_RISEN_WITCH_DOCTOR  = 36941,
    NPC_ABON                = 37069
};

enum GameObjects
{
    GO_FROSTMOURNE          = 202302,
    GO_FROSTMOURNE_ALTAR    = 202236,
    GO_FRONT_DOOR           = 201976,
    GO_FROSTWORN_DOOR       = 197341,
    GO_ARTHAS_DOOR          = 197342,
    GO_RUN_DOOR             = 197343,

    GO_ICE_WALL_1           = 201385,
    GO_ICE_WALL_2           = 201885,
    GO_ICE_WALL_3           = 202396,
    GO_ICE_WALL_4           = 400002,
    GO_CAVE                 = 201596,

    GO_STAIRS_SKYBREAKER    = 201709,
    GO_SKYBREAKER           = 400003,
    GO_STAIRS_ORGRIM_HAMMER = 202211,
    GO_ORGRIM_HAMMER        = 400004,
    GO_PORTAL               = 202079,
    GO_CAPTAIN_CHEST_1      = 202212, // 3145
    GO_CAPTAIN_CHEST_2      = 201710, // 30357
    GO_CAPTAIN_CHEST_3      = 202337, // 3246
    GO_CAPTAIN_CHEST_4      = 202336  // 3333
};

enum Achievements
{
    ACHIEV_HALLS_OF_REFLECTION_N    = 4518,
    ACHIEV_HALLS_OF_REFLECTION_H    = 4521,
    ACHIEV_NOT_RETREATING_EVENT     = 22615,
    SPELL_ACHIEV_CHECK              = 72830
};

enum QuelDelar
{
    ITEM_GETEMPERTES_QUELDELAR  = 49766,
    ITEM_GEDAEMPFTES_QUELDELAR  = 50254,

    SPELL_QUELDELARS_ZWANG              = 70013, // Spieler Aura
    SPELL_BOESES_QUELDELAR_BESCHWOEREN  = 69966, // Summon NPC_QUELDELAR
    SPELL_QUELDELAR_WERFEN_1            = 70586, // 15m auf Trigger
    SPELL_QUELDELAR_WERFEN_2            = 70700, // Spieler AOE Cast
    SPELL_QUELDELARS_WILLE              = 70698  // Vom Trigger casten
};

enum DiverseSpells
{
    SPELL_SUICIDE = 69908
};

const Position OutroSpawns[2] =
{
    {5564.25f, 2274.69f, 733.01f, 3.93f}, // Lich King
    {5556.27f, 2266.28f, 733.01f, 0.8f}   // Jaina/Sylvana
};

enum HorWorldStates
{
    WORLD_STATE_HOR             = 4884,
    WORLD_STATE_HOR_WAVE_COUNT  = 4882
};

// Common actions from Instance Script to Boss Script
enum Actions
{
    ACTION_ENTER_COMBAT
};

// Base class for FALRIC and MARWYN
// handled the summonList and the notification events to/from the InstanceScript
struct boss_horAI : ScriptedAI
{
    boss_horAI(Creature* creature) : ScriptedAI(creature), summons(creature)
    {
        instance = me->GetInstanceScript();
    }

    InstanceScript * instance;
    EventMap events;
    SummonList summons;
    uint8 AliveSumCnt;

    void Reset()
    {
        events.Reset();
        summons.DespawnAll();
        AliveSumCnt = 0;

        me->SetVisible(false);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        me->SetReactState(REACT_PASSIVE);
    }

    void DamageTaken(Unit * /*who*/, uint32 & dmg)
    {
        if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            dmg = 0;
    }

    void DoAction(const int32 action)
    {
        switch (action)
        {
            case ACTION_ENTER_COMBAT:
                me->SetVisible(true);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_AGGRESSIVE);

                if (Unit * target = me->FindNearestPlayer(100.0f))
                    AttackStart(target);

                DoZoneInCombat();

                break;
        }
    }

    void JustSummoned(Creature * summon)
    {
        switch(me->GetEntry())
        {
            case NPC_FALRIC:
                if (instance && instance->GetData(DATA_FALRIC_EVENT) != IN_PROGRESS)
                    instance->SetData(DATA_FALRIC_EVENT, IN_PROGRESS);
                break;
            case NPC_MARWYN:
                if (instance && instance->GetData(DATA_MARWYN_EVENT) != IN_PROGRESS)
                    instance->SetData(DATA_MARWYN_EVENT, IN_PROGRESS);
                break;
        }

        summons.Summon(summon);
        ++AliveSumCnt;

        if (Unit * target = me->FindNearestPlayer(100.0f))
        {
            summon->AI()->AttackStart(target);
            summon->AI()->DoZoneInCombat();
        }
        else if (instance) // Kein Ziel zu finden. Reset!
            switch(me->GetEntry())
            {
                case NPC_FALRIC:
                    instance->SetData(DATA_FALRIC_EVENT, NOT_STARTED);
                    instance->SetData(DATA_WAVE_COUNT, 0);
                    break;
                case NPC_MARWYN:
                    instance->SetData(DATA_MARWYN_EVENT, NOT_STARTED);
                    instance->SetData(DATA_WAVE_COUNT, 0);
                    break;
            }

    }

    void SummonedCreatureDespawn(Creature * summon)
    {
        summons.Despawn(summon);

        if (instance && summons.empty())
        {
            if (instance->GetData(DATA_WAVE_COUNT) == 4 || instance->GetData(DATA_WAVE_COUNT) == 9)
                instance->SetData(DATA_WAVE_COUNT, 77);
            else
                instance->SetData(DATA_WAVE_COUNT, 55);
        }
    }

    void SummonedCreatureDies(Creature * /*summon*/, Unit * /*killer*/)
    {
        --AliveSumCnt;

        if (!AliveSumCnt && instance)
        {
            if (instance->GetData(DATA_WAVE_COUNT) == 4 || instance->GetData(DATA_WAVE_COUNT) == 9)
                instance->SetData(DATA_WAVE_COUNT, 77);
            else
                instance->SetData(DATA_WAVE_COUNT, 55);
        }
    }

    void UpdateAI(const uint32 diff)
    {
        if (!instance || !summons.empty() || me->isInCombat())
            return;

        switch(me->GetEntry())
        {
            case NPC_FALRIC:
                if (instance->GetData(DATA_WAVE_COUNT) == 4)
                {
                    instance->SetData(DATA_WAVE_COUNT, 5);
                    DoAction(ACTION_ENTER_COMBAT);
                }
                break;
            case NPC_MARWYN:
                if (instance->GetData(DATA_WAVE_COUNT) == 9)
                {
                    instance->SetData(DATA_WAVE_COUNT, 10);
                    DoAction(ACTION_ENTER_COMBAT);
                }
                break;
        }
    }
};

#endif
