DROP TABLE IF EXISTS `mail_external`;
CREATE TABLE `mail_external` (
  `id` INT(11) UNSIGNED NOT NULL auto_increment,
  `receiver` INT(11) UNSIGNED NOT NULL,
  `subject` VARCHAR(255) DEFAULT 'UWoM-Team Support-Nachricht',
  `message` LONGTEXT,
  `money` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `item` INT(11) UNSIGNED NOT NULL DEFAULT '0',
  `itemcnt` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0',
  PRIMARY KEY (`id`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;
