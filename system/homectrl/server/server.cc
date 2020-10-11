/***************************************************************************
    Copyright (C) 2003   Walter Pirri

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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <gmonitor/gmerror.h>
#include <gmonitor/gms.h>
#include <gmonitor/svcstru.h>

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <string>
#include <iostream>
#include <csignal>
#include <vector>
using namespace std;

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h> 
#include <fcntl.h>

#include <postgresql/libpq-fe.h>

#include "gmstring.h"

typedef struct _EVENT_NOTIF
{
  int     address;
  char    port[2];
  int     bit;
  int     on_off;
  long    time;
} EVENT_NOTIF;

/*
        La variable 'void* m_gptr' es un puntero para uso generico por el server
        es el unico puntero miembro de la clase que puede ser utilizado libremente
        al realizar el programa server.
        La variable 'CGMInitData m_ClientData' se completarï¿½con los valores del
        cliente que solicitï¿½el servicio antes del llamado a la funciï¿½ PreMain()
        y mantendrï¿½este valos para ser utilizado por el server si es necesario
        hasta el final del servicio.
*/
extern CGLog* pLog;
PGconn *g_postgres;
char g_str_conn[256];

long  g_time_fcn_auto_act;
bool  log_all_events;

CGMServer::CGMServer()
{

}

CGMServer::~CGMServer()
{

}

/* Colocar en esta funcion lo que se necesite correr al levantar el server */
int CGMServer::Init()
{
  CGMServerBase::GMIOS gmio;
  ST_STIMER pt;
  int rc;
  int i;
  char *dbhost = NULL;
  char *dbname = NULL;
  char *dbuser = NULL;
  char *dbpass = NULL;

  pLog->Add(1, "Iniciando server");
  if(Suscribe("port-change", GM_MSG_TYPE_MSG) != GME_OK)
    pLog->Add(1, "ERROR al suscribir a port-change");
  if(Suscribe("hardware-update", GM_MSG_TYPE_MSG) != GME_OK)
    pLog->Add(1, "ERROR al suscribir a hardware-update");

  g_time_fcn_auto_act = 0;
  log_all_events = false;

  for(i = 1; i < m_arg_cnt; i++)
  {
    if( !strcmp(m_arg_ptr[i], "--dbhost"))
    {
      i++;
      dbhost = m_arg_ptr[i];
    }
    else if( !strcmp(m_arg_ptr[i], "--dbname"))
    {
      i++;
      dbname = m_arg_ptr[i];
    }
    else if( !strcmp(m_arg_ptr[i], "--dbuser"))
    {
      i++;
      dbuser = m_arg_ptr[i];
    }
    else if( !strcmp(m_arg_ptr[i], "--dbpass"))
    {
      i++;
      dbpass = m_arg_ptr[i];
    }
  }

  sprintf(g_str_conn, "host=%s dbname=%s user=%s password=%s",
          (dbhost)?dbhost:"localhost",
          (dbname)?dbname:"home",
          (dbuser)?dbuser:"home",
          (dbpass)?dbpass:"home" );


  g_postgres = PQconnectdb(g_str_conn);
  if(PQstatus(g_postgres) != CONNECTION_OK)
  {
    pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Al conectarse a la base de datos [%s].", PQerrorMessage(g_postgres));
    return GME_UNDEFINED;
  }

  /* levanto un time para que unos minutos despues de iniciado el sistema se actialicen todos los estados */
  strcpy(pt.set_timer.servicio, "hardware-update");
  pt.set_timer.modo_servicio = GM_MSG_TYPE_MSG;
  pt.set_timer.delay = 300; /* 5 minutos */
  pt.set_timer.tipo_timer = 'U';
  pt.set_timer.len = 0;
  pLog->Add(100, "Se levanta un timer para hardware-update");
  rc = Call("hardware-update", &pt, sizeof(ST_STIMER), &gmio, 3000);
/*
  if(rc != GME_OK)
  {
    pLog->Add(1, "ERROR: Error %i al setear el timer para hardware-update", rc);
    return GME_UNDEFINED;
  }
*/

  return 0;
}

