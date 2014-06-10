/*
 * Lacrimi Scripts Copyright 2010 - 2011
 *
 * ############################################################
 * # ##            #       ####### ####### ##    #    #    ## #
 * # ##           ###      ##      ##   ## ##   ###  ###   ## #
 * # ##          ## ##     ##      ##   ## ##   ###  ###   ## #
 * # ##         #######    ##      ####### ##  ## #### ##  ## #
 * # ##        ##     ##   ##      #####   ##  ## #### ##  ## #
 * # ##       ##       ##  ##      ##  ##  ## ##   ##   ## ## #
 * # ####### ##         ## ####### ##   ## ## ##   ##   ## ## #
 * # :::::::.::.........::.:::::::.::...::.::.::...::...::.:: #
 * ############################################################
 *
 */

#include "LacrimiStdAfx.h"

LuaSpellEntry luaSpellVars[] = {
    {"Id", 0, offsetof(SpellEntry, Id)},
    {"Category", 0, offsetof(SpellEntry, Category)},
    {"DispelType", 0, offsetof(SpellEntry, DispelType)},
    {"MechanicsType", 0, offsetof(SpellEntry, MechanicsType)},
    {"Attributes", 0, offsetof(SpellEntry, Attributes)},
    {"AttributesEx", 0, offsetof(SpellEntry, AttributesEx)},
    {"Flags3", 0, offsetof(SpellEntry, Flags3)},
    {"Flags4", 0, offsetof(SpellEntry, Flags4)},
    {"Flags5", 0, offsetof(SpellEntry, Flags5)},
    {"Flags6", 0, offsetof(SpellEntry, Flags6)},
    {"Flags7", 0, offsetof(SpellEntry, Flags7)},
    {"AttributesExB", 0, offsetof(SpellEntry, Flags3)},
    {"AttributesExC", 0, offsetof(SpellEntry, Flags4)},
    {"AttributesExD", 0, offsetof(SpellEntry, Flags5)},
    {"AttributesExE", 0, offsetof(SpellEntry, Flags6)},
    {"AttributesExF", 0, offsetof(SpellEntry, Flags7)},
    {"RequiredShapeShift", 0, offsetof(SpellEntry, RequiredShapeShift)},
    {"NotAllowedShapeShift", 0, offsetof(SpellEntry, ShapeshiftExclude)},
    {"ShapeshiftExclude", 0, offsetof(SpellEntry, ShapeshiftExclude)},
    {"Targets", 0, offsetof(SpellEntry, Targets)},
    {"TargetCreatureType", 0, offsetof(SpellEntry, TargetCreatureType)},
    {"RequiresSpellFocus", 0, offsetof(SpellEntry, RequiresSpellFocus)},
    {"FacingCasterFlags", 0, offsetof(SpellEntry, FacingCasterFlags)},
    {"CasterAuraState", 0, offsetof(SpellEntry, CasterAuraState)},
    {"TargetAuraState", 0, offsetof(SpellEntry, TargetAuraState)},
    {"CasterAuraStateNot", 0, offsetof(SpellEntry, CasterAuraStateNot)},
    {"TargetAuraStateNot", 0, offsetof(SpellEntry, TargetAuraStateNot)},
    {"CastingTimeIndex", 0, offsetof(SpellEntry, CastingTimeIndex)},
    {"RecoveryTime", 0, offsetof(SpellEntry, RecoveryTime)},
    {"CategoryRecoveryTime", 0, offsetof(SpellEntry, CategoryRecoveryTime)},
    {"InterruptFlags", 0, offsetof(SpellEntry, InterruptFlags)},
    {"AuraInterruptFlags", 0, offsetof(SpellEntry, AuraInterruptFlags)},
    {"ChannelInterruptFlags", 0, offsetof(SpellEntry, ChannelInterruptFlags)},
    {"procFlags", 0, offsetof(SpellEntry, procFlags)},
    {"procChance", 0, offsetof(SpellEntry, procChance)},
    {"procCharges", 0, offsetof(SpellEntry, procCharges)},
    {"maxLevel", 0, offsetof(SpellEntry, maxLevel)},
    {"baseLevel", 0, offsetof(SpellEntry, baseLevel)},
    {"spellLevel", 0, offsetof(SpellEntry, spellLevel)},
    {"DurationIndex", 0, offsetof(SpellEntry, DurationIndex)},
    {"powerType", 0, offsetof(SpellEntry, powerType)},
    {"manaCost", 0, offsetof(SpellEntry, ManaCost)},
    {"manaCostPerlevel", 0, offsetof(SpellEntry, ManaCostPerlevel)},
    {"manaPerSecond", 0, offsetof(SpellEntry, ManaPerSecond)},
    {"rangeIndex", 0, offsetof(SpellEntry, rangeIndex)},
    {"speed", 3, offsetof(SpellEntry, speed)},
    {"maxstack", 0, offsetof(SpellEntry, maxstack)},
    {"Totem", 0, offsetof(SpellEntry, Totem[0])},
    {"Reagent", 0, offsetof(SpellEntry, Reagent[0])},
    {"ReagentCount", 0, offsetof(SpellEntry, ReagentCount[0])},
    {"EquippedItemClass", 0, offsetof(SpellEntry, EquippedItemClass)},
    {"EquippedItemSubClass", 0, offsetof(SpellEntry, EquippedItemSubClass)},
    {"RequiredItemFlags", 0, offsetof(SpellEntry, RequiredItemFlags)},
    {"Effect", 0, offsetof(SpellEntry, Effect[0])},
    {"EffectDieSides", 0, offsetof(SpellEntry, EffectDieSides[0])},
    {"EffectRealPointsPerLevel", 3, offsetof(SpellEntry, EffectRealPointsPerLevel[0])},
    {"EffectBasePoints", 0, offsetof(SpellEntry, EffectBasePoints[0])},
    {"EffectMechanic", 0, offsetof(SpellEntry, EffectMechanic[0])},
    {"EffectImplicitTargetA", 0, offsetof(SpellEntry, EffectImplicitTargetA[0])},
    {"EffectImplicitTargetB", 0, offsetof(SpellEntry, EffectImplicitTargetB[0])},
    {"EffectRadiusIndex", 0, offsetof(SpellEntry, EffectRadiusIndex[0])},
    {"EffectApplyAuraName", 0, offsetof(SpellEntry, EffectApplyAuraName[0])},
    {"EffectAmplitude", 0, offsetof(SpellEntry, EffectAmplitude[0])},
    {"EffectMultipleValue", 3, offsetof(SpellEntry, EffectValueMultiplier[0])},
    {"EffectValueMultiplier", 3, offsetof(SpellEntry, EffectValueMultiplier[0])},
    {"EffectChainTarget", 0, offsetof(SpellEntry, EffectChainTarget[0])},
    {"EffectItemType", 0, offsetof(SpellEntry, EffectItemType[0])},
    {"EffectMiscValue", 0, offsetof(SpellEntry, EffectMiscValue[0])},
    {"EffectMiscValueB", 0, offsetof(SpellEntry, EffectMiscValueB[0])},
    {"EffectTriggerSpell", 0, offsetof(SpellEntry, EffectTriggerSpell[0])},
    {"EffectPointsPerComboPoint", 3, offsetof(SpellEntry, EffectPointsPerComboPoint[0])},
    {"EffectSpellClassMask", 0, offsetof(SpellEntry, EffectSpellClassMask[0][0])},
    {"SpellVisual", 0, offsetof(SpellEntry, SpellVisual[0])},
    {"SpellVisual1", 0, offsetof(SpellEntry, SpellVisual[0])},
    {"SpellVisual2", 0, offsetof(SpellEntry, SpellVisual[1])},
    {"field114", 0, offsetof(SpellEntry, SpellVisual[1])},
    {"spellIconID", 0, offsetof(SpellEntry, SpellIconID)},
    {"activeIconID", 0, offsetof(SpellEntry, ActiveIconID)},
    {"Name", 1, offsetof(SpellEntry, Name)},
    {"Rank", 1, offsetof(SpellEntry, Rank)},
    {"Description", 1, offsetof(SpellEntry, Description)},
    {"BuffDescription", 1, offsetof(SpellEntry, BuffDescription)},
    {"ManaCostPercentage", 0, offsetof(SpellEntry, ManaCostPercentage)},
    {"StartRecoveryCategory", 0, offsetof(SpellEntry, StartRecoveryCategory)},
    {"StartRecoveryTime", 0, offsetof(SpellEntry, StartRecoveryTime)},
    {"MaxTargetLevel", 0, offsetof(SpellEntry, MaxTargetLevel)},
    {"SpellFamilyName", 0, offsetof(SpellEntry, SpellFamilyName)},
    {"SpellGroupType", 0, offsetof(SpellEntry, SpellGroupType[0])},
    {"MaxTargets", 0, offsetof(SpellEntry, MaxTargets)},
    {"Spell_Dmg_Type", 0, offsetof(SpellEntry, Spell_Dmg_Type)},
    {"PreventionType", 0, offsetof(SpellEntry, PreventionType)},
    {"dmg_multiplier", 3, offsetof(SpellEntry, EffectDamageMultiplier[0])},
    {"RequiresAreaId", 0, offsetof(SpellEntry, AreaGroupId)},
    {"School", 0, offsetof(SpellEntry, School)},
    {"RuneCostID", 0, offsetof(SpellEntry, RuneCostID)},
    {"proc_interval", 0, offsetof(SpellEntry, proc_interval)},
    {"c_is_flags", 0, offsetof(SpellEntry, c_is_flags)},
    {"RankNumber", 0, offsetof(SpellEntry, RankNumber)},
    {"NameHash", 0, offsetof(SpellEntry, NameHash)},
    {"talent_tree", 0, offsetof(SpellEntry, talent_tree)},
    {"in_front_status", 0, offsetof(SpellEntry, AllowBackAttack)},
    {"AllowBackAttack", 0, offsetof(SpellEntry, AllowBackAttack)},
    {"ThreatForSpell", 0, offsetof(SpellEntry, ThreatForSpell)},
    {"base_range_or_radius_sqr", 3, offsetof(SpellEntry, base_range_or_radius_sqr)},
    {"cone_width", 3, offsetof(SpellEntry, cone_width)},
    {"SP_coef_override", 3, offsetof(SpellEntry, SP_coef_override)},
    {"self_cast_only", 2, offsetof(SpellEntry, self_cast_only)},
    {"apply_on_shapeshift_change", 2, offsetof(SpellEntry, apply_on_shapeshift_change)},
    {"always_apply", 2, offsetof(SpellEntry, always_apply)},
    {"is_melee_spell", 2, offsetof(SpellEntry, is_melee_spell)},
    {"is_ranged_spell", 2, offsetof(SpellEntry, is_ranged_spell)},
    {NULL, NULL, NULL},
};

