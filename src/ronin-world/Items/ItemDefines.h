/***
 * Demonstrike Core
 */

#pragma once

#define CONTAINER_SLOT_NONE 0xFFFF

enum InventorySlots : uint8
{
    EQUIPMENT_SLOT_START = 0,
    EQUIPMENT_SLOT_HEAD = 0,
    EQUIPMENT_SLOT_NECK,
    EQUIPMENT_SLOT_SHOULDERS,
    EQUIPMENT_SLOT_BODY,
    EQUIPMENT_SLOT_CHEST,
    EQUIPMENT_SLOT_WAIST,
    EQUIPMENT_SLOT_LEGS,
    EQUIPMENT_SLOT_FEET,
    EQUIPMENT_SLOT_WRISTS,
    EQUIPMENT_SLOT_HANDS,
    EQUIPMENT_SLOT_FINGER1,
    EQUIPMENT_SLOT_FINGER2,
    EQUIPMENT_SLOT_TRINKET1,
    EQUIPMENT_SLOT_TRINKET2,
    EQUIPMENT_SLOT_BACK,
    EQUIPMENT_SLOT_MAINHAND,
    EQUIPMENT_SLOT_OFFHAND,
    EQUIPMENT_SLOT_RANGED,
    EQUIPMENT_SLOT_TABARD,
    EQUIPMENT_SLOT_END,

    INVENTORY_SLOT_BAG_START = 19,
    INVENTORY_SLOT_BAG_1 = 19,
    INVENTORY_SLOT_BAG_2,
    INVENTORY_SLOT_BAG_3,
    INVENTORY_SLOT_BAG_4,
    INVENTORY_SLOT_BAG_END,

    INVENTORY_SLOT_ITEM_START = 23,
    INVENTORY_SLOT_ITEM_1 = 23,
    INVENTORY_SLOT_ITEM_2,
    INVENTORY_SLOT_ITEM_3,
    INVENTORY_SLOT_ITEM_4,
    INVENTORY_SLOT_ITEM_5,
    INVENTORY_SLOT_ITEM_6,
    INVENTORY_SLOT_ITEM_7,
    INVENTORY_SLOT_ITEM_8,
    INVENTORY_SLOT_ITEM_9,
    INVENTORY_SLOT_ITEM_10,
    INVENTORY_SLOT_ITEM_11,
    INVENTORY_SLOT_ITEM_12,
    INVENTORY_SLOT_ITEM_13,
    INVENTORY_SLOT_ITEM_14,
    INVENTORY_SLOT_ITEM_15,
    INVENTORY_SLOT_ITEM_16,
    INVENTORY_SLOT_ITEM_END,

    BANK_SLOT_ITEM_START = 39,
    BANK_SLOT_ITEM_1 = 39,
    BANK_SLOT_ITEM_2,
    BANK_SLOT_ITEM_3,
    BANK_SLOT_ITEM_4,
    BANK_SLOT_ITEM_5,
    BANK_SLOT_ITEM_6,
    BANK_SLOT_ITEM_7,
    BANK_SLOT_ITEM_8,
    BANK_SLOT_ITEM_9,
    BANK_SLOT_ITEM_10,
    BANK_SLOT_ITEM_11,
    BANK_SLOT_ITEM_12,
    BANK_SLOT_ITEM_13,
    BANK_SLOT_ITEM_14,
    BANK_SLOT_ITEM_15,
    BANK_SLOT_ITEM_16,
    BANK_SLOT_ITEM_17,
    BANK_SLOT_ITEM_18,
    BANK_SLOT_ITEM_19,
    BANK_SLOT_ITEM_20,
    BANK_SLOT_ITEM_21,
    BANK_SLOT_ITEM_22,
    BANK_SLOT_ITEM_23,
    BANK_SLOT_ITEM_24,
    BANK_SLOT_ITEM_25,
    BANK_SLOT_ITEM_26,
    BANK_SLOT_ITEM_27,
    BANK_SLOT_ITEM_28,
    BANK_SLOT_ITEM_END,

    BANK_SLOT_BAG_START = 67,
    BANK_SLOT_BAG_1 = 67,
    BANK_SLOT_BAG_2,
    BANK_SLOT_BAG_3,
    BANK_SLOT_BAG_4,
    BANK_SLOT_BAG_5,
    BANK_SLOT_BAG_6,
    BANK_SLOT_BAG_7,
    BANK_SLOT_BAG_END,

