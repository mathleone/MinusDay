CREATE DATABASE siem;

CREATE USER 'siemadmin'@'localhost' IDENTIFIED BY 'siemadmin';
GRANT ALL PRIVILEGES ON *.* TO 'siemadmin'@'localhost';
FLUSH PRIVILEGES;

-- SET PASSWORD FOR 'siemadmin'@'localhost' = 'siemadmin';

CREATE TABLE siem.events(
	id int(16) NOT NULL AUTO_INCREMENT,
	time timestamp DEFAULT 0,
	host varchar(64),
	type varchar(64),
	path varchar(256),
	log blob,
	indextime timestamp DEFAULT now(),
	PRIMARY KEY (id)
);