LuaSpellEntry GetLuaSpellEntryByName(const char* name)
{
    for (uint32 itr = 0; luaSpellVars[itr].name != NULL; itr++)
    {
        LuaSpellEntry l = luaSpellVars[itr];
        if (strcmp(l.name, name) == 0) //they entered a correct var name
            return l;
    }
    int lElem = sizeof(luaSpellVars) / sizeof(luaSpellVars[0]) - 1;
    return luaSpellVars[lElem];
}

int LuaSpell_GetCaster(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    if (sp->u_caster) //unit caster
    {
        Lunar<Unit>::push(L, sp->u_caster);
        return 1;
    }
    else if (sp->g_caster) //gameobject
    {
        Lunar<GameObject>::push(L, sp->g_caster);
        return 1;
    }
    else if (sp->i_caster) //item
    {
        Lunar<Item>::push(L, sp->i_caster);
        return 1;
    }

    RET_NIL(true);
}

int LuaSpell_GetEntry(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    lua_pushinteger(L, sp->GetSpellProto()->Id);
    return 1;
}

int LuaSpell_IsDuelSpell(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    lua_pushboolean(L, sp->duelSpell ? 1 : 0);
    return 1;
}

int LuaSpell_GetSpellType(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    lua_pushinteger(L, sp->GetType());
    return 1;
}

