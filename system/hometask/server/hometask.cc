/***************************************************************************
    Copyright (C) 2007   Walter Pirri

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 ***************************************************************************/
#include "hometask.h"

#include <string>
#include <iostream>
#include <csignal>
using namespace std;

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <syslog.h>
#include <time.h>
#include <string.h>

#include <gmonitor/svcstru.h>

CHomeTask::CHomeTask()
{
  m_pDB = NULL;
  m_timer1_id = 0;
  m_timer2_id = 0;
  m_timer3_id = 0;
  m_desactivacion_time = 0l;
}

CHomeTask::~CHomeTask()
{
  if(m_pDB) PQfinish(m_pDB);  
}

int CHomeTask::Init(CGMServer *pserver, CGLog *plog)
{
  int i;
  char *dbhost = NULL;
  char *dbname = NULL;
  char *dbuser = NULL;
  char *dbpass = NULL;

  m_pServer = pserver;
  m_pLog = plog;

  for(i = 1; i < m_pServer->m_arg_cnt; i++)
  {
    if( !strcmp(m_pServer->m_arg_ptr[i], "--dbhost"))
    {
      i++;
      dbhost = m_pServer->m_arg_ptr[i];
    }
    else if( !strcmp(m_pServer->m_arg_ptr[i], "--dbname"))
    {
      i++;
      dbname = m_pServer->m_arg_ptr[i];
    }
    else if( !strcmp(m_pServer->m_arg_ptr[i], "--dbuser"))
    {
      i++;
      dbuser = m_pServer->m_arg_ptr[i];
    }
    else if( !strcmp(m_pServer->m_arg_ptr[i], "--dbpass"))
    {
      i++;
      dbpass = m_pServer->m_arg_ptr[i];
    }
  }
  
  sprintf(m_str_conn, "host=%s dbname=%s user=%s password=%s", 
          (dbhost)?dbhost:"localhost",
          (dbname)?dbname:"home",
          (dbuser)?dbuser:"home",
          (dbpass)?dbpass:"home" );

  if(ReConnect() != GME_OK) return GME_UNDEFINED;

  LoadConfig();

  m_startup_time = time(&m_startup_time);
  m_startup_time += 60;
  
  m_pLog->Add(1, "CHomeTask Init Ok");

  return GME_OK;
}

int CHomeTask::ReConnect(void)
{
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: CHomeTask::ReConnect()");

  if(m_pDB)
  {
    PQfinish(m_pDB);  
    sleep(3);
  }
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Conectando a la base de datos...");
  m_pDB = PQconnectdb(m_str_conn);
  if(PQstatus(m_pDB) != CONNECTION_OK)
  {
    m_pLog->Add(GLOG_LEVEL_ERROR,
                "ERROR: Al conectarse a la base de datos [%s].",
                PQerrorMessage(m_pDB));
    return GME_UNDEFINED;
  }
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Conexion a la base de datos OK.");
  return GME_OK;
}

int CHomeTask::LoadConfig()
{
  char s_query[1024];
  PGresult  *res;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: CHomeTask::LoadConfig()");

  LoadTimer();

  /* leo lo parametros de configuracion */
  sprintf(s_query, "SELECT autoactivacion_alarma,autoreactivacion_alarma FROM home_config WHERE id = 0");
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
  res = PQexec(m_pDB, s_query);
  if(PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Error en query, intentando reconectar...");
    if(ReConnect() != GME_OK) return -1;
    res = PQexec(m_pDB, s_query);
    if(PQresultStatus(res) != PGRES_TUPLES_OK) return -1;
  }
  if(PQntuples(res) == 1)
  {
    m_enable_autoactivacion = atoi(PQgetvalue(res, 0, 0));
    m_enable_autoreactivacion = atoi(PQgetvalue(res, 0, 1));

    if(m_enable_autoactivacion)
      m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: AutoActivacion ON");
    else
      m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: AutoActivacion OFF");

    if(m_enable_autoreactivacion)
      m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: AutoReactivacion ON");
    else
      m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: AutoReactivacion OFF");
  }
  /* leo donde tengo que mandar el comando para activar la alarma */
  sprintf(s_query, 
    "SELECT dst_cmd, dst_param1, dst_param2, dst_group, flag, flag_condicion, flag_valor "
    "FROM auto_event "
    "WHERE id = 'autoactivacion' AND enable > 0");
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
  res = PQexec(m_pDB, s_query);
  if(PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Error en query, intentando reconectar...");
    if(ReConnect() != GME_OK) return -1;
    res = PQexec(m_pDB, s_query);
    if(PQresultStatus(res) != PGRES_TUPLES_OK) return -1;
  }
  if(PQntuples(res) == 1)
  {
    m_act_dst_cmd        = PQgetvalue(res, 0, 0);
    m_act_dst_param1     = PQgetvalue(res, 0, 1);
    m_act_dst_param2     = PQgetvalue(res, 0, 2);
    m_act_dst_group      = PQgetvalue(res, 0, 3);
    m_act_flag           = PQgetvalue(res, 0, 4);
    m_act_flag_condicion = PQgetvalue(res, 0, 5);
    m_act_flag_valor     = PQgetvalue(res, 0, 6);
  }
  else
  {
    m_pLog->Add(GLOG_LEVEL_WARNING, "WARNING: AutoActivacion OFF (por falta de parametros)");
    m_pLog->Add(GLOG_LEVEL_WARNING, "WARNING: AutoReactivacion OFF (por falta de parametros)");
    m_enable_autoactivacion = 0;
    m_enable_autoreactivacion = 0;
  }

  InitAutoActivacion();

  return 0;
}

