
SET FOREIGN_KEY_CHECKS=0;

CREATE TABLE `chatlog` (
  `Timestamp` int(11) NOT NULL default '0',
  `AccountID` int(11) NOT NULL default '0',
  `AccountName` varchar(255) NOT NULL default '',
  `IPAdress` varchar(255) NOT NULL default '',
  `PlayerName` varchar(255) NOT NULL default '',
  `Message` longtext NOT NULL,
  UNIQUE KEY `Key` (`Timestamp`,`AccountID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `cheaterlog` (
  `Timestamp` int(11) NOT NULL default '0',
  `AccountID` int(11) NOT NULL default '0',
  `AccountName` varchar(255) NOT NULL default '',
  `IPAdress` varchar(255) NOT NULL default '',
  `PlayerName` varchar(255) NOT NULL default '',
  `Message` longtext NOT NULL,
  UNIQUE KEY `Key` (`Timestamp`,`AccountID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `gmlog` (
  `Timestamp` int(11) NOT NULL default '0',
  `AccountID` int(11) NOT NULL default '0',
  `AccountName` varchar(255) NOT NULL default '',
  `IPAdress` varchar(255) NOT NULL default '',
  `PlayerName` varchar(255) NOT NULL default '',
  `Message` longtext NOT NULL,
  UNIQUE KEY `Key` (`Timestamp`,`AccountID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

CREATE TABLE `playerlog` (
  `Timestamp` int(11) NOT NULL default '0',
  `AccountID` int(11) NOT NULL default '0',
  `AccountName` varchar(255) NOT NULL default '',
  `IPAdress` varchar(255) NOT NULL default '',
  `PlayerName` varchar(255) NOT NULL default '',
  `Message` longtext NOT NULL,
  UNIQUE KEY `Key` (`Timestamp`,`AccountID`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