int LuaSpell_GetSpellState(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    /*
    SPELL_STATE_NULL      = 0,
    SPELL_STATE_PREPARING = 1,
    SPELL_STATE_CASTING   = 2,
    SPELL_STATE_FINISHED  = 3,
    SPELL_STATE_IDLE      = 4
    */
    lua_pushinteger(L, sp->getState());
    return 1;
}

int LuaSpell_Cancel(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(false);

    sp->cancel();
    return 1;
}

int LuaSpell_Cast(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(false);

    bool check = CHECK_BOOL(L, 1);
    sp->cast(check);
    return 1;
}

int LuaSpell_CanCast(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    lua_pushinteger(L, sp->CanCast(false));
    return 1;
}

int LuaSpell_Finish(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(false);

    sp->finish();
    return 1;
}

int LuaSpell_GetTarget(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    if (sp->GetUnitTarget())
    {
        Lunar<Unit>::push(L, sp->GetUnitTarget());
        return 1;
    }
    else if (sp->GetItemTarget())
    {
        Lunar<Item>::push(L,sp->GetItemTarget());
        return 1;
    }
    else if (sp->GetGameObjectTarget())
    {
        Lunar<GameObject>::push(L, sp->GetGameObjectTarget());
        return 1;
    }
    else
        RET_NIL(true);
}