int CHomeTask::LoadTimer()
{
  char  s_query[1024];
  PGresult  *res;
  int num_line;
  char  *id, *dia_semana, *hora, *minuto, *last_exec;
  char  *dst_cmd, *dst_param1, *dst_param2, *flag;
  char  *flag_condicion, *flag_valor, *dst_group;

  time_t t, next_t, task_time_t;
  long next_t_id = -1;
  struct tm *task_time;

  int rc;
  ST_STIMER *timer_data;
  char  timer_data_s[sizeof(ST_STIMER)+sizeof(long)];
  timer_data = (ST_STIMER*)&timer_data_s[0];
  CGMServerBase::GMIOS rc_timer_data;
  
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: CHomeTask::LoadTimer()");

  t = time(&t);
  next_t = 0;
  sprintf(s_query, "SELECT id, dia_semana, hora, minuto, last_exec, "
           "dst_cmd, dst_param1, dst_param2, flag, flag_condicion, "
           "flag_valor, dst_group FROM tarea_programada WHERE enable > 0");
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
  res = PQexec(m_pDB, s_query);
  if(PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "Error en query, intentando reconectar...");
    if(ReConnect() != GME_OK) return -1;
    res = PQexec(m_pDB, s_query);
    if(PQresultStatus(res) != PGRES_TUPLES_OK) return -1;
  }
  for(num_line = 0; num_line < PQntuples(res); num_line++)
  {
    id = PQgetvalue(res, num_line, 0);
    dia_semana = PQgetvalue(res, num_line, 1);
    hora = PQgetvalue(res, num_line, 2);
    minuto = PQgetvalue(res, num_line, 3);
    last_exec = PQgetvalue(res, num_line, 4);

    dst_cmd = PQgetvalue(res, num_line, 5);
    dst_param1 = PQgetvalue(res, num_line, 6);
    dst_param2 = PQgetvalue(res, num_line, 7);
    flag = PQgetvalue(res, num_line, 8);
    flag_condicion = PQgetvalue(res, num_line, 9);
    flag_valor = PQgetvalue(res, num_line, 10);
    dst_group = PQgetvalue(res, num_line, 11);
    /* armo la estructura tm usando como base la hora actual y modificando
        horas y minutos */
    task_time = localtime(&t);
    task_time->tm_sec = 0;
    task_time->tm_min = atoi(minuto);
    task_time->tm_hour = atoi(hora);
    task_time_t = mktime(task_time);
    m_pLog->Add(GLOG_LEVEL_DEBUG, "TAREA: %s %02i:%02i cmd: %s grp:%s",
                id, atoi(hora), atoi(minuto), dst_cmd, dst_group);
    /* busco el primer momento posterior a la ultima ejecución */
    if(last_exec && strlen(last_exec))
    {
      while( task_time_t < atol(last_exec) || !WDayExist(task_time->tm_wday, atoi(dia_semana)) )
      {
        task_time = localtime(&task_time_t);
        task_time->tm_mday++;
        task_time_t = mktime(task_time);
      }
      m_pLog->Add(GLOG_LEVEL_DEBUG, "Proxima ejecucion: %s", ctime(&task_time_t));
    }
    else
    {
      last_exec = NULL;
      m_pLog->Add(GLOG_LEVEL_DEBUG, "Tarea nunca ejecutada"); 
      m_pLog->Add(GLOG_LEVEL_DEBUG, "Proxima ejecucion: AHORA");
    }
    /* verifico si el evento no debería haber ocurrido ya */
    if(task_time_t <= t || last_exec == NULL)
    {
      m_pLog->Add(GLOG_LEVEL_DEBUG, "Ejecutando tarea [%s]", id); 
      /* debería ejecutarse ahora */
      ExecCmd(dst_cmd, dst_param1, dst_param2, flag, flag_condicion, flag_valor, dst_group);
      /* y actualizar last_exec */
      MakeExecuted(atol(id));
      /* busco el siguiente */
      /* incluyo en la busqueda del proximo tambien al que acabo de ejacutar */
      while( task_time_t < t || !WDayExist(task_time->tm_wday, atoi(dia_semana)) )
      {
        task_time = localtime(&task_time_t);
        task_time->tm_mday++;
        task_time_t = mktime(task_time);
      }
      if(next_t == 0 || task_time_t < next_t)
      {
        next_t = task_time_t;
        next_t_id = atol(id);
      }
    }
    else
    {
      /* entre los que faltan ejecutar busco el mas cercano */
      if(next_t == 0 || task_time_t < next_t)
      {
        next_t = task_time_t;
        next_t_id = atol(id);
      }
    }
  }
  PQclear(res);

  m_pLog->Add(GLOG_LEVEL_DEBUG, "Proxima tarea: id: %li %s", next_t_id, ctime(&next_t));

  if(next_t > 0)
  {
    /* seteo un timer para hometask-timer-event */
    strcpy(timer_data->set_timer.servicio, "hometask-timer-event");
    timer_data->set_timer.modo_servicio = GM_MSG_TYPE_MSG;
    timer_data->set_timer.delay = 0;
    timer_data->set_timer.at = next_t;
    timer_data->set_timer.tipo_timer = 'U'; /* Unico (no repetitivo) */
    timer_data->set_timer.len = sizeof(long);
    memcpy(&timer_data->set_timer.data[0], &next_t_id, sizeof(long));
    m_pLog->Add(GLOG_LEVEL_DEBUG, "Se levanta un timer para hometask-timer-event");
    if((rc = m_pServer->Call(".set_timer",
          timer_data, sizeof(ST_STIMER)+sizeof(long), &rc_timer_data, 500)) == GME_OK)
    {
      memcpy(timer_data, rc_timer_data.data, sizeof(ST_STIMER));
      m_timer1_id = timer_data->set_timer.id;
      m_pServer->Free(rc_timer_data);
    }
    else
    {
      /* no hay mas id's disponibles */
      m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Error %i al setear el timer para hometask-timer-event", rc);
    }
  }
  return 0;
}

