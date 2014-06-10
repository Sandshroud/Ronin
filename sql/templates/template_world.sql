SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for achievement_rewards
-- ----------------------------
CREATE TABLE `achievement_rewards` (
  `achievementid` smallint(5) unsigned NOT NULL,
  `title_alliance` smallint(5) unsigned NOT NULL,
  `title_horde` smallint(5) unsigned NOT NULL,
  `itemid` mediumint(8) unsigned NOT NULL,
  `SendMessage` int(3) NOT NULL DEFAULT '0',
  `SenderEntry` int(10) NOT NULL DEFAULT '0',
  `messageheader` longtext CHARACTER SET utf8 COLLATE utf8_unicode_ci,
  `messagebody` longtext CHARACTER SET utf8 COLLATE utf8_unicode_ci,
  PRIMARY KEY (`achievementid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for ai_agents
-- ----------------------------
CREATE TABLE `ai_agents` (
  `CreatureEntry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `SpellEntry` int(10) unsigned NOT NULL DEFAULT '0',
  `TargetType` enum('Self','Current','Destination','SummonOwner','SecondMostHated','Predefined','RandomUnit','RandomUnitDestination','RandomUnitApplyAura','RandomPlayer','RandomPlayerDestination','RandomPlayerApplyAura','ManaClass') NOT NULL DEFAULT 'Self',
  `TargetFilter` enum('None','Closest','Friendly','NotCurrent','Wounded','SecondMostHated','Aggroed','Corpse','InMeleeRange','InRangeOnly','IgnoreSpecialStates','IgnoreLineOfSight','ManaClass','ClosestFriendly','ClosestNotCurrent','WoundedFriendly','FriendlyCorpse','ClosestFriendlyCorpse') NOT NULL DEFAULT 'None',
  `TriggerChance` float unsigned NOT NULL DEFAULT '0',
  `TriggerCooldown` int(8) unsigned NOT NULL DEFAULT '0',
  `AttackStopTimer` int(8) unsigned NOT NULL DEFAULT '0',
  `SpellCastTime` int(8) unsigned NOT NULL DEFAULT '0',
  `SendSoundId` int(8) unsigned NOT NULL DEFAULT '0',
  `MaxProcsPerSequence` int(8) unsigned NOT NULL DEFAULT '0',
  `ProcTimerSequence` int(8) unsigned NOT NULL DEFAULT '0',
  `MinRange` float unsigned NOT NULL DEFAULT '0',
  `MaxRange` float unsigned NOT NULL DEFAULT '0',
  `MinHPPercReqToCast` float NOT NULL DEFAULT '0',
  `data_1` float NOT NULL DEFAULT '0',
  `data_2` float NOT NULL DEFAULT '0',
  `data_3` float NOT NULL DEFAULT '0',
  `DifficultyMask` int(11) NOT NULL DEFAULT '-1',
  PRIMARY KEY (`CreatureEntry`,`SpellEntry`),
  UNIQUE KEY `1PerCtr` (`CreatureEntry`,`SpellEntry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='AI System';

-- ----------------------------
-- Table structure for ai_threattospellid
-- ----------------------------
CREATE TABLE `ai_threattospellid` (
  `spell` int(10) unsigned NOT NULL DEFAULT '0',
  `mod` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`spell`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='AI System';

-- ----------------------------
-- Table structure for areatriggers
-- ----------------------------
CREATE TABLE `areatriggers` (
  `entry` smallint(8) unsigned NOT NULL DEFAULT '0',
  `type` tinyint(3) unsigned DEFAULT '0',
  `requiredteam` smallint(3) NOT NULL DEFAULT '-1',
  `map` smallint(5) unsigned DEFAULT NULL,
  `name` varchar(100) DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `required_honor_rank` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `required_level` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Trigger System';

-- ----------------------------
-- Table structure for auctionhouse
-- ----------------------------
CREATE TABLE `auctionhouse` (
  `creature_entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `group` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`creature_entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Auction House';

-- ----------------------------
-- Table structure for clientaddons
-- ----------------------------
CREATE TABLE `clientaddons` (
  `id` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(50) COLLATE utf8_unicode_ci DEFAULT NULL,
  `crc` bigint(20) unsigned DEFAULT NULL,
  `banned` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `showinlist` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`),
  KEY `index` (`name`)
) ENGINE=MyISAM AUTO_INCREMENT=167 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Client Addons';

-- ----------------------------
-- Table structure for command_overrides
-- ----------------------------
CREATE TABLE `command_overrides` (
  `command_name` varchar(100) NOT NULL,
  `access_level` varchar(10) NOT NULL,
  PRIMARY KEY (`command_name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Commands System';

-- ----------------------------
-- Table structure for creature_formations
-- ----------------------------
CREATE TABLE `creature_formations` (
  `spawn_id` int(30) unsigned NOT NULL DEFAULT '0',
  `target_spawn_id` int(30) unsigned NOT NULL DEFAULT '0',
  `follow_angle` float NOT NULL DEFAULT '0',
  `follow_dist` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`spawn_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Creature System';

-- ----------------------------
-- Table structure for creature_info
-- ----------------------------
CREATE TABLE `creature_info` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `default_emote_state` int(10) unsigned NOT NULL DEFAULT '0',
  `default_spawn_flags` int(10) unsigned NOT NULL DEFAULT '0',
  `default_stand_state` int(10) unsigned NOT NULL DEFAULT '0',
  `default_mount_displayid` int(10) unsigned NOT NULL DEFAULT '0',
  `CanRange` tinyint(3) NOT NULL DEFAULT '0',
  `CanCallForHelp` tinyint(3) NOT NULL DEFAULT '0',
  `CallForHelpHPPercent` float(8,4) NOT NULL DEFAULT '10.0000',
  `CanFlee` tinyint(3) NOT NULL DEFAULT '0',
  `FleeHealthPercent` float(8,4) NOT NULL DEFAULT '10.0000',
  `FleeMSDuration` int(10) NOT NULL DEFAULT '5000',
  `SendFleeMessage` int(10) NOT NULL DEFAULT '0',
  `FleeMessage` varchar(255) NOT NULL DEFAULT '' COMMENT 'DON''T KILL MEH!',
  `GivesNoSkill` tinyint(3) NOT NULL DEFAULT '0',
  `GivesNoXP` tinyint(3) DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for creature_names
-- ----------------------------
CREATE TABLE `creature_names` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `name` varchar(100) NOT NULL DEFAULT '',
  `subname` varchar(100) NOT NULL DEFAULT '',
  `info_str` varchar(500) NOT NULL DEFAULT '',
  `Flags1` int(10) unsigned NOT NULL DEFAULT '0',
  `type` smallint(5) unsigned NOT NULL DEFAULT '0',
  `loot_skill_type` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `family` smallint(5) unsigned NOT NULL DEFAULT '0',
  `rank` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `unk4` int(10) unsigned NOT NULL DEFAULT '0',
  `spelldataid` int(10) unsigned NOT NULL DEFAULT '0',
  `male_displayid` mediumint(8) unsigned NOT NULL,
  `female_displayid` mediumint(8) unsigned NOT NULL,
  `male_displayid2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `female_displayid2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `unknown_float1` float unsigned NOT NULL DEFAULT '0',
  `unknown_float2` float unsigned NOT NULL DEFAULT '0',
  `civilian` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `leader` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Creature System';

-- ----------------------------
-- Table structure for creature_names_localized
-- ----------------------------
CREATE TABLE `creature_names_localized` (
  `id` mediumint(8) unsigned NOT NULL,
  `language_code` varchar(5) CHARACTER SET utf8 NOT NULL,
  `name` varchar(100) CHARACTER SET utf8 NOT NULL,
  `subname` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`,`language_code`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for creature_proto
-- ----------------------------
CREATE TABLE `creature_proto` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `minlevel` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `maxlevel` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `faction` smallint(5) unsigned NOT NULL DEFAULT '0',
  `minhealth` int(10) unsigned NOT NULL DEFAULT '1',
  `maxhealth` int(10) unsigned NOT NULL DEFAULT '1',
  `powertype` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `minpower` int(10) unsigned NOT NULL DEFAULT '0',
  `maxpower` int(10) unsigned NOT NULL DEFAULT '0',
  `scale` float unsigned NOT NULL DEFAULT '1',
  `npcflags` int(10) unsigned NOT NULL DEFAULT '0',
  `attacktime` mediumint(8) unsigned NOT NULL DEFAULT '2500',
  `attacktype` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mindamage` float unsigned NOT NULL DEFAULT '1',
  `maxdamage` float unsigned NOT NULL DEFAULT '2',
  `rangedattacktime` mediumint(8) unsigned NOT NULL DEFAULT '3000',
  `rangedmindamage` float unsigned NOT NULL DEFAULT '1',
  `rangedmaxdamage` float unsigned NOT NULL DEFAULT '2',
  `Item1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Item2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Item3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `respawntime` int(10) unsigned NOT NULL DEFAULT '360000',
  `Armor` int(10) unsigned NOT NULL DEFAULT '0',
  `HolyResistance` int(10) unsigned NOT NULL DEFAULT '0',
  `FireResistance` int(10) unsigned NOT NULL DEFAULT '0',
  `NatureResistance` int(10) unsigned NOT NULL DEFAULT '0',
  `FrostResistance` int(10) unsigned NOT NULL DEFAULT '0',
  `ShadowResistance` int(10) unsigned NOT NULL DEFAULT '0',
  `ArcaneResistance` int(10) unsigned NOT NULL DEFAULT '0',
  `combat_reach` float unsigned NOT NULL DEFAULT '0',
  `bounding_radius` float unsigned NOT NULL DEFAULT '0',
  `auras` varchar(255) NOT NULL DEFAULT '',
  `boss` int(11) unsigned NOT NULL DEFAULT '0',
  `money` int(10) NOT NULL DEFAULT '0',
  `invisibility_type` int(10) unsigned NOT NULL DEFAULT '0',
  `walk_speed` float unsigned NOT NULL DEFAULT '2.5',
  `run_speed` float unsigned NOT NULL DEFAULT '7.7',
  `fly_speed` float unsigned NOT NULL DEFAULT '14',
  `extra_a9_flags` int(10) unsigned NOT NULL DEFAULT '0',
  `auraimmune_flag` int(10) unsigned NOT NULL DEFAULT '0',
  `vehicle_entry` mediumint(8) NOT NULL DEFAULT '0',
  `battlemastertype` int(11) NOT NULL DEFAULT '0',
  `SpellClickId` mediumint(8) NOT NULL DEFAULT '-1',
  `CanMove` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `bonusdamage_holy` int(10) unsigned NOT NULL DEFAULT '0',
  `bonusdamage_fire` int(10) unsigned NOT NULL DEFAULT '0',
  `bonusdamage_nature` int(10) unsigned NOT NULL DEFAULT '0',
  `bonusdamage_frost` int(10) unsigned NOT NULL DEFAULT '0',
  `bonusdamage_shadow` int(10) unsigned NOT NULL DEFAULT '0',
  `bonusdamage_arcane` int(10) unsigned NOT NULL DEFAULT '0',
  `bonushealing` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Creature System';

-- ----------------------------
-- Table structure for creature_proto_mode
-- ----------------------------
CREATE TABLE `creature_proto_mode` (
  `entry` mediumint(10) unsigned NOT NULL,
  `mode` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `minlevel` tinyint(8) unsigned NOT NULL DEFAULT '1',
  `maxlevel` tinyint(8) unsigned NOT NULL DEFAULT '1',
  `minhealth` int(10) unsigned NOT NULL DEFAULT '1',
  `maxhealth` int(10) unsigned NOT NULL DEFAULT '1',
  `mindmg` float unsigned NOT NULL DEFAULT '1',
  `maxdmg` float unsigned NOT NULL DEFAULT '1',
  `power` int(10) unsigned NOT NULL DEFAULT '0',
  `resistance1` int(10) unsigned NOT NULL DEFAULT '0',
  `resistance2` int(10) unsigned NOT NULL DEFAULT '0',
  `resistance3` int(10) unsigned NOT NULL DEFAULT '0',
  `resistance4` int(10) unsigned NOT NULL DEFAULT '0',
  `resistance5` int(10) unsigned NOT NULL DEFAULT '0',
  `resistance6` int(10) unsigned NOT NULL DEFAULT '0',
  `resistance7` int(10) unsigned NOT NULL DEFAULT '0',
  `auras` longtext NOT NULL,
  `auraimmune_flag` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`,`mode`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for creature_proto_vehicle
-- ----------------------------
CREATE TABLE `creature_proto_vehicle` (
  `vehiclecreatureid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `increasehealthbydriver` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `healthforitemlevel` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell5` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `spell6` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `movementflags` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `accessoryseat0` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ejectondeath0` int(10) unsigned NOT NULL DEFAULT '0',
  `unselectable0` int(10) unsigned NOT NULL DEFAULT '0',
  `accessoryseat1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ejectondeath1` int(10) unsigned NOT NULL DEFAULT '0',
  `unselectable1` int(10) unsigned NOT NULL DEFAULT '0',
  `accessoryseat2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ejectondeath2` int(10) unsigned NOT NULL DEFAULT '0',
  `unselectable2` int(10) unsigned NOT NULL DEFAULT '0',
  `accessoryseat3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ejectondeath3` int(10) unsigned NOT NULL DEFAULT '0',
  `unselectable3` int(10) unsigned NOT NULL DEFAULT '0',
  `accessoryseat4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ejectondeath4` int(10) unsigned NOT NULL DEFAULT '0',
  `unselectable4` int(10) unsigned NOT NULL DEFAULT '0',
  `accessoryseat5` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ejectondeath5` int(10) unsigned NOT NULL DEFAULT '0',
  `unselectable5` int(10) unsigned NOT NULL DEFAULT '0',
  `accessoryseat6` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ejectondeath6` int(10) unsigned NOT NULL DEFAULT '0',
  `unselectable6` int(10) unsigned NOT NULL DEFAULT '0',
  `accessoryseat7` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ejectondeath7` int(10) unsigned NOT NULL DEFAULT '0',
  `unselectable7` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`vehiclecreatureid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for creature_quest_finisher
-- ----------------------------
CREATE TABLE `creature_quest_finisher` (
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED COMMENT='Creature System';

-- ----------------------------
-- Table structure for creature_quest_starter
-- ----------------------------
CREATE TABLE `creature_quest_starter` (
  `id` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 ROW_FORMAT=FIXED COMMENT='Creature System';

-- ----------------------------
-- Table structure for creature_spawns
-- ----------------------------
CREATE TABLE `creature_spawns` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `entry` mediumint(8) unsigned NOT NULL,
  `map` smallint(5) unsigned NOT NULL,
  `position_x` float NOT NULL,
  `position_y` float NOT NULL,
  `position_z` float NOT NULL,
  `orientation` float NOT NULL,
  `faction` smallint(5) unsigned NOT NULL DEFAULT '14',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `bytes0` int(10) unsigned NOT NULL DEFAULT '0',
  `bytes1` int(10) unsigned NOT NULL DEFAULT '0',
  `bytes2` int(10) unsigned NOT NULL DEFAULT '0',
  `emote_state` int(10) unsigned NOT NULL DEFAULT '0',
  `death_state` int(10) NOT NULL DEFAULT '0',
  `standstate` int(10) unsigned NOT NULL DEFAULT '0',
  `channel_spell` int(10) unsigned NOT NULL DEFAULT '0',
  `channel_target_sqlid` int(10) unsigned NOT NULL DEFAULT '0',
  `channel_target_sqlid_creature` int(10) unsigned NOT NULL DEFAULT '0',
  `MountedDisplayID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `phase` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `vehicle` int(10) unsigned NOT NULL DEFAULT '0',
  `CanMove` int(8) unsigned NOT NULL DEFAULT '1',
  `vendormask` int(10) NOT NULL DEFAULT '1',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=124582 DEFAULT CHARSET=utf8 COMMENT='Spawn System';

-- ----------------------------
-- Table structure for creature_staticspawns
-- ----------------------------
CREATE TABLE `creature_staticspawns` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `entry` mediumint(8) unsigned NOT NULL,
  `Map` smallint(5) unsigned NOT NULL,
  `x` float NOT NULL,
  `y` float NOT NULL,
  `z` float NOT NULL,
  `orientation` float NOT NULL,
  `movetype` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `displayid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `faction` smallint(5) unsigned NOT NULL DEFAULT '35',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `bytes` int(10) unsigned NOT NULL DEFAULT '0',
  `bytes1` int(10) unsigned NOT NULL,
  `bytes2` int(10) unsigned NOT NULL DEFAULT '0',
  `emote_state` int(10) unsigned NOT NULL DEFAULT '0',
  `channel_spell` int(10) unsigned NOT NULL DEFAULT '0',
  `channel_target_sqlid` int(10) unsigned NOT NULL,
  `channel_target_sqlid_creature` int(10) unsigned NOT NULL,
  `standstate` int(10) unsigned NOT NULL,
  `Phase` tinyint(3) unsigned NOT NULL,
  `vehicle` int(10) unsigned NOT NULL DEFAULT '0',
  `eventid` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Relates to events table',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Spawn System';

-- ----------------------------
-- Table structure for creature_waypoints
-- ----------------------------
CREATE TABLE `creature_waypoints` (
  `spawnid` int(10) unsigned NOT NULL DEFAULT '0',
  `waypointid` mediumint(5) unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `waittime` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `forwardemoteoneshot` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `forwardemoteid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `backwardemoteoneshot` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `backwardemoteid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `forwardskinid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `backwardskinid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `forwardStandState` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `backwardStandState` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `forwardSpellToCast` int(10) unsigned NOT NULL DEFAULT '0',
  `backwardSpellToCast` int(10) unsigned NOT NULL DEFAULT '0',
  `forwardSayText` varchar(256) NOT NULL,
  `backwardSayText` varchar(256) NOT NULL,
  PRIMARY KEY (`spawnid`,`waypointid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Creature System';

-- ----------------------------
-- Table structure for creatureloot
-- ----------------------------
CREATE TABLE `creatureloot` (
  `entryid` mediumint(8) NOT NULL DEFAULT '0' COMMENT 'npc_entry',
  `itemid` mediumint(8) NOT NULL DEFAULT '0' COMMENT 'item_entry',
  `difficulty0percentchance` float NOT NULL DEFAULT '0',
  `difficulty1percentchance` float NOT NULL DEFAULT '0',
  `difficulty2percentchance` float NOT NULL DEFAULT '0',
  `difficulty3percentchance` float NOT NULL DEFAULT '0',
  `mincount` smallint(5) unsigned NOT NULL DEFAULT '1',
  `maxcount` smallint(5) unsigned NOT NULL DEFAULT '1',
  `ffa_loot` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entryid`,`itemid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for creatureloot_gathering
-- ----------------------------
CREATE TABLE `creatureloot_gathering` (
  `entryid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `itemid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `difficulty0percentchance` float unsigned DEFAULT NULL,
  `difficulty1percentchance` float unsigned DEFAULT NULL,
  `difficulty2percentchance` float unsigned NOT NULL DEFAULT '0',
  `difficulty3percentchance` float unsigned NOT NULL DEFAULT '0',
  `mincount` smallint(5) unsigned DEFAULT '1',
  `maxcount` smallint(5) unsigned DEFAULT '1',
  `ffa_loot` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entryid`,`itemid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Loot System';

-- ----------------------------
-- Table structure for fishing
-- ----------------------------
CREATE TABLE `fishing` (
  `Zone` mediumint(10) unsigned NOT NULL DEFAULT '0',
  `MinSkill` smallint(5) unsigned DEFAULT NULL,
  `MaxSkill` smallint(5) unsigned DEFAULT NULL,
  PRIMARY KEY (`Zone`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Fishing System';

-- ----------------------------
-- Table structure for fishingloot
-- ----------------------------
CREATE TABLE `fishingloot` (
  `entryid` smallint(5) unsigned NOT NULL DEFAULT '0',
  `itemid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `percentchance` float NOT NULL DEFAULT '0',
  `mincount` smallint(5) unsigned NOT NULL DEFAULT '1',
  `maxcount` smallint(5) unsigned NOT NULL DEFAULT '1',
  `ffa_loot` mediumint(8) unsigned NOT NULL DEFAULT '0'
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Loot System';

-- ----------------------------
-- Table structure for gameobject_names
-- ----------------------------
CREATE TABLE `gameobject_names` (
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `Type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `DisplayID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Name` varchar(100) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `IconName` varchar(100) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `CastBarText` varchar(100) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `RespawnTimer` int(10) NOT NULL DEFAULT '900000',
  `defaultflags` int(10) NOT NULL DEFAULT '0',
  `listeddata0` int(10) NOT NULL DEFAULT '0',
  `listeddata1` int(10) unsigned NOT NULL DEFAULT '0',
  `listeddata2` int(10) NOT NULL DEFAULT '0',
  `listeddata3` int(10) NOT NULL DEFAULT '0',
  `listeddata4` int(10) NOT NULL DEFAULT '0',
  `listeddata5` int(10) NOT NULL DEFAULT '0',
  `listeddata6` int(10) unsigned NOT NULL DEFAULT '0',
  `listeddata7` int(10) NOT NULL DEFAULT '0',
  `listeddata8` int(10) NOT NULL DEFAULT '0',
  `listeddata9` int(10) NOT NULL DEFAULT '0',
  `listeddata10` int(10) NOT NULL DEFAULT '0',
  `listeddata11` int(10) NOT NULL DEFAULT '0',
  `listeddata12` int(10) NOT NULL DEFAULT '0',
  `listeddata13` int(10) NOT NULL DEFAULT '0',
  `listeddata14` int(10) NOT NULL DEFAULT '0',
  `listeddata15` int(10) NOT NULL DEFAULT '0',
  `listeddata16` int(10) NOT NULL DEFAULT '0',
  `listeddata17` int(10) NOT NULL DEFAULT '0',
  `listeddata18` int(10) NOT NULL DEFAULT '0',
  `listeddata19` int(10) NOT NULL DEFAULT '0',
  `listeddata20` int(10) NOT NULL DEFAULT '0',
  `listeddata21` int(10) NOT NULL DEFAULT '0',
  `listeddata22` int(10) NOT NULL DEFAULT '0',
  `listeddata23` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Gameobject System';

-- ----------------------------
-- Table structure for gameobject_names_localized
-- ----------------------------
CREATE TABLE `gameobject_names_localized` (
  `entry` int(30) NOT NULL,
  `language_code` char(5) COLLATE utf8_unicode_ci NOT NULL,
  `name` varchar(100) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`entry`,`language_code`),
  KEY `lol` (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for gameobject_quest_finisher
-- ----------------------------
CREATE TABLE `gameobject_quest_finisher` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Quest System';

-- ----------------------------
-- Table structure for gameobject_quest_item_binding
-- ----------------------------
CREATE TABLE `gameobject_quest_item_binding` (
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `item_count` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`,`quest`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Quest System';

-- ----------------------------
-- Table structure for gameobject_quest_pickup_binding
-- ----------------------------
CREATE TABLE `gameobject_quest_pickup_binding` (
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `required_count` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Quest System';

-- ----------------------------
-- Table structure for gameobject_quest_starter
-- ----------------------------
CREATE TABLE `gameobject_quest_starter` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Quest System';

-- ----------------------------
-- Table structure for gameobject_spawns
-- ----------------------------
CREATE TABLE `gameobject_spawns` (
  `ID` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `Entry` int(10) unsigned NOT NULL DEFAULT '0',
  `Map` smallint(5) unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `Facing` float NOT NULL DEFAULT '0',
  `State` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `Flags` int(10) unsigned NOT NULL DEFAULT '0',
  `Faction` smallint(5) unsigned NOT NULL DEFAULT '0',
  `Scale` float unsigned NOT NULL DEFAULT '1',
  `phase` tinyint(3) unsigned NOT NULL DEFAULT '1',
  PRIMARY KEY (`ID`)
) ENGINE=MyISAM AUTO_INCREMENT=52657 DEFAULT CHARSET=cp1251 COLLATE=cp1251_general_cs PACK_KEYS=0 ROW_FORMAT=FIXED COMMENT='Spawn System';

-- ----------------------------
-- Table structure for gameobject_staticspawns
-- ----------------------------
CREATE TABLE `gameobject_staticspawns` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `entry` int(10) unsigned NOT NULL,
  `map` smallint(5) unsigned NOT NULL DEFAULT '0',
  `x` float NOT NULL,
  `y` float NOT NULL,
  `z` float NOT NULL,
  `facing` float NOT NULL,
  `o1` float NOT NULL,
  `o2` float NOT NULL,
  `o3` float NOT NULL,
  `o4` float NOT NULL,
  `state` int(10) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `faction` int(10) unsigned NOT NULL DEFAULT '0',
  `scale` float unsigned NOT NULL,
  `ncpRespawnLink` int(10) unsigned NOT NULL DEFAULT '0',
  `phase` tinyint(3) unsigned NOT NULL,
  `vehicle` int(10) unsigned NOT NULL DEFAULT '0',
  `eventid` tinyint(3) unsigned NOT NULL DEFAULT '0' COMMENT 'Relates to events table',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Spawn System';

-- ----------------------------
-- Table structure for graveyards
-- ----------------------------
CREATE TABLE `graveyards` (
  `id` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `zoneid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `adjacentzoneid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `mapid` smallint(5) unsigned NOT NULL DEFAULT '0',
  `faction` smallint(5) unsigned NOT NULL DEFAULT '0',
  `name` varchar(100) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=1721 DEFAULT CHARSET=latin1 COMMENT='Graveyard System';

-- ----------------------------
-- Table structure for item_quest_association
-- ----------------------------
CREATE TABLE `item_quest_association` (
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `quest` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `item_count` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`item`,`quest`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Quest System';

-- ----------------------------
-- Table structure for item_randomprop_groups
-- ----------------------------
CREATE TABLE `item_randomprop_groups` (
  `entry_id` mediumint(8) unsigned NOT NULL,
  `randomprops_entryid` mediumint(8) unsigned NOT NULL,
  `chance` float NOT NULL,
  PRIMARY KEY (`entry_id`,`randomprops_entryid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Item System';

-- ----------------------------
-- Table structure for item_randomsuffix_groups
-- ----------------------------
CREATE TABLE `item_randomsuffix_groups` (
  `entry_id` mediumint(8) unsigned NOT NULL,
  `randomsuffix_entryid` mediumint(8) unsigned NOT NULL,
  `chance` float NOT NULL,
  PRIMARY KEY (`entry_id`,`randomsuffix_entryid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Item System';

-- ----------------------------
-- Table structure for itemloot
-- ----------------------------
CREATE TABLE `itemloot` (
  `entryid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `itemid` mediumint(8) unsigned NOT NULL DEFAULT '25',
  `percentchance` float NOT NULL DEFAULT '0',
  `mincount` smallint(5) unsigned NOT NULL DEFAULT '1',
  `maxcount` smallint(5) unsigned NOT NULL DEFAULT '1',
  `ffa_loot` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entryid`,`itemid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Loot System';

-- ----------------------------
-- Table structure for itempages
-- ----------------------------
CREATE TABLE `itempages` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `text` longtext CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `next_page` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Item System';

-- ----------------------------
-- Table structure for itempages_localized
-- ----------------------------
CREATE TABLE `itempages_localized` (
  `entry` mediumint(8) NOT NULL,
  `language_code` char(3) COLLATE utf8_unicode_ci NOT NULL,
  `text` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`entry`,`language_code`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for itempetfood
-- ----------------------------
CREATE TABLE `itempetfood` (
  `entry` mediumint(8) unsigned NOT NULL,
  `food_type` tinyint(3) unsigned NOT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for items
-- ----------------------------
CREATE TABLE `items` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `class` int(10) NOT NULL DEFAULT '0',
  `subclass` int(10) NOT NULL DEFAULT '0',
  `field4` int(10) NOT NULL DEFAULT '-1',
  `name1` varchar(100) NOT NULL,
  `displayid` mediumint(10) unsigned NOT NULL DEFAULT '0',
  `quality` int(10) NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `faction` int(11) NOT NULL DEFAULT '0',
  `buyprice` int(10) unsigned NOT NULL DEFAULT '0',
  `sellprice` int(10) unsigned NOT NULL DEFAULT '0',
  `inventorytype` int(10) NOT NULL DEFAULT '0',
  `allowableclass` int(10) NOT NULL DEFAULT '0',
  `allowablerace` int(10) NOT NULL DEFAULT '0',
  `itemlevel` int(10) NOT NULL DEFAULT '0',
  `requiredlevel` int(10) NOT NULL DEFAULT '0',
  `RequiredSkill` int(10) NOT NULL DEFAULT '0',
  `RequiredSkillRank` int(10) NOT NULL DEFAULT '0',
  `RequiredSkillSubRank` int(10) NOT NULL DEFAULT '0',
  `RequiredPlayerRank1` int(10) NOT NULL DEFAULT '0',
  `RequiredPlayerRank2` int(10) NOT NULL DEFAULT '0',
  `RequiredFaction` int(10) NOT NULL DEFAULT '0',
  `RequiredFactionStanding` int(10) NOT NULL DEFAULT '0',
  `Unique` int(10) NOT NULL DEFAULT '0',
  `maxcount` int(10) NOT NULL DEFAULT '0',
  `ContainerSlots` int(10) NOT NULL DEFAULT '0',
  `stat_type1` int(10) NOT NULL DEFAULT '0',
  `stat_value1` int(10) NOT NULL DEFAULT '0',
  `stat_type2` int(10) NOT NULL DEFAULT '0',
  `stat_value2` int(10) NOT NULL DEFAULT '0',
  `stat_type3` int(10) NOT NULL DEFAULT '0',
  `stat_value3` int(10) NOT NULL DEFAULT '0',
  `stat_type4` int(10) NOT NULL DEFAULT '0',
  `stat_value4` int(10) NOT NULL DEFAULT '0',
  `stat_type5` int(10) NOT NULL DEFAULT '0',
  `stat_value5` int(10) NOT NULL DEFAULT '0',
  `stat_type6` int(10) NOT NULL DEFAULT '0',
  `stat_value6` int(10) NOT NULL DEFAULT '0',
  `stat_type7` int(10) NOT NULL DEFAULT '0',
  `stat_value7` int(10) NOT NULL DEFAULT '0',
  `stat_type8` int(10) NOT NULL DEFAULT '0',
  `stat_value8` int(10) NOT NULL DEFAULT '0',
  `stat_type9` int(10) NOT NULL DEFAULT '0',
  `stat_value9` int(10) NOT NULL DEFAULT '0',
  `stat_type10` int(10) NOT NULL DEFAULT '0',
  `stat_value10` int(10) NOT NULL DEFAULT '0',
  `ScalingStatId` int(30) NOT NULL DEFAULT '0',
  `ScalingStatFlags` int(30) NOT NULL DEFAULT '0',
  `dmg_min1` float NOT NULL DEFAULT '0',
  `dmg_max1` float NOT NULL DEFAULT '0',
  `dmg_type1` int(10) NOT NULL DEFAULT '0',
  `dmg_min2` float NOT NULL DEFAULT '0',
  `dmg_max2` float NOT NULL DEFAULT '0',
  `dmg_type2` int(10) NOT NULL DEFAULT '0',
  `armor` int(10) NOT NULL DEFAULT '0',
  `holy_res` int(10) NOT NULL DEFAULT '0',
  `fire_res` int(10) NOT NULL DEFAULT '0',
  `nature_res` int(10) NOT NULL DEFAULT '0',
  `frost_res` int(10) NOT NULL DEFAULT '0',
  `shadow_res` int(10) NOT NULL DEFAULT '0',
  `arcane_res` int(10) NOT NULL DEFAULT '0',
  `delay` int(10) NOT NULL DEFAULT '0',
  `ammo_type` int(10) NOT NULL DEFAULT '0',
  `range` float NOT NULL DEFAULT '0',
  `spellid_1` int(10) NOT NULL DEFAULT '0',
  `spelltrigger_1` int(10) NOT NULL DEFAULT '0',
  `spellcharges_1` int(10) NOT NULL DEFAULT '0',
  `spellcooldown_1` int(10) NOT NULL DEFAULT '0',
  `spellcategory_1` int(10) NOT NULL DEFAULT '0',
  `spellcategorycooldown_1` int(10) NOT NULL DEFAULT '0',
  `spellid_2` int(10) NOT NULL DEFAULT '0',
  `spelltrigger_2` int(10) NOT NULL DEFAULT '0',
  `spellcharges_2` int(10) NOT NULL DEFAULT '0',
  `spellcooldown_2` int(10) NOT NULL DEFAULT '0',
  `spellcategory_2` int(10) NOT NULL DEFAULT '0',
  `spellcategorycooldown_2` int(10) NOT NULL DEFAULT '0',
  `spellid_3` int(10) NOT NULL DEFAULT '0',
  `spelltrigger_3` int(10) NOT NULL DEFAULT '0',
  `spellcharges_3` int(10) NOT NULL DEFAULT '0',
  `spellcooldown_3` int(10) NOT NULL DEFAULT '0',
  `spellcategory_3` int(10) NOT NULL DEFAULT '0',
  `spellcategorycooldown_3` int(10) NOT NULL DEFAULT '0',
  `spellid_4` int(10) NOT NULL DEFAULT '0',
  `spelltrigger_4` int(10) NOT NULL DEFAULT '0',
  `spellcharges_4` int(10) NOT NULL DEFAULT '0',
  `spellcooldown_4` int(10) NOT NULL DEFAULT '0',
  `spellcategory_4` int(10) NOT NULL DEFAULT '0',
  `spellcategorycooldown_4` int(10) NOT NULL DEFAULT '0',
  `spellid_5` int(10) NOT NULL DEFAULT '0',
  `spelltrigger_5` int(10) NOT NULL DEFAULT '0',
  `spellcharges_5` int(10) NOT NULL DEFAULT '0',
  `spellcooldown_5` int(10) NOT NULL DEFAULT '0',
  `spellcategory_5` int(10) NOT NULL DEFAULT '0',
  `spellcategorycooldown_5` int(10) NOT NULL DEFAULT '0',
  `bonding` int(10) NOT NULL DEFAULT '0',
  `description` varchar(255) NOT NULL DEFAULT '',
  `page_id` int(10) NOT NULL DEFAULT '0',
  `page_language` int(10) NOT NULL DEFAULT '0',
  `page_material` int(10) NOT NULL DEFAULT '0',
  `quest_id` int(10) NOT NULL DEFAULT '0',
  `lock_id` int(10) NOT NULL DEFAULT '0',
  `lock_material` int(10) NOT NULL DEFAULT '0',
  `sheathID` int(10) NOT NULL DEFAULT '0',
  `randomprop` int(10) NOT NULL DEFAULT '0',
  `RandomSuffixId` int(10) NOT NULL DEFAULT '0',
  `block` int(10) NOT NULL DEFAULT '0',
  `itemset` int(10) NOT NULL DEFAULT '0',
  `MaxDurability` int(10) NOT NULL DEFAULT '0',
  `ZoneNameID` int(10) NOT NULL DEFAULT '0',
  `mapid` int(10) DEFAULT NULL,
  `bagfamily` int(10) NOT NULL DEFAULT '0',
  `TotemCategory` int(10) DEFAULT NULL,
  `socket_color_1` int(10) DEFAULT NULL,
  `unk201_3` int(10) NOT NULL DEFAULT '0',
  `socket_color_2` int(10) DEFAULT NULL,
  `unk201_5` int(10) NOT NULL DEFAULT '0',
  `socket_color_3` int(10) DEFAULT NULL,
  `unk201_7` int(10) NOT NULL DEFAULT '0',
  `socket_bonus` int(10) DEFAULT NULL,
  `GemProperties` int(10) DEFAULT NULL,
  `ReqDisenchantSkill` int(10) NOT NULL DEFAULT '-1',
  `Lootgold` int(10) NOT NULL DEFAULT '0',
  `ArmorDamageModifier` int(10) NOT NULL DEFAULT '0',
  `ItemLimitCategory` int(10) DEFAULT '0',
  `HolidayId` int(10) DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Item System';

-- ----------------------------
-- Table structure for items_localized
-- ----------------------------
CREATE TABLE `items_localized` (
  `entry` mediumint(30) NOT NULL,
  `language_code` char(5) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `name` varchar(255) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `description` varchar(255) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL,
  `last_update` int(10) unsigned NOT NULL DEFAULT '0',
  UNIQUE KEY `entry` (`entry`,`language_code`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for lfd_rewards
-- ----------------------------
CREATE TABLE `lfd_rewards` (
  `dungeonid` int(10) unsigned NOT NULL DEFAULT '0',
  `comments` varchar(255) NOT NULL DEFAULT '',
  `questid1` int(10) unsigned NOT NULL DEFAULT '0',
  `moneyreward1` int(10) NOT NULL DEFAULT '0',
  `xpreward1` int(10) NOT NULL DEFAULT '0',
  `questid2` int(10) NOT NULL DEFAULT '0',
  `moneyreward2` int(10) NOT NULL DEFAULT '0',
  `xpreward2` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`dungeonid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Loot System';

-- ----------------------------
-- Table structure for map_checkpoint
-- ----------------------------
CREATE TABLE `map_checkpoint` (
  `entry` smallint(5) unsigned NOT NULL,
  `prereq_checkpoint_id` smallint(5) unsigned NOT NULL,
  `creature_id` int(10) unsigned NOT NULL,
  `name` varchar(255) NOT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Map System';

-- ----------------------------
-- Table structure for news_announcements
-- ----------------------------
CREATE TABLE `news_announcements` (
  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `faction_mask` smallint(5) unsigned NOT NULL,
  `lastTime` int(10) unsigned NOT NULL,
  `message_text` longtext NOT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for npc_gossip_textid
-- ----------------------------
CREATE TABLE `npc_gossip_textid` (
  `creatureid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `textid` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`creatureid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='NPC System';

-- ----------------------------
-- Table structure for npc_monstersay
-- ----------------------------
CREATE TABLE `npc_monstersay` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `event` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `chance` float NOT NULL DEFAULT '0',
  `language` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `monstername` longtext CHARACTER SET utf8 COLLATE utf8_unicode_ci,
  `text0` longtext CHARACTER SET utf8 COLLATE utf8_unicode_ci,
  `text1` longtext CHARACTER SET utf8 COLLATE utf8_unicode_ci,
  `text2` longtext CHARACTER SET utf8 COLLATE utf8_unicode_ci,
  `text3` longtext CHARACTER SET utf8 COLLATE utf8_unicode_ci,
  `text4` longtext CHARACTER SET utf8 COLLATE utf8_unicode_ci,
  PRIMARY KEY (`entry`,`event`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='AI system';

-- ----------------------------
-- Table structure for npc_text
-- ----------------------------
CREATE TABLE `npc_text` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `text0_0` longtext NOT NULL,
  `text0_1` longtext NOT NULL,
  `text1_0` longtext NOT NULL,
  `text1_1` longtext NOT NULL,
  `text2_0` longtext NOT NULL,
  `text2_1` longtext NOT NULL,
  `text3_0` longtext NOT NULL,
  `text3_1` longtext NOT NULL,
  `text4_0` longtext NOT NULL,
  `text4_1` longtext NOT NULL,
  `text5_0` longtext NOT NULL,
  `text5_1` longtext NOT NULL,
  `text6_0` longtext NOT NULL,
  `text6_1` longtext NOT NULL,
  `text7_0` longtext NOT NULL,
  `text7_1` longtext NOT NULL,
  `prob0` float NOT NULL DEFAULT '0',
  `lang0` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `em0_1_Delay` longtext NOT NULL,
  `em0_1_Id` longtext NOT NULL,
  `em0_2_Delay` longtext NOT NULL,
  `em0_2_Id` longtext NOT NULL,
  `em0_3_Delay` longtext NOT NULL,
  `em0_3_Id` longtext NOT NULL,
  `prob1` float NOT NULL DEFAULT '0',
  `lang1` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `em1_1_Delay` longtext NOT NULL,
  `em1_1_Id` longtext NOT NULL,
  `em1_2_Delay` longtext NOT NULL,
  `em1_2_Id` longtext NOT NULL,
  `em1_3_Delay` longtext NOT NULL,
  `em1_3_Id` longtext NOT NULL,
  `prob2` float NOT NULL DEFAULT '0',
  `lang2` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `em2_1_Delay` longtext NOT NULL,
  `em2_1_Id` longtext NOT NULL,
  `em2_2_Delay` longtext NOT NULL,
  `em2_2_Id` longtext NOT NULL,
  `em2_3_Delay` longtext NOT NULL,
  `em2_3_Id` longtext NOT NULL,
  `prob3` float NOT NULL DEFAULT '0',
  `lang3` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `em3_1_Delay` longtext NOT NULL,
  `em3_1_Id` longtext NOT NULL,
  `em3_2_Delay` longtext NOT NULL,
  `em3_2_Id` longtext NOT NULL,
  `em3_3_Delay` longtext NOT NULL,
  `em3_3_Id` longtext NOT NULL,
  `prob4` float NOT NULL DEFAULT '0',
  `lang4` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `em4_1_Delay` longtext NOT NULL,
  `em4_1_Id` longtext NOT NULL,
  `em4_2_Delay` longtext NOT NULL,
  `em4_2_Id` longtext NOT NULL,
  `em4_3_Delay` longtext NOT NULL,
  `em4_3_Id` longtext NOT NULL,
  `prob5` float NOT NULL DEFAULT '0',
  `lang5` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `em5_1_Delay` longtext NOT NULL,
  `em5_1_Id` longtext NOT NULL,
  `em5_2_Delay` longtext NOT NULL,
  `em5_2_Id` longtext NOT NULL,
  `em5_3_Delay` longtext NOT NULL,
  `em5_3_Id` longtext NOT NULL,
  `prob6` float NOT NULL DEFAULT '0',
  `lang6` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `em6_1_Delay` longtext NOT NULL,
  `em6_1_Id` longtext NOT NULL,
  `em6_2_Delay` longtext NOT NULL,
  `em6_2_Id` longtext NOT NULL,
  `em6_3_Delay` longtext NOT NULL,
  `em6_3_Id` longtext NOT NULL,
  `prob7` float NOT NULL DEFAULT '0',
  `lang7` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `em7_1_Delay` longtext NOT NULL,
  `em7_1_Id` longtext NOT NULL,
  `em7_2_Delay` longtext NOT NULL,
  `em7_2_Id` longtext NOT NULL,
  `em7_3_Delay` longtext NOT NULL,
  `em7_3_Id` longtext NOT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='NPC System';

-- ----------------------------
-- Table structure for npc_text_localized
-- ----------------------------
CREATE TABLE `npc_text_localized` (
  `entry` mediumint(8) NOT NULL,
  `language_code` char(5) COLLATE utf8_unicode_ci NOT NULL,
  `text0` text COLLATE utf8_unicode_ci NOT NULL,
  `text0_1` text COLLATE utf8_unicode_ci NOT NULL,
  `text1` text COLLATE utf8_unicode_ci NOT NULL,
  `text1_1` text COLLATE utf8_unicode_ci NOT NULL,
  `text2` text COLLATE utf8_unicode_ci NOT NULL,
  `text2_1` text COLLATE utf8_unicode_ci NOT NULL,
  `text3` text COLLATE utf8_unicode_ci NOT NULL,
  `text3_1` text COLLATE utf8_unicode_ci NOT NULL,
  `text4` text COLLATE utf8_unicode_ci NOT NULL,
  `text4_1` text COLLATE utf8_unicode_ci NOT NULL,
  `text5` text COLLATE utf8_unicode_ci NOT NULL,
  `text5_1` text COLLATE utf8_unicode_ci NOT NULL,
  `text6` text COLLATE utf8_unicode_ci NOT NULL,
  `text6_1` text COLLATE utf8_unicode_ci NOT NULL,
  `text7` text COLLATE utf8_unicode_ci NOT NULL,
  `text7_1` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`entry`,`language_code`),
  KEY `lol` (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for objectloot
-- ----------------------------
CREATE TABLE `objectloot` (
  `entryid` int(10) unsigned NOT NULL DEFAULT '0',
  `itemid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `difficulty0percentchance` float NOT NULL,
  `difficulty1percentchance` float NOT NULL,
  `difficulty2percentchance` float NOT NULL DEFAULT '0',
  `difficulty3percentchance` float NOT NULL DEFAULT '0',
  `mincount` smallint(5) unsigned NOT NULL DEFAULT '1',
  `maxcount` smallint(5) unsigned NOT NULL DEFAULT '1',
  `ffa_loot` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entryid`,`itemid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Loot System';

-- ----------------------------
-- Table structure for petdefaultspells
-- ----------------------------
CREATE TABLE `petdefaultspells` (
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `spell` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`,`spell`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Pet System';

-- ----------------------------
-- Table structure for pickpocketingloot
-- ----------------------------
CREATE TABLE `pickpocketingloot` (
  `entryid` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `itemid` mediumint(8) unsigned NOT NULL DEFAULT '25',
  `percentchance` float NOT NULL DEFAULT '100',
  `mincount` tinyint(3) DEFAULT '1',
  `maxcount` tinyint(3) DEFAULT '1',
  `ffa_loot` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entryid`,`itemid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Loot System';

-- ----------------------------
-- Table structure for playercreateinfo
-- ----------------------------
CREATE TABLE `playercreateinfo` (
  `Index` tinyint(3) unsigned NOT NULL AUTO_INCREMENT,
  `race` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `factiontemplate` smallint(5) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `mapID` smallint(5) unsigned NOT NULL DEFAULT '0',
  `zoneID` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `positionX` float NOT NULL DEFAULT '0',
  `positionY` float NOT NULL DEFAULT '0',
  `positionZ` float NOT NULL DEFAULT '0',
  `Orientation` float NOT NULL DEFAULT '0',
  `displayID` smallint(5) unsigned NOT NULL DEFAULT '0',
  `BaseStrength` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `BaseAgility` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `BaseStamina` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `BaseIntellect` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `BaseSpirit` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `BaseHealth` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `BaseMana` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `BaseRage` smallint(5) unsigned NOT NULL DEFAULT '0',
  `BaseFocus` smallint(5) unsigned NOT NULL DEFAULT '0',
  `BaseEnergy` smallint(5) unsigned NOT NULL DEFAULT '0',
  `BaseRunic` smallint(5) unsigned NOT NULL DEFAULT '0',
  `attackpower` int(8) unsigned NOT NULL DEFAULT '0',
  `mindmg` float NOT NULL DEFAULT '0',
  `maxdmg` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`Index`,`race`)
) ENGINE=MyISAM AUTO_INCREMENT=66 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci COMMENT='Player System';

-- ----------------------------
-- Table structure for playercreateinfo_bars
-- ----------------------------
CREATE TABLE `playercreateinfo_bars` (
  `race` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `button` int(10) unsigned NOT NULL DEFAULT '0',
  `action` int(10) unsigned DEFAULT NULL,
  `type` int(10) unsigned DEFAULT NULL,
  `misc` int(10) unsigned DEFAULT NULL,
  PRIMARY KEY (`race`,`class`,`button`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Player System';

-- ----------------------------
-- Table structure for playercreateinfo_items
-- ----------------------------
CREATE TABLE `playercreateinfo_items` (
  `indexid` tinyint(3) NOT NULL DEFAULT '0',
  `protoid` mediumint(8) NOT NULL DEFAULT '0',
  `slotid` tinyint(3) NOT NULL DEFAULT '0',
  `amount` smallint(3) NOT NULL DEFAULT '0',
  PRIMARY KEY (`indexid`,`protoid`,`slotid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Player System';

-- ----------------------------
-- Table structure for playercreateinfo_skills
-- ----------------------------
CREATE TABLE `playercreateinfo_skills` (
  `indexid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `skillid` smallint(5) unsigned NOT NULL DEFAULT '0',
  `level` smallint(5) unsigned NOT NULL DEFAULT '0',
  `maxlevel` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`indexid`,`skillid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Player System';

-- ----------------------------
-- Table structure for playercreateinfo_spells
-- ----------------------------
CREATE TABLE `playercreateinfo_spells` (
  `indexid` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `spellid` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`indexid`,`spellid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Player System';

-- ----------------------------
-- Table structure for prestartqueries
-- ----------------------------
CREATE TABLE `prestartqueries` (
  `Query` varchar(1024) NOT NULL,
  `SingleShot` int(10) unsigned NOT NULL DEFAULT '1',
  `Seq` int(10) NOT NULL AUTO_INCREMENT,
  PRIMARY KEY (`Seq`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for professiondiscoveries
-- ----------------------------
CREATE TABLE `professiondiscoveries` (
  `SpellId` int(10) unsigned NOT NULL DEFAULT '0',
  `SpellToDiscover` int(10) unsigned NOT NULL DEFAULT '0',
  `SkillValue` smallint(5) unsigned NOT NULL DEFAULT '0',
  `Chance` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`SpellId`,`SpellToDiscover`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for quest_poi
-- ----------------------------
CREATE TABLE `quest_poi` (
  `questId` int(10) unsigned NOT NULL DEFAULT '0',
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `objIndex` int(10) NOT NULL DEFAULT '0',
  `mapid` int(10) unsigned NOT NULL DEFAULT '0',
  `WorldMapAreaId` int(10) unsigned NOT NULL DEFAULT '0',
  `FloorId` int(10) unsigned NOT NULL DEFAULT '0',
  `unk3` int(10) unsigned NOT NULL DEFAULT '0',
  `unk4` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`questId`,`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for quest_poi_points
-- ----------------------------
CREATE TABLE `quest_poi_points` (
  `questId` int(10) unsigned NOT NULL DEFAULT '0',
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `internalIndex` int(10) NOT NULL AUTO_INCREMENT,
  `x` int(10) NOT NULL DEFAULT '0',
  `y` int(10) NOT NULL DEFAULT '0',
  UNIQUE KEY `Unique` (`questId`,`id`,`internalIndex`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for quests
-- ----------------------------
CREATE TABLE `quests` (
  `entry` mediumint(8) unsigned NOT NULL,
  `Title` text NOT NULL,
  `Details` text NOT NULL,
  `ObjectiveText` text NOT NULL,
  `CompletionText` text NOT NULL,
  `EndText` text NOT NULL,
  `IncompleteText` text NOT NULL,
  `ObjectiveText1` text NOT NULL,
  `ObjectiveText2` text NOT NULL,
  `ObjectiveText3` text NOT NULL,
  `ObjectiveText4` text NOT NULL,
  `ZoneId` smallint(5) NOT NULL DEFAULT '0',
  `Sort` int(10) NOT NULL DEFAULT '0',
  `Type` int(10) NOT NULL DEFAULT '0',
  `Flags` int(10) unsigned NOT NULL DEFAULT '0',
  `MinLevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `Questlevel` tinyint(3) NOT NULL DEFAULT '0',
  `SuggestedPlayers` int(10) unsigned NOT NULL DEFAULT '0',
  `PrevQuestId` mediumint(8) NOT NULL DEFAULT '0',
  `NextQuestId` mediumint(8) NOT NULL DEFAULT '0',
  `Start_phase` int(10) NOT NULL DEFAULT '0',
  `Complete_phase` int(10) NOT NULL DEFAULT '0',
  `IsRepeatable` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Quests System';

-- ----------------------------
-- Table structure for quests_localized
-- ----------------------------
CREATE TABLE `quests_localized` (
  `entry` mediumint(10) NOT NULL,
  `language_code` char(5) COLLATE utf8_unicode_ci NOT NULL,
  `Title` text COLLATE utf8_unicode_ci NOT NULL,
  `Details` text COLLATE utf8_unicode_ci NOT NULL,
  `Objectives` text COLLATE utf8_unicode_ci NOT NULL,
  `CompletionText` text COLLATE utf8_unicode_ci NOT NULL,
  `IncompleteText` text COLLATE utf8_unicode_ci NOT NULL,
  `EndText` text COLLATE utf8_unicode_ci NOT NULL,
  `ObjectiveText1` text COLLATE utf8_unicode_ci NOT NULL,
  `ObjectiveText2` text COLLATE utf8_unicode_ci NOT NULL,
  `ObjectiveText3` text COLLATE utf8_unicode_ci NOT NULL,
  `ObjectiveText4` text COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`entry`,`language_code`),
  KEY `lol` (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for quests_objectives
-- ----------------------------
CREATE TABLE `quests_objectives` (
  `entry` mediumint(8) unsigned NOT NULL,
  `ReqItemId1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqItemId2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqItemId3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqItemId4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ReqItemId5` mediumint(8) NOT NULL DEFAULT '0',
  `ReqItemId6` mediumint(8) NOT NULL DEFAULT '0',
  `ReqItemCount1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqItemCount2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqItemCount3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqItemCount4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqItemCount5` smallint(5) NOT NULL DEFAULT '0',
  `ReqItemCount6` smallint(5) NOT NULL DEFAULT '0',
  `ReqKillMobOrGOId1` mediumint(8) NOT NULL DEFAULT '0',
  `ReqKillMobOrGOId2` mediumint(8) NOT NULL DEFAULT '0',
  `ReqKillMobOrGOId3` mediumint(8) NOT NULL DEFAULT '0',
  `ReqKillMobOrGOId4` mediumint(8) NOT NULL DEFAULT '0',
  `ReqModType1` tinyint(3) NOT NULL DEFAULT '0',
  `ReqModType2` tinyint(3) NOT NULL DEFAULT '0',
  `ReqModType3` tinyint(3) NOT NULL DEFAULT '0',
  `ReqModType4` tinyint(3) NOT NULL DEFAULT '0',
  `ReqKillMobOrGOCount1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqKillMobOrGOCount2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqKillMobOrGOCount3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqKillMobOrGOCount4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReqCastSpellId1` int(10) NOT NULL DEFAULT '0',
  `ReqCastSpellId2` int(10) NOT NULL DEFAULT '0',
  `ReqCastSpellId3` int(10) NOT NULL DEFAULT '0',
  `ReqCastSpellId4` int(10) NOT NULL DEFAULT '0',
  `ExploreTrigger1` int(10) unsigned NOT NULL DEFAULT '0',
  `ExploreTrigger2` int(10) unsigned NOT NULL DEFAULT '0',
  `ExploreTrigger3` int(10) unsigned NOT NULL DEFAULT '0',
  `ExploreTrigger4` int(10) unsigned NOT NULL DEFAULT '0',
  `ReqKillPlayer` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `QuestTimeLimit` int(10) unsigned NOT NULL DEFAULT '0',
  `RequiredMoney` int(10) unsigned NOT NULL DEFAULT '0',
  `ReqPointMapId` int(10) unsigned NOT NULL DEFAULT '0',
  `ReqPointX` float NOT NULL DEFAULT '0',
  `ReqPointY` float NOT NULL DEFAULT '0',
  `ReqPointRadius` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Quests System';

-- ----------------------------
-- Table structure for quests_requirements
-- ----------------------------
CREATE TABLE `quests_requirements` (
  `entry` mediumint(8) unsigned NOT NULL,
  `RequiredTeam` tinyint(3) NOT NULL DEFAULT '-1',
  `RequiredRaces` int(10) NOT NULL DEFAULT '0',
  `RequiredClass` int(10) NOT NULL DEFAULT '0',
  `RequiredTradeskill` int(10) NOT NULL DEFAULT '0',
  `RequiredTradeskillValue` int(10) NOT NULL DEFAULT '0',
  `RequiredRepFaction` int(10) NOT NULL DEFAULT '0',
  `RequiredRepValue` int(10) NOT NULL DEFAULT '0',
  `RequiredQuest1` int(10) NOT NULL DEFAULT '0',
  `RequiredQuest2` int(10) unsigned NOT NULL DEFAULT '0',
  `RequiredQuest3` int(10) unsigned NOT NULL DEFAULT '0',
  `RequiredQuest4` int(10) unsigned NOT NULL DEFAULT '0',
  `RequiredQuest_one_or_all` longtext NOT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Quests System';

-- ----------------------------
-- Table structure for quests_rewards
-- ----------------------------
CREATE TABLE `quests_rewards` (
  `entry` mediumint(8) unsigned NOT NULL,
  `srcItem` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `SrcItemCount` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemId1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemId2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemId3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemId4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemId5` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemId6` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemCount1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemCount2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemCount3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemCount4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemCount5` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewChoiceItemCount6` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewItemId1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewItemId2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewItemId3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewItemId4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `RewItemCount1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewItemCount2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewItemCount3` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewItemCount4` smallint(5) unsigned NOT NULL DEFAULT '0',
  `ReceiveItemId1` int(10) unsigned NOT NULL DEFAULT '0',
  `ReceiveItemId2` int(10) unsigned NOT NULL DEFAULT '0',
  `ReceiveItemId3` int(10) unsigned NOT NULL DEFAULT '0',
  `ReceiveItemId4` int(10) unsigned NOT NULL DEFAULT '0',
  `ReceiveItemCount1` int(10) unsigned NOT NULL DEFAULT '0',
  `ReceiveItemCount2` int(10) unsigned NOT NULL DEFAULT '0',
  `ReceiveItemCount3` int(10) unsigned NOT NULL DEFAULT '0',
  `ReceiveItemCount4` int(10) unsigned NOT NULL DEFAULT '0',
  `RewRepFaction1` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewRepFaction2` smallint(5) unsigned NOT NULL DEFAULT '0',
  `RewRepFaction3` smallint(5) NOT NULL DEFAULT '0',
  `RewRepFaction4` smallint(5) NOT NULL DEFAULT '0',
  `RewRepFaction5` smallint(5) NOT NULL DEFAULT '0',
  `RewRepValue1` smallint(5) NOT NULL DEFAULT '0',
  `RewRepValue2` smallint(5) NOT NULL DEFAULT '0',
  `RewRepValue3` smallint(5) NOT NULL DEFAULT '0',
  `RewRepValue4` smallint(5) NOT NULL DEFAULT '0',
  `RewRepValue5` smallint(5) NOT NULL DEFAULT '0',
  `RewRepLimit1` int(10) unsigned NOT NULL DEFAULT '0',
  `RewRepLimit2` int(10) unsigned NOT NULL DEFAULT '0',
  `RewRepLimit3` int(10) unsigned NOT NULL DEFAULT '0',
  `RewRepLimit4` int(10) unsigned NOT NULL DEFAULT '0',
  `RewRepLimit5` int(10) unsigned NOT NULL DEFAULT '0',
  `RewTitle` int(10) unsigned NOT NULL DEFAULT '0',
  `RewMoney` int(10) NOT NULL DEFAULT '0',
  `RewBonusHonor` int(10) unsigned NOT NULL DEFAULT '0',
  `RewardBonusArenaPoints` int(10) unsigned NOT NULL DEFAULT '0',
  `RewXP` int(10) unsigned NOT NULL DEFAULT '0',
  `RewSpell` int(10) unsigned NOT NULL DEFAULT '0',
  `RewTalent` int(10) unsigned NOT NULL DEFAULT '0',
  `RewCastOnPlayer` int(10) unsigned NOT NULL DEFAULT '0',
  `RewXPId` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Quests System';

-- ----------------------------
-- Table structure for randomcardcreation
-- ----------------------------
CREATE TABLE `randomcardcreation` (
  `SpellId` int(10) unsigned NOT NULL DEFAULT '0',
  `ItemId0` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId1` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId2` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId3` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId4` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId5` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId6` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId7` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId8` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId9` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId10` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId11` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId12` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId13` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId14` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId15` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId16` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId17` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId18` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId19` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId20` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId21` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId22` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId23` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId24` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId25` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId26` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId27` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId28` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId29` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId30` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemId31` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `ItemCount` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`SpellId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for randomitemcreation
-- ----------------------------
CREATE TABLE `randomitemcreation` (
  `SpellId` int(10) unsigned NOT NULL DEFAULT '0',
  `ItemToCreate` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `Skill` int(10) unsigned NOT NULL DEFAULT '0',
  `Chance` float unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ItemToCreate`,`SpellId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for recall
-- ----------------------------
CREATE TABLE `recall` (
  `id` smallint(5) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(100) NOT NULL,
  `MapId` smallint(5) unsigned NOT NULL DEFAULT '0',
  `positionX` float NOT NULL DEFAULT '0',
  `positionY` float NOT NULL DEFAULT '0',
  `positionZ` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM AUTO_INCREMENT=1037 DEFAULT CHARSET=utf8 COMMENT='Tele Command';

-- ----------------------------
-- Table structure for reputation_creature_onkill
-- ----------------------------
CREATE TABLE `reputation_creature_onkill` (
  `creature_id` mediumint(8) NOT NULL,
  `faction_change_alliance` smallint(5) NOT NULL,
  `faction_change_horde` smallint(5) NOT NULL,
  `change_value` smallint(5) NOT NULL,
  `rep_limit` mediumint(8) NOT NULL,
  KEY `index` (`creature_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Creature System';

-- ----------------------------
-- Table structure for reputation_faction_onkill
-- ----------------------------
CREATE TABLE `reputation_faction_onkill` (
  `faction_id` smallint(5) NOT NULL,
  `change_factionid_alliance` smallint(5) NOT NULL,
  `change_deltamin_alliance` smallint(5) NOT NULL,
  `change_deltamax_alliance` smallint(5) NOT NULL,
  `change_factionid_horde` smallint(5) NOT NULL,
  `change_deltamin_horde` smallint(5) NOT NULL,
  `change_deltamax_horde` smallint(5) NOT NULL,
  KEY `factindex` (`faction_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Creature System';

-- ----------------------------
-- Table structure for reputation_instance_onkill
-- ----------------------------
CREATE TABLE `reputation_instance_onkill` (
  `mapid` smallint(5) NOT NULL,
  `mob_rep_reward` smallint(5) NOT NULL,
  `mob_rep_reward_heroic` smallint(5) NOT NULL,
  `mob_rep_limit` mediumint(8) NOT NULL,
  `mob_rep_limit_heroic` mediumint(8) NOT NULL,
  `boss_rep_reward` smallint(5) NOT NULL,
  `boss_rep_reward_heroic` smallint(5) NOT NULL,
  `boss_rep_limit` mediumint(8) NOT NULL,
  `boss_rep_limit_heroic` mediumint(8) NOT NULL,
  `faction_change_alliance` smallint(5) NOT NULL,
  `faction_change_horde` smallint(5) NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Creature System';

-- ----------------------------
-- Table structure for scrollcreation
-- ----------------------------
CREATE TABLE `scrollcreation` (
  `SpellId` int(10) unsigned NOT NULL DEFAULT '0',
  `ItemId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`SpellId`,`ItemId`)
) ENGINE=InnoDB DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for spell_coef_override
-- ----------------------------
CREATE TABLE `spell_coef_override` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `spell_coef_override` float NOT NULL DEFAULT '0',
  `AP_coef_override` float NOT NULL DEFAULT '0',
  `RAP_coef_override` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for spell_disable
-- ----------------------------
CREATE TABLE `spell_disable` (
  `spellid` int(10) unsigned NOT NULL,
  `replacement_spellid` int(10) unsigned NOT NULL,
  PRIMARY KEY (`spellid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Spell System';

-- ----------------------------
-- Table structure for spell_disable_trainers
-- ----------------------------
CREATE TABLE `spell_disable_trainers` (
  `spellid` int(10) unsigned NOT NULL,
  `replacement_spellid` int(10) unsigned NOT NULL,
  PRIMARY KEY (`spellid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Trainer System';

-- ----------------------------
-- Table structure for spell_effects_override
-- ----------------------------
CREATE TABLE `spell_effects_override` (
  `spellId` int(10) unsigned NOT NULL DEFAULT '0',
  `EffectID` int(10) NOT NULL DEFAULT '0',
  `Disable` int(10) NOT NULL DEFAULT '0',
  `Effect` int(10) NOT NULL DEFAULT '0',
  `BasePoints` int(10) NOT NULL DEFAULT '0',
  `ApplyAuraName` int(10) NOT NULL DEFAULT '0',
  `SpellGroupRelation` int(10) NOT NULL DEFAULT '0',
  `MiscValue` int(10) NOT NULL DEFAULT '0',
  `TriggerSpell` int(10) NOT NULL DEFAULT '0',
  `ImplicitTargetA` int(10) NOT NULL DEFAULT '0',
  `ImplicitTargetB` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`spellId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for spell_forced_targets
-- ----------------------------
CREATE TABLE `spell_forced_targets` (
  `spellid` int(10) unsigned NOT NULL,
  `target` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`spellid`,`target`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Spell System';

-- ----------------------------
-- Table structure for spell_proc
-- ----------------------------
CREATE TABLE `spell_proc` (
  `spellID` int(10) unsigned NOT NULL DEFAULT '0',
  `ProcOnNameHash` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`spellID`,`ProcOnNameHash`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for spellfixes
-- ----------------------------
CREATE TABLE `spellfixes` (
  `spellId` int(10) unsigned NOT NULL DEFAULT '0',
  `procFlags` int(10) NOT NULL DEFAULT '0',
  `procChance` int(10) NOT NULL DEFAULT '0',
  `procCharges` int(10) NOT NULL DEFAULT '0',
  `effect0ClassMask0` int(10) unsigned NOT NULL DEFAULT '0',
  `effect0ClassMask1` int(10) unsigned NOT NULL DEFAULT '0',
  `effect0ClassMask2` int(10) unsigned NOT NULL DEFAULT '0',
  `effect1ClassMask0` int(10) unsigned NOT NULL DEFAULT '0',
  `effect1ClassMask1` int(10) unsigned NOT NULL DEFAULT '0',
  `effect1ClassMask2` int(10) unsigned NOT NULL DEFAULT '0',
  `effect2ClassMask0` int(10) unsigned NOT NULL DEFAULT '0',
  `effect2ClassMask1` int(10) unsigned NOT NULL DEFAULT '0',
  `effect2ClassMask2` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`spellId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for spelloverride
-- ----------------------------
CREATE TABLE `spelloverride` (
  `overrideId` int(10) unsigned NOT NULL DEFAULT '0',
  `spellId` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`overrideId`,`spellId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Spell System';

-- ----------------------------
-- Table structure for teleport_coords
-- ----------------------------
CREATE TABLE `teleport_coords` (
  `id` int(10) unsigned NOT NULL DEFAULT '0' COMMENT 'spell id',
  `name` char(255) CHARACTER SET utf8 COLLATE utf8_unicode_ci NOT NULL DEFAULT '''''',
  `mapId` smallint(5) unsigned NOT NULL DEFAULT '0',
  `position_x` float NOT NULL DEFAULT '0',
  `position_y` float NOT NULL DEFAULT '0',
  `position_z` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='World System';

-- ----------------------------
-- Table structure for totemspells
-- ----------------------------
CREATE TABLE `totemspells` (
  `spell` int(10) unsigned NOT NULL DEFAULT '0',
  `castspell1` int(10) unsigned NOT NULL DEFAULT '0',
  `castspell2` int(10) unsigned NOT NULL DEFAULT '0',
  `castspell3` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`spell`,`castspell1`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Spell System';

-- ----------------------------
-- Table structure for trainer_defs
-- ----------------------------
CREATE TABLE `trainer_defs` (
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `required_skill` int(10) unsigned NOT NULL DEFAULT '0',
  `required_skillvalue` smallint(5) unsigned DEFAULT '0',
  `req_class` smallint(10) unsigned NOT NULL DEFAULT '0',
  `trainer_type` tinyint(10) unsigned NOT NULL DEFAULT '0',
  `trainer_ui_window_message` text,
  `can_train_gossip_textid` int(10) unsigned NOT NULL,
  `cannot_train_gossip_textid` int(10) unsigned NOT NULL,
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Trainer System';

-- ----------------------------
-- Table structure for trainer_spells
-- ----------------------------
CREATE TABLE `trainer_spells` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `cast_spell` int(10) unsigned NOT NULL DEFAULT '0',
  `learn_spell` int(10) unsigned NOT NULL,
  `spellcost` int(10) unsigned NOT NULL DEFAULT '0',
  `reqspell` int(10) unsigned NOT NULL DEFAULT '0',
  `reqskill` smallint(5) unsigned NOT NULL DEFAULT '0',
  `reqskillvalue` smallint(5) unsigned NOT NULL DEFAULT '0',
  `reqlevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `deletespell` int(10) unsigned NOT NULL DEFAULT '0',
  `is_prof` tinyint(1) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`,`cast_spell`,`learn_spell`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COMMENT='Trainer System';

-- ----------------------------
-- Table structure for trainerspelloverride
-- ----------------------------
CREATE TABLE `trainerspelloverride` (
  `spellid` int(10) unsigned NOT NULL DEFAULT '0',
  `cost` int(10) unsigned NOT NULL DEFAULT '0',
  `requiredspell` int(10) unsigned NOT NULL DEFAULT '0',
  `deletespell` int(10) unsigned NOT NULL DEFAULT '0',
  `requiredskill` int(10) unsigned NOT NULL DEFAULT '0',
  `requiredskillvalue` int(10) unsigned NOT NULL DEFAULT '0',
  `reqlevel` int(10) unsigned NOT NULL DEFAULT '0',
  `requiredclass` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`spellid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='Trainer System';

-- ----------------------------
-- Table structure for transport_creatures
-- ----------------------------
CREATE TABLE `transport_creatures` (
  `transport_entry` int(10) unsigned NOT NULL,
  `creature_entry` int(10) unsigned NOT NULL,
  `position_x` float NOT NULL,
  `position_y` float NOT NULL,
  `position_z` float NOT NULL,
  `orientation` float NOT NULL
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for vendors
-- ----------------------------
CREATE TABLE `vendors` (
  `entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `listindex` mediumint(8) NOT NULL AUTO_INCREMENT,
  `item` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `amount` smallint(5) NOT NULL DEFAULT '1',
  `max_amount` smallint(5) NOT NULL DEFAULT '0',
  `inctime` bigint(20) NOT NULL DEFAULT '0',
  `extendedcost` int(11) NOT NULL DEFAULT '1',
  `isdependent` int(3) unsigned NOT NULL DEFAULT '1',
  `vendormask` int(10) NOT NULL DEFAULT '1',
  PRIMARY KEY (`entry`,`listindex`,`item`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='NPC System';

-- ----------------------------
-- Table structure for weather
-- ----------------------------
CREATE TABLE `weather` (
  `zoneId` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `high_chance` float unsigned NOT NULL DEFAULT '0',
  `high_type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `med_chance` float unsigned NOT NULL DEFAULT '0',
  `med_type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `low_chance` float unsigned NOT NULL DEFAULT '0',
  `low_type` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`zoneId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 ROW_FORMAT=FIXED COMMENT='Weather System';

-- ----------------------------
-- Table structure for wordfilter_character_names
-- ----------------------------
CREATE TABLE `wordfilter_character_names` (
  `regex_match` varchar(255) NOT NULL,
  `regex_ignore_if_matched` varchar(255) DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for wordfilter_chat
-- ----------------------------
CREATE TABLE `wordfilter_chat` (
  `regex_match` varchar(500) NOT NULL,
  `regex_ignore_if_matched` varchar(500) DEFAULT ''
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for worldmap_info
-- ----------------------------
CREATE TABLE `worldmap_info` (
  `entry` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(100) NOT NULL DEFAULT '',
  `load` int(3) unsigned NOT NULL DEFAULT '1',
  `type` int(10) unsigned NOT NULL DEFAULT '0',
  `maxplayers` int(10) unsigned NOT NULL DEFAULT '0',
  `minlevel` int(10) unsigned NOT NULL DEFAULT '1',
  `linkedareatrigger` int(10) NOT NULL DEFAULT '0',
  `repopx` float NOT NULL DEFAULT '0',
  `repopy` float NOT NULL DEFAULT '0',
  `repopz` float NOT NULL DEFAULT '0',
  `repopentry` int(10) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `cooldown` int(10) unsigned NOT NULL DEFAULT '0',
  `required_quest` int(10) unsigned NOT NULL DEFAULT '0',
  `required_item` int(10) unsigned NOT NULL DEFAULT '0',
  `heroic_keyid_1` int(10) unsigned NOT NULL DEFAULT '0',
  `heroic_keyid_2` int(10) unsigned NOT NULL DEFAULT '0',
  `hordephase` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `alliancephase` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='World System';

-- ----------------------------
-- Table structure for worldstate_template
-- ----------------------------
CREATE TABLE `worldstate_template` (
  `mapid` int(10) NOT NULL,
  `zone_mask` int(10) NOT NULL,
  `faction_mask` int(10) NOT NULL,
  `field_number` int(10) unsigned NOT NULL,
  `initial_value` int(10) NOT NULL,
  `comment` varchar(200) NOT NULL,
  PRIMARY KEY (`field_number`,`mapid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for zoneguards
-- ----------------------------
CREATE TABLE `zoneguards` (
  `zone` smallint(5) unsigned NOT NULL,
  `horde_entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  `alliance_entry` mediumint(8) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`zone`,`horde_entry`,`alliance_entry`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='World System';
