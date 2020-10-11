-- Base: SYSHOME

-- DROP
DROP TABLE web_object;
DROP TABLE image_object;



-- CREATE
CREATE TABLE image_object (
idObject  numeric(3) NOT NULL,
name			varchar(255),
src_unk		varchar(255),
src_on 		varchar(255),
src_off		varchar(255),
PRIMARY KEY(idObject)
);


CREATE TABLE web_object (
page			numeric(5) NOT NULL,
idTag 		varchar(255) NOT NULL,
name			varchar(255),
idObject  numeric(3),
xPos			numeric(5),
yPos			numeric(5),
aTag			varchar(255),
disp			numeric(3),
port			char(2),
bit				numeric(1),
PRIMARY KEY(page, idTag),
FOREIGN KEY (idObject) REFERENCES image_object (idObject)
);


-- GRANT
GRANT SELECT, INSERT, UPDATE, DELETE ON web_object TO syshome;
GRANT SELECT, INSERT, UPDATE, DELETE ON image_object TO syshome;
