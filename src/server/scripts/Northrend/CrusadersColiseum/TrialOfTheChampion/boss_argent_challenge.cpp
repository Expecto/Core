/*
 * Copyright (C) 2008-2015 TrinityCore <http://www.trinitycore.org/>
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
#include "trial_of_the_champion.h"
#include "ScriptedEscortAI.h"

enum Spells
{
    //Eadric
    SPELL_EADRIC_ACHIEVEMENT    = 68197,
    SPELL_HAMMER_JUSTICE        = 66863,
    SPELL_HAMMER_JUSTICE_STUN   = 66940,
    SPELL_HAMMER_RIGHTEOUS      = 66867,
    SPELL_HAMMER_OVERRIDE_BAR   = 66904, // overrides players cast bar
    SPELL_HAMMER_THROWBACK_DMG  = 66905, // the hammer that is thrown back by the player
    SPELL_RADIANCE              = 66935,
    SPELL_VENGEANCE             = 66865,

    //Paletress
    SPELL_CONFESSOR_ACHIEVEMENT = 68206,
    SPELL_SMITE                 = 66536,
    SPELL_SMITE_H               = 67674,
    SPELL_HOLY_FIRE             = 66538,
    SPELL_HOLY_FIRE_H           = 67676,
    SPELL_RENEW                 = 66537,
    SPELL_RENEW_H               = 67675,
    SPELL_HOLY_NOVA             = 66546,
    SPELL_SHIELD                = 66515,
    SPELL_CONFESS               = 66680,
    SPELL_SUMMON_MEMORY         = 66545,
    
    // Monk soldier
    SPELL_PUMMEL                = 67235,
    SPELL_FLURRY                = 67233,
    SPELL_FINAL                 = 67255,
    SPELL_DIVINE                = 67251,
	
    // Lightwielder soldier
    SPELL_LIGHT                 = 67247,
    SPELL_CLEAVE                = 15284,
    SPELL_STRIKE                = 67237,
	
    // Priest soldier
    SPELL_HOLY_SMITE            = 36176,
    SPELL_HOLY_SMITE_H          = 67289,
    SPELL_SHADOW_WORD_PAIN      = 34941,
    SPELL_SHADOW_WORD_PAIN_H    = 34942,
    SPELL_MIND                  = 67229,
    SPELL_FOUNTAIN_OF_LIGHT     = 67194,

    //Memory
    SPELL_OLD_WOUNDS            = 66620,
    SPELL_OLD_WOUNDS_H          = 67679,
    SPELL_SHADOWS_PAST          = 66619,
    SPELL_SHADOWS_PAST_H        = 67678,
    SPELL_WAKING_NIGHTMARE      = 66552,
    SPELL_WAKING_NIGHTMARE_H    = 67677
};

enum Misc
{
    DATA_FACEROLLER             = 1,
    ACHIEV_CONF                 = 3802
};

enum Talk
{
    SAY_ARGENT_ENTERS           = 19,
    SAY_ARGENT_READY            = 20,
    SAY_MEMORY_NIGHTMARE        = 0,

    // Paletress
    SAY_PALETRESS_AGGRO         = 2,
    SAY_PALETRESS_SUMMON_MEMORY = 3,
    SAY_PALETRESS_MEMORY_DIES   = 4,
    SAY_PALETRESS_PLAYER_DIES   = 5,
    SAY_PALETRESS_DEFEATED      = 6,

    // Eadric
    SAY_EADRIC_AGGRO            = 1,
    SAY_EADRIC_RADIATE_LIGHT    = 2,
    SAY_EADRIC_HAMMER_TARGET    = 3,
    SAY_EADRIC_HAMMER           = 4,
    SAY_EADRIC_PLAYER_DIES      = 5,
    SAY_EADRIC_DEFEATED         = 6
};

enum Events
{
    EVENT_HOLYSMITE,
    // Argent Soldiers
    EVENT_CLEAVE,
    EVENT_STRIKE,
    EVENT_BLAZINGLIGHT,
    EVENT_FLURRY,
    EVENT_PUMMEL,
    EVENT_MINDCONTROL,
    EVENT_PAIN,
    EVENT_FOUNTAIN
};

class OrientationCheck
{
    public:
        explicit OrientationCheck(Unit* _caster) : caster(_caster) { }
        bool operator() (WorldObject* object)
        {
            return !object->isInFront(caster, 2.5f) || !object->IsWithinDist(caster, 40.0f);
        }

    private:
        Unit* caster;
};

class spell_eadric_radiance: public SpellScriptLoader
{
    public:
        spell_eadric_radiance(): SpellScriptLoader("spell_eadric_radiance") { }

        class spell_eadric_radiance_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eadric_radiance_SpellScript);

            void FilterTargets(std::list<WorldObject*>& unitList)
            {
                unitList.remove_if(OrientationCheck(GetCaster()));
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_eadric_radiance_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_eadric_radiance_SpellScript::FilterTargets, EFFECT_1, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_eadric_radiance_SpellScript();
        }
};

class spell_eadric_hammer_missile: public SpellScriptLoader
{
    public:
        spell_eadric_hammer_missile(): SpellScriptLoader("spell_eadric_hammer_missile") { }

        class spell_eadric_hammer_missile_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eadric_hammer_missile_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/)
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_HAMMER_OVERRIDE_BAR))
                    return false;

                return true;
            }

            void HandleTriggerMissile(SpellEffIndex /*effIndex*/)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();

                if (caster && target && !target->HasAura(SPELL_HAMMER_JUSTICE_STUN))
                {
                    PreventHitDefaultEffect(EFFECT_0);
                    caster->CastSpell(target, SPELL_HAMMER_OVERRIDE_BAR, true);
                }
            }

            void Register()
            {
                OnEffectHitTarget += SpellEffectFn(spell_eadric_hammer_missile_SpellScript::HandleTriggerMissile, EFFECT_0, SPELL_EFFECT_TRIGGER_MISSILE);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_eadric_hammer_missile_SpellScript();
        }
};

