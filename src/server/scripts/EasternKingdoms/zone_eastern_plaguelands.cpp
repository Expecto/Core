/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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
SDName: Eastern_Plaguelands
SD%Complete: 100
SDComment: Quest support: 5211, 5742. Special vendor Augustus the Touched
SDCategory: Eastern Plaguelands
EndScriptData */

/* ContentData
npc_ghoul_flayer
npc_augustus_the_touched
npc_darrowshire_spirit
npc_tirion_fordring
EndContentData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "WorldSession.h"

class npc_ghoul_flayer : public CreatureScript
{
public:
    npc_ghoul_flayer() : CreatureScript("npc_ghoul_flayer") { }

    struct npc_ghoul_flayerAI : public ScriptedAI
    {
        npc_ghoul_flayerAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override { }

        void EnterCombat(Unit* /*who*/) override { }

        void JustDied(Unit* killer) override
        {
            if (killer->GetTypeId() == TYPEID_PLAYER)
                me->SummonCreature(11064, 0.0f, 0.0f, 0.0f, 0.0f, TEMPSUMMON_TIMED_DESPAWN, 60000);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_ghoul_flayerAI(creature);
    }
};

/*######
## npc_augustus_the_touched
######*/

class npc_augustus_the_touched : public CreatureScript
{
public:
    npc_augustus_the_touched() : CreatureScript("npc_augustus_the_touched") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        if (action == GOSSIP_ACTION_TRADE)
            player->GetSession()->SendListInventory(creature->GetGUID());
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (creature->IsVendor() && player->GetQuestRewardStatus(6164))
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_VENDOR, GOSSIP_TEXT_BROWSE_GOODS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_TRADE);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }
};

/*######
## npc_darrowshire_spirit
######*/

enum DarrowshireSpirit
{
    SPELL_SPIRIT_SPAWNIN    = 17321
};

class npc_darrowshire_spirit : public CreatureScript
{
public:
    npc_darrowshire_spirit() : CreatureScript("npc_darrowshire_spirit") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->SEND_GOSSIP_MENU(3873, creature->GetGUID());
        player->TalkedToCreature(creature->GetEntry(), creature->GetGUID());
        creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_darrowshire_spiritAI(creature);
    }

    struct npc_darrowshire_spiritAI : public ScriptedAI
    {
        npc_darrowshire_spiritAI(Creature* creature) : ScriptedAI(creature) { }

        void Reset() override
        {
            DoCast(me, SPELL_SPIRIT_SPAWNIN);
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
        }

        void EnterCombat(Unit* /*who*/) override { }
    };
};

/*######
## npc_tirion_fordring
######*/

#define GOSSIP_HELLO    "I am ready to hear your tale, Tirion."
#define GOSSIP_SELECT1  "Thank you, Tirion.  What of your identity?"
#define GOSSIP_SELECT2  "That is terrible."
#define GOSSIP_SELECT3  "I will, Tirion."

class npc_tirion_fordring : public CreatureScript
{
public:
    npc_tirion_fordring() : CreatureScript("npc_tirion_fordring") { }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();
        switch (action)
        {
            case GOSSIP_ACTION_INFO_DEF+1:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT1, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 2);
                player->SEND_GOSSIP_MENU(4493, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+2:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT2, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 3);
                player->SEND_GOSSIP_MENU(4494, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+3:
                player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_SELECT3, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 4);
                player->SEND_GOSSIP_MENU(4495, creature->GetGUID());
                break;
            case GOSSIP_ACTION_INFO_DEF+4:
                player->CLOSE_GOSSIP_MENU();
                player->AreaExploredOrEventHappens(5742);
                break;
        }
        return true;
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(5742) == QUEST_STATUS_INCOMPLETE && player->getStandState() == UNIT_STAND_STATE_SIT)
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, GOSSIP_HELLO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF+1);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());

        return true;
    }
};

/*######
## npc_eris_havenfire
######*/

enum ErisHavenfire
{
    SAY_PHASE_HEAL                      = 1,
    SAY_EVENT_END                       = 2,
    SAY_EVENT_FAIL_1                    = 3,
    SAY_EVENT_FAIL_2                    = 4,
    SAY_PEASANT_APPEAR_1                = 5,
    SAY_PEASANT_APPEAR_2                = 6,
    SAY_PEASANT_APPEAR_3                = 7,

    // SPELL_DEATHS_DOOR                 = 23127,           // damage spells cast on the peasants
    // SPELL_SEETHING_PLAGUE             = 23072,
    SPELL_ENTER_THE_LIGHT_DND           = 23107,
    SPELL_BLESSING_OF_NORDRASSIL        = 23108,

    NPC_INJURED_PEASANT                 = 14484,
    NPC_PLAGUED_PEASANT                 = 14485,
    NPC_SCOURGE_ARCHER                  = 14489,
    NPC_SCOURGE_FOOTSOLDIER             = 14486,
    NPC_THE_CLEANER                     = 14503,            // can be summoned if the priest has more players in the party for help. requires further research