    BUYBACK_SLOT_START = 74,
    BUYBACK_SLOT_1 = 74,
    BUYBACK_SLOT_2,
    BUYBACK_SLOT_3,
    BUYBACK_SLOT_4,
    BUYBACK_SLOT_5,
    BUYBACK_SLOT_6,
    BUYBACK_SLOT_7,
    BUYBACK_SLOT_8,
    BUYBACK_SLOT_9,
    BUYBACK_SLOT_10,
    BUYBACK_SLOT_11,
    BUYBACK_SLOT_12,
    BUYBACK_SLOT_END,
    INVENTORY_SLOT_MAX = BUYBACK_SLOT_END,

    INVENTORY_SLOT_NONE = 255
};

enum InventoryErrors : uint8
{
    INV_ERR_OK                                           = 0,
    INV_ERR_CANT_EQUIP_LEVEL_N                           = 1,  // You must reach level %d to use that item.
    INV_ERR_CANT_EQUIP_SKILL                             = 2,  // You aren't skilled enough to use that item.
    INV_ERR_WRONG_SLOT                                   = 3,  // That item does not go in that slot.
    INV_ERR_BAG_FULL                                     = 4,  // That bag is full.
    INV_ERR_BAG_IN_BAG                                   = 5,  // Can't put non-empty bags in other bags.
    INV_ERR_TRADE_EQUIPPED_BAG                           = 6,  // You can't trade equipped bags.
    INV_ERR_AMMO_ONLY                                    = 7,  // Only ammo can go there.
    INV_ERR_PROFICIENCY_NEEDED                           = 8,  // You do not have the required proficiency for that item.
    INV_ERR_NO_SLOT_AVAILABLE                            = 9,  // No equipment slot is available for that item.
    INV_ERR_CANT_EQUIP_EVER                              = 10, // You can never use that item.
    INV_ERR_CANT_EQUIP_EVER_2                            = 11, // You can never use that item.
    INV_ERR_NO_SLOT_AVAILABLE_2                          = 12, // No equipment slot is available for that item.
    INV_ERR_2HANDED_EQUIPPED                             = 13, // Cannot equip that with a two-handed weapon.
    INV_ERR_2HSKILLNOTFOUND                              = 14, // You cannot dual-wield
    INV_ERR_WRONG_BAG_TYPE                               = 15, // That item doesn't go in that container.
    INV_ERR_WRONG_BAG_TYPE_2                             = 16, // That item doesn't go in that container.
    INV_ERR_ITEM_MAX_COUNT                               = 17, // You can't carry any more of those items.
    INV_ERR_NO_SLOT_AVAILABLE_3                          = 18, // No equipment slot is available for that item.
    INV_ERR_CANT_STACK                                   = 19, // This item cannot stack.
    INV_ERR_NOT_EQUIPPABLE                               = 20, // This item cannot be equipped.
    INV_ERR_CANT_SWAP                                    = 21, // These items can't be swapped.
    INV_ERR_SLOT_EMPTY                                   = 22, // That slot is empty.
    INV_ERR_ITEM_NOT_FOUND                               = 23, // The item was not found.
    INV_ERR_DROP_BOUND_ITEM                              = 24, // You can't drop a soulbound item.
    INV_ERR_OUT_OF_RANGE                                 = 25, // Out of range.
    INV_ERR_TOO_FEW_TO_SPLIT                             = 26, // Tried to split more than number in stack.
    INV_ERR_SPLIT_FAILED                                 = 27, // Couldn't split those items.
    INV_ERR_SPELL_FAILED_REAGENTS_GENERIC                = 28, // Missing reagent
    INV_ERR_NOT_ENOUGH_MONEY                             = 29, // You don't have enough money.
    INV_ERR_NOT_A_BAG                                    = 30, // Not a bag.
    INV_ERR_DESTROY_NONEMPTY_BAG                         = 31, // You can only do that with empty bags.
    INV_ERR_NOT_OWNER                                    = 32, // You don't own that item.
    INV_ERR_ONLY_ONE_QUIVER                              = 33, // You can only equip one quiver.
    INV_ERR_NO_BANK_SLOT                                 = 34, // You must purchase that bag slot first
    INV_ERR_NO_BANK_HERE                                 = 35, // You are too far away from a bank.
    INV_ERR_ITEM_LOCKED                                  = 36, // Item is locked.
    INV_ERR_GENERIC_STUNNED                              = 37, // You are stunned
    INV_ERR_PLAYER_DEAD                                  = 38, // You can't do that when you're dead.
    INV_ERR_CLIENT_LOCKED_OUT                            = 39, // You can't do that right now.
    INV_ERR_INTERNAL_BAG_ERROR                           = 40, // Internal Bag Error
    INV_ERR_ONLY_ONE_BOLT                                = 41, // You can only equip one quiver.
    INV_ERR_ONLY_ONE_AMMO                                = 42, // You can only equip one ammo pouch.
    INV_ERR_CANT_WRAP_STACKABLE                          = 43, // Stackable items can't be wrapped.
    INV_ERR_CANT_WRAP_EQUIPPED                           = 44, // Equipped items can't be wrapped.
    INV_ERR_CANT_WRAP_WRAPPED                            = 45, // Wrapped items can't be wrapped.
    INV_ERR_CANT_WRAP_BOUND                              = 46, // Bound items can't be wrapped.
    INV_ERR_CANT_WRAP_UNIQUE                             = 47, // Unique items can't be wrapped.
    INV_ERR_CANT_WRAP_BAGS                               = 48, // Bags can't be wrapped.
    INV_ERR_LOOT_GONE                                    = 49, // Already looted
    INV_ERR_INV_FULL                                     = 50, // Inventory is full.
    INV_ERR_BANK_FULL                                    = 51, // Your bank is full
    INV_ERR_VENDOR_SOLD_OUT                              = 52, // That item is currently sold out.
    INV_ERR_BAG_FULL_2                                   = 53, // That bag is full.
    INV_ERR_ITEM_NOT_FOUND_2                             = 54, // The item was not found.
    INV_ERR_CANT_STACK_2                                 = 55, // This item cannot stack.
    INV_ERR_BAG_FULL_3                                   = 56, // That bag is full.
    INV_ERR_VENDOR_SOLD_OUT_2                            = 57, // That item is currently sold out.
    INV_ERR_OBJECT_IS_BUSY                               = 58, // That object is busy.
    INV_ERR_CANT_BE_DISENCHANTED                         = 59,
    INV_ERR_NOT_IN_COMBAT                                = 60, // You can't do that while in combat
    INV_ERR_NOT_WHILE_DISARMED                           = 61, // You can't do that while disarmed
    INV_ERR_BAG_FULL_4                                   = 62, // That bag is full.
    INV_ERR_CANT_EQUIP_RANK                              = 63, // You don't have the required rank for that item
    INV_ERR_CANT_EQUIP_REPUTATION                        = 64, // You don't have the required reputation for that item
    INV_ERR_TOO_MANY_SPECIAL_BAGS                        = 65, // You cannot equip another bag of that type
    INV_ERR_LOOT_CANT_LOOT_THAT_NOW                      = 66, // You can't loot that item now.
    INV_ERR_ITEM_UNIQUE_EQUIPPABLE                       = 67, // You cannot equip more than one of those.
    INV_ERR_VENDOR_MISSING_TURNINS                       = 68, // You do not have the required items for that purchase
    INV_ERR_NOT_ENOUGH_HONOR_POINTS                      = 69, // You don't have enough honor points
    INV_ERR_NOT_ENOUGH_ARENA_POINTS                      = 70, // You don't have enough arena points
    INV_ERR_ITEM_MAX_COUNT_SOCKETED                      = 71, // You have the maximum number of those gems in your inventory or socketed into items.
    INV_ERR_MAIL_BOUND_ITEM                              = 72, // You can't mail soulbound items.
    INV_ERR_INTERNAL_BAG_ERROR_2                         = 73, // Internal Bag Error
    INV_ERR_BAG_FULL_5                                   = 74, // That bag is full.
    INV_ERR_ITEM_MAX_COUNT_EQUIPPED_SOCKETED             = 75, // You have the maximum number of those gems socketed into equipped items.
    INV_ERR_ITEM_UNIQUE_EQUIPPABLE_SOCKETED              = 76, // You cannot socket more than one of those gems into a single item.
    INV_ERR_TOO_MUCH_GOLD                                = 77, // At gold limit
    INV_ERR_NOT_DURING_ARENA_MATCH                       = 78, // You can't do that while in an arena match
    INV_ERR_TRADE_BOUND_ITEM                             = 79, // You can't trade a soulbound item.
    INV_ERR_CANT_EQUIP_RATING                            = 80, // You don't have the personal, team, or battleground rating required to buy that item
    INV_ERR_NO_OUTPUT                                    = 81,
    INV_ERR_NOT_SAME_ACCOUNT                             = 82, // Account-bound items can only be given to your own characters.
    INV_ERR_ITEM_MAX_LIMIT_CATEGORY_COUNT_EXCEEDED_IS    = 84, // You can only carry %d %s
    INV_ERR_ITEM_MAX_LIMIT_CATEGORY_SOCKETED_EXCEEDED_IS = 85, // You can only equip %d |4item:items in the %s category
    INV_ERR_SCALING_STAT_ITEM_LEVEL_EXCEEDED             = 86, // Your level is too high to use that item
    INV_ERR_PURCHASE_LEVEL_TOO_LOW                       = 87, // You must reach level %d to purchase that item.
    INV_ERR_CANT_EQUIP_NEED_TALENT                       = 88, // You do not have the required talent to equip that.
    INV_ERR_ITEM_MAX_LIMIT_CATEGORY_EQUIPPED_EXCEEDED_IS = 89, // You can only equip %d |4item:items in the %s category
    INV_ERR_SHAPESHIFT_FORM_CANNOT_EQUIP                 = 90, // Cannot equip item in this form
    INV_ERR_ITEM_INVENTORY_FULL_SATCHEL                  = 91, // Your inventory is full. Your satchel has been delivered to your mailbox.
    INV_ERR_SCALING_STAT_ITEM_LEVEL_TOO_LOW              = 92, // Your level is too low to use that item
    INV_ERR_CANT_BUY_QUANTITY                            = 93, // You can't buy the specified quantity of that item.
};