class spell_eadric_hammer_throw_back: public SpellScriptLoader
{
    public:
        spell_eadric_hammer_throw_back(): SpellScriptLoader("spell_eadric_hammer_throw_back") { }

        class spell_eadric_hammer_throw_back_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_eadric_hammer_throw_back_SpellScript);

            void RemoveAura()
            {
                GetCaster()->RemoveAurasDueToSpell(SPELL_HAMMER_OVERRIDE_BAR);
            }

            void CheckDamage()
            {
                Creature* target = GetHitCreature();
                if (!target)
                    return;

                if (GetHitDamage() >= int32(target->GetHealth()))
                    target->AI()->SetData(DATA_FACEROLLER, 1);
            }

            void Register()
            {
                AfterCast += SpellCastFn(spell_eadric_hammer_throw_back_SpellScript::RemoveAura);
                OnHit += SpellHitFn(spell_eadric_hammer_throw_back_SpellScript::CheckDamage);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_eadric_hammer_throw_back_SpellScript();
        }
};

class boss_eadric : public CreatureScript
{
    public:
        boss_eadric(): CreatureScript("boss_eadric") { }

    struct boss_eadricAI : public BossAI
    {
        boss_eadricAI(Creature* creature) : BossAI(creature, BOSS_ARGENT_CHALLENGE_E)
        {
            instance = creature->GetInstanceScript();
            creature->SetReactState(REACT_PASSIVE);
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

            hasBeenInCombat = false;
            bCredit = false;
        }

        InstanceScript* instance;

        uint32 uiVenganceTimer;
        uint32 uiRadianceTimer;
        uint32 uiHammerJusticeTimer;
        uint32 uiResetTimer;

        uint64 uiBasePoints;

        bool bDone;
        bool hasBeenInCombat;
        bool bCredit;
        bool _theFaceRoller;

        void Reset()
        {
            uiVenganceTimer = 10000;
            uiRadianceTimer = 16000;
            uiHammerJusticeTimer = 25000;
            uiResetTimer = 5000;
            uiBasePoints = 0;

            _theFaceRoller = false;
            bDone = false;
            Map* pMap = me->GetMap();
            if (hasBeenInCombat && pMap && pMap->IsDungeon())
            {
                Map::PlayerList const &players = pMap->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                     if (itr->GetSource() && itr->GetSource()->IsAlive() && !itr->GetSource()->IsGameMaster())
                         return;
                }
                
                if (instance)
                {
                    GameObject* GO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1));
                    if (GO)
                        instance->HandleGameObject(GO->GetGUID(),true);
                    Creature* announcer=pMap->GetCreature(instance->GetGuidData(DATA_ANNOUNCER));
                    instance->SetData(DATA_ARGENT_SOLDIER_DEFEATED,0);
                    announcer->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }

                me->RemoveFromWorld();
            }
        }

        void DamageTaken(Unit* /*who*/, uint32& damage)
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;
                if (!bCredit)
                {
                    bCredit = true;
                    HandleSpellOnPlayersInInstanceToC5(me, 68575);
                }
                EnterEvadeMode();
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                Talk(SAY_EADRIC_DEFEATED);
                me->setFaction(35);
                bDone = true;
                if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                    instance->HandleGameObject(pGO->GetGUID(),true);
                if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                    instance->HandleGameObject(pGO->GetGUID(),true);
                if (instance)
                    instance->SetData(BOSS_ARGENT_CHALLENGE_E, DONE);
                HandleKillCreditForAllPlayers(me);
                HandleInstanceBind(me);
            }
        }

        void MovementInform(uint32 MovementType, uint32 /*Data*/)
        {
            if (MovementType != POINT_MOTION_TYPE)
                return;
        }

        void EnterCombat(Unit* /*who*/)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            _EnterCombat();
            me->SetHomePosition(746.843f, 665.000f, 412.339f, 4.670f);
            Talk(SAY_EADRIC_AGGRO);
            hasBeenInCombat = true;
        }

        void SetData(uint32 type, uint32 /*value*/)
        {
            if (type == DATA_FACEROLLER)
                _theFaceRoller = true;
        }

        void SpellHit(Unit* caster, SpellInfo const* spell)
        {
            if (IsHeroic() && !bDone && spell->Id == SPELL_HAMMER_THROWBACK_DMG && caster->GetTypeId() == TYPEID_PLAYER)
            {            
                uiBasePoints = spell->Effects[0].BasePoints;
                if (me->GetHealth() <= uiBasePoints)
                {
                    _theFaceRoller = true;
                    HandleSpellOnPlayersInInstanceToC5(me, SPELL_EADRIC_ACHIEVEMENT);
                }
            }
        }

        uint32 GetData(uint32 type) const override
        {
            if (type == DATA_THE_FACEROLLER)
                return _theFaceRoller;

            return 0;
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (bDone && uiResetTimer <= uiDiff)
            {
                me->GetMotionMaster()->MovePoint(0,746.843f, 695.68f, 412.339f);
                bDone = false;
                if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                    instance->HandleGameObject(pGO->GetGUID(),false);
            } else uiResetTimer -= uiDiff;

            if (!UpdateVictim())
                return;

            if (uiHammerJusticeTimer <= uiDiff)
            {
                me->InterruptNonMeleeSpells(true);

                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 250, true))
                {
                    if (target && target->IsAlive())
                    {
                        Talk(SAY_EADRIC_HAMMER);
                        Talk(SAY_EADRIC_HAMMER_TARGET, target);
                        DoCast(target, SPELL_HAMMER_JUSTICE);
                        DoCast(target, SPELL_HAMMER_RIGHTEOUS);
                    }
                }
                uiHammerJusticeTimer = 25000;
            } else uiHammerJusticeTimer -= uiDiff;

            if (uiVenganceTimer <= uiDiff)
            {
                DoCast(me, SPELL_VENGEANCE);

                uiVenganceTimer = 10000;
            } else uiVenganceTimer -= uiDiff;

            if (uiRadianceTimer <= uiDiff)
            {
                DoCastAOE(SPELL_RADIANCE);
                Talk(SAY_EADRIC_RADIATE_LIGHT);
                uiRadianceTimer = 16000;
            } else uiRadianceTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new boss_eadricAI(creature);
    };
};

class boss_paletress : public CreatureScript
{
    public:
        boss_paletress(): CreatureScript("boss_paletress") { }

    struct boss_paletressAI : public BossAI
    {
        boss_paletressAI(Creature* creature) : BossAI(creature, BOSS_ARGENT_CHALLENGE_P)
        {
            instance = creature->GetInstanceScript();

            hasBeenInCombat = false;
            bCredit = false;

            creature->SetReactState(REACT_PASSIVE);
            creature->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
            creature->RestoreFaction();
        }