int CHomeTask::Process(const char *funcion, char typ,
      void* in, unsigned long inlen,
      void** out, unsigned long *outlen)
{
  EVENT_NOTIF *ev = (EVENT_NOTIF*)in;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: CHomeTask::Process(%s, %c, ...)", funcion, typ);

  if( !strcmp(funcion, "hometask-timer-event"))
  {
    LoadTimer();
    return GME_OK;
  }
  else if( !strcmp(funcion, "hometask-change-config"))
  {
    if(m_timer1_id)
    {
      KillTimerId(m_timer1_id);
      m_timer1_id = 0;
    }
    LoadConfig();
    return GME_OK;
  }
  else if( !strcmp(funcion, "port-change"))
  {
    if( CheckAutoActivacion(ev) != 0)
    {
      m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR: CheckAutoActivacion termino con error");
    }
    if( CheckAutoReactivacion(ev) != 0)
    {
      m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR: CheckAutoReactivacion termino con error");
    }
    return GME_OK;
  }
  else if( !strcmp(funcion, "hometask-auto-event"))
  {
    ExecAutoActivacion();
    return GME_OK;
  }
  else if( !strcmp(funcion, "hometask-re-auto-event"))
  {
    ExecAutoReactivacion();
    return GME_OK;
  }
  return GME_SVC_UNRESOLVED;
}