enum dbcItemStatType : uint8
{
    ITEM_STAT_POWER                           = 0,
    ITEM_STAT_HEALTH                          = 1,
    ITEM_STAT_AGILITY                         = 3,
    ITEM_STAT_STRENGTH                        = 4,
    ITEM_STAT_INTELLECT                       = 5,
    ITEM_STAT_SPIRIT                          = 6,
    ITEM_STAT_STAMINA                         = 7,
    ITEM_STAT_CUSTOM_DAMAGE_MIN               = 8,
    ITEM_STAT_CUSTOM_DAMAGE_MAX               = 9,
    ITEM_STAT_CUSTOM_WEAPON_DELAY             = 10,
    ITEM_STAT_DEFENSE_RATING                  = 12,
    ITEM_STAT_DODGE_RATING                    = 13,
    ITEM_STAT_PARRY_RATING                    = 14,
    ITEM_STAT_BLOCK_RATING                    = 15,
    ITEM_STAT_MELEE_HIT_RATING                = 16,
    ITEM_STAT_RANGED_HIT_RATING               = 17,
    ITEM_STAT_SPELL_HIT_RATING                = 18,
    ITEM_STAT_MELEE_CRITICAL_STRIKE_RATING    = 19,
    ITEM_STAT_RANGED_CRITICAL_STRIKE_RATING   = 20,
    ITEM_STAT_SPELL_CRITICAL_STRIKE_RATING    = 21,
    ITEM_STAT_MELEE_HIT_AVOIDANCE_RATING      = 22,
    ITEM_STAT_RANGED_HIT_AVOIDANCE_RATING     = 23,
    ITEM_STAT_SPELL_HIT_AVOIDANCE_RATING      = 24,
    ITEM_STAT_MELEE_RESILIENCE                = 25,
    ITEM_STAT_RANGED_RESILIENCE               = 26,
    ITEM_STAT_SPELL_RESILIENCE                = 27,
    ITEM_STAT_MELEE_HASTE_RATING              = 28,
    ITEM_STAT_RANGED_HASTE_RATING             = 29,
    ITEM_STAT_SPELL_HASTE_RATING              = 30,
    ITEM_STAT_HIT_RATING                      = 31,
    ITEM_STAT_CRITICAL_STRIKE_RATING          = 32,
    ITEM_STAT_HIT_REDUCTION_RATING            = 33,
    ITEM_STAT_CRITICAL_REDUCTION_RATING       = 34,
    ITEM_STAT_RESILIENCE_RATING               = 35,
    ITEM_STAT_HASTE_RATING                    = 36,
    ITEM_STAT_EXPERTISE_RATING                = 37,
    ITEM_STAT_ATTACK_POWER                    = 38,
    ITEM_STAT_RANGED_ATTACK_POWER             = 39,
    ITEM_STAT_SPELL_HEALING_DONE              = 41,
    ITEM_STAT_SPELL_DAMAGE_DONE               = 42,
    ITEM_STAT_MANA_REGENERATION               = 43,
    ITEM_STAT_ARMOR_PENETRATION_RATING        = 44,
    ITEM_STAT_SPELL_POWER                     = 45,
    ITEM_STAT_HEALTH_REGEN                    = 46,
    ITEM_STAT_SPELL_PENETRATION               = 47,
    ITEM_STAT_BLOCK_VALUE                     = 48,
    ITEM_STAT_MASTERY_RATING                  = 49,