        InstanceScript* instance;

        Creature* pMemory;
        ObjectGuid MemoryGUID;

        bool bHealth;
        bool bDone;
        bool hasBeenInCombat;
        bool bCredit;

        uint32 uiHolyFireTimer;
        uint32 uiHolySmiteTimer;
        uint32 uiRenewTimer;
        uint32 uiResetTimer;

        void Reset()
        {
            me->RemoveAllAuras();

            uiHolyFireTimer     = urand(9000,12000);
            uiHolySmiteTimer    = urand(5000,7000);
            uiRenewTimer        = urand(2000,5000);

            uiResetTimer        = 7000;

            bHealth = false;
            bDone = false;

            if (Creature* pMemory = ObjectAccessor::GetCreature(*me, MemoryGUID))
                if (pMemory->IsAlive())
                    pMemory->RemoveFromWorld();

            Map* pMap = me->GetMap();
            if (hasBeenInCombat && pMap && pMap->IsDungeon())
            {
                Map::PlayerList const &players = pMap->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                {
                    if (itr->GetSource() && itr->GetSource()->IsAlive() && !itr->GetSource()->IsGameMaster())
                        return;
                }

                if (instance)
                {
                    GameObject* GO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1));
                    if(GO)
                       instance->HandleGameObject(GO->GetGUID(),true);
                    Creature* announcer = pMap->GetCreature(instance->GetGuidData(DATA_ANNOUNCER));
                    instance->SetData(DATA_ARGENT_SOLDIER_DEFEATED, 0);
                    announcer->SetFlag(UNIT_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                }

                me->RemoveFromWorld();
            }
        }

        void EnterCombat(Unit* /*who*/)
        {
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            _EnterCombat();
            me->SetHomePosition(746.843f, 665.000f, 412.339f, 4.670f);
            hasBeenInCombat = true;
            Talk(SAY_PALETRESS_AGGRO);
        }

        void SetData(uint32 uiId, uint32 /*uiValue*/)
        {
            if (uiId == 1)
                me->RemoveAura(SPELL_SHIELD);
                Talk(SAY_PALETRESS_MEMORY_DIES);
        }

