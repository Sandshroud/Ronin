/*
MySQL Data Transfer
Source Host: 10.0.0.0
Source Database: dummy_logon
Target Host: 10.0.0.0
Target Database: dummy_logon
Date: 4/27/2019 11:03:00 PM
*/

SET FOREIGN_KEY_CHECKS=0;
-- ----------------------------
-- Table structure for account_characters
-- ----------------------------
CREATE TABLE `account_characters` (
  `accountId` int(11) unsigned NOT NULL DEFAULT '0',
  `charGuid` int(11) unsigned NOT NULL DEFAULT '0',
  `orderId` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`charGuid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for account_data
-- ----------------------------
CREATE TABLE `account_data` (
  `accountid` int(10) unsigned NOT NULL DEFAULT '0',
  `uiconfig0` blob,
  `uiconfig1` blob,
  `uiconfig2` blob,
  `uiconfig3` blob,
  `uiconfig4` blob,
  `uiconfig5` blob,
  `uiconfig6` blob,
  `uiconfig7` blob,
  `uiconfig8` blob,
  PRIMARY KEY (`accountid`),
  UNIQUE KEY `a` (`accountid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for account_forced_permissions
-- ----------------------------
CREATE TABLE `account_forced_permissions` (
  `login` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  `permissions` varchar(32) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`login`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for account_tutorials
-- ----------------------------
CREATE TABLE `account_tutorials` (
  `acct` int(11) unsigned NOT NULL DEFAULT '0',
  `tut0` int(11) unsigned NOT NULL DEFAULT '0',
  `tut1` int(11) unsigned NOT NULL DEFAULT '0',
  `tut2` int(11) unsigned NOT NULL DEFAULT '0',
  `tut3` int(11) unsigned NOT NULL DEFAULT '0',
  `tut4` int(11) unsigned NOT NULL DEFAULT '0',
  `tut5` int(11) unsigned NOT NULL DEFAULT '0',
  `tut6` int(11) unsigned NOT NULL DEFAULT '0',
  `tut7` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`acct`),
  UNIQUE KEY `a` (`acct`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for arenateams
-- ----------------------------
CREATE TABLE `arenateams` (
  `id` int(10) unsigned NOT NULL DEFAULT '0',
  `type` int(10) unsigned NOT NULL DEFAULT '0',
  `leader` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(24) NOT NULL,
  `emblemstyle` int(10) unsigned NOT NULL DEFAULT '0',
  `emblemcolour` int(10) unsigned NOT NULL DEFAULT '0',
  `borderstyle` int(10) unsigned NOT NULL DEFAULT '0',
  `bordercolor` int(10) unsigned NOT NULL DEFAULT '0',
  `backgroundcolor` int(10) unsigned NOT NULL DEFAULT '0',
  `rating` int(10) unsigned NOT NULL DEFAULT '0',
  `data` varchar(24) NOT NULL,
  `ranking` int(10) unsigned NOT NULL,
  `player_data1` varchar(32) NOT NULL,
  `player_data2` varchar(32) NOT NULL,
  `player_data3` varchar(32) NOT NULL,
  `player_data4` varchar(32) NOT NULL,
  `player_data5` varchar(32) NOT NULL,
  `player_data6` varchar(32) NOT NULL,
  `player_data7` varchar(32) NOT NULL,
  `player_data8` varchar(32) NOT NULL,
  `player_data9` varchar(32) NOT NULL,
  `player_data10` varchar(32) NOT NULL,
  PRIMARY KEY (`id`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for auctions
-- ----------------------------
CREATE TABLE `auctions` (
  `auctionId` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `auctionHouse` int(32) unsigned NOT NULL DEFAULT '1',
  `item` bigint(10) unsigned NOT NULL,
  `owner` bigint(10) unsigned NOT NULL,
  `buyout` int(32) unsigned NOT NULL,
  `time` int(32) unsigned NOT NULL,
  `bidder` bigint(10) unsigned NOT NULL,
  `bid` int(32) NOT NULL,
  `deposit` int(32) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`auctionId`),
  KEY `auctionHouse` (`auctionHouse`)
) ENGINE=MyISAM AUTO_INCREMENT=7 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for banned_characters
-- ----------------------------
CREATE TABLE `banned_characters` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `banTimeExpiration` bigint(20) unsigned NOT NULL DEFAULT '0',
  `banReason` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for banned_names
-- ----------------------------
CREATE TABLE `banned_names` (
  `name` varchar(30) NOT NULL,
  PRIMARY KEY (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for character_achievements
-- ----------------------------
CREATE TABLE `character_achievements` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `achievementId` int(10) unsigned NOT NULL DEFAULT '0',
  `timestamp` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`achievementId`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_actions
-- ----------------------------
CREATE TABLE `character_actions` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `spec` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `actionoffset` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `actionvalue` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spec`,`actionoffset`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_auras
-- ----------------------------
CREATE TABLE `character_auras` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `auraslot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `spellid` int(10) unsigned NOT NULL DEFAULT '0',
  `auraflags` int(10) unsigned NOT NULL DEFAULT '0',
  `auralevel` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `aurastackorcharges` smallint(6) NOT NULL DEFAULT '0',
  `casterguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `durationtimestamp` bigint(20) unsigned NOT NULL DEFAULT '0',
  `modamount1` int(10) NOT NULL DEFAULT '0',
  `modamount2` int(10) NOT NULL DEFAULT '0',
  `modamount3` int(10) NOT NULL DEFAULT '0',
  `modbonus1` int(10) unsigned NOT NULL DEFAULT '0',
  `modbonus2` int(10) unsigned NOT NULL DEFAULT '0',
  `modbonus3` int(10) unsigned NOT NULL DEFAULT '0',
  `modfixed1` int(10) NOT NULL DEFAULT '0',
  `modfixed2` int(10) NOT NULL DEFAULT '0',
  `modfixed3` int(10) NOT NULL DEFAULT '0',
  `modfixedfloat1` float(10,4) NOT NULL DEFAULT '0.0000',
  `modfixedfloat2` float(10,4) NOT NULL DEFAULT '0.0000',
  `modfixedfloat3` float(10,4) NOT NULL DEFAULT '0.0000',
  PRIMARY KEY (`guid`,`auraslot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_bans
-- ----------------------------
CREATE TABLE `character_bans` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `bantime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `banreason` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_cooldowns
-- ----------------------------
CREATE TABLE `character_cooldowns` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `spellid` int(10) unsigned NOT NULL DEFAULT '0',
  `type` smallint(6) unsigned NOT NULL DEFAULT '0',
  `category` int(10) unsigned NOT NULL DEFAULT '0',
  `expiretimestamp` bigint(20) unsigned NOT NULL DEFAULT '0',
  `itemid` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spellid`,`type`,`category`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_criteria_data
-- ----------------------------
CREATE TABLE `character_criteria_data` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `criteriaid` int(10) unsigned NOT NULL DEFAULT '0',
  `criteriacounter` bigint(20) unsigned NOT NULL DEFAULT '0',
  `startDate` bigint(20) unsigned NOT NULL DEFAULT '0',
  `lastDate` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`criteriaid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_currency
-- ----------------------------
CREATE TABLE `character_currency` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `currency` mediumint(6) unsigned NOT NULL DEFAULT '0',
  `amount` int(10) unsigned NOT NULL DEFAULT '0',
  `weekAmount` int(10) unsigned NOT NULL DEFAULT '0',
  `totalAmount` int(10) unsigned NOT NULL DEFAULT '0',
  `flags` mediumint(6) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`currency`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_data
-- ----------------------------
CREATE TABLE `character_data` (
  `guid` int(10) unsigned NOT NULL AUTO_INCREMENT,
  `name` varchar(25) CHARACTER SET latin1 NOT NULL DEFAULT '',
  `race` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `class` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `team` tinyint(3) unsigned NOT NULL DEFAULT '4',
  `appearance` int(10) unsigned NOT NULL DEFAULT '0',
  `appearance2` int(10) unsigned NOT NULL DEFAULT '0',
  `appearance3` int(10) unsigned NOT NULL DEFAULT '0',
  `customizeFlags` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `deathState` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `mapId` smallint(5) unsigned NOT NULL DEFAULT '0',
  `instanceId` int(10) unsigned NOT NULL DEFAULT '0',
  `positionX` float NOT NULL DEFAULT '0',
  `positionY` float NOT NULL DEFAULT '0',
  `positionZ` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `zoneId` smallint(5) unsigned NOT NULL DEFAULT '0',
  `load_data` int(10) unsigned NOT NULL DEFAULT '655370000',
  `load_data2` int(10) unsigned NOT NULL DEFAULT '0',
  `playerBytes` int(10) unsigned NOT NULL DEFAULT '0',
  `playerBytes2` int(10) unsigned NOT NULL DEFAULT '0',
  `experience` int(10) unsigned NOT NULL DEFAULT '0',
  `gold` bigint(20) unsigned NOT NULL DEFAULT '0',
  `availableProfPoints` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `selectedTitle` int(10) unsigned NOT NULL DEFAULT '0',
  `watchedFaction` int(11) unsigned NOT NULL DEFAULT '0',
  `talentActivespec` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `talentSpecCount` tinyint(3) unsigned NOT NULL DEFAULT '1',
  `talentResetCounter` int(10) unsigned NOT NULL DEFAULT '0',
  `talentBonusPoints` int(10) NOT NULL DEFAULT '0',
  `talentStack` int(10) unsigned NOT NULL DEFAULT '0',
  `bindmapId` smallint(5) unsigned NOT NULL DEFAULT '0',
  `bindpositionX` float NOT NULL DEFAULT '0',
  `bindpositionY` float NOT NULL DEFAULT '0',
  `bindpositionZ` float NOT NULL DEFAULT '0',
  `bindzoneId` smallint(5) unsigned NOT NULL DEFAULT '0',
  `taxiPath` int(10) unsigned NOT NULL DEFAULT '0',
  `taxiMoveTime` int(10) unsigned NOT NULL DEFAULT '0',
  `taxiTravelTime` int(10) unsigned NOT NULL DEFAULT '0',
  `taxiMountId` int(10) unsigned NOT NULL DEFAULT '0',
  `transportGuid` int(10) unsigned NOT NULL DEFAULT '0',
  `transportX` float NOT NULL DEFAULT '0',
  `transportY` float NOT NULL DEFAULT '0',
  `transportZ` float NOT NULL DEFAULT '0',
  `entryPointMapId` int(10) unsigned NOT NULL DEFAULT '0',
  `entryPointX` float NOT NULL DEFAULT '0',
  `entryPointY` float NOT NULL DEFAULT '0',
  `entryPointZ` float NOT NULL DEFAULT '0',
  `entryOrientation` float NOT NULL DEFAULT '0',
  `instanceDifficulty` int(10) unsigned NOT NULL DEFAULT '0',
  `raidDifficulty` int(10) unsigned NOT NULL DEFAULT '0',
  `isResting` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `restState` tinyint(1) unsigned NOT NULL DEFAULT '0',
  `restTime` int(10) unsigned NOT NULL DEFAULT '0',
  `restAreaTrigger` int(10) unsigned NOT NULL DEFAULT '0',
  `playedTimeLevel` int(10) unsigned NOT NULL DEFAULT '0',
  `playedTimeTotal` int(10) unsigned NOT NULL DEFAULT '0',
  `lastWeekResetTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `lastSaveTime` bigint(20) unsigned NOT NULL DEFAULT '0',
  `needPositionReset` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `needTalentReset` tinyint(3) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=MyISAM AUTO_INCREMENT=259 DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_equipmentsets
-- ----------------------------
CREATE TABLE `character_equipmentsets` (
  `setguid` bigint(20) NOT NULL AUTO_INCREMENT,
  `ownerguid` int(10) unsigned NOT NULL DEFAULT '0',
  `name` varchar(35) NOT NULL DEFAULT '',
  `iconname` varchar(35) NOT NULL DEFAULT '',
  `item0` int(11) unsigned NOT NULL DEFAULT '0',
  `item1` int(11) unsigned NOT NULL DEFAULT '0',
  `item2` int(11) unsigned NOT NULL DEFAULT '0',
  `item3` int(11) unsigned NOT NULL DEFAULT '0',
  `item4` int(11) unsigned NOT NULL DEFAULT '0',
  `item5` int(11) unsigned NOT NULL DEFAULT '0',
  `item6` int(11) unsigned NOT NULL DEFAULT '0',
  `item7` int(11) unsigned NOT NULL DEFAULT '0',
  `item8` int(11) unsigned NOT NULL DEFAULT '0',
  `item9` int(11) unsigned NOT NULL DEFAULT '0',
  `item10` int(11) unsigned NOT NULL DEFAULT '0',
  `item11` int(11) unsigned NOT NULL DEFAULT '0',
  `item12` int(11) unsigned NOT NULL DEFAULT '0',
  `item13` int(11) unsigned NOT NULL DEFAULT '0',
  `item14` int(11) unsigned NOT NULL DEFAULT '0',
  `item15` int(11) unsigned NOT NULL DEFAULT '0',
  `item16` int(11) unsigned NOT NULL DEFAULT '0',
  `item17` int(11) unsigned NOT NULL DEFAULT '0',
  `item18` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`setguid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_exploration
-- ----------------------------
CREATE TABLE `character_exploration` (
  `guid` int(11) unsigned NOT NULL DEFAULT '0',
  `exploration_offset` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `exploration_flag` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`exploration_offset`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_glyphs
-- ----------------------------
CREATE TABLE `character_glyphs` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `spec` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `index` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `glyph` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spec`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_inventory
-- ----------------------------
CREATE TABLE `character_inventory` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `itemguid` int(10) unsigned NOT NULL DEFAULT '0',
  `container` int(10) NOT NULL DEFAULT '-1',
  `slot` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`itemguid`),
  UNIQUE KEY `Unique` (`guid`,`itemguid`),
  UNIQUE KEY `Unique2` (`guid`,`container`,`slot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_known_titles
-- ----------------------------
CREATE TABLE `character_known_titles` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `title_offset` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `title_flag` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`title_offset`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_powers
-- ----------------------------
CREATE TABLE `character_powers` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `powertype` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `powervalue` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`powertype`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_questlog
-- ----------------------------
CREATE TABLE `character_questlog` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `slot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `questid` int(10) unsigned NOT NULL DEFAULT '0',
  `expiretimestamp` bigint(20) unsigned NOT NULL DEFAULT '0',
  `objectivecount1` smallint(6) unsigned NOT NULL DEFAULT '0',
  `objectivecount2` smallint(6) unsigned NOT NULL DEFAULT '0',
  `objectivecount3` smallint(6) unsigned NOT NULL DEFAULT '0',
  `objectivecount4` smallint(6) unsigned NOT NULL DEFAULT '0',
  `explorationflags` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `playerslaincounter` smallint(6) unsigned NOT NULL DEFAULT '0',
  UNIQUE KEY `Slot` (`guid`,`slot`),
  UNIQUE KEY `QuestId` (`guid`,`questid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_quests_completed_repeatable
-- ----------------------------
CREATE TABLE `character_quests_completed_repeatable` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `questid` int(10) unsigned NOT NULL DEFAULT '0',
  `completedtimestamp` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`questid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_quests_completion_masks
-- ----------------------------
CREATE TABLE `character_quests_completion_masks` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `index` mediumint(6) unsigned NOT NULL DEFAULT '0',
  `mask` bigint(20) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`index`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_reputation
-- ----------------------------
CREATE TABLE `character_reputation` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `reputation` smallint(6) unsigned NOT NULL DEFAULT '0',
  `repflag` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `repbasestanding` int(8) NOT NULL DEFAULT '0',
  `repstanding` int(8) NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`reputation`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_skills
-- ----------------------------
CREATE TABLE `character_skills` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `skillid` smallint(5) unsigned NOT NULL DEFAULT '0',
  `skillpos` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `step` smallint(5) unsigned NOT NULL DEFAULT '0',
  `currentlevel` smallint(5) unsigned NOT NULL DEFAULT '0',
  `maxlevel` smallint(5) unsigned NOT NULL DEFAULT '0',
  UNIQUE KEY `Skills` (`guid`,`skillid`),
  UNIQUE KEY `SkillPositions` (`guid`,`skillpos`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_social
-- ----------------------------
CREATE TABLE `character_social` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `socialtype` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `socialguid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `socialnote` varchar(100) NOT NULL DEFAULT '',
  PRIMARY KEY (`guid`,`socialtype`,`socialguid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_spells
-- ----------------------------
CREATE TABLE `character_spells` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `spellid` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spellid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_talents
-- ----------------------------
CREATE TABLE `character_talents` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `spec` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `talentid` int(10) unsigned NOT NULL DEFAULT '0',
  `rank` smallint(5) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`spec`,`talentid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_taximasks
-- ----------------------------
CREATE TABLE `character_taximasks` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `taxi_offset` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `taxi_flag` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`,`taxi_offset`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for character_timestamps
-- ----------------------------
CREATE TABLE `character_timestamps` (
  `guid` int(10) unsigned NOT NULL DEFAULT '0',
  `logintimestamp` bigint(20) unsigned NOT NULL DEFAULT '0',
  `creationtimestamp` bigint(20) unsigned NOT NULL DEFAULT '0',
  `playedtime` int(10) unsigned NOT NULL DEFAULT '0',
  `playedtimethislevel` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for characters_insert_queue
-- ----------------------------
CREATE TABLE `characters_insert_queue` (
  `guid` int(10) unsigned NOT NULL,
  `acct` int(10) unsigned NOT NULL,
  `name` varchar(21) NOT NULL,
  `race` tinyint(3) unsigned NOT NULL,
  `class` tinyint(3) unsigned NOT NULL,
  `gender` tinyint(3) unsigned NOT NULL,
  `custom_faction` int(10) unsigned NOT NULL DEFAULT '0',
  `level` tinyint(3) unsigned NOT NULL,
  `xp` int(10) unsigned NOT NULL,
  `exploration_data` longtext NOT NULL,
  `skills` longtext NOT NULL,
  `watched_faction_index` int(10) unsigned NOT NULL DEFAULT '0',
  `selected_pvp_title` int(10) unsigned NOT NULL DEFAULT '0',
  `available_pvp_titles` int(10) unsigned NOT NULL DEFAULT '0',
  `gold` int(10) unsigned NOT NULL DEFAULT '0',
  `ammo_id` int(10) unsigned NOT NULL DEFAULT '0',
  `available_prof_points` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `available_talent_points` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `current_hp` int(10) unsigned NOT NULL DEFAULT '0',
  `current_power` int(10) unsigned NOT NULL DEFAULT '0',
  `pvprank` int(10) unsigned NOT NULL DEFAULT '0',
  `bytes` int(10) unsigned NOT NULL DEFAULT '0',
  `bytes2` int(10) unsigned NOT NULL DEFAULT '0',
  `player_flags` int(10) unsigned NOT NULL DEFAULT '0',
  `player_bytes` int(10) unsigned NOT NULL DEFAULT '0',
  `positionX` float NOT NULL DEFAULT '0',
  `positionY` float NOT NULL DEFAULT '0',
  `positionZ` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `mapId` int(10) unsigned NOT NULL DEFAULT '0',
  `zoneId` int(10) unsigned NOT NULL DEFAULT '0',
  `taximask` longtext NOT NULL,
  `banned` int(40) NOT NULL,
  `banReason` varchar(50) NOT NULL,
  `timestamp` int(10) unsigned NOT NULL DEFAULT '0',
  `online` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `bindpositionX` float NOT NULL DEFAULT '0',
  `bindpositionY` float NOT NULL DEFAULT '0',
  `bindpositionZ` float NOT NULL DEFAULT '0',
  `bindmapId` int(10) unsigned NOT NULL DEFAULT '0',
  `bindzoneId` int(10) unsigned NOT NULL DEFAULT '0',
  `isResting` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `restState` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `restTime` int(10) unsigned NOT NULL DEFAULT '0',
  `playedtime` longtext NOT NULL,
  `deathstate` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `TalentResetTimes` int(10) unsigned NOT NULL DEFAULT '0',
  `first_login` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `forced_rename_pending` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `arenaPoints` int(10) unsigned NOT NULL DEFAULT '0',
  `totalstableslots` int(10) unsigned NOT NULL DEFAULT '0',
  `instance_id` int(10) unsigned NOT NULL DEFAULT '0',
  `entrypointmap` int(10) unsigned NOT NULL DEFAULT '0',
  `entrypointx` float NOT NULL DEFAULT '0',
  `entrypointy` float NOT NULL DEFAULT '0',
  `entrypointz` float NOT NULL DEFAULT '0',
  `entrypointo` float NOT NULL DEFAULT '0',
  `entrypointinstance` int(10) unsigned NOT NULL DEFAULT '0',
  `taxi_path` int(10) unsigned NOT NULL DEFAULT '0',
  `taxi_lastnode` int(10) unsigned NOT NULL DEFAULT '0',
  `taxi_mountid` int(10) unsigned NOT NULL DEFAULT '0',
  `transporter` int(10) unsigned NOT NULL DEFAULT '0',
  `transporter_xdiff` float NOT NULL DEFAULT '0',
  `transporter_ydiff` float NOT NULL DEFAULT '0',
  `transporter_zdiff` float NOT NULL DEFAULT '0',
  `spells` longtext NOT NULL,
  `deleted_spells` longtext NOT NULL,
  `reputation` longtext NOT NULL,
  `actions` longtext NOT NULL,
  `auras` longtext NOT NULL,
  `finished_quests` longtext NOT NULL,
  `honorPointsToAdd` int(11) NOT NULL,
  `killsToday` int(10) unsigned NOT NULL DEFAULT '0',
  `killsYesterday` int(10) unsigned NOT NULL DEFAULT '0',
  `killsLifeTime` int(10) unsigned NOT NULL DEFAULT '0',
  `honorToday` int(10) unsigned NOT NULL DEFAULT '0',
  `honorYesterday` int(10) unsigned NOT NULL DEFAULT '0',
  `honorPoints` int(10) unsigned NOT NULL DEFAULT '0',
  `difficulty` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guid`),
  UNIQUE KEY `name` (`name`),
  KEY `acct` (`acct`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for charters
-- ----------------------------
CREATE TABLE `charters` (
  `charterId` int(10) unsigned NOT NULL,
  `chartertype` int(10) NOT NULL DEFAULT '0',
  `leaderGuid` int(20) unsigned NOT NULL DEFAULT '0',
  `guildName` varchar(32) NOT NULL DEFAULT '',
  `itemGuid` bigint(40) unsigned NOT NULL DEFAULT '0',
  `signer1` int(10) unsigned NOT NULL DEFAULT '0',
  `signer2` int(10) unsigned NOT NULL DEFAULT '0',
  `signer3` int(10) unsigned NOT NULL DEFAULT '0',
  `signer4` int(10) unsigned NOT NULL DEFAULT '0',
  `signer5` int(10) unsigned NOT NULL DEFAULT '0',
  `signer6` int(10) unsigned NOT NULL DEFAULT '0',
  `signer7` int(10) unsigned NOT NULL DEFAULT '0',
  `signer8` int(10) unsigned NOT NULL DEFAULT '0',
  `signer9` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`charterId`),
  UNIQUE KEY `leaderGuid` (`leaderGuid`),
  UNIQUE KEY `guildName` (`guildName`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1 COMMENT='InnoDB free: 11264 kB; InnoDB free: 18432 kB';

-- ----------------------------
-- Table structure for corpses
-- ----------------------------
CREATE TABLE `corpses` (
  `guid` bigint(20) unsigned NOT NULL DEFAULT '0',
  `positionX` float NOT NULL DEFAULT '0',
  `positionY` float NOT NULL DEFAULT '0',
  `positionZ` float NOT NULL DEFAULT '0',
  `orientation` float NOT NULL DEFAULT '0',
  `zoneId` int(11) NOT NULL DEFAULT '38',
  `mapId` int(11) NOT NULL DEFAULT '0',
  `instanceId` int(11) NOT NULL DEFAULT '0',
  `data` longtext NOT NULL,
  PRIMARY KEY (`guid`),
  KEY `b` (`mapId`),
  KEY `c` (`instanceId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for groups
-- ----------------------------
CREATE TABLE `groups` (
  `group_id` int(30) NOT NULL,
  `group_type` tinyint(2) NOT NULL,
  `subgroup_count` tinyint(2) NOT NULL,
  `loot_method` tinyint(2) NOT NULL,
  `loot_threshold` tinyint(2) NOT NULL,
  `difficulty` int(30) NOT NULL DEFAULT '0',
  `raiddifficulty` int(30) NOT NULL DEFAULT '0',
  `assistant_leader` int(30) NOT NULL DEFAULT '0',
  `main_tank` int(30) NOT NULL DEFAULT '0',
  `main_assist` int(30) NOT NULL DEFAULT '0',
  `group1member1` int(30) NOT NULL,
  `group1member2` int(30) NOT NULL,
  `group1member3` int(30) NOT NULL,
  `group1member4` int(30) NOT NULL,
  `group1member5` int(30) NOT NULL,
  `group2member1` int(30) NOT NULL,
  `group2member2` int(30) NOT NULL,
  `group2member3` int(30) NOT NULL,
  `group2member4` int(30) NOT NULL,
  `group2member5` int(30) NOT NULL,
  `group3member1` int(30) NOT NULL,
  `group3member2` int(30) NOT NULL,
  `group3member3` int(30) NOT NULL,
  `group3member4` int(30) NOT NULL,
  `group3member5` int(30) NOT NULL,
  `group4member1` int(30) NOT NULL,
  `group4member2` int(30) NOT NULL,
  `group4member3` int(30) NOT NULL,
  `group4member4` int(30) NOT NULL,
  `group4member5` int(30) NOT NULL,
  `group5member1` int(30) NOT NULL,
  `group5member2` int(30) NOT NULL,
  `group5member3` int(30) NOT NULL,
  `group5member4` int(30) NOT NULL,
  `group5member5` int(30) NOT NULL,
  `group6member1` int(30) NOT NULL,
  `group6member2` int(30) NOT NULL,
  `group6member3` int(30) NOT NULL,
  `group6member4` int(30) NOT NULL,
  `group6member5` int(30) NOT NULL,
  `group7member1` int(30) NOT NULL,
  `group7member2` int(30) NOT NULL,
  `group7member3` int(30) NOT NULL,
  `group7member4` int(30) NOT NULL,
  `group7member5` int(30) NOT NULL,
  `group8member1` int(30) NOT NULL,
  `group8member2` int(30) NOT NULL,
  `group8member3` int(30) NOT NULL,
  `group8member4` int(30) NOT NULL,
  `group8member5` int(30) NOT NULL,
  `timestamp` int(30) NOT NULL,
  PRIMARY KEY (`group_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for guild_bankitems
-- ----------------------------
CREATE TABLE `guild_bankitems` (
  `guildId` int(30) NOT NULL,
  `tabId` int(30) NOT NULL,
  `slotId` int(30) NOT NULL,
  `itemGuid` int(30) NOT NULL,
  PRIMARY KEY (`guildId`,`tabId`,`slotId`,`itemGuid`),
  UNIQUE KEY `ItemGuid` (`itemGuid`),
  UNIQUE KEY `Slots` (`guildId`,`slotId`,`tabId`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for guild_banklogs
-- ----------------------------
CREATE TABLE `guild_banklogs` (
  `log_id` int(30) NOT NULL,
  `guildid` int(30) NOT NULL,
  `tabid` bigint(30) NOT NULL,
  `action` bigint(30) NOT NULL,
  `player_guid` bigint(30) NOT NULL,
  `item_entry` bigint(30) NOT NULL,
  `stack_count` bigint(30) NOT NULL,
  `timestamp` bigint(30) NOT NULL,
  PRIMARY KEY (`log_id`,`guildid`),
  KEY `a` (`log_id`),
  KEY `b` (`guildid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for guild_banktabs
-- ----------------------------
CREATE TABLE `guild_banktabs` (
  `guildId` int(30) NOT NULL,
  `tabId` int(30) NOT NULL,
  `tabName` varchar(200) COLLATE utf8_unicode_ci NOT NULL,
  `tabIcon` varchar(200) COLLATE utf8_unicode_ci NOT NULL,
  `tabInfo` varchar(200) COLLATE utf8_unicode_ci NOT NULL,
  PRIMARY KEY (`guildId`,`tabId`),
  KEY `a` (`guildId`),
  KEY `b` (`tabId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for guild_data
-- ----------------------------
CREATE TABLE `guild_data` (
  `guildId` bigint(20) NOT NULL AUTO_INCREMENT,
  `guildName` varchar(32) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `leaderGuid` bigint(20) NOT NULL DEFAULT '0',
  `guildLevel` int(20) NOT NULL DEFAULT '0',
  `emblemStyle` int(10) NOT NULL DEFAULT '0',
  `emblemColor` int(10) NOT NULL DEFAULT '0',
  `borderStyle` int(10) NOT NULL DEFAULT '0',
  `borderColor` int(10) NOT NULL DEFAULT '0',
  `backgroundColor` int(10) NOT NULL DEFAULT '0',
  `guildInfo` varchar(1024) COLLATE utf8_unicode_ci NOT NULL,
  `motd` varchar(1024) COLLATE utf8_unicode_ci NOT NULL,
  `createdate` int(10) NOT NULL DEFAULT '0',
  `balance` bigint(30) unsigned NOT NULL DEFAULT '0',
  `guildXPToday` bigint(30) unsigned NOT NULL DEFAULT '0',
  `guildExperience` bigint(30) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`guildId`),
  UNIQUE KEY `guildName` (`guildName`),
  UNIQUE KEY `leaderGuid` (`leaderGuid`)
) ENGINE=MyISAM AUTO_INCREMENT=25 DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for guild_logs
-- ----------------------------
CREATE TABLE `guild_logs` (
  `log_id` int(30) NOT NULL,
  `guildid` int(30) NOT NULL,
  `timestamp` int(30) NOT NULL,
  `event_type` int(30) NOT NULL,
  `misc1` int(30) NOT NULL,
  `misc2` int(30) NOT NULL,
  `misc3` int(30) NOT NULL,
  PRIMARY KEY (`log_id`,`guildid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for guild_members
-- ----------------------------
CREATE TABLE `guild_members` (
  `guildid` int(10) NOT NULL,
  `playerid` int(10) NOT NULL,
  `guildRank` int(10) NOT NULL,
  `publicNote` varchar(300) NOT NULL,
  `officerNote` varchar(300) NOT NULL,
  `lastWithdrawReset` int(10) NOT NULL DEFAULT '0',
  `withdrawlsSinceLastReset` int(10) NOT NULL DEFAULT '0',
  `lastItemWithdrawReset0` int(10) NOT NULL DEFAULT '0',
  `itemWithdrawlsSinceLastReset0` int(10) NOT NULL DEFAULT '0',
  `lastItemWithdrawReset1` int(10) NOT NULL DEFAULT '0',
  `itemWithdrawlsSinceLastReset1` int(10) NOT NULL DEFAULT '0',
  `lastItemWithdrawReset2` int(10) NOT NULL DEFAULT '0',
  `itemWithdrawlsSinceLastReset2` int(10) NOT NULL DEFAULT '0',
  `lastItemWithdrawReset3` int(10) NOT NULL DEFAULT '0',
  `itemWithdrawlsSinceLastReset3` int(10) NOT NULL DEFAULT '0',
  `lastItemWithdrawReset4` int(10) NOT NULL DEFAULT '0',
  `itemWithdrawlsSinceLastReset4` int(10) NOT NULL DEFAULT '0',
  `lastItemWithdrawReset5` int(10) NOT NULL DEFAULT '0',
  `itemWithdrawlsSinceLastReset5` int(10) NOT NULL DEFAULT '0',
  `lastItemWithdrawReset6` int(10) NOT NULL DEFAULT '0',
  `itemWithdrawlsSinceLastReset6` int(10) NOT NULL DEFAULT '0',
  `lastItemWithdrawReset7` int(10) NOT NULL DEFAULT '0',
  `itemWithdrawlsSinceLastReset7` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`playerid`,`guildRank`),
  UNIQUE KEY `guildid` (`guildid`,`playerid`),
  KEY `a` (`guildid`),
  KEY `b` (`playerid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for guild_ranks
-- ----------------------------
CREATE TABLE `guild_ranks` (
  `guildId` int(6) unsigned NOT NULL,
  `rankId` int(1) NOT NULL DEFAULT '0',
  `rankName` varchar(64) COLLATE utf8_unicode_ci NOT NULL DEFAULT '',
  `rankRights` int(3) unsigned NOT NULL DEFAULT '0',
  `goldLimitPerDay` int(10) NOT NULL DEFAULT '0',
  `bankTabFlags0` int(10) NOT NULL DEFAULT '0',
  `itemStacksPerDay0` int(10) NOT NULL DEFAULT '0',
  `bankTabFlags1` int(10) NOT NULL DEFAULT '0',
  `itemStacksPerDay1` int(10) NOT NULL DEFAULT '0',
  `bankTabFlags2` int(10) NOT NULL DEFAULT '0',
  `itemStacksPerDay2` int(10) NOT NULL DEFAULT '0',
  `bankTabFlags3` int(10) NOT NULL DEFAULT '0',
  `itemStacksPerDay3` int(10) NOT NULL DEFAULT '0',
  `bankTabFlags4` int(10) NOT NULL DEFAULT '0',
  `itemStacksPerDay4` int(10) NOT NULL DEFAULT '0',
  `bankTabFlags5` int(10) NOT NULL DEFAULT '0',
  `itemStacksPerDay5` int(10) NOT NULL DEFAULT '0',
  `bankTabFlags6` int(10) NOT NULL DEFAULT '0',
  `itemStacksPerDay6` int(10) NOT NULL DEFAULT '0',
  `bankTabFlags7` int(10) DEFAULT '0',
  `itemStacksPerDay7` int(10) DEFAULT '0',
  PRIMARY KEY (`guildId`,`rankId`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8 COLLATE=utf8_unicode_ci;

-- ----------------------------
-- Table structure for item_data
-- ----------------------------
CREATE TABLE `item_data` (
  `ownerguid` int(10) unsigned NOT NULL DEFAULT '0',
  `itemguid` int(10) unsigned NOT NULL DEFAULT '0',
  `itementry` int(10) unsigned NOT NULL DEFAULT '0',
  `containerguid` int(10) unsigned NOT NULL DEFAULT '0',
  `creatorguid` int(10) unsigned NOT NULL DEFAULT '0',
  `count` int(10) unsigned NOT NULL DEFAULT '0',
  `flags` int(10) unsigned NOT NULL DEFAULT '0',
  `randomseed` int(10) unsigned NOT NULL DEFAULT '0',
  `randomproperty` int(10) unsigned NOT NULL DEFAULT '0',
  `durability` int(10) unsigned NOT NULL DEFAULT '0',
  `textid` int(10) unsigned NOT NULL DEFAULT '0',
  `playedtime` int(10) unsigned NOT NULL DEFAULT '0',
  `spellcharges` int(10) NOT NULL DEFAULT '-1',
  `giftitemid` int(10) unsigned NOT NULL DEFAULT '0',
  `giftcreatorguid` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ownerguid`,`itemguid`),
  UNIQUE KEY `Unique` (`ownerguid`,`itemguid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for item_enchantments
-- ----------------------------
CREATE TABLE `item_enchantments` (
  `itemguid` int(10) unsigned NOT NULL DEFAULT '0',
  `enchantslot` tinyint(3) unsigned NOT NULL DEFAULT '0',
  `enchantid` int(10) unsigned NOT NULL DEFAULT '0',
  `enchantsuffix` int(10) NOT NULL DEFAULT '0',
  `enchantexpiretimer` int(10) unsigned NOT NULL DEFAULT '0',
  `enchantcharges` int(10) NOT NULL DEFAULT '0',
  PRIMARY KEY (`itemguid`,`enchantslot`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for mailbox
-- ----------------------------
CREATE TABLE `mailbox` (
  `message_id` int(30) NOT NULL AUTO_INCREMENT,
  `message_type` int(30) NOT NULL DEFAULT '0',
  `player_guid` int(30) NOT NULL DEFAULT '0',
  `sender_guid` int(30) NOT NULL DEFAULT '0',
  `subject` varchar(255) NOT NULL DEFAULT '',
  `body` varchar(4096) NOT NULL,
  `money` int(30) NOT NULL DEFAULT '0',
  `attached_item_guids` varchar(200) NOT NULL DEFAULT '',
  `cod` int(30) NOT NULL DEFAULT '0',
  `stationary` int(30) NOT NULL DEFAULT '0',
  `expiry_time` int(30) NOT NULL DEFAULT '0',
  `delivery_time` int(30) NOT NULL DEFAULT '0',
  `copy_made` int(30) NOT NULL DEFAULT '0',
  `read_flag` int(30) NOT NULL DEFAULT '0',
  `deleted_flag` int(30) NOT NULL DEFAULT '0',
  `returned_flag` int(10) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`message_id`),
  KEY `b` (`player_guid`)
) ENGINE=MyISAM AUTO_INCREMENT=112 DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for pet_actionbar
-- ----------------------------
CREATE TABLE `pet_actionbar` (
  `ownerguid` bigint(20) NOT NULL DEFAULT '0',
  `petnumber` int(11) NOT NULL DEFAULT '0',
  `spellid_1` int(11) unsigned NOT NULL DEFAULT '0',
  `spellid_2` int(11) unsigned NOT NULL DEFAULT '0',
  `spellid_3` int(11) unsigned NOT NULL DEFAULT '0',
  `spellid_4` int(11) unsigned NOT NULL DEFAULT '0',
  `spellstate_1` int(11) unsigned NOT NULL DEFAULT '0',
  `spellstate_2` int(11) unsigned NOT NULL DEFAULT '0',
  `spellstate_3` int(11) unsigned NOT NULL DEFAULT '0',
  `spellstate_4` int(11) unsigned NOT NULL DEFAULT '0',
  PRIMARY KEY (`ownerguid`,`petnumber`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8;

-- ----------------------------
-- Table structure for pet_data
-- ----------------------------
CREATE TABLE `pet_data` (
  `ownerguid` bigint(20) NOT NULL DEFAULT '0',
  `petnumber` int(11) NOT NULL DEFAULT '0',
  `name` varchar(21) NOT NULL DEFAULT '',
  `entry` bigint(20) NOT NULL DEFAULT '0',
  `fields` longtext NOT NULL,
  `xp` int(11) NOT NULL DEFAULT '0',
  `active` tinyint(1) NOT NULL DEFAULT '1',
  `level` int(11) NOT NULL DEFAULT '0',
  `happiness` int(11) NOT NULL DEFAULT '0',
  `happinessupdate` int(11) NOT NULL DEFAULT '0',
  `summon` int(11) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ownerguid`,`petnumber`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for pet_spells
-- ----------------------------
CREATE TABLE `pet_spells` (
  `ownerguid` bigint(100) NOT NULL DEFAULT '0',
  `petnumber` int(100) NOT NULL DEFAULT '0',
  `spellid` int(100) NOT NULL DEFAULT '0',
  `flags` int(4) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ownerguid`,`petnumber`,`spellid`),
  UNIQUE KEY `a` (`ownerguid`,`petnumber`,`spellid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for pet_talents
-- ----------------------------
CREATE TABLE `pet_talents` (
  `ownerguid` bigint(20) NOT NULL DEFAULT '0',
  `petnumber` int(4) NOT NULL DEFAULT '0',
  `talentid` int(11) NOT NULL DEFAULT '0',
  `rank` tinyint(4) NOT NULL DEFAULT '0',
  PRIMARY KEY (`ownerguid`,`petnumber`,`talentid`),
  UNIQUE KEY `a` (`ownerguid`,`petnumber`,`talentid`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for server_settings
-- ----------------------------
CREATE TABLE `server_settings` (
  `setting_id` varchar(200) NOT NULL,
  `setting_value` bigint(50) unsigned NOT NULL,
  PRIMARY KEY (`setting_id`)
) ENGINE=MyISAM DEFAULT CHARSET=latin1;

-- ----------------------------
-- Table structure for worldstate_save_data
-- ----------------------------
CREATE TABLE `worldstate_save_data` (
  `setting_id` int(8) unsigned NOT NULL DEFAULT '0',
  `setting_value` int(8) NOT NULL DEFAULT '0',
  PRIMARY KEY (`setting_id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
