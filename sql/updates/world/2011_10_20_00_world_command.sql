DELETE FROM `command` WHERE `name`='reset arenaworld';
DELETE FROM `command` WHERE `name`='reset honorworld';

INSERT INTO `command` (`name`, `security`, `help`) VALUES
('reset arenaworld', 50, 'Syntax: .reset arenaworld [delete]\nSetzt alle Arenapunkte / Wertungen etc. auf die standard Startwerte. Mit [delete] werden auch alle Teams gelöscht! Alle Spieler werden dazu ausgelogt!'),
('reset honorworld', 50, 'Syntax: .reset honorworld [kills]\nSetzt alle Ehrenpunkte auf die standard Startwerte. Mit [kills] werden auch alle Kills gelöscht! Alle Spieler werden dazu ausgelogt!');