        void DamageTaken(Unit* /*who*/, uint32& damage)
        {
            if (damage >= me->GetHealth())
            {
                damage = 0;
                if (!bCredit)
                {
                    bCredit = true;
                    HandleSpellOnPlayersInInstanceToC5(me, 68574);
                }
                EnterEvadeMode();
                me->SetFlag(UNIT_FIELD_FLAGS,UNIT_FLAG_NON_ATTACKABLE);
                Talk(SAY_PALETRESS_DEFEATED);
                me->setFaction(35);
                bDone = true;
                if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                    instance->HandleGameObject(pGO->GetGUID(),true);
                if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE1)))
                    instance->HandleGameObject(pGO->GetGUID(),true);
                instance->SetData(BOSS_ARGENT_CHALLENGE_P, DONE);
                HandleKillCreditForAllPlayers(me);
                HandleInstanceBind(me);

                if (Creature* memory = me->GetMap()->ToInstanceMap()->GetCreature(MemoryGUID))
                    HandleSpellOnPlayersInInstanceToC5(memory, SPELL_CONFESSOR_ACHIEVEMENT);
            }
        }

        void MovementInform(uint32 MovementType, uint32 /*Data*/)
        {
            if (MovementType != POINT_MOTION_TYPE)
                return;

        }

        void UpdateAI(uint32 uiDiff)
        {
            if (bDone && uiResetTimer <= uiDiff)
            {
                me->GetMotionMaster()->MovePoint(0, 746.843f, 695.68f, 412.339f);
                bDone = false;
                if (GameObject* pGO = ObjectAccessor::GetGameObject(*me, instance->GetGuidData(DATA_MAIN_GATE)))
                    instance->HandleGameObject(pGO->GetGUID(),false);
            } else uiResetTimer -= uiDiff;

            if (!UpdateVictim())
                return;

            if (uiHolyFireTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 250, true))
                {
                    if (target && target->IsAlive())
                        DoCast(target,DUNGEON_MODE(SPELL_HOLY_FIRE,SPELL_HOLY_FIRE_H));
                }
                if (me->HasAura(SPELL_SHIELD))
                    uiHolyFireTimer = 13000;
                else
                    uiHolyFireTimer = urand(9000,12000);
            } else uiHolyFireTimer -= uiDiff;

            if (uiHolySmiteTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 250, true))
                {
                    if (target && target->IsAlive())
                        DoCast(target,DUNGEON_MODE(SPELL_SMITE,SPELL_SMITE_H));
                }
                if (me->HasAura(SPELL_SHIELD))
                    uiHolySmiteTimer = 9000;
                else
                    uiHolySmiteTimer = urand(5000,7000);
            } else uiHolySmiteTimer -= uiDiff;

            if (me->HasAura(SPELL_SHIELD))
            {
                if (uiRenewTimer <= uiDiff)
                {
                    me->InterruptNonMeleeSpells(true);
                    uint8 uiTarget = urand(0,1);
                    switch(uiTarget)
                    {
                        case 0:
                            DoCast(me,DUNGEON_MODE(SPELL_RENEW,SPELL_RENEW_H));
                            break;
                        case 1:
                            if (Creature* pMemory = ObjectAccessor::GetCreature(*me, MemoryGUID))
                                if (pMemory->IsAlive())
                                    DoCast(pMemory, DUNGEON_MODE(SPELL_RENEW,SPELL_RENEW_H));
                            break;
                    }
                    uiRenewTimer = urand(15000,17000);
                } else uiRenewTimer -= uiDiff;
            }

            if (!bHealth && me->HealthBelowPct(25))
            {
                Talk(SAY_PALETRESS_SUMMON_MEMORY);
                me->InterruptNonMeleeSpells(true);
                DoCastAOE(SPELL_HOLY_NOVA, false);
                DoCast(me, SPELL_SHIELD);
                DoCastAOE(SPELL_CONFESS, false);

                bHealth = true;
                DoCast(SPELL_SUMMON_MEMORY);
            }

            DoMeleeAttackIfReady();
        }

        void JustSummoned(Creature* summon) override
        {
            MemoryGUID = summon->GetGUID();
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new boss_paletressAI(creature);
    };
};

class npc_memory : public CreatureScript
{
    public:
        npc_memory(): CreatureScript("npc_memory") { }

    struct npc_memoryAI : public ScriptedAI
    {
        npc_memoryAI(Creature* creature) : ScriptedAI(creature) { }

        uint32 uiOldWoundsTimer;
        uint32 uiShadowPastTimer;
        uint32 uiWakingNightmare;

        void Reset()
        {
            uiOldWoundsTimer = 12000;
            uiShadowPastTimer = 5000;
            uiWakingNightmare = 7000;
        }

        void UpdateAI(uint32 uiDiff) override
        {
            if (!UpdateVictim())
                return;

            if (uiOldWoundsTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM,0))
                {
                    if (target && target->IsAlive())
                        DoCast(target, DUNGEON_MODE(SPELL_OLD_WOUNDS,SPELL_OLD_WOUNDS_H));
                }
                uiOldWoundsTimer = 23000;
            } else uiOldWoundsTimer -= uiDiff;

            if (uiWakingNightmare <= uiDiff)
            {
                Talk(SAY_MEMORY_NIGHTMARE);
                DoCast(me, DUNGEON_MODE(SPELL_WAKING_NIGHTMARE,SPELL_WAKING_NIGHTMARE_H));
                uiWakingNightmare = 15000;
            } else uiWakingNightmare -= uiDiff;

