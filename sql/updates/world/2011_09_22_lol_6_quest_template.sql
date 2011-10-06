-- correct some Reputation Reward of Oracle & Freanzyheart Quests for Ulduar Content (confirmed by some www.wowhead.com comments)
UPDATE `quest_template` SET `RewRepValueId1`=7 WHERE `entry` IN (12735,12736,12737,12726,12704, 12758,12734,12741,12732,12702);
UPDATE `quest_template` SET `RewRepValue1`=70000 WHERE `entry` IN (12761,12762,12705, 12703,12760,12760);