    ITEM_STAT_PHYSICAL_RESISTANCE             = 50,
    ITEM_STAT_FIRE_RESISTANCE                 = 51,
    ITEM_STAT_FROST_RESISTANCE                = 52,
    ITEM_STAT_HOLY_RESISTANCE                 = 53,
    ITEM_STAT_SHADOW_RESISTANCE               = 54,
    ITEM_STAT_NATURE_RESISTANCE               = 55,
    ITEM_STAT_ARCANE_RESISTANCE               = 56,

    ITEM_STAT_MOD_DAMAGE_PHYSICAL,
    ITEM_STAT_MOD_DAMAGE_FIRE,
    ITEM_STAT_MOD_DAMAGE_FROST,
    ITEM_STAT_MOD_DAMAGE_HOLY,
    ITEM_STAT_MOD_DAMAGE_SHADOW,
    ITEM_STAT_MOD_DAMAGE_NATURE,
    ITEM_STAT_MOD_DAMAGE_ARCANE,
    ITEM_STAT_MAXIMUM
};

enum dbcItemDamageType : uint8
{
    NORMAL_DAMAGE   = 0,
    HOLY_DAMAGE     = 1,
    FIRE_DAMAGE     = 2,
    NATURE_DAMAGE   = 3,
    FROST_DAMAGE    = 4,
    SHADOW_DAMAGE   = 5,
    ARCANE_DAMAGE   = 6,
};