int CHomeTask::ExecCmd(char *dst_cmd, char *dst_param1, char *dst_param2, char *flag,
            char *flag_condicion, char *flag_valor, char *dst_group)
{
  char  s_query[1024];
  PGresult  *res_flag;
  PGresult  *res_group;
  char *home_flag_valor;
  int cmd;
  int  id, bit;
  int group_line;
  char *port;
  char  data[256];

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: CHomeTask::ExecCmd(...)");

  /* controlo el flag que regula el tratamiento de este evento, si lo hay */
  if(flag)
  {
    if( strlen(flag) && strcmp(flag, "siempre"))
    {
      sprintf(s_query, "SELECT valor FROM home_flag WHERE nombre = '%s'", flag);
      m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
      res_flag = PQexec(m_pDB, s_query);
      if(PQresultStatus(res_flag) == PGRES_TUPLES_OK)
      {
        if(PQntuples(res_flag))
        {
          home_flag_valor = PQgetvalue(res_flag, 0, 0);
          if(home_flag_valor)
          {
          /* condicion
              1     ==
              2     >
              3     <
              4     >=
              5     <=
              6     <> */
            switch(atoi(flag_condicion))
            {
            case 1:
              if( strcmp(home_flag_valor, flag_valor)) return 0;
              break;
            case 2:
              if( strcmp(home_flag_valor, flag_valor) <= 0) return 0;
              break;
            case 3:
              if( strcmp(home_flag_valor, flag_valor) >= 0) return 0;
              break;
            case 4:
              if( strcmp(home_flag_valor, flag_valor) < 0) return 0;
              break;
            case 5:
              if( strcmp(home_flag_valor, flag_valor) > 0) return 0;
              break;
            case 6:
              if( !strcmp(home_flag_valor, flag_valor)) return 0;
              break;
            }
          }
        }
      }
      PQclear(res_flag);
    }
  }
  cmd = atoi(dst_cmd);
  /* una vez controlado el flag procedo según el comando */
  if(cmd == 5 || cmd == 6 || cmd == 7 || cmd == 8 || cmd == 9 || cmd == 10)
  {
    /* comandos que no necesitan hacer consultas a la base */
    switch(cmd)
    {
      case 9: /* Set Flag */
        sprintf(s_query, "UPDATE home_flag SET valor = '%s' WHERE nombre = '%s'", dst_param2, dst_param1);
        m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
        PQclear( PQexec(m_pDB, s_query) );
        break;
      case 10: /* Exec */
        sprintf(s_query, "%s %s", dst_param1, dst_param2);
        m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Ejecutando %s", s_query);
        if( system(s_query) < 0)
          m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Al ejecutar %s", s_query);
        break;
    }
  }
  else
  {
    if(dst_group)
    {
      if(strlen(dst_group))
      {
        /* comandos que requieren consulta a la tabla de grupos */
        sprintf(s_query, "SELECT id, port, bit FROM group_rel WHERE group_id = %i", atoi(dst_group));
        m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
        res_group = PQexec(m_pDB, s_query);
        if(PQresultStatus(res_group) == PGRES_TUPLES_OK)
        {
          for(group_line = 0; group_line < PQntuples(res_group); group_line++)
          {
            id   = atoi(PQgetvalue(res_group, group_line, 0));
            port = PQgetvalue(res_group, group_line, 1);
            bit  = atoi(PQgetvalue(res_group, group_line, 2));

            switch(cmd)
            {
              case 1:   /* Set */
                sprintf(data, "%03i %03i 001", id, bit);
                if( !memcmp(port, "O1", 2))
                {
                  m_pLog->Add(1, "ACCION: set dev: %03i port: Salida1 bit: %i.", id, bit);
                  m_pServer->Notify("sercmd-s1-bit-on-off", data, strlen(data)+1);
                }
                else if( !memcmp(port, "O2", 2))
                {
                  m_pLog->Add(1, "ACCION: set dev: %03i port: Salida2 bit: %i.", id, bit);
                  m_pServer->Notify("sercmd-s2-bit-on-off", data, strlen(data)+1);
                }
                break;
              case 2:   /* Reset */
                sprintf(data, "%03i %03i 000", id, bit);
                if( !memcmp(port, "O1", 2))
                {
                  m_pLog->Add(1, "ACCION: reset dev: %03i port: Salida1 bit: %i.", id, bit);
                  m_pServer->Notify("sercmd-s1-bit-on-off", data, strlen(data)+1);
                }
                else if( !memcmp(port, "O2", 2))
                {
                  m_pLog->Add(1, "ACCION: reset dev: %03i port: Salida2 bit: %i.", id, bit);
                  m_pServer->Notify("sercmd-s2-bit-on-off", data, strlen(data)+1);
                }
                break;
              case 3:   /* Pulse */
                sprintf(data, "%03i %03i %03i", id, bit, (int)((atoi(dst_param1) > 0)?atoi(dst_param1):1) );
                if( !memcmp(port, "O1", 2))
                {
                  m_pLog->Add(1, "ACCION: pulse dev: %03i port: Salida1 bit: %i.", id, bit);
                  m_pServer->Notify("serpulse-s1-bit", data, strlen(data)+1);
                }
                else if( !memcmp(port, "O2", 2))
                {
                  m_pLog->Add(1, "ACCION: pulse dev: %03i port: Salida2 bit: %i.", id, bit);
                  m_pServer->Notify("serpulse-s2-bit", data, strlen(data)+1);
                }
                break;
              case 4:   /* Invert */
                sprintf(data, "%03i %03i", id, bit);
                if( !memcmp(port, "O1", 2))
                {
                  m_pLog->Add(1, "ACCION: invert dev: %03i port: Salida1 bit: %i.", id, bit);
                  m_pServer->Notify("serinvert-s1-bit", data, strlen(data)+1);
                }
                else if( !memcmp(port, "O2", 2))
                {
                  m_pLog->Add(1, "ACCION: invert dev: %03i port: Salida2 bit: %i.", id, bit);
                  m_pServer->Notify("serinvert-s2-bit", data, strlen(data)+1);
                }
                break;


            }
          }
        }
        PQclear(res_group);
      }
    }
  }
  return 0;
}

int CHomeTask::MakeExecuted(long id)
{
  char  s_query[1024];
  time_t t;

  t = time(&t);

  sprintf(s_query, "UPDATE tarea_programada "
          "SET last_exec = %li WHERE id = %li", (long)t, id);
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
  PQclear( PQexec(m_pDB, s_query) );
  sleep(1);
  return 0;
}

void CHomeTask::KillTimerId(unsigned int id)
{
  ST_STIMER timer_data;

  if(id >= 0)
  {
    timer_data.kill_timer.id = id;
    m_pServer->Notify(".kill_timer", &timer_data, sizeof(ST_STIMER));
  }
}

bool CHomeTask::WDayExist(int tm_wday, int db_wday)
{
  switch (tm_wday)
  {
    case 0:         /* domingo */
      if(db_wday & 64) return true;
      break;
    case 1:         /* lunes */
      if(db_wday & 1) return true;
      break;
    case 2:         /* martes */
      if(db_wday & 2) return true;
      break;
    case 3:         /* miercoles */
      if(db_wday & 4) return true;
      break;
    case 4:         /* jueves */
      if(db_wday & 8) return true;
      break;
    case 5:         /* viernes */
      if(db_wday & 16) return true;
      break;
    case 6:         /* sábado */
      if(db_wday & 32) return true;
      break;
    default:
      break;
  }
  return false;
}