            if (uiShadowPastTimer <= uiDiff)
            {
                if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM,1))
                {
                    if (target && target->IsAlive())
                        DoCast(target, DUNGEON_MODE(SPELL_SHADOWS_PAST,SPELL_SHADOWS_PAST_H));
                }
                uiShadowPastTimer = 20000;
            } else uiShadowPastTimer -= uiDiff;

            DoMeleeAttackIfReady();
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (me->IsSummon())
            {
                if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                {
                    if (summoner && summoner->IsAlive() && summoner->GetTypeId() == TYPEID_UNIT)
                        summoner->ToCreature()->AI()->SetData(1,0);
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const
    {
        return new npc_memoryAI(creature);
    };
};

class npc_argent_soldier : public CreatureScript
{
    public:
        npc_argent_soldier(): CreatureScript("npc_argent_soldier") { }

        struct npc_argent_soldierAI : public npc_escortAI
        {
            npc_argent_soldierAI(Creature* creature) : npc_escortAI(creature)
            {
                instance = creature->GetInstanceScript();
                me->SetReactState(REACT_DEFENSIVE);
                SetDespawnAtEnd(false);
                uiWaypoint = 0;
            }

            void Reset()
            {
                _shielded = false;
                _events.Reset();
            }

            void EnterCombat(Unit* /*who*/)
            {
                switch (me->GetEntry())
                {
                    case NPC_ARGENT_LIGHWIELDER:
                        _events.ScheduleEvent(EVENT_CLEAVE, urand(4000, 8000));
                        _events.ScheduleEvent(EVENT_BLAZINGLIGHT, urand(7000, 10000));
                        if (IsHeroic())
                            _events.ScheduleEvent(EVENT_STRIKE, urand(9000, 12000));
                        break;
                    case NPC_ARGENT_MONK:
                        _events.ScheduleEvent(EVENT_FLURRY, urand(8000, 12000));
                        _events.ScheduleEvent(EVENT_PUMMEL, urand(5000, 6000));
                        break;
                    case NPC_PRIESTESS:
                        _events.ScheduleEvent(EVENT_PAIN, urand(3000, 6000));
                        _events.ScheduleEvent(EVENT_FOUNTAIN, urand(15000, 20000));
                        _events.ScheduleEvent(EVENT_HOLYSMITE, urand(4000, 8000));
                        if (IsHeroic())
                            _events.ScheduleEvent(EVENT_MINDCONTROL, urand(17000, 25000));
                        break;
                }
            }

            void WaypointReached(uint32 uiPoint) override
            {
                if (uiPoint == 0)
                {
                    switch (uiWaypoint)
                    {
                        case 0:
                            me->SetOrientation(5.81f);
                            break;
                        case 1:
                            me->SetOrientation(4.60f);
                            break;
                        case 2:
                            me->SetOrientation(2.79f);
                            break;
                    }

                    me->SendMovementFlagUpdate();
                }
            }

            void SetData(uint32 uiType, uint32 /*uiData*/) override
            {
                switch (me->GetEntry())
                {
                    case NPC_ARGENT_LIGHWIELDER:
                        switch (uiType)
                        {
                            case 0:
                                AddWaypoint(0, 712.14f, 628.42f, 411.88f);
                                break;
                            case 1:
                                AddWaypoint(0, 742.44f, 650.29f, 411.79f);
                                break;
                            case 2:
                                AddWaypoint(0, 783.33f, 615.29f, 411.84f);
                                break;
                        }
                        break;
                    case NPC_ARGENT_MONK:
                        switch (uiType)
                        {
                            case 0:
                                AddWaypoint(0, 713.12f, 632.97f, 411.90f);
                                break;
                            case 1:
                                AddWaypoint(0, 746.73f, 650.24f, 411.56f);
                                break;
                            case 2:
                                AddWaypoint(0, 781.32f, 610.54f, 411.82f);
                                break;
                        }
                        break;
                    case NPC_PRIESTESS:
                        switch (uiType)
                        {
                            case 0:
                                AddWaypoint(0, 715.06f, 637.07f, 411.91f);
                                break;
                            case 1:
                                AddWaypoint(0, 750.72f, 650.20f, 411.77f);
                                break;
                            case 2:
                                AddWaypoint(0, 779.77f, 607.03f, 411.81f);
                                break;
                        }
                        break;
                }

                Start(false, true);
                uiWaypoint = uiType;
            }

            void DamageTaken(Unit* /*attacker*/, uint32 &damage)
            {
                if (!IsHeroic() || _shielded || me->GetEntry() != NPC_ARGENT_MONK)
                    return;

                if (damage >= me->GetHealth())
                {
                    DoCast(me, SPELL_FINAL);
                    DoCast(me, SPELL_DIVINE, true);
                    me->SetHealth(1);
                    damage = 0;
                    _shielded = true;
                }
            }

            void UpdateAI(uint32 uiDiff) override
            {
                npc_escortAI::UpdateAI(uiDiff);

                if (!UpdateVictim())
                    return;

                _events.Update(uiDiff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = _events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CLEAVE:
                            DoCastVictim(SPELL_CLEAVE);
                            _events.ScheduleEvent(EVENT_CLEAVE, urand(5000, 8000));
                            break;
                        case EVENT_STRIKE:
                            DoCastVictim(SPELL_STRIKE);
                            _events.ScheduleEvent(EVENT_STRIKE, urand(10000, 12000));
                            break;
                        case EVENT_BLAZINGLIGHT:
                            DoCast(me, SPELL_LIGHT);
                            _events.ScheduleEvent(EVENT_BLAZINGLIGHT, urand(9000, 13000));
                            break;
                        case EVENT_FLURRY:
                            DoCast(me, SPELL_FLURRY);
                            _events.ScheduleEvent(EVENT_FLURRY, urand(13000, 15000));
                            break;
                        case EVENT_PUMMEL:
                            DoCastVictim(SPELL_PUMMEL);
                            _events.ScheduleEvent(EVENT_PUMMEL, urand(4000, 7000));
                            break;
                        case EVENT_HOLYSMITE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 40.0f))
                                DoCast(target, DUNGEON_MODE(SPELL_HOLY_SMITE, SPELL_HOLY_SMITE_H));
                            _events.ScheduleEvent(EVENT_HOLYSMITE, urand(5000, 7000));
                            break;
                        case EVENT_MINDCONTROL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 30.0f))
                                DoCast(target, SPELL_MIND);
                            _events.ScheduleEvent(EVENT_MINDCONTROL, urand(20000, 25000));
                            break;
                        case EVENT_PAIN:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, false, DUNGEON_MODE(-SPELL_SHADOW_WORD_PAIN, -SPELL_SHADOW_WORD_PAIN_H)))
                                DoCast(target, DUNGEON_MODE(SPELL_SHADOW_WORD_PAIN, SPELL_SHADOW_WORD_PAIN_H));
                            _events.ScheduleEvent(EVENT_PAIN, urand(7000, 90000));
                            break;
                        case EVENT_FOUNTAIN:
                            DoCast(SPELL_FOUNTAIN_OF_LIGHT);
                            _events.ScheduleEvent(EVENT_FOUNTAIN, urand(20000, 30000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

            void JustDied(Unit* /*killer*/) override
            {
                if (instance)
                    instance->SetData(DATA_ARGENT_SOLDIER_DEFEATED, instance->GetData(DATA_ARGENT_SOLDIER_DEFEATED) + 1);
            }

        private:
            InstanceScript* instance;
            EventMap _events;
            bool _shielded;
            uint8 uiWaypoint;
        };

        CreatureAI* GetAI(Creature* creature) const
        {
            return new npc_argent_soldierAI(creature);
        }
};