enum dbcItemSpellTriggerType : uint8
{
    USE             = 0,
    ON_EQUIP        = 1,
    CHANCE_ON_HIT   = 2,
    SOULSTONE       = 4,
    LEARNING        = 6,
};

enum dbcItemBondingType : uint8
{
    ITEM_BIND_NONE      =   0,
    ITEM_BIND_ON_PICKUP =   1,
    ITEM_BIND_ON_EQUIP  =   2,
    ITEM_BIND_ON_USE    =   3,
    ITEM_BIND_QUEST     =   4,
    ITEM_BIND_QUEST2    =   5,
};

enum dbcInventoryTypes : uint8
{
    INVTYPE_NON_EQUIP       = 0x00, //0
    INVTYPE_HEAD            = 0x01, //1
    INVTYPE_NECK            = 0x02, //2
    INVTYPE_SHOULDERS       = 0x03, //3
    INVTYPE_BODY            = 0x04, //4
    INVTYPE_CHEST           = 0x05, //5
    INVTYPE_WAIST           = 0x06, //6 
    INVTYPE_LEGS            = 0x07, //7
    INVTYPE_FEET            = 0x08, //8
    INVTYPE_WRISTS          = 0x09, //9
    INVTYPE_HANDS           = 0x0A, //10
    INVTYPE_FINGER          = 0x0B, //11
    INVTYPE_TRINKET         = 0x0C, //12
    INVTYPE_WEAPON          = 0x0D, //13
    INVTYPE_SHIELD          = 0x0E, //14
    INVTYPE_RANGED          = 0x0F, //15
    INVTYPE_CLOAK           = 0x10, //16
    INVTYPE_2HWEAPON        = 0x11, //17
    INVTYPE_BAG             = 0x12, //18
    INVTYPE_TABARD          = 0x13, //19
    INVTYPE_ROBE            = 0x14, //20
    INVTYPE_WEAPONMAINHAND  = 0x15, //21
    INVTYPE_WEAPONOFFHAND   = 0x16, //22
    INVTYPE_HOLDABLE        = 0x17, //23
    INVTYPE_AMMO            = 0x18, //24
    INVTYPE_THROWN          = 0x19, //25
    INVTYPE_RANGEDRIGHT     = 0x1A, //26
    INVTYPE_QUIVER          = 0x1B, //27
    INVTYPE_RELIC           = 0x1C, //28
    NUM_INVENTORY_TYPES
};

enum dbcItemClass : uint8
{
    ITEM_CLASS_CONSUMABLE   = 0,
    ITEM_CLASS_CONTAINER    = 1,
    ITEM_CLASS_WEAPON       = 2,
    ITEM_CLASS_JEWELRY      = 3,
    ITEM_CLASS_ARMOR        = 4,
    ITEM_CLASS_REAGENT      = 5,
    ITEM_CLASS_PROJECTILE   = 6,
    ITEM_CLASS_TRADEGOODS   = 7,
    ITEM_CLASS_GENERIC      = 8,
    ITEM_CLASS_RECIPE       = 9,
    ITEM_CLASS_MONEY        = 10,
    ITEM_CLASS_QUIVER       = 11,
    ITEM_CLASS_QUEST        = 12,
    ITEM_CLASS_KEY          = 13,
    ITEM_CLASS_PERMANENT    = 14,
    ITEM_CLASS_MISCELLANEOUS= 15,
    ITEM_CLASS_GLYPHS       = 16,
};