int CHomeTask::InitAutoActivacion()
{
  char s_query[1024];
  PGresult  *res;
  int num_line;
  char *id, *port, *bit, *ultimo;
  int i;
  int rc = 0;
  
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: CHomeTask::InitAutoActivacion()");

  memset(m_DemasSensores, 0, sizeof(EVENT_NOTIF)*256);

  sprintf(s_query, "SELECT id, port, bit, ultimo FROM autoactivacion");
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
  res = PQexec(m_pDB, s_query);
  if(PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Error en query, intentando reconectar...");
    if(ReConnect() != GME_OK) return -1;
    res = PQexec(m_pDB, s_query);
    if(PQresultStatus(res) != PGRES_TUPLES_OK) return -1;
  }
  for(num_line = 0; num_line < PQntuples(res); num_line++)
  {
    id = PQgetvalue(res, num_line, 0);
    port = PQgetvalue(res, num_line, 1);
    bit = PQgetvalue(res, num_line, 2);
    ultimo = PQgetvalue(res, num_line, 3);
    m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: DATOS: id= %s  port= %s  bit= %s  ultimo= %s",
                id, port, bit, ultimo);
    if( !strcmp(ultimo, "1"))
    {
      m_UltimoSensor.address = atoi(id);
      memcpy(m_UltimoSensor.port, port, 2);
      m_UltimoSensor.bit = atoi(bit);
      m_UltimoSensor.time = 0;
    }
    else
    {
      for(i = 0; i < 256 && m_DemasSensores[i].address != 0; i++);
      if(i < 256)
      {
        m_DemasSensores[i].address = atoi(id);
        memcpy(m_DemasSensores[i].port, port, 2);
        m_DemasSensores[i].bit = atoi(bit);
        m_DemasSensores[0].time = 0;
      }
      else
      {
        m_pLog->Add(GLOG_LEVEL_WARNING, "WARNING: Se supero la cantidad de sensores a monitorear en AUTOACTIVACION");
        rc = -2;
      }
    }
  }
  return rc;
}

int CHomeTask::CheckAutoActivacion(EVENT_NOTIF* ev)
{
  int i;
  int rc;
  ST_STIMER timer_data;
  CGMServerBase::GMIOS rc_timer_data;

  /* valido algunos datos */
  if(ev->address == 0 || ev->address >= 254) return -1; /* direccion invalida */
  if(ev->time == 0) return -1; /* hora invalida */
  if(ev->on_off == 1) return 0; /* capturo solamente los apagados */

  if(m_enable_autoactivacion == 0) return 0;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: CHomeTask::CheckAutoActivacion(...)");

  /* me fijo si el evento corresponde al ultimo sensor */
  if( ev->address == m_UltimoSensor.address &&
      !memcmp(ev->port, m_UltimoSensor.port, 2) &&
      ev->bit == m_UltimoSensor.bit)
  {
    m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: El evento corresponde al ultimo sensor");
    if(m_UltimoSensor.time > 0l)
    {
      m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: El ultimo sensor tenia un timer y se elimina");
      KillTimerId(m_timer2_id);
      m_timer2_id = 0;
    }
    m_UltimoSensor.time = ev->time;
    /* seteo un timer para hometask-auto-event */
    strcpy(timer_data.set_timer.servicio, "hometask-auto-event");
    timer_data.set_timer.modo_servicio = GM_MSG_TYPE_MSG;
    timer_data.set_timer.delay = 0;
    timer_data.set_timer.at = ev->time + AUTOACTIVACION_DELAY;
    timer_data.set_timer.tipo_timer = 'U'; /* Unico (no repetitivo) */
    timer_data.set_timer.len = 0l;
    m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Se levanta un timer para hometask-auto-event");
    if((rc = m_pServer->Call(".set_timer",
          &timer_data, sizeof(ST_STIMER), &rc_timer_data, 500)) == GME_OK)
    {
      memcpy(&timer_data, rc_timer_data.data, sizeof(ST_STIMER));
      m_timer2_id = timer_data.set_timer.id;
      m_pServer->Free(rc_timer_data);
    }
    else
    {
      /* no hay mas id's disponibles */
      m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Error %i al setear el timer para hometask-auto-event", rc);
    }
  }
  /* si no corresponde al ultimo me fijo si corresponde a alguno de los monitoreados */
  else
  {
    for(i = 0; i < 256; i++)
    {
      if(m_DemasSensores[i].address == 0) break;
      if( ev->address == m_DemasSensores[i].address &&
          !memcmp(ev->port, m_DemasSensores[i].port, 2) &&
          ev->bit == m_DemasSensores[i].bit)
      {
        m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: El evento corresponde a un sensor que no es el ultimo");
        /* el evento corresponde a otro de la lista */
        if(m_UltimoSensor.time > 0l)
        {
          m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: El ultimo sensor tenia un timer y se elimina");
          KillTimerId(m_timer2_id);
          m_timer2_id = 0;
          m_UltimoSensor.time = 0l;
        }
      }
    }
  }
  return 0;
}

