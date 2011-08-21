-- Limit "Denying the Scion" to respective Map-Version
DELETE FROM achievement_criteria_data WHERE criteria_id IN (7573,7574) AND type=12;
INSERT INTO achievement_criteria_data VALUES (7573,12,0,0,''),(7574,12,1,0,'');

-- condition for gift-beam-visual
DELETE FROM conditions WHERE SourceTypeOrReferenceId=13 AND SourceEntry=61028;
INSERT INTO conditions VALUES (13,0,61028,0,18,1,32448,0,0,'','Eye of Eternity: Alexstrasza Gift Beam targeting');

-- disable LoS for Eye of Eternity
DELETE FROM disables WHERE sourceType=6 AND entry=616;
INSERT INTO disables VALUES (6,616,0x4,0,0,'Eye of Eternity - Line of Sight');

-- add SmartScript to Static Fields so they despawn in time
UPDATE creature_template SET AIName='SmartAI' WHERE entry=30592;
DELETE FROM smart_scripts WHERE entryOrGuid=30592;
INSERT INTO smart_scripts VALUES (30592,0,0,0,54,0,100,0,0,0,0,0,41,30000,0,0,0,0,0,1,0,0,0,0,0,0,0,'Static Field - Despawn after 30sec');

-- give energy sparks a visible aura
DELETE FROM creature_template_addon WHERE entry IN (30084,32187);
INSERT INTO creature_template_addon (entry,auras) VALUES (30084,55845),(32187,55845);

-- models for 'Vortex' are inversed (flag as trigger..? (128))
UPDATE creature_template SET modelid2=11686, modelid1=169 WHERE entry=30090;

-- add sound to mute Alexstrasza
UPDATE creature_text SET sound=14406 WHERE entry=32295 AND groupid=0;
UPDATE creature_text SET sound=14407 WHERE entry=32295 AND groupid=1;
UPDATE creature_text SET sound=14408 WHERE entry=32295 AND groupid=2;
UPDATE creature_text SET sound=14409 WHERE entry=32295 AND groupid=3;
-- this one obsolete?
DELETE FROM creature_text WHERE entry=31253;
INSERT INTO creature_text VALUES (31253,0,0,'Champions! My children, blessed with my powers, shall aid you in your fight!',14,0,0,0,0,14405,'Alexstrasza (Proxy) - Spawn Drakes');