enum dbcItemSubclass : uint8
{
   // Weapon
   ITEM_SUBCLASS_WEAPON_AXE                 = 0,
   ITEM_SUBCLASS_WEAPON_TWOHAND_AXE         = 1,
   ITEM_SUBCLASS_WEAPON_BOW                 = 2,
   ITEM_SUBCLASS_WEAPON_GUN                 = 3,
   ITEM_SUBCLASS_WEAPON_MACE                = 4,
   ITEM_SUBCLASS_WEAPON_TWOHAND_MACE        = 5,
   ITEM_SUBCLASS_WEAPON_POLEARM             = 6,
   ITEM_SUBCLASS_WEAPON_SWORD               = 7,
   ITEM_SUBCLASS_WEAPON_TWOHAND_SWORD       = 8,
   ITEM_SUBCLASS_WEAPON_STAFF               = 10,
   ITEM_SUBCLASS_WEAPON_EXOTIC              = 11,
   ITEM_SUBCLASS_WEAPON_EXOTIC2             = 12,
   ITEM_SUBCLASS_WEAPON_FIST_WEAPON         = 13,
   ITEM_SUBCLASS_WEAPON_MISC_WEAPON         = 14,
   ITEM_SUBCLASS_WEAPON_DAGGER              = 15,
   ITEM_SUBCLASS_WEAPON_THROWN              = 16,
   ITEM_SUBCLASS_WEAPON_SPEAR               = 17,
   ITEM_SUBCLASS_WEAPON_CROSSBOW            = 18,
   ITEM_SUBCLASS_WEAPON_WAND                = 19,
   ITEM_SUBCLASS_WEAPON_FISHING_POLE        = 20,

   // Armor
   ITEM_SUBCLASS_ARMOR_MISC                 = 0,
   ITEM_SUBCLASS_ARMOR_CLOTH                = 1,
   ITEM_SUBCLASS_ARMOR_LEATHER              = 2,
   ITEM_SUBCLASS_ARMOR_MAIL                 = 3,
   ITEM_SUBCLASS_ARMOR_PLATE_MAIL           = 4,
   ITEM_SUBCLASS_ARMOR_BUCKLER              = 5,
   ITEM_SUBCLASS_ARMOR_SHIELD               = 6,
   ITEM_SUBCLASS_ARMOR_LIBRAM               = 7,
   ITEM_SUBCLASS_ARMOR_IDOL                 = 8,
   ITEM_SUBCLASS_ARMOR_TOTEM                = 9,
   ITEM_SUBCLASS_ARMOR_SIGIL                = 10,

   // Projectile
   ITEM_SUBCLASS_PROJECTILE_WAND            = 0,
   ITEM_SUBCLASS_PROJECTILE_BOLT            = 1,
   ITEM_SUBCLASS_PROJECTILE_ARROW           = 2,
   ITEM_SUBCLASS_PROJECTILE_BULLET          = 3,
   ITEM_SUBCLASS_PROJECTILE_THROWN          = 4,

   // Recipe
   ITEM_SUBCLASS_RECIPE_BOOK                = 0,
   ITEM_SUBCLASS_RECIPE_LEATHERWORKING      = 1,
   ITEM_SUBCLASS_RECIPE_TAILORING           = 2,
   ITEM_SUBCLASS_RECIPE_ENGINEERING         = 3,
   ITEM_SUBCLASS_RECIPE_BLACKSMITHING       = 4,
   ITEM_SUBCLASS_RECIPE_COOKING             = 5,
   ITEM_SUBCLASS_RECIPE_ALCHEMY             = 6,
   ITEM_SUBCLASS_RECIPE_FIRST_AID           = 7,
   ITEM_SUBCLASS_RECIPE_ENCHANTING          = 8,
   ITEM_SUBCLASS_RECIPE_FISHING             = 9,
   ITEM_SUBCLASS_RECIPE_JEWELCRAFTING       = 10,
   ITEM_SUBCLASS_RECIPE_INSCRIPTION         = 11, //Guessed

   // Quiver
   ITEM_SUBCLASS_QUIVER_QUIVER              = 2,
   ITEM_SUBCLASS_QUIVER_AMMO_POUCH          = 3,