int LuaSpell_IsStealthSpell(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    lua_pushboolean(L, sp->IsStealthSpell() ? 1 : 0);
    return 1;
}

int LuaSpell_IsInvisibilitySpell(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    lua_pushboolean(L, sp->IsInvisibilitySpell() ? 1 : 0);
    return 1;
}

int LuaSpell_GetPossibleEnemy(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    float range = (float)luaL_optnumber(L, 1, 0.0f);
    if (range < 0)
        RET_NIL(true);

    GuidMgr::push(L, sp->GetSinglePossibleEnemy(0, range));
    return 1;
}

int LuaSpell_GetPossibleFriend(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    float range = (float)luaL_optnumber(L, 1, 0.0f);
    if (range < 0)
        RET_NIL(true);

    GuidMgr::push(L, sp->GetSinglePossibleFriend(0, range));
    return 1;
}

int LuaSpell_HasPower(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    lua_pushboolean(L, sp->HasPower() ? 1 : 0);
    return 1;
}

int LuaSpell_IsAspect(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    lua_pushboolean(L, sp->IsAspect() ? 1 : 0);
    return 1;
}

int LuaSpell_IsSeal(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    lua_pushboolean(L, sp->IsSeal() ? 1 : 0);
    return 1;
}

int LuaSpell_GetObjectType(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    lua_pushstring(L, "Spell");
    return 1;
}

int LuaSpell_SetVar(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_BOOL(false);

    const char* var = luaL_checkstring(L,1);
    if (var == NULL)
        RET_BOOL(false);

    int subindex = 0;
    if (lua_gettop(L) == 3)
    {
        subindex = luaL_optint(L,2,0);
    }
    if (subindex < 0)
        RET_BOOL(false);

    int valindex = 2;
    if (subindex)
        valindex++;

//    sp->InitProtoOverride();
    SpellEntry * proto = sp->GetSpellProto();
    if(proto == NULL)
        RET_BOOL(false);

    LuaSpellEntry l = GetLuaSpellEntryByName(var);
    if (!l.name)
        RET_BOOL(false);

    switch (l.typeId) //0: int, 1: char*, 2: bool, 3: float
    {
    case 0:
        GET_SPELLVAR_INT(proto,l.offset,subindex) = luaL_checkinteger(L, valindex);
        lua_pushboolean(L, 1);
        break;
    case 1:
        strcpy(GET_SPELLVAR_CHAR(proto,l.offset,subindex), luaL_checkstring(L, valindex));
        lua_pushboolean(L, 1);
        break;
    case 2:
        GET_SPELLVAR_BOOL(proto,l.offset,subindex) = CHECK_BOOL(L, valindex);
        lua_pushboolean(L, 1);
        break;
    case 3:
        GET_SPELLVAR_FLOAT(proto,l.offset,subindex) = (float)luaL_checknumber(L, valindex);
        lua_pushboolean(L, 1);
        break;
    default:
        RET_BOOL(false);
        break;
    }
    return 1;
}