enum ReflectiveShield
{
    SPELL_REFLECTIVE_SHIELD_TRIGGERED = 33619
};

// Reflective Shield 66515
class spell_gen_reflective_shield : public SpellScriptLoader
{
    public:
        spell_gen_reflective_shield() : SpellScriptLoader("spell_gen_reflective_shield") { }

        class spell_gen_reflective_shield_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_gen_reflective_shield_AuraScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellStore.LookupEntry(SPELL_REFLECTIVE_SHIELD_TRIGGERED))
                    return false;

                return true;
            }

            void Trigger(AuraEffect * aurEff, DamageInfo & dmgInfo, uint32 & absorbAmount)
            {
                Unit * target = dmgInfo.GetAttacker();
                if (!target)
                    return;
                Unit * caster = GetCaster();
                if (!caster)
                    return;
                int32 bp = CalculatePct(absorbAmount, 25);
                target->CastCustomSpell(target, SPELL_REFLECTIVE_SHIELD_TRIGGERED, &bp, NULL, NULL, true, NULL, aurEff);
            }

            void Register()
            {
                 AfterEffectAbsorb += AuraEffectAbsorbFn(spell_gen_reflective_shield_AuraScript::Trigger, EFFECT_0);
            }
        };

        AuraScript *GetAuraScript() const
        {
            return new spell_gen_reflective_shield_AuraScript();
        }
};