/* Colocar en esta funcion lo que se necesite correr al bajar el serer */
int CGMServer::Exit()
{
  pLog->Add(1, "Terminando server");
  UnSuscribe("port-change", GM_MSG_TYPE_MSG);
  UnSuscribe("hardware-update", GM_MSG_TYPE_MSG);
  PQfinish(g_postgres);
  return 0;
}

/* Estas rutinas son llamadas para el manejo de transaccion se debe colocar
  en ellas el cóigo necesario para cada uno de los procesos */
int CGMServer::BeginTrans(unsigned int trans) { return 0; }
int CGMServer::CommitTrans(unsigned int trans) { return 0; }
int CGMServer::RollbackTrans(unsigned int trans) { return 0; }

/* estas rutinas se llaman antes y después de la de procesamiento de mensaje */
int CGMServer::PreMain() { return 0; }
int CGMServer::PosMain() { return 0; }

/* Colocar en esta funcion el proceso que intepreta el mensaje recibido */
int CGMServer::Main(const char *funcion, char typ,
      void* in, unsigned long inlen,
      void** out, unsigned long *outlen)
{
  EVENT_NOTIF *ev = (EVENT_NOTIF*)in;
  PGresult   *res_event;
  PGresult   *res_group;
  PGresult   *res_flag;
  char  data[256];
  char  my_query[1024];
  int event_line, group_line;
  struct tm *p_stm;
  int cmd, reintentos = 0;

  char *dst_cmd, *dst_param1, *dst_param2, *dst_group, *flag, *flag_condicion, *flag_valor;
  char *home_flag_valor;
  int  id, bit;
  char *port;

  if(!strcmp(funcion, "hardware-update"))
  {
    /* hay que buscar todos los dispositivos testigo y actualizarles el estado */


  }
  else if(!strcmp(funcion, "port-change"))
  {
    p_stm = localtime(&ev->time);
    pLog->Add(GLOG_LEVEL_DEBUG, "EVENTO: port-change dev: %03i port: %2.2s bit: %i %s time %02i:%02i:%02i.",
                  ev->address, ev->port, ev->bit, ((ev->on_off)?"On":"Off"),
                  p_stm->tm_hour, p_stm->tm_min, p_stm->tm_sec );
    if(inlen < sizeof(EVENT_NOTIF))
    {
      pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Datos recibidos insuficientes para port-change.");
      return GME_UNDEFINED;
    }

    if(log_all_events)
    {
      /* agrego el evento al histórico */
      reintentos = 0;
      do
      {
        sprintf(my_query, 
          "INSERT INTO historico (id, port, bit, fecha_hora, estado) "
          "VALUES (%i, '%2.2s', %i, now(), %i)",
          (int)(ev->address&0xFF), ev->port, (int)(ev->bit&0xFF), (int)(ev->on_off&0x3FF) );
        pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", my_query);
        res_event = PQexec(g_postgres, my_query);
        if(PQresultStatus(res_event) != PGRES_COMMAND_OK)
        {
          /* intento de reconexión --> */
          if(++reintentos > 5) return GME_UNDEFINED;
          pLog->Add(GLOG_LEVEL_ERROR, "ERROR: [%s] en el query [%s].", PQerrorMessage(g_postgres), my_query);
          PQclear(res_event);
          PQfinish(g_postgres);
          sleep(5);
          g_postgres = PQconnectdb(g_str_conn);
          if(PQstatus(g_postgres) != CONNECTION_OK)
          {
            pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Al conectarse a la base de datos [%s].", PQerrorMessage(g_postgres));
            return GME_UNDEFINED;
          }
          continue;
          /* <-- intento de reconexión */
        }
        break;
      } while (true);
      PQclear(res_event);
    }


    /* Busco una acción para el evento */
    reintentos = 0;
    do
    {
      sprintf(my_query, 
        "SELECT dst_cmd, dst_param1, dst_param2, dst_group, flag, flag_condicion, flag_valor "
        "FROM event "
        "WHERE src_dev = %i AND src_port = '%2.2s' AND src_bit = %i AND src_on_off = %i AND enable > 0",
        ev->address, ev->port, ev->bit, ev->on_off);
      pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", my_query);
      res_event = PQexec(g_postgres, my_query);
      if(PQresultStatus(res_event) != PGRES_TUPLES_OK)
      {
        /* intento de reconexión --> */
        if(++reintentos > 5) return GME_UNDEFINED;
        pLog->Add(GLOG_LEVEL_ERROR, "ERROR: [%s] en el query [%s].", PQerrorMessage(g_postgres), my_query);
        PQclear(res_event);
        PQfinish(g_postgres);
        sleep(5);
        g_postgres = PQconnectdb(g_str_conn);
        if(PQstatus(g_postgres) != CONNECTION_OK)
        {
          pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Al conectarse a la base de datos [%s].", PQerrorMessage(g_postgres));
          return GME_UNDEFINED;
        }
        continue;
        /* <-- intento de reconexión */
      }
      break;
    } while (true);
    for(event_line = 0; event_line < PQntuples(res_event); event_line++)
    {
      dst_cmd        = PQgetvalue(res_event, event_line, 0);
      dst_param1     = PQgetvalue(res_event, event_line, 1);
      dst_param2     = PQgetvalue(res_event, event_line, 2);
      dst_group      = PQgetvalue(res_event, event_line, 3);
      flag           = PQgetvalue(res_event, event_line, 4);
      flag_condicion = PQgetvalue(res_event, event_line, 5);
      flag_valor     = PQgetvalue(res_event, event_line, 6);

      /* controlo el flag que regula el tratamiento de este evento, si lo hay */
      if(flag)
      {
        if( strlen(flag) && strcmp(flag, "siempre"))
        {
          sprintf(my_query, "SELECT valor FROM home_flag WHERE nombre = '%s'", flag);
          pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", my_query);
          res_flag = PQexec(g_postgres, my_query);
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
                  if( strcmp(home_flag_valor, flag_valor)) continue;
                  break;
                case 2:
                  if( strcmp(home_flag_valor, flag_valor) <= 0) continue;
                  break;
                case 3:
                  if( strcmp(home_flag_valor, flag_valor) >= 0) continue;
                  break;
                case 4:
                  if( strcmp(home_flag_valor, flag_valor) < 0) continue;
                  break;
                case 5:
                  if( strcmp(home_flag_valor, flag_valor) > 0) continue;
                  break;
                case 6:
                  if( !strcmp(home_flag_valor, flag_valor)) continue;
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
            sprintf(my_query, "UPDATE home_flag SET valor = '%s' WHERE nombre = '%s'", dst_param2, dst_param1);
            pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", my_query);
            PQclear( PQexec(g_postgres, my_query) );
            break;
          case 10: /* Exec */
            sprintf(my_query, "%s %s", dst_param1, dst_param2);
            pLog->Add(GLOG_LEVEL_INFO, "INFO: Ejecutando %s", my_query);
            if( system(my_query) < 0)
              pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Al ejecutar %s", my_query);
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
            sprintf(my_query, "SELECT id, port, bit FROM group_rel WHERE group_id = %i", atoi(dst_group));
            pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", my_query);
            res_group = PQexec(g_postgres, my_query);
            if(PQresultStatus(res_group) == PGRES_TUPLES_OK)
            {
              pLog->Add(1, "EVENTO: port-change dev: %03i port: %2.2s bit: %i %s time %02i:%02i:%02i.",
                            ev->address, ev->port, ev->bit, ((ev->on_off)?"On":"Off"),
                            p_stm->tm_hour, p_stm->tm_min, p_stm->tm_sec );
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
                      pLog->Add(1, "ACCION: set dev: %03i port: Salida1 bit: %i.", id, bit);
                      Notify("sercmd-s1-bit-on-off", data, strlen(data)+1);
                    }
                    else if( !memcmp(port, "O2", 2))
                    {
                      pLog->Add(1, "ACCION: set dev: %03i port: Salida2 bit: %i.", id, bit);
                      Notify("sercmd-s2-bit-on-off", data, strlen(data)+1);
                    }
                    break;
                  case 2:   /* Reset */
                    sprintf(data, "%03i %03i 000", id, bit);
                    if( !memcmp(port, "O1", 2))
                    {
                      pLog->Add(1, "ACCION: reset dev: %03i port: Salida1 bit: %i.", id, bit);
                      Notify("sercmd-s1-bit-on-off", data, strlen(data)+1);
                    }
                    else if( !memcmp(port, "O2", 2))
                    {
                      pLog->Add(1, "ACCION: reset dev: %03i port: Salida2 bit: %i.", id, bit);
                      Notify("sercmd-s2-bit-on-off", data, strlen(data)+1);
                    }
                    break;
                  case 3:   /* Pulse */
                    sprintf(data, "%03i %03i %03i", id, bit, (int)((atoi(dst_param1) > 0)?atoi(dst_param1):1) );
                    if( !memcmp(port, "O1", 2))
                    {
                      pLog->Add(1, "ACCION: pulse dev: %03i port: Salida1 bit: %i.", id, bit);
                      Notify("serpulse-s1-bit", data, strlen(data)+1);
                    }
                    else if( !memcmp(port, "O2", 2))
                    {
                      pLog->Add(1, "ACCION: pulse dev: %03i port: Salida2 bit: %i.", id, bit);
                      Notify("serpulse-s2-bit", data, strlen(data)+1);
                    }
                    break;
                  case 4:   /* Invert */
                    sprintf(data, "%03i %03i", id, bit);
                    if( !memcmp(port, "O1", 2))
                    {
                      pLog->Add(1, "ACCION: invert dev: %03i port: Salida1 bit: %i.", id, bit);
                      Notify("serinvert-s1-bit", data, strlen(data)+1);
                    }
                    else if( !memcmp(port, "O2", 2))
                    {
                      pLog->Add(1, "ACCION: invert dev: %03i port: Salida2 bit: %i.", id, bit);
                      Notify("serinvert-s2-bit", data, strlen(data)+1);
                    }
                    break;


                }
              }
            }
            PQclear(res_group);
          }
        }
      }
    }
    PQclear(res_event);

    /* Busco un testigo para el evento */

    reintentos = 0;
    do
    {
      sprintf(my_query, 
        "SELECT id_testigo, port_testigo, bit_testigo "
        "FROM testigo "
        "WHERE id = %i AND port = '%2.2s' AND bit = %i",
        ev->address, ev->port, ev->bit);
      pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", my_query);
      res_event = PQexec(g_postgres, my_query);
      if(PQresultStatus(res_event) != PGRES_TUPLES_OK)
      {
        /* intento de reconexión --> */
        if(++reintentos > 5) return GME_UNDEFINED;
        pLog->Add(GLOG_LEVEL_ERROR, "ERROR: [%s] en el query [%s].", PQerrorMessage(g_postgres), my_query);
        PQclear(res_event);
        PQfinish(g_postgres);
        sleep(5);
        g_postgres = PQconnectdb(g_str_conn);
        if(PQstatus(g_postgres) != CONNECTION_OK)
        {
          pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Al conectarse a la base de datos [%s].", PQerrorMessage(g_postgres));
          return GME_UNDEFINED;
        }
        continue;
        /* <-- intento de reconexión */
      }
      break;
    } while (true);
    if(PQntuples(res_event) > 0)
    {
      pLog->Add(1, "EVENTO: port-change dev: %03i port: %2.2s bit: %i %s time %02i:%02i:%02i.",
                    ev->address, ev->port, ev->bit, ((ev->on_off)?"On":"Off"),
                    p_stm->tm_hour, p_stm->tm_min, p_stm->tm_sec );
    }
    for(event_line = 0; event_line < PQntuples(res_event); event_line++)
    {
      port = PQgetvalue(res_event, event_line, 1);
      bit  = atoi(PQgetvalue(res_event, event_line, 2));
      id   = atoi(PQgetvalue(res_event, event_line, 0));

      /* acá me tengo que fijar qué tipo de hardware */
      /* si es MIO-16x16/AD mandar un set o reset al dispositivo, puerto y bit */
      /* si es MPAD-6 mandar un '1' o un '0' al dispositivo y puerto */

      /* ahora vamos a hacerlo solamente para MPAD-6 */
      bit = (port[1] & 0x0F);
      sprintf(data, "%03i %03i %03i", id, bit, ev->on_off);
      pLog->Add(1, "ACCION: testigo %s - dev: %03i port: %i.", ((ev->on_off)?"ON":"OFF"), id, bit);
      Notify("set-out", data, strlen(data)+1);
    }
    PQclear(res_event);

    return GME_OK;
  }
  return GME_SVC_UNRESOLVED;
}
