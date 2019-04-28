-- Cleanup the extra matching names
DELETE n1 FROM recall n1, recall n2 WHERE n1.id > n2.id AND n1.name = n2.name;
-- Drop column id
ALTER TABLE `recall` drop column `id`;
-- Add name as the primary key
ALTER TABLE `recall` ADD PRIMARY KEY (name);
-- Add orientation column
ALTER TABLE `recall` ADD COLUMN `orientation` float(0) NOT NULL DEFAULT '0';