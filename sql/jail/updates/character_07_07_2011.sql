ALTER TABLE `jail_conf`
ADD COLUMN `amnesty` int(11) NOT NULL DEFAULT 3 COMMENT 'Zeit in Monaten, nach denen die Jaileinträge verfallen.' AFTER `gm_char`,
ROW_FORMAT=FIXED;
