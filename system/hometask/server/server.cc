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

#include "hometask.h"

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

CHomeTask task;

CGMServer::CGMServer()
{

}

CGMServer::~CGMServer()
{

}

/* Colocar en esta funcion lo que se necesite correr al levantar el server */
int CGMServer::Init()
{
  pLog->Add(1, "Iniciando server");
  if(Suscribe("port-change", GM_MSG_TYPE_MSG) != GME_OK)
    pLog->Add(1, "ERROR al suscribir a port-change");
  if(Suscribe("hometask-timer-event", GM_MSG_TYPE_MSG) != GME_OK)
    pLog->Add(1, "ERROR al suscribir a hometask-timer-event");
  if(Suscribe("hometask-change-config", GM_MSG_TYPE_MSG) != GME_OK)
    pLog->Add(1, "ERROR al suscribir a hometask-change-config");
  if(Suscribe("hometask-auto-event", GM_MSG_TYPE_MSG) != GME_OK)
    pLog->Add(1, "ERROR al suscribir a hometask-auto-event");
  if(Suscribe("hometask-re-auto-event", GM_MSG_TYPE_MSG) != GME_OK)
    pLog->Add(1, "ERROR al suscribir a hometask-re-auto-event");

  return task.Init(this, pLog);
}

/* Colocar en esta funcion lo que se necesite correr al bajar el serer */
int CGMServer::Exit()
{
  pLog->Add(1, "Terminando server");
  UnSuscribe("port-change", GM_MSG_TYPE_MSG);
  UnSuscribe("hometask-timer-event", GM_MSG_TYPE_MSG);
  UnSuscribe("hometask-change-config", GM_MSG_TYPE_MSG);
  UnSuscribe("hometask-auto-event", GM_MSG_TYPE_MSG);
  UnSuscribe("hometask-re-auto-event", GM_MSG_TYPE_MSG);
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
  return task.Process(funcion, typ, in, inlen, out, outlen);
}
