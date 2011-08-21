-- SpellScripts (mostly target selection)
DELETE FROM spell_script_names WHERE spell_id IN (56548,56397,63934,57459,61694,60936);
INSERT INTO spell_script_names VALUES
(56548,'spell_malygos_surge_of_power'),
(56397,'spell_malygos_arcane_barrage'),
(63934,'spell_malygos_arcane_barrage'),
(57459,'spell_malygos_arcane_storm'),
(61694,'spell_malygos_arcane_storm'),
(60936,'spell_malygos_surge_of_power_p3');