class achievement_toc5_argent_challenge : public AchievementCriteriaScript
{
    public:
        uint32 creature_entry;

        achievement_toc5_argent_challenge(const char* name, uint32 original_entry) : AchievementCriteriaScript(name) {
            creature_entry = original_entry;
        }

        bool OnCheck(Player* /*source*/, Unit* target) override
        {
            if (!target)
                return false;

            if (Creature* creature = target->ToCreature())
                if (creature->GetOriginalEntry() == creature_entry)
                    return true;

            return false;
        }
};

class achievement_toc5_argent_confessor : public AchievementCriteriaScript
{
    public:
        uint32 creature_entry;

        achievement_toc5_argent_confessor(const char* name, uint32 original_entry) : AchievementCriteriaScript(name) 
        {
            creature_entry = original_entry;
        }

        bool OnCheck(Player* /*source*/, Unit* target) override
        {
            if (!target)
                return false;

            if (Creature* creature = target->ToCreature())
                if (creature->GetEntry() == creature_entry && creature->GetMap()->ToInstanceMap()->IsHeroic())
                    return true;

            return false;
        }
};

class achievement_toc5_the_faceroller : public AchievementCriteriaScript
{
    public:
        achievement_toc5_the_faceroller(const char* name) : AchievementCriteriaScript(name) {}

        bool OnCheck(Player* /*source*/, Unit* target) override
        {
            if (target && target->GetMap()->ToInstanceMap()->IsHeroic())
                return target->GetAI()->GetData(DATA_THE_FACEROLLER);

            return false;
        }
};

class spell_light_rain: public SpellScriptLoader
{
    public:
        spell_light_rain(): SpellScriptLoader("spell_light_rain") { }

        class spell_light_rain_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_light_rain_SpellScript);

            void SelectTarget(std::list<WorldObject*>& unitList)
            {
                if (unitList.empty())
                    return;

                unitList.sort(Trinity::HealthPctOrderPred());
                unitList.resize(1);
            }

            void Register()
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_light_rain_SpellScript::SelectTarget, EFFECT_0, TARGET_UNIT_SRC_AREA_ALLY);
            }
        };

        SpellScript* GetSpellScript() const
        {
            return new spell_light_rain_SpellScript();
        }
};

void AddSC_boss_argent_challenge()
{
    new boss_eadric();
    new boss_paletress();
    new npc_memory();
    new npc_argent_soldier();
    new spell_gen_reflective_shield();
    new spell_light_rain();
    new spell_eadric_radiance();
    new spell_eadric_hammer_missile();
    new spell_eadric_hammer_throw_back();
    new achievement_toc5_argent_challenge("achievement_toc5_paletress", NPC_PALETRESS);
    new achievement_toc5_argent_challenge("achievement_toc5_eadric", NPC_EADRIC);
    
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_hogger", MEMORY_HOGGER);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_vancleef", MEMORY_VANCLEEF);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_mutanus", MEMORY_MUTANUS);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_herod", MEMORY_HEROD);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_lucifron", MEMORY_LUCIFRON);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_thunderaan", MEMORY_THUNDERAAN);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_chromaggus", MEMORY_CHROMAGGUS);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_hakkar", MEMORY_HAKKAR);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_veknilash", MEMORY_VEKNILASH);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_kalithresh", MEMORY_KALITHRESH);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_malchezar", MEMORY_MALCHEZAAR);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_gruul", MEMORY_GRUUL);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_vashj", MEMORY_VASHJ);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_archimonde", MEMORY_ARCHIMONDE);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_illidan", MEMORY_ILLIDAN);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_delrissa", MEMORY_DELRISSA);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_muru", MEMORY_ENTROPIUS);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_ingvar", MEMORY_INGVAR);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_cyanigosa", MEMORY_CYANIGOSA);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_eck", MEMORY_ECK);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_onyxia", MEMORY_ONYXIA);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_heigan", MEMORY_HEIGAN);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_ignis", MEMORY_IGNIS);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_vezax", MEMORY_VEZAX);
    new achievement_toc5_argent_confessor("achivement_toc5_argent_confessor_algalon", MEMORY_ALGALON);

    new achievement_toc5_the_faceroller("achievement_toc5_the_faceroller");
}