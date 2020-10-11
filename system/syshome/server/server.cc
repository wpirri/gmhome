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
PGconn *CGMServer_g_postgres;
char CGMServer_g_str_conn[256];

CGMServer::CGMServer()
{

}

CGMServer::~CGMServer()
{

}

/* Colocar en esta funcion lo que se necesite correr al levantar el server */
int CGMServer::Init()
{
  int i;
  char *dbhost = NULL;
  char *dbname = NULL;
  char *dbuser = NULL;
  char *dbpass = NULL;

  pLog->Add(1, "Iniciando server");

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

  sprintf(CGMServer_g_str_conn, "host=%s dbname=%s user=%s password=%s",
          (dbhost)?dbhost:"localhost",
          (dbname)?dbname:"syshome",
          (dbuser)?dbuser:"syshome",
          (dbpass)?dbpass:"syshome" );


  CGMServer_g_postgres = PQconnectdb(CGMServer_g_str_conn);
  if(PQstatus(CGMServer_g_postgres) != CONNECTION_OK)
  {
    pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Al conectarse a la base de datos [%s].", PQerrorMessage(CGMServer_g_postgres));
    return GME_UNDEFINED;
  }

  if(Suscribe("syshome_web_object", GM_MSG_TYPE_CR) != GME_OK)
    pLog->Add(1, "ERROR al suscribir a syshome_web_object");
  if(Suscribe("syshome_web_object", GM_MSG_TYPE_INT) != GME_OK)
    pLog->Add(1, "ERROR al suscribir a syshome_web_object");

  return 0;
}

/* Colocar en esta funcion lo que se necesite correr al bajar el serer */
int CGMServer::Exit()
{
  pLog->Add(1, "Terminando server");
  UnSuscribe("syshome_web_object", GM_MSG_TYPE_CR);
  UnSuscribe("syshome_web_object", GM_MSG_TYPE_INT);
  PQfinish(CGMServer_g_postgres);
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
#define XML_HEAD          "<?xml version=\"1.0\" standalone=\"yes\"?>"
#define START_OBJECT_LIST "<list>"
#define END_OBJECT_LIST   "</list>"
#define START_OBJECT      "<item>"
#define END_OBJECT        "</item>"

  char sql_query[1024];
  PGresult   *res;
  int reintentos;
  int line;
  char* rc_line;
  CGMBuffer xml_buffer;
  long page;

  page = atol((char*)in);

  if( !strcmp(funcion, "syshome_web_object"))
  {

    reintentos = 0;
    do
    {
      /* select xmlforest(page, object, name, src, xpos, ypos, atag) from web_object; */
      if(page > 0)
      {
        sprintf(sql_query, "SELECT xmlforest(web_object.idtag, web_object.name, "
                                            "web_object.xpos, web_object.ypos, web_object.atag, "
                                            "web_object.disp, web_object.port, web_object.bit, "
                                            "image_object.src_unk, image_object.src_on, image_object.src_off) "
                           "FROM web_object, image_object "
                           "WHERE web_object.idobject = image_object.idobject AND page = %li", page);
      }
      else
      {
        sprintf(sql_query, "SELECT xmlforest(web_object.idtag, web_object.name, "
                                            "web_object.xpos, web_object.ypos, web_object.atag, "
                                            "web_object.disp, web_object.port, web_object.bit, "
                                            "image_object.src_unk, image_object.src_on, image_object.src_off) "
                           "FROM web_object, image_object "
                           "WHERE web_object.idobject = image_object.idobject");
      }
      pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: query: [%s].", sql_query);
      res = PQexec(CGMServer_g_postgres, sql_query);
      if(PQresultStatus(res) != PGRES_TUPLES_OK)
      {
        /* intento de reconexión --> */
        if(++reintentos > 5) return GME_UNDEFINED;
        pLog->Add(GLOG_LEVEL_ERROR, "ERROR: [%s] en el query [%s].", PQerrorMessage(CGMServer_g_postgres), sql_query);
        PQclear(res);
        PQfinish(CGMServer_g_postgres);
        sleep(5);
        CGMServer_g_postgres = PQconnectdb(CGMServer_g_str_conn);
        if(PQstatus(CGMServer_g_postgres) != CONNECTION_OK)
        {
          pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Al conectarse a la base de datos [%s].", PQerrorMessage(CGMServer_g_postgres));
          return GME_UNDEFINED;
        }
        continue;
        /* <-- intento de reconexión */
      }
      break;
    } while (true);

    /* Inicio del array de datos XML y de la lista */
    pLog->Add(GLOG_LEVEL_INFO, "INFO: %s", XML_HEAD);
    xml_buffer = XML_HEAD;
    xml_buffer += "\n";
    pLog->Add(GLOG_LEVEL_INFO, "INFO: %s", START_OBJECT_LIST);
    xml_buffer += START_OBJECT_LIST;
    xml_buffer += "\n";
    for(line = 0; line < PQntuples(res); line++)
    {
      /* Inicio de un registro */
      pLog->Add(GLOG_LEVEL_INFO, "INFO: %s", START_OBJECT);
      xml_buffer += START_OBJECT;
      /* Datos del registro */
      rc_line = PQgetvalue(res, line, 0);
      pLog->Add(GLOG_LEVEL_INFO, "INFO: %s", rc_line);
      xml_buffer += rc_line;
      /* Fin de un registro */
      pLog->Add(GLOG_LEVEL_INFO, "INFO: %s", END_OBJECT);
      xml_buffer += END_OBJECT;
      xml_buffer += "\n";
    }
    /* Fin de la lista */
    pLog->Add(GLOG_LEVEL_INFO, "INFO: %s", END_OBJECT_LIST);
    xml_buffer += END_OBJECT_LIST;
    xml_buffer += "\n";

    *outlen = xml_buffer.Length();
    *out = (char*)malloc(*outlen);
    memcpy(*out, xml_buffer.C_Str(), *outlen);

    pLog->Add(GLOG_LEVEL_INFO, "INFO: Listado: %lu bytes", *outlen);

    return GME_OK;
  }
  return GME_SVC_UNRESOLVED;
}