int CHomeTask::ExecAutoActivacion()
{
  time_t t;
  char s_query[1024];
  PGresult  *res;
  char *valor;
  int rc;
  ST_STIMER timer_data;
  CGMServerBase::GMIOS rc_timer_data;

  if(m_enable_autoactivacion == 0) return 0;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: CHomeTask::ExecAutoActivacion()");

  /* hay que verificar si es necesario activar */
  if(m_UltimoSensor.time == 0l) return 0;
  t = time(&t);
  if( (t - m_UltimoSensor.time) < AUTOACTIVACION_DELAY ) return 0;
  /* verifico el estado de la alarma */
  sprintf(s_query, "SELECT valor FROM home_flag WHERE nombre = 'alarma'");
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
  res = PQexec(m_pDB, s_query);
  if(PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Error en query, intentando reconectar...");
    if(ReConnect() != GME_OK) return -1;
    res = PQexec(m_pDB, s_query);
    if(PQresultStatus(res) != PGRES_TUPLES_OK) return -1;
  }
  if(PQntuples(res) == 1)
  {
    valor = PQgetvalue(res, 0, 0);
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Estado de alarma: %s", valor);
    if( !strcmp(valor, "activada"))
    {
      /* si la alarma ya está activada --> listo */
      m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: La alarma ya estaba activada");
      m_UltimoSensor.time = 0l;
    }
    else
    {
      /* si la alarma no está activada mando un comando para activarla y */
      m_pLog->Add(1, "INFO: Activacion automatica de alarma");
      EnviarComandoActivacion();
      /* seteo un timer para verificar la activacion */
      strcpy(timer_data.set_timer.servicio, "hometask-auto-event");
      timer_data.set_timer.modo_servicio = GM_MSG_TYPE_MSG;
      timer_data.set_timer.delay = 0;
      timer_data.set_timer.at = t + 60;
      timer_data.set_timer.tipo_timer = 'U'; /* Unico (no repetitivo) */
      timer_data.set_timer.len = 0l;
      m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Se levanta un timer para hometask-auto-event, para verificar la activacion");
      if((rc = m_pServer->Call(".set_timer",
            &timer_data, sizeof(ST_STIMER), &rc_timer_data, 500)) == GME_OK)
      {
        memcpy(&timer_data, rc_timer_data.data, sizeof(ST_STIMER));
        m_timer2_id = timer_data.set_timer.id;
        m_pServer->Free(rc_timer_data);
      }
      else
      {
        /* no hay mas id's disponibles */
        m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Error %i al setear el timer para hometask-auto-event", rc);
      }
    }
  }
  return 0;
}

int CHomeTask::CheckAutoReactivacion(EVENT_NOTIF* ev)
{
  int rc;
  int found = 0;
  ST_STIMER timer_data;
  CGMServerBase::GMIOS rc_timer_data;
  int i;
  time_t time_now;

  /* valido algunos datos */
  if(ev->address == 0 || ev->address >= 254) return -1; /* direccion invalida */
  if(ev->time == 0) return -1; /* hora invalida */

  if(m_enable_autoreactivacion == 0) return 0;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: CHomeTask::CheckAutoReactivacion()");

  time_now = time(&time_now);
  /* me fijo si el evento corresponde a algun sensor */
  if( ev->address == m_UltimoSensor.address &&
      !memcmp(ev->port, m_UltimoSensor.port, 2) &&
      ev->bit == m_UltimoSensor.bit)
  {
    m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: El evento corresponde a un sensor");
    found = 1;
  }
  else
  {
    for(i = 0; i < 256; i++)
    {
      if(m_DemasSensores[i].address == 0) break;
      if( ev->address == m_DemasSensores[i].address &&
          !memcmp(ev->port, m_DemasSensores[i].port, 2) &&
          ev->bit == m_DemasSensores[i].bit)
      {
        m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: El evento corresponde a un sensor");
        found = 1;
        break;
      }
    }
  }

  if(found )
  {
    /* el evento corresponde a un sensor de la lista */
    m_desactivacion_time = 0l;
    /* si ya hay un timer dando vuelta lo borro porque hubo movimiento */
    if(m_timer3_id)
    {
      m_pLog->Add(1, "INFO: Movimiento detectado en alguno de los sensores, se cancela la autoreactivacion");
      KillTimerId(m_timer3_id);
      m_timer3_id = 0;
    }
  }
  /* Para que no entre a ver cambios en la alarma durante el primer minuto */
  else if(m_startup_time < time_now)
  {
    /* me fijo si el evento corresponde a la desactivacion de la alarma */
    /* Direccion de la entrada de activacion / desactivacion de la alarma */
    if( ev->address == 1 &&
        !memcmp(ev->port, "I1", 2) &&
        ev->bit == 4 &&
        ev->on_off == 1 )
    {
      m_pLog->Add(1, "INFO: Alarma desactivada, preparando reactivacion automatica");
      m_desactivacion_time = time(&m_desactivacion_time);
      /* si ya hay un timer dando vuelta lo borro antes de crear uno nuevo */
      if(m_timer3_id)
      {
        KillTimerId(m_timer3_id);
        m_timer3_id = 0;
      }
      /* doy de alta un nuevo timer */
      /* seteo un timer para hometask-reauto-event */
      strcpy(timer_data.set_timer.servicio, "hometask-re-auto-event");
      timer_data.set_timer.modo_servicio = GM_MSG_TYPE_MSG;
      timer_data.set_timer.delay = 0;
      timer_data.set_timer.at = ev->time + AUTOREACTIVACION_DELAY;
      timer_data.set_timer.tipo_timer = 'U'; /* Unico (no repetitivo) */
      timer_data.set_timer.len = 0l;
      m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Se levanta un timer para hometask-re-auto-event");
      if((rc = m_pServer->Call(".set_timer",
            &timer_data, sizeof(ST_STIMER), &rc_timer_data, 500)) == GME_OK)
      {
        memcpy(&timer_data, rc_timer_data.data, sizeof(ST_STIMER));
        m_timer3_id = timer_data.set_timer.id;
        m_pServer->Free(rc_timer_data);
      }
      else
      {
        /* no hay mas id's disponibles */
        m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Error %i al setear el timer para hometask-re-auto-event", rc);
      }
    }
    /* me fijo si el evento corresponde a la activacion de la alarma */
    /* Direccion de la entrada de activacion / desactivacion de la alarma */
    else if( ev->address == 1 &&
            !memcmp(ev->port, "I1", 2) &&
            ev->bit == 4 &&
            ev->on_off == 0 )
    {
      m_pLog->Add(1, "INFO: Se activo la alarma");
      m_desactivacion_time = 0l;
      /* si ya hay un timer dando vuelta lo borro porque hubo movimiento */
      if(m_timer3_id)
      {
        m_pLog->Add(1, "INFO: Alarma activada, se cancela la autoreactivacion");
        KillTimerId(m_timer3_id);
        m_timer3_id = 0;
      }
    }
  }
  return 0;
}