    QUEST_BALANCE_OF_LIGHT_AND_SHADOW   = 7622,

    MAX_PEASANTS                        = 12,
    MAX_ARCHERS                         = 8
};

enum Events
{
    EVENT_SPAWN_PEASANT_1               = 1,
    EVENT_SPAWN_PEASANT_2               = 2
};

enum Actions
{
    ACTION_START_EVENT                 = 1
};

Position const ArcherSpawn[8][4] =
{
    {3327.42f, -3021.11f, 170.57f, 6.01f},
    {3335.4f,  -3054.3f,  173.63f, 0.49f},
    {3351.3f,  -3079.08f, 178.67f, 1.15f},
    {3358.93f, -3076.1f,  174.87f, 1.57f},
    {3371.58f, -3069.24f, 175.20f, 1.99f},
    {3369.46f, -3023.11f, 171.83f, 3.69f},
    {3383.25f, -3057.01f, 181.53f, 2.21f},
    {3380.03f, -3062.73f, 181.90f, 2.31f}
};

Position const PeasantSpawnLoc[3] = {3360.12f, -3047.79f, 165.26f};
Position const PeasantMoveLoc[3] = {3335.0f, -2994.04f, 161.14f};

Position const int32 PeasantSpawnYells[3] = {SAY_PEASANT_APPEAR_1, SAY_PEASANT_APPEAR_2, SAY_PEASANT_APPEAR_3};

class npc_eris_havenfire : public CreatureScript
{
    public:
        npc_eris_havenfire() : CreatureScript("npc_eris_havenfire") { }

        bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) 
        {
            if (quest->GetQuestId() == QUEST_BALANCE_OF_LIGHT_AND_SHADOW)
            {
                creature->AI()->DoAction(ACTION_START_EVENT);
            }
        return true;
        }
		
    struct npc_eris_havenfireAI : public ScriptedAI
    {
        npc_eris_havenfireAI(Creature* creature) : ScriptedAI(creature)
        {
            Initialize();
        }
		
        uint32 uiEventTimer;
        uint32 uiSadEndTimer;
        uint8 uiPhase;
        uint8 uiCurrentWave;
        uint8 uiKillCounter;
        uint8 uiSaveCounter;

        ObjectGuid uiPlayerGUID;
        GuidList summonedGuidList;

        void Initialize()
        {
            _events.Reset();
            uiEventTimer      = 0;
            uiSadEndTimer     = 0;
            uiPhase           = 0;
            uiCurrentWave     = 0;
            uiKillCounter     = 0;
            uiSaveCounter     = 0;

            uiPlayerGUID.Clear();
            summonedGuidList.clear();
            me->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER);
        }
		
        void Reset() override
        {
            Initialize();
        }

        void JustSummoned(Creature* summon) override
        {
            switch (summon->GetEntry())
            {
                case NPC_INJURED_PEASANT:
                case NPC_PLAGUED_PEASANT:
                    float fX, fY, fZ;
                    summon->GetRandomPoint(PeasantMoveLoc[0], PeasantMoveLoc[1], PeasantMoveLoc[2], 10.0f, fX, fY, fZ);
                    summon->GetMotionMaster()->MovePoint(1, fX, fY, fZ);
                    summonedGuidList.push_back(summon->GetObjectGuid());
                    break;
                case NPC_SCOURGE_FOOTSOLDIER:
                case NPC_THE_CLEANER:
                    if (Player* player = me->GetMap()->GetPlayer(uiPlayerGUID))
                        summon->AI()->AttackStart(player);
                    break;
                case NPC_SCOURGE_ARCHER:
                    // ToDo: make these ones attack the peasants
                    break;
            }

            summonedGuidList.push_back(summon->GetObjectGuid());
        }

        void SummonedMovementInform(Creature* summon, uint32 type, uint32 pointId)
        {
            if (type != POINT_MOTION_TYPE || !pointId)
                return;

            if (pointId)
            {
                ++uiSaveCounter;
                summon->GetMotionMaster()->Clear();

                summon->RemoveAllAuras();
                summon->CastSpell(summon, SPELL_ENTER_THE_LIGHT_DND, false);
                summon->ForcedDespawn(10000);

                // Event ended
                if (uiSaveCounter >= 50 && uiCurrentWave == 5)
                    DoBalanceEventEnd();
                // Phase ended
                else if (uiSaveCounter + uiKillCounter == uiCurrentWave * MAX_PEASANTS)
                    DoHandlePhaseEnd();
            }
        }

        void SummonedCreatureJustDied(Creature* summon) override
        {
            if (summon->GetEntry() == NPC_INJURED_PEASANT || summon->GetEntry() == NPC_PLAGUED_PEASANT)
            {
                ++uiKillCounter;

                // If more than 15 peasants have died, then fail the quest
                if (uiKillCounter == MAX_PEASANTS)
                {
                    if (Player* player = me->GetMap()->GetPlayer(uiPlayerGUID))
                        player->FailQuest(QUEST_BALANCE_OF_LIGHT_AND_SHADOW);

                    Talk(SAY_EVENT_FAIL_1);
                    uiSadEndTimer = 4000;
                }
                else if (uiSaveCounter + uiKillCounter == uiCurrentWave * MAX_PEASANTS)
                    DoHandlePhaseEnd();
            }
        }

        void DoSummonWave(uint32 uiSummonId = 0)
        {
            float fX, fY, fZ;

            if (!uiSummonId)
            {
                for (uint8 i = 0; i < MAX_PEASANTS; ++i)
                {
                    uint32 uiSummonEntry = roll_chance_i(70) ? NPC_INJURED_PEASANT : NPC_PLAGUED_PEASANT;
                    me->GetRandomPoint(PeasantSpawnLoc[0], PeasantSpawnLoc[1], PeasantSpawnLoc[2], 10.0f, fX, fY, fZ);
                    if (Creature* pTemp = me->SummonCreature(uiSummonEntry, fX, fY, fZ, 0, TEMPSUMMON_DEAD_DESPAWN, 0))
                    {
                        // Only the first mob needs to yell
                        if (!i)
                            Talk(PeasantSpawnYells[urand(0, 2)]);
                    }
                }

                ++uiCurrentWave;
            }
            else if (uiSummonId == NPC_SCOURGE_FOOTSOLDIER)
            {
                uint8 uiRand = urand(2, 3);
                for (uint8 i = 0; i < uiRand; ++i)
                {
                    me->GetRandomPoint(PeasantSpawnLoc[0], PeasantSpawnLoc[1], PeasantSpawnLoc[2], 15.0f, fX, fY, fZ);
                    me->SummonCreature(NPC_SCOURGE_FOOTSOLDIER, fX, fY, fZ, 0, TEMPSUMMON_DEAD_DESPAWN, 0);
                }
            }
            else if (uiSummonId == NPC_SCOURGE_ARCHER)
            {
                for (uint8 i = 0; i < MAX_ARCHERS; ++i)
                    me->SummonCreature(NPC_SCOURGE_ARCHER, ArcherSpawn[i][0], ArcherSpawn[i][1], ArcherSpawn[i][2], ArcherSpawn[i][3], TEMPSUMMON_DEAD_DESPAWN, 0);
            }
        }

        void DoHandlePhaseEnd()
        {
            if (Player* player = me->GetMap()->GetPlayer(uiPlayerGUID))
                player->CastSpell(player, SPELL_BLESSING_OF_NORDRASSIL, true);

            Talk(SAY_PHASE_HEAL);

            // Send next wave
            if (uiCurrentWave < 5)
                DoSummonWave();
        }

        void DoAction(int32 action) override
        {
            switch (action)
            {
                case ACTION_START_EVENT:
                    me->RemoveFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    uiEventTimer = 5000;
                    break;
            }
        }

        void DoBalanceEventEnd()
        {
            if (Player* player = me->GetMap()->GetPlayer(uiPlayerGUID))
                player->AreaExploredOrEventHappens(QUEST_BALANCE_OF_LIGHT_AND_SHADOW);

            Talk(SAY_EVENT_END);
            DoDespawnSummons(true);
            EnterEvadeMode();
        }

        void DoDespawnSummons(bool bIsEventEnd = false)
        {
            for (GuidList::const_iterator itr = summonedGuidList.begin(); itr != summonedGuidList.end(); ++itr)
            {
                if (Creature* pTemp = me->GetMap()->GetCreature(*itr))
                {
                    if (bIsEventEnd && (pTemp->GetEntry() == NPC_INJURED_PEASANT || pTemp->GetEntry() == NPC_PLAGUED_PEASANT))
                        continue;

                    pTemp->ForcedDespawn();
                }
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (uiEventTimer)
            {
                if (uiEventTimer <= diff)
                {
                    switch (uiPhase)
                    {
                        case EVENT_SPAWN_PEASANT_1:
                            DoSummonWave(NPC_SCOURGE_ARCHER);
                            uiEventTimer = 5000;
                            break;
                        case EVENT_SPAWN_PEASANT_2:
                            DoSummonWave();
                            uiEventTimer = urand(60000, 80000);
                            break;
                        default:
                            // The summoning timer of the soldiers isn't very clear
                            DoSummonWave(NPC_SCOURGE_FOOTSOLDIER);
                            uiEventTimer = urand(5000, 30000);
                            break;
                    }
                    ++uiPhase;
                }
                else
                    uiEventTimer -= diff;
            }

            // Handle event end in case of fail
            if (uiSadEndTimer)
            {
                if (uiSadEndTimer <= diff)
                {
                    Talk(SAY_EVENT_FAIL_2);
                    me->ForcedDespawn(5000);
                    DoDespawnSummons();
                    uiSadEndTimer = 0;
                }
                else
                    uiSadEndTimer -= diff;
            }
        }
    };


    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_eris_havenfireAI(creature);
    }
};

void AddSC_eastern_plaguelands()
{
    new npc_ghoul_flayer();
    new npc_augustus_the_touched();
    new npc_darrowshire_spirit();
    new npc_tirion_fordring();
	new npc_eris_havenfire();
}
