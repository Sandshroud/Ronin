DROP TABLE IF EXISTS `tutorials`;
DROP TABLE IF EXISTS `account_tutorials`;
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