int CHomeTask::ExecAutoReactivacion()
{
  time_t t;
  char s_query[1024];
  PGresult  *res;
  char *valor;
  int rc;
  ST_STIMER timer_data;
  CGMServerBase::GMIOS rc_timer_data;

  if(m_enable_autoreactivacion == 0) return 0;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: CHomeTask::ExecAutoReactivacion()");

  /* si la hora de desactivacion esta en cero es porque hubo movimiento */
  if(m_desactivacion_time == 0) return 0;
  /* verifico que no sea un timer mal borrado */
  if( !m_timer3_id) return 0;
  /* verifico que no sea un timer perdido */
  t = time(&t);
  if( (t - m_desactivacion_time) < AUTOREACTIVACION_DELAY ) return 0;
  /* verifico el estado de la alarma */
  sprintf(s_query, "SELECT valor FROM home_flag WHERE nombre = 'alarma'");
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
  res = PQexec(m_pDB, s_query);
  if(PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Error en query, intentando reconectar...");
    if(ReConnect() != GME_OK) return -1;
    res = PQexec(m_pDB, s_query);
    if(PQresultStatus(res) != PGRES_TUPLES_OK) return -1;
  }
  if(PQntuples(res) == 1)
  {
    valor = PQgetvalue(res, 0, 0);
    m_pLog->Add(GLOG_LEVEL_DEBUG, "Estado de alarma: %s", valor);
    if( !strcmp(valor, "activada"))
    {
      /* si la alarma ya está activada --> listo */
      m_desactivacion_time = 0l;
    }
    else
    {
      /* si la alarma no está activada mando un comando para activarla y */
      m_pLog->Add(1, "INFO: Reactivacion de alarma");
      EnviarComandoActivacion(1);
      /* seteo un timer para verificar la activacion */
      strcpy(timer_data.set_timer.servicio, "hometask-re-auto-event");
      timer_data.set_timer.modo_servicio = GM_MSG_TYPE_MSG;
      timer_data.set_timer.delay = 0;
      timer_data.set_timer.at = t + 60;
      timer_data.set_timer.tipo_timer = 'U'; /* Unico (no repetitivo) */
      timer_data.set_timer.len = 0l;
      m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Se levanta un timer para hometask-re-auto-event");
      if((rc = m_pServer->Call(".set_timer",
            &timer_data, sizeof(ST_STIMER), &rc_timer_data, 500)) == GME_OK)
      {
        memcpy(&timer_data, rc_timer_data.data, sizeof(ST_STIMER));
        m_timer3_id = timer_data.set_timer.id;
        m_pServer->Free(rc_timer_data);
      }
      else
      {
        /* no hay mas id's disponibles */
        m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Error %i al setear el timer para hometask-auto-event", rc);
      }
    }
  }
  return 0;
}