int LuaSpell_GetVar(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    const char* var = luaL_checkstring(L,1);
    if (var == NULL)
        RET_NIL(true);

    int subindex = luaL_optint(L,2,0);
    if (subindex < 0)
        RET_NIL(true);

    SpellEntry * proto = sp->GetSpellProto();
    if (proto == NULL)
        RET_NIL(true);

    LuaSpellEntry l = GetLuaSpellEntryByName(var);
    if (!l.name)
        RET_NIL(true);

    switch (l.typeId) //0: int, 1: char*, 2: bool, 3: float
    {
    case 0:
        lua_pushinteger(L, GET_SPELLVAR_INT(proto,l.offset,subindex));
        break;
    case 1:
        lua_pushstring(L, GET_SPELLVAR_CHAR(proto,l.offset,subindex));
        break;
    case 2:
        lua_pushboolean(L, (GET_SPELLVAR_BOOL(proto,l.offset,subindex)) ? 1 : 0);
        break;
    case 3:
        lua_pushnumber(L, GET_SPELLVAR_FLOAT(proto,l.offset,subindex));
        break;
    default:
        RET_NIL(false);
        break;
    }
    return 1;
}

int LuaSpell_ResetVar(lua_State * L, Spell * sp)
{
    if (sp == NULL)
        RET_BOOL(false);

    const char* var = luaL_checkstring(L,1);
    if (var == NULL)
        RET_BOOL(false);

    int subindex = luaL_optint(L,2,0);
    if (subindex < 0)
        RET_BOOL(false);

    SpellEntry * proto = sp->GetSpellProto();
    if (proto == NULL)
        RET_NIL(false);

    LuaSpellEntry l = GetLuaSpellEntryByName(var);
    if (!l.name)
        RET_BOOL(false);

    switch (l.typeId) //0: int, 1: char*, 2: bool, 3: float
    {
    case 0:
        GET_SPELLVAR_INT(proto, l.offset,subindex) = GET_SPELLVAR_INT(sp->m_spellInfo,l.offset,subindex);
        lua_pushboolean(L, 1);
        break;
    case 1:
        GET_SPELLVAR_CHAR(proto, l.offset,subindex) = GET_SPELLVAR_CHAR(sp->m_spellInfo,l.offset,subindex);
        lua_pushboolean(L, 1);
        break;
    case 2:
        GET_SPELLVAR_BOOL(proto, l.offset,subindex) = GET_SPELLVAR_BOOL(sp->m_spellInfo,l.offset,subindex);
        lua_pushboolean(L, 1);
        break;
    case 3:
        GET_SPELLVAR_FLOAT(proto, l.offset,subindex) = GET_SPELLVAR_FLOAT(sp->m_spellInfo,l.offset,subindex);
        lua_pushboolean(L, 1);
        break;
    default:
        RET_BOOL(false);
        break;
    }
    return 1;
}

int LuaSpell_ResetAllVars(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(false);

    return 1;
}

int LuaSpell_GetCastedItemId(lua_State * L, Spell * sp)
{
    if(sp == NULL)
        RET_NIL(true);

    lua_pushnumber(L, sp->castedItemId);
    return 1;
}
