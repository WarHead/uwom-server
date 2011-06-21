SET NAMES utf8;

DELETE FROM `command` WHERE `name` LIKE '%jail%';
INSERT INTO `command` (name, security, help) VALUES
('jail info',       0, 'Syntax: .jail info\nZeigt dir deinen Knaststatus an.'),
('jail pinfo',      1, 'Syntax: .jail pinfo [Charakter]\nZeigt Jail-Infos vom selektiertem, oder angegeben Charakter an.'),
('jail arrest',     2, 'Syntax: .jail arrest Charakter Stunden Grund\nBuchtet den Charakter für Stunden aus dem Grund ein.'),
('jail release',    2, 'Syntax: .jail release Charakter\nEntlässt den Charakter aus dem Knast.'),
('jail reset',      2, 'Syntax: .jail reset [Charakter]\nGibt dem selektiertem, oder angegebenen Charakter, wieder eine weisse Weste.\nAls wäre nie etwas geschehen. ;-)\nAmnasty International lässt grüssen! :D'),
('jail reload',     2, 'Syntax: .jail reload\nLädt die Jail-Konfiguration und die Inhaftierungen neu aus der Datenbank.'),
('jail enable',     2, 'Syntax: .jail enable\nSchaltet das Jail ein.'),
('jail disable',    2, 'Syntax: .jail disable\nSchaltet das Jail aus.'),
('jail delete',     3, 'Syntax: .jail delete\nWie reset, aber löscht auch Jails mit Bannungen!\nDie Bannung wird dadurch auch gelöscht!');