   // Containers
   ITEM_SUBCLASS_CONTAINER                  = 0,
   ITEM_SUBCLASS_CONTAINER_SOUL             = 1,
   ITEM_SUBCLASS_CONTAINER_HERB             = 2,
   ITEM_SUBCLASS_CONTAINER_ENCHANTING       = 3,
   ITEM_SUBCLASS_CONTAINER_ENGINEERING      = 4,
   ITEM_SUBCLASS_CONTAINER_GEM              = 5,
   ITEM_SUBCLASS_CONTAINER_MINING           = 6,
   ITEM_SUBCLASS_CONTAINER_LEATHERWORKING   = 7,
   ITEM_SUBCLASS_CONTAINER_INSCRIPTION      = 8,

   // Consumable
   ITEM_SUBCLASS_CONSUMABLE                 = 0,
   ITEM_SUBCLASS_CONSUMABLE_POTION          = 1,
   ITEM_SUBCLASS_CONSUMABLE_ELIXIR          = 2,
   ITEM_SUBCLASS_CONSUMABLE_FLASK           = 3,
   ITEM_SUBCLASS_CONSUMABLE_SCROLL          = 4,
   ITEM_SUBCLASS_CONSUMABLE_FOOD            = 5,
   ITEM_SUBCLASS_CONSUMABLE_ITEM_ENCHANT    = 6,
   ITEM_SUBCLASS_CONSUMABLE_BANDAGE         = 7,
   ITEM_SUBCLASS_CONSUMABLE_OTHER           = 8,

   // Gem
   ITEM_SUBCLASS_GEM_RED                    = 0,
   ITEM_SUBCLASS_GEM_BLUE                   = 1,
   ITEM_SUBCLASS_GEM_YELLOW                 = 2,
   ITEM_SUBCLASS_GEM_PURPLE                 = 3,
   ITEM_SUBCLASS_GEM_GREEN                  = 4,
   ITEM_SUBCLASS_GEM_ORANGE                 = 5,
   ITEM_SUBCLASS_GEM_META                   = 6,
   ITEM_SUBCLASS_GEM_SIMPLE                 = 7,
   ITEM_SUBCLASS_GEM_PRISMATIC              = 8,

   // Trade goods
   ITEM_SUBCLASS_TRADE_GOODS                = 0,
   ITEM_SUBCLASS_PROJECTILE_PARTS           = 1,
   ITEM_SUBCLASS_PROJECTILE_EXPLOSIVES      = 2,
   ITEM_SUBCLASS_PROJECTILE_DEVICES         = 3,
   ITEM_SUBCLASS_JEWELCRAFTING              = 4,
   ITEM_SUBCLASS_CLOTH                      = 5,
   ITEM_SUBCLASS_LEATHER                    = 6,
   ITEM_SUBCLASS_METAL_STONE                = 7,
   ITEM_SUBCLASS_MEAT                       = 8,
   ITEM_SUBCLASS_HERB                       = 9,
   ITEM_SUBCLASS_ELEMENTAL                  = 10,
   ITEM_SUBCLASS_TRADE_GOODS_OTHER          = 11,
   ITEM_SUBCLASS_ENCHANTING                 = 12,
   ITEM_SUBCLASS_MATERIAL                   = 13,
   ITEM_SUBCLASS_ARMOR_ENCHANTMENT          = 14,
   ITEM_SUBCLASS_WEAPON_ENCHANTMENT         = 15,

   //Glyph
   ITEM_SUBCLASS_GLYPH_WARRIOR              = 1,
   ITEM_SUBCLASS_GLYPH_PALADIN              = 2,
   ITEM_SUBCLASS_GLYPH_HUNTER               = 3,
   ITEM_SUBCLASS_GLYPH_ROGUE                = 4,
   ITEM_SUBCLASS_GLYPH_PRIEST               = 5,
   ITEM_SUBCLASS_GLYPH_DEATH_KNIGHT         = 6,
   ITEM_SUBCLASS_GLYPH_SHAMAN               = 7,
   ITEM_SUBCLASS_GLYPH_MAGE                 = 8,
   ITEM_SUBCLASS_GLYPH_WARLOCK              = 9,
   ITEM_SUBCLASS_GLYPH_DRUID                = 11,

   // Key
   ITEM_SUBCLASS_KEY                        = 0,
   ITEM_SUBCLASS_LOCKPICK                   = 1,

