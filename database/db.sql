CREATE TABLE access_string (
  sistema varchar(32) NOT NULL,
  acceso varchar(255) NOT NULL,
  PRIMARY KEY  (sistema,acceso)
);

CREATE SEQUENCE product_access_seq;
CREATE TABLE product_access (
  id numeric(11) NOT NULL default nextval('product_access_seq'),
  producto varchar(255) NOT NULL,
  usuario varchar(16) NOT NULL,
  PRIMARY KEY  (id,producto)
);

CREATE TABLE device (
  id numeric(3) NOT NULL,
  hw_typ numeric(3) NOT NULL,
  hw_ver numeric(3) NOT NULL,
  sw_ver numeric(3) NOT NULL,
  status numeric(1),
  last_online timestamp,
  device_name varchar(255),
  PRIMARY KEY  (id)
);

CREATE TABLE devconf (
  id              numeric(3) NOT NULL,
  Mascara_E1S1    numeric(3),
  Mascara_E2S2    numeric(3),
  ANALOG1_MAX     numeric(4),
  ANALOG1_MIN     numeric(4),
  ANALOG2_MAX     numeric(4),
  ANALOG2_MIN     numeric(4),
  ANALOG3_MAX     numeric(4),
  ANALOG3_MIN     numeric(4),
  ANALOG4_MAX     numeric(4),
  ANALOG4_MIN     numeric(4),
  PRIMARY KEY (id),
  FOREIGN KEY (id) REFERENCES device (id)
);

CREATE TABLE assign (
  id numeric(3) NOT NULL,
  port char(2) NOT NULL,
  bit numeric(1) NOT NULL,
  assign_name varchar(255),
  PRIMARY KEY  (id,port,bit),
  FOREIGN KEY (id) REFERENCES device (id)
);

CREATE SEQUENCE group_list_seq;
CREATE TABLE group_list (
  id numeric(11) NOT NULL default nextval('group_list_seq'),
  group_name varchar(255) NOT NULL,
  PRIMARY KEY  (id)
);

CREATE TABLE group_rel (
  group_id numeric(11) NOT NULL,
  id numeric(3) NOT NULL,
  port char(2) NOT NULL,
  bit numeric(1) NOT NULL,
  PRIMARY KEY  (group_id,id,port,bit),
  FOREIGN KEY (group_id) REFERENCES group_list (id),
  FOREIGN KEY (id,port,bit) REFERENCES assign (id,port,bit)
);

CREATE TABLE comando (
  id numeric(3) NOT NULL,
  cmd_name varchar(255) NOT NULL,
  PRIMARY KEY  (id)
);

CREATE TABLE home_flag (
  nombre varchar(32) NOT NULL,
  valor varchar(255),
  descripcion varchar(255),
  PRIMARY KEY  (nombre)
);

CREATE SEQUENCE event_seq;
CREATE TABLE event (
  id numeric(11) NOT NULL default nextval('event_seq'),
  src_dev numeric(3) NOT NULL,
  src_port char(2) NOT NULL,
  src_bit numeric(1),
  src_on_off numeric(1) NOT NULL,
  dst_cmd numeric(3) NOT NULL,
  dst_param1 varchar(255),
  dst_param2 varchar(255),
  flag  varchar(32),
  flag_condicion numeric (1),
  flag_valor varchar(255),
  dst_group numeric(11),
  enable numeric(1) NOT NULL,
  event_name varchar(255),
  PRIMARY KEY  (id),
  FOREIGN KEY (src_dev,src_port,src_bit) REFERENCES assign (id,port,bit),
  FOREIGN KEY (dst_cmd) REFERENCES comando (id),
  FOREIGN KEY (dst_group) REFERENCES group_list (id),
  FOREIGN KEY (flag) REFERENCES home_flag (nombre)
);

CREATE TABLE autoactivacion (
  id numeric(3) NOT NULL,
  port char(2) NOT NULL,
  bit numeric(1) NOT NULL,
  ultimo numeric(1),
  PRIMARY KEY  (id,port,bit),
  FOREIGN KEY (id,port,bit) REFERENCES assign (id,port,bit)
);

