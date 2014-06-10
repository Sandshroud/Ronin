ALTER TABLE `guilds` CHANGE COLUMN `bankBalance` `balanceLow` int(30) NOT NULL DEFAULT '0';
ALTER TABLE `guilds` ADD COLUMN `balanceHigh` int(30) NOT NULL DEFAULT '0' AFTER `createdate`;