   // MISCELLANEOUS
   ITEM_SUBCLASS_MISCELLANEOUS              = 0,
   ITEM_SUBCLASS_MISCELLANEOUS_REAGENT      = 1,
   ITEM_SUBCLASS_MISCELLANEOUS_PET          = 2,
   ITEM_SUBCLASS_MISCELLANEOUS_HOLIDAY      = 3,
   ITEM_SUBCLASS_MISCELLANEOUS_OTHER        = 4,
   ITEM_SUBCLASS_MISCELLANEOUS_MOUNT        = 5,
};

enum dbcItemQuality : uint8
{
    ITEM_QUALITY_POOR_GREY             = 0,
    ITEM_QUALITY_NORMAL_WHITE          = 1,
    ITEM_QUALITY_UNCOMMON_GREEN        = 2,
    ITEM_QUALITY_RARE_BLUE             = 3,
    ITEM_QUALITY_EPIC_PURPLE           = 4,
    ITEM_QUALITY_LEGENDARY_ORANGE      = 5,
    ITEM_QUALITY_ARTIFACT_LIGHT_YELLOW = 6,
    ITEM_QUALITY_DBC_MAX               = 7
};

enum dbcItemFlags : uint32
{
    DBC_ITEMFLAG_SOULBOUND              = 0x00000001,
    DBC_ITEMFLAG_CONJURED               = 0x00000002,
    DBC_ITEMFLAG_OPENABLE               = 0x00000004,
    DBC_ITEMFLAG_HEROIC                 = 0x00000008,
    DBC_ITEMFLAG_DEPRECATED             = 0x00000010,
    DBC_ITEMFLAG_INDESTRUCTABLE         = 0x00000020,
    DBC_ITEMFLAG_NO_EQUIP_COOLDOWN      = 0x00000080,
    DBC_ITEMFLAG_ITEM_WRAPPER           = 0x00000100,
    DBC_ITEMFLAG_PARTY_LOOT             = 0x00000800,
    DBC_ITEMFLAG_REFUNDABLE             = 0x00001000,
    DBC_ITEMFLAG_CHARTER                = 0x00002000,
    DBC_ITEMFLAG_PROSPECTABLE           = 0x00040000,
    DBC_ITEMFLAG_UNIQUE_EQUIPPED        = 0x00080000,
    DBC_ITEMFLAG_USABLE_IN_ARENA        = 0x00200000,
    DBC_ITEMFLAG_THROWABLE_WEAPON       = 0x00400000,
    DBC_ITEMFLAG_USABLE_IN_SHAPESHIFT   = 0x00800000,
    DBC_ITEMFLAG_SMART_LOOT             = 0x02000000,
    DBC_ITEMFLAG_NOT_USABLE_IN_ARENA    = 0x04000000,
    DBC_ITEMFLAG_BIND_TO_ACCOUNT        = 0x08000000,
    DBC_ITEMFLAG_TRIGGERED_CAST         = 0x10000000,
    DBC_ITEMFLAG_MILLABLE               = 0x20000000,
    DBC_ITEMFLAG_BOP_TRADEABLE          = 0x80000000
};

enum dbcItemType : uint32
{
    ITEM_TYPE_BOWAMMO       = 0x0001, // Arrows
    ITEM_TYPE_GUNAMMO       = 0x0002, // Bullets
    ITEM_TYPE_SOULSHARD     = 0x0004, // Soul Shards
    ITEM_TYPE_LEATHERWORK   = 0x0008, // Leatherworking Supplies
    ITEM_TYPE_INSCRIPTION   = 0x0010, // Inscripton Supplies
    ITEM_TYPE_HERBALISM     = 0x0020, // Herbs
    ITEM_TYPE_ENCHANTMENT   = 0x0040, // Enchanting Supplies
    ITEM_TYPE_ENGINEERING   = 0x0080, // Engineering Supplies
    ITEM_TYPE_KEYRING       = 0x0100, // Keys
    ITEM_TYPE_GEMS          = 0x0200, // Gems
    ITEM_TYPE_MINING        = 0x0400, // Mining Supplies
    ITEM_TYPE_SBEQUIPMENT   = 0x0800, // Soulbound Equipment
    ITEM_TYPE_VANITYPETS    = 0x1000, // Vanity Pets
    ITEM_TYPE_CURRENCY      = 0x2000,
    ITEM_TYPE_QUEST_ITEMS   = 0x4000
};

enum dbcSocketGemColor : uint8
{
    META_SOCKET     = 0x01,
    RED_SOCKET      = 0x02,
    YELLOW_SOCKET   = 0x04,
    BLUE_SOCKET     = 0x08
};