CREATE TABLE auto_event (
  id varchar(32) NOT NULL,
  dst_cmd numeric(3) NOT NULL,
  dst_param1 varchar(255),
  dst_param2 varchar(255),
  flag  varchar(32),
  flag_condicion numeric (1),
  flag_valor varchar(255),
  dst_group numeric(11),
  enable numeric(1) NOT NULL,
  event_name varchar(255),
  PRIMARY KEY  (id),
  FOREIGN KEY (dst_cmd) REFERENCES comando (id),
  FOREIGN KEY (dst_group) REFERENCES group_list (id),
  FOREIGN KEY (flag) REFERENCES home_flag (nombre)
);

CREATE TABLE home_config (
  id numeric(3) NOT NULL,
  autoactivacion_alarma numeric(1),
  autoreactivacion_alarma numeric(1),
  PRIMARY KEY  (id)
);

CREATE SEQUENCE tarea_programada_seq;
CREATE TABLE tarea_programada (
  id 		numeric(11) NOT NULL default nextval('tarea_programada_seq'),
  dia_semana	numeric(3),
  hora		numeric(2),
  minuto	numeric(2),
  dst_cmd         numeric(3) NOT NULL,
  dst_param1      varchar(255),
  dst_param2      varchar(255),
  flag            varchar(32),
  flag_condicion  numeric (1),
  flag_valor      varchar(255),
  dst_group       numeric(11),
  enable          numeric(1) NOT NULL,
  task_name       varchar(255),
  last_exec       numeric(10),
  PRIMARY KEY  (id),
  FOREIGN KEY (dst_cmd) REFERENCES comando (id),
  FOREIGN KEY (dst_group) REFERENCES group_list (id),
  FOREIGN KEY (flag) REFERENCES home_flag (nombre)
);

CREATE TABLE pais (
  codigo numeric(3) NOT NULL,
  moneda numeric(3) NOT NULL,
  sigla char(2) NOT NULL,
  abrev char(3) NOT NULL,
  nombre varchar(64) NOT NULL,
  PRIMARY KEY  (codigo)
);

CREATE SEQUENCE persona_seq;
CREATE TABLE persona (
  id numeric(11) NOT NULL default nextval('persona_seq'),
  apellido varchar(32) NOT NULL,
  nombre varchar(64) NOT NULL,
  dia numeric(2),
  mes numeric(2),
  anio numeric(4),
  mail varchar(32),
  postal varchar(64),
  sms varchar(32),
  telefono varchar(32),
  pais_nacimiento numeric(3),
  pais_radicacion numeric(3),
  password varchar(255),
  user_name varchar(16),
  PRIMARY KEY  (id),
  FOREIGN KEY (pais_nacimiento) REFERENCES pais (codigo),
  FOREIGN KEY (pais_radicacion) REFERENCES pais (codigo)
);  FOREIGN KEY (id,port,bit) REFERENCES assign (id,port,bit)


CREATE TABLE vinculo (
  padre numeric(11) NOT NULL,
  madre numeric(11) NOT NULL,
  hijo numeric(11) NOT NULL,
  PRIMARY KEY  (padre,madre,hijo),
  FOREIGN KEY (madre) REFERENCES persona (id),
  FOREIGN KEY (hijo) REFERENCES persona (id)
);

CREATE TABLE testigo (
  id numeric(3) NOT NULL,
  port char(2) NOT NULL,
  bit numeric(1) NOT NULL,
  id_testigo numeric(3) NOT NULL,
  port_testigo char(2) NOT NULL,
  bit_testigo numeric(1) NOT NULL,
  valor_on  numeric(1),
  valor_off  numeric(1),
  invertir  numeric(1),
  PRIMARY KEY  (id,port,bit,id_testigo,port_testigo),
  FOREIGN KEY (id,port,bit) REFERENCES assign (id,port,bit),
  FOREIGN KEY (id_testigo,port_testigo, bit_testigo) REFERENCES assign (id,port,bit)
);

CREATE TABLE historico (
  id numeric(3) NOT NULL,
  port char(2) NOT NULL,
  bit numeric(1) NOT NULL,
  fecha_hora timestamp NOT NULL,
  estado numeric(4),
  PRIMARY KEY  (id,port,bit,fecha_hora),
  FOREIGN KEY (id) REFERENCES device (id)
);