int CHomeTask::EnviarComandoActivacion(int force)
{
  char s_query[1024];
  PGresult  *res_flag, *res_group;
  char  *home_flag_valor;
  int cmd;
  int  id, bit;
  char *port;
  int group_line;
  char  data[256];

  /* controlo el flag que regula el tratamiento de este evento, si lo hay */
  if(m_act_flag && !force)
  {
    if( strlen(m_act_flag) && strcmp(m_act_flag, "siempre"))
    {
      sprintf(s_query, "SELECT valor FROM home_flag WHERE nombre = '%s'", m_act_flag);
      m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
      res_flag = PQexec(m_pDB, s_query);
      if(PQresultStatus(res_flag) == PGRES_TUPLES_OK)
      {
        if(PQntuples(res_flag))
        {
          home_flag_valor = PQgetvalue(res_flag, 0, 0);
          if(home_flag_valor)
          {
          /* condicion
              1     ==
              2     >
              3     <
              4     >=
              5     <=
              6     <> */
            switch(atoi(m_act_flag_condicion))
            {
            case 1:
              if( strcmp(home_flag_valor, m_act_flag_valor)) return 0;
              break;
            case 2:
              if( strcmp(home_flag_valor, m_act_flag_valor) <= 0) return 0;
              break;
            case 3:
              if( strcmp(home_flag_valor, m_act_flag_valor) >= 0) return 0;
              break;
            case 4:
              if( strcmp(home_flag_valor, m_act_flag_valor) < 0) return 0;
              break;
            case 5:
              if( strcmp(home_flag_valor, m_act_flag_valor) > 0) return 0;
              break;
            case 6:
              if( !strcmp(home_flag_valor, m_act_flag_valor)) return 0;
              break;
            }
          }
        }
      }
      PQclear(res_flag);
    }
  }
  cmd = atoi(m_act_dst_cmd);
  /* una vez controlado el flag procedo según el comando */
  if(cmd == 5 || cmd == 6 || cmd == 7 || cmd == 8 || cmd == 9)
  {
    /* comandos que no necesitan hacer consultas a la base */
    switch(cmd)
    {
      case 9: /* Set Flag */
        sprintf(s_query, "UPDATE home_flag SET valor = '%s' WHERE nombre = '%s'", m_act_dst_param2, m_act_dst_param1);
        m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
        PQclear( PQexec(m_pDB, s_query) );
        break;

    }
  }
  else
  {
    if(m_act_dst_group)
    {
      if(strlen(m_act_dst_group))
      {
        /* comandos que requieren consulta a la tabla de grupos */
        sprintf(s_query, "SELECT id, port, bit FROM group_rel WHERE group_id = %i", atoi(m_act_dst_group));
        m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", s_query);
        res_group = PQexec(m_pDB, s_query);
        if(PQresultStatus(res_group) == PGRES_TUPLES_OK)
        {
          for(group_line = 0; group_line < PQntuples(res_group); group_line++)
          {
            id   = atoi(PQgetvalue(res_group, group_line, 0));
            port = PQgetvalue(res_group, group_line, 1);
            bit  = atoi(PQgetvalue(res_group, group_line, 2));

            switch(cmd)
            {
              case 1:   /* Set */
                sprintf(data, "%03i %03i 001", id, bit);
                if( !memcmp(port, "O1", 2))
                {
                  m_pLog->Add(1, "ACCION: set dev: %03i port: Salida1 bit: %i.", id, bit);
                  m_pServer->Notify("sercmd-s1-bit-on-off", data, strlen(data)+1);
                }
                else if( !memcmp(port, "O2", 2))
                {
                  m_pLog->Add(1, "ACCION: set dev: %03i port: Salida2 bit: %i.", id, bit);
                  m_pServer->Notify("sercmd-s2-bit-on-off", data, strlen(data)+1);
                }
                break;
              case 2:   /* Reset */
                sprintf(data, "%03i %03i 000", id, bit);
                if( !memcmp(port, "O1", 2))
                {
                  m_pLog->Add(1, "ACCION: reset dev: %03i port: Salida1 bit: %i.", id, bit);
                  m_pServer->Notify("sercmd-s1-bit-on-off", data, strlen(data)+1);
                }
                else if( !memcmp(port, "O2", 2))
                {
                  m_pLog->Add(1, "ACCION: reset dev: %03i port: Salida2 bit: %i.", id, bit);
                  m_pServer->Notify("sercmd-s2-bit-on-off", data, strlen(data)+1);
                }
                break;
              case 3:   /* Pulse */
                sprintf(data, "%03i %03i %03i", id, bit, (int)((atoi(m_act_dst_param1) > 0)?atoi(m_act_dst_param1):1) );
                if( !memcmp(port, "O1", 2))
                {
                  m_pLog->Add(1, "ACCION: pulse dev: %03i port: Salida1 bit: %i.", id, bit);
                  m_pServer->Notify("serpulse-s1-bit", data, strlen(data)+1);
                }
                else if( !memcmp(port, "O2", 2))
                {
                  m_pLog->Add(1, "ACCION: pulse dev: %03i port: Salida2 bit: %i.", id, bit);
                  m_pServer->Notify("serpulse-s2-bit", data, strlen(data)+1);
                }
                break;
              case 4:   /* Invert */
                sprintf(data, "%03i %03i", id, bit);
                if( !memcmp(port, "O1", 2))
                {
                  m_pLog->Add(1, "ACCION: invert dev: %03i port: Salida1 bit: %i.", id, bit);
                  m_pServer->Notify("serinvert-s1-bit", data, strlen(data)+1);
                }
                else if( !memcmp(port, "O2", 2))
                {
                  m_pLog->Add(1, "ACCION: invert dev: %03i port: Salida2 bit: %i.", id, bit);
                  m_pServer->Notify("serinvert-s2-bit", data, strlen(data)+1);
                }
                break;


            }
          }
        }
        PQclear(res_group);
      }
    }
  }
  return 0;
}

