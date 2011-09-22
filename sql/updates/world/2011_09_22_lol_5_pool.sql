DELETE FROM `pool_template` WHERE `entry` IN (12000,12001,12002,12003);
INSERT INTO `pool_template` (`entry`,`max_limit`,`description`) VALUES 
(12000,1,'Oracle Soo-nee - Oracle Dailies'),
(12001,1,'Oracle Soo-dow - Oracle Dailies'),
(12002,1,'Rejek - Frenzyheart Tribe Dailies'),
(12003,1,'Vekgar - Frenzyheart Tribe Dailies');

DELETE FROM `pool_quest` WHERE `entry` IN (12735,12736,12737,12726,12761,12762,12705,12758,12734,12741,12732,12703,12760,12759);
INSERT INTO `pool_quest` (`entry`,`pool_entry`,`description`) VALUES 
-- ORACLE
-- Oracle Soo-nee
(12735,12000,'A Cleansing Song / Oracle Daily / Oracle Soo-nee'),
(12736,12000,'Song of Fecundity / Oracle Daily / Oracle Soo-nee'),
(12737,12000,'Song of Reflection / Oracle Daily / Oracle Soo-nee'),
(12726,12000,'Song of Wind and Water / Oracle Daily / Oracle Soo-nee'),
-- Oracle Soo-say
-- 12704|Appeasing the Great Rain Stone|everyday available 
-- Oracle Soo-dow
(12761,12001,'Mastery of the Crystals / Oracle Daily / Oracle Soo-dow'),
(12762,12001,'Power of the Great Ones / Oracle Daily / Oracle Soo-dow'),
(12705,12001,'Will of the Titans / Oracle Daily / Oracle Soo-dow'),
-- FRENZYHEART TRIBE
-- Rejek
(12758,12002,'A Heros''s Headgear / Frenzyheart Tribe Daily / Rejek'),
(12734,12002,'Rejek: First Blood / Frenzyheart Tribe Daily / Rejek'),
(12741,12002,'Strength of the Tempest / Frenzyheart Tribe Daily / Rejek'),
(12732,12002,'The Heartblood''s Strength / Frenzyheart Tribe Daily / Rejek'),
-- Elder Harkek
-- 12702|Chicken Party|everyday available 
-- Vekgar
(12703,12003,'Kartak''s Rampage / Frenzyheart Tribe Daily / Vekgar'),
(12760,12003,'Secret Strength of the Frenzyheart / Frenzyheart Tribe Daily / Vekgar'),
(12759,12003,'Tools of War / Frenzyheart Tribe Daily / Vekgar');