/***
 * Demonstrike Core
 */

#pragma once

#define NUM_MAPS 900
#define CL_BUILD_SUPPORT 13623

class CBattleground;
class CBattlegroundManager;
class AlteracValley;
class ArathiBasin;
class Arena;
class Aura;
class Container;
class Corpse;
class Creature;
class Summon;
class DynamicObject;
class EventableObject;
class EyeOfTheStorm;
class GameObject;
class Item;
class LootRoll;
class MapMgr;
class Object;
class Pet;
class Player;
class QuestMgr;
class Spell;
class Transporter;
class Unit;
class Vehicle;
class WarsongGulch;
class WeatherInfo;
class World;
class StrandOfTheAncients;

#define CharacterDatabase (*Database_Character)
#define WorldDatabase (*Database_World)
#define LogDatabase (*Database_Log)

#define CAST(x, y) static_cast<x*>(y)
#define TO_PLAYER(ptr) ((Player*)ptr)
#define TO_UNIT(ptr) ((Unit*)ptr)
#define TO_CREATURE(ptr) ((Creature*)ptr)
#define TO_SUMMON(ptr) ((Summon*)ptr)
#define TO_PET(ptr) ((Pet*)ptr)
#define TO_CONTAINER(ptr) ((Container*)ptr)
#define TO_ITEM(ptr) ((Item*)ptr)
#define TO_OBJECT(ptr) ((Object*)ptr)
#define TO_GAMEOBJECT(ptr) ((GameObject*)ptr)
#define TO_DYNAMICOBJECT(ptr) ((DynamicObject*)ptr)
#define TO_CORPSE(ptr) ((Corpse*)ptr)
#define TO_EVENTABLEOBJECT(ptr) ((EventableObject*)ptr)
#define TO_CBATTLEGROUND(ptr) ((CBattleground*)ptr)
#define TO_CBATTLEGROUNDMGR(ptr) ((CBattlegroundManager*)ptr)
#define TO_ALTERACVALLEY(ptr) ((AlteracValley*)ptr)
#define TO_EYEOFTHESTORM(ptr) ((EyeOfTheStorm*)ptr)
#define TO_WARSONGGULCH(ptr) ((WarsongGulch*)ptr)
#define TO_SOTA(ptr) ((StrandOfTheAncients*)ptr)
#define TO_ARENA(ptr) ((Arena*)ptr)
#define TO_ARATHIBASIN(ptr) ((ArathiBasin*)ptr)
#define TO_TRANSPORT(ptr) ((Transporter*)ptr)
#define TO_AURA(ptr) ((Aura*)ptr)
#define TO_SPELL(ptr) ((Spell*)ptr)
#define TO_LOOTROLL(ptr) ((LootRoll*)ptr)
#define TO_VEHICLE(ptr) ((Vehicle*)ptr)
#define TO_WEATHER(ptr) ((WeatherInfo*)ptr)
#define TO_TAXI(ptr) ((TaxiPath*)ptr)
#define TO_WORLDSESSION(ptr) ((WorldSession*)ptr)

#define NULLPTR NULL
#define NULLGOB NULL
#define NULLPLR NULL
#define NULLOBJ NULL
#define NULLUNIT NULL
#define NULLPET NULL
#define NULLITEM NULL
#define NULLDYN NULL
#define NULLCREATURE NULL
#define NULLSUMMON NULL
#define NULLVEHICLE NULL
#define NULLCORPSE NULL
#define NULLTRANSPORT NULL
#define NULLEVENTABLEOBJECT NULL
#define NULLCONTAINER NULL
#define NULLBATTLEGROUND NULL
#define NULLWEATHER NULL
#define NULLAURA NULL
#define NULLSPELL NULL
#define NULLROLL NULL
#define NULLBGMGR NULL
#define NULLWORLD NULL
#define NULLMAPMGR NULL
#define NULLINSTANCEMGR NULL

SERVER_DECL extern DirectDatabase* Database_Character;
SERVER_DECL extern DirectDatabase* Database_Account;
SERVER_DECL extern DirectDatabase* Database_World;
SERVER_DECL extern DirectDatabase* Database_Log;

SERVER_DECL bool Rand(float chance);
SERVER_DECL bool Rand(uint32 chance);
SERVER_DECL bool Rand(int32 chance);
