\c home
\t
\pset border 0
\a
\f ''
\o home-data.sql
select 'insert into access_string (sistema,acceso) values (''',sistema,''',''',acceso,''');' from access_string;
select 'create sequence product_access_seq start ', nextval('product_access_seq'),';';
select 'insert into product_access (id,producto,usuario) values (''',id,''',''',producto,''',''',usuario,''');' from product_access;
select 'insert into device (id,hw_typ,hw_ver,sw_ver,status,last_online,device_name) values (''',id,''',''',hw_typ,''',''',hw_ver,''',''',sw_ver,''',''',status,''',''',last_online,''',''',device_name,''');' from device;
select 'insert into devconf (id,mascara_e1s1,mascara_e2s2,analog1_max,analog1_min,analog2_max,analog2_min,analog3_max,analog3_min,analog4_max,analog4_min) values (''',id,''',''',mascara_e1s1,''',''',mascara_e2s2,''',''',analog1_max,''',''',analog1_max,''',''',analog2_max,''',''',analog2_min,''',''',analog3_max,''',''',analog3_min,''',''',analog4_max,''',''',analog4_min,''');' from devconf;
select 'insert into assign (id,port,bit,assign_name) values (''',id,''',''',port,''',''',bit,''',''',assign_name,''');' from assign;
select 'create sequence group_list_seq start ', nextval('group_list_seq'),';';
select 'insert into group_list (id,group_name) values (''',id,''',''',group_name,''');' from group_list;
select 'insert into group_rel (group_id,id,port,bit) values (''',group_id,''',''',id,''',''',port,''',''',bit,''');' from group_rel;
select 'insert into comando (id,cmd_name) values (''',id,''',''',cmd_name,''');' from comando;
select 'create sequence event_seq start ', nextval('event_seq'),';';
select 'insert into event (id,src_dev,src_port,src_bit,src_on_off,dst_cmd,dst_param1,dst_param2,flag,flag_condicion,flag_valor,dst_group,enable,event_name) values (''',id,''',''',src_dev,''',''',src_port,''',''',src_bit,''',''',src_on_off,''',''',dst_cmd,''',''',dst_param1,''',''',dst_param2,''',''',flag,''',''',flag_condicion,''',''',flag_valor,''',''',dst_group,''',''',enable,''',''',event_name,''');' from event;
select 'insert into autoactivacion (id,port,bit,ultimo) values (''',id,''',''',port,''',''',bit,''',''',ultimo,''');' from autoactivacion;
select 'insert into auto_event (id,dst_cmd,dst_param1,dst_param2,flag,flag_condicion,flag_valor,dst_group,enable,event_name) values (''',id,''',''',dst_cmd,''',''',dst_param1,''',''',dst_param2,''',''',flag,''',''',flag_condicion,''',''',flag_valor,''',''',dst_group,''',''',enable,''',''',event_name,''');' from auto_event;
select 'insert into home_config (id,autoactivacion_alarma,autoreactivacion_alarma) values (''',id,''',''',autoactivacion_alarma,''',''',autoreactivacion_alarma,''');' from home_config;
select 'insert into home_flag (nombre,valor,descripcion) values (''',nombre,''',''',valor,''',''',descripcion,''');' from home_flag;
select 'create sequence tarea_programada_seq start ', nextval('tarea_programada_seq'),';';
select 'insert into tarea_programada (id,dia_semana,hora,minuto,dst_cmd,dst_param1,dst_param2,flag,flag_condicion,flag_valor,dst_group,enable,task_name,last_exec) values (''',id,''',''',dia_semana,''',''',hora,''',''',minuto,''',''',dst_cmd,''',''',dst_param1,''',''',dst_param2,''',''',flag,''',''',flag_condicion,''',''',flag_valor,''',''',dst_group,''',''',enable,''',''',task_name,''',''',last_exec,''');' from tarea_programada;
select 'insert into pais (codigo,moneda,sigla,abrev,nombre) values (''',codigo,''',''',moneda,''',''',sigla,''',''',abrev,''',''',nombre,''');' from pais;
select 'insert into persona (id,apellido,nombre,dia,mes,anio,mail,postal,sms,telefono,pais_nacimiento,pais_radicacion,password,user_name) values (''',id,''',''',apellido,''',''',nombre,''',''',dia,''',''',mes,''',''',anio,''',''',mail,''',''',postal,''',''',sms,''',''',telefono,''',''',pais_nacimiento,''',''',pais_radicacion,''',''',password,''',''',user_name,''');' from persona;
select 'insert into vinculo (padre,madre,hijo) values (''',padre,''',''',madre,''',''',hijo,''');' from vinculo;
\q

