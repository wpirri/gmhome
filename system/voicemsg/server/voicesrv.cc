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
#include <gmonitor/gmerror.h>
#include <gmonitor/gmontdb.h>
#include <gmonitor/svcstru.h>
#include <gmonitor/gmswaited.h>

#include <string>
#include <iostream>
#include <csignal>

#ifdef HAVE_CONFIG_H    /* Festival usa este define y busca un header que no existe */
# undef HAVE_CONFIG_H
#   include <festival.h>
# define HAVE_CONFIG_H
#else
#   include <festival.h>
#endif

using namespace std;

#include <unistd.h>
#include <time.h>
#include <syslog.h>

CGMServerWait *m_pServer;
bool m_bOcupado;

void OnClose(int sig);
void SayText(const char* text);
void SetTimer(void);
void CheckQueue(void);

int main(int argc, char** argv, char** env)
{
  char fn[33];
  char typ;
  unsigned long inlen;
  char buffer[4096];


  int heap_size = 210000;  /* default scheme heap size */
  int load_init_files = 1; /* we want the festival init files loaded */

  signal(SIGPIPE, SIG_IGN);
  signal(SIGKILL, OnClose);
  signal(SIGTERM, OnClose);
  signal(SIGSTOP, OnClose);
  signal(SIGABRT, OnClose);
  signal(SIGQUIT, OnClose);
  signal(SIGINT, OnClose);
  signal(SIGILL, OnClose);
  signal(SIGFPE, OnClose);
  signal(SIGSEGV, OnClose);
  signal(SIGBUS, OnClose);

  m_pServer = new CGMServerWait;
  m_pServer->Init("gmh_voicesrv");
  m_pServer->m_pLog->Add(1, "Iniciando server");
  m_pServer->Suscribe("text2speech", GM_MSG_TYPE_NOT);
  m_pServer->Suscribe("queue2speech", GM_MSG_TYPE_NOT);
  m_bOcupado = true;

  festival_initialize(load_init_files, heap_size);
  festival_eval_command("(voice_abc_diphone)");
  /*festival_eval_command("(set! output_type \'ulaw)");*/
  /*festival_eval_command("(set! volume 5.0)");*/

  SayText("Iniciando mensajes de voz");
  SetTimer();

  while(m_pServer->Wait(fn, &typ, buffer, 4096, &inlen) > 0)
  {
    m_pServer->m_pLog->Add(100, "Query recibido fn = [%s]", fn);
    if( !strcmp(fn, "text2speech"))
    {
      m_pServer->Resp(NULL, 0, GME_OK);
      m_pServer->m_pLog->Add(50, "MENSAJE: %*.*s", inlen, inlen, buffer);
      if(inlen)
      {
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else
      {
        m_pServer->Resp(NULL, 0, GME_UNDEFINED);
        continue;
      }
      if(m_bOcupado)
      {
        /* el canal está ocupado asi que lo encolo */
        buffer[inlen] = '\0';
        m_pServer->m_pLog->Add(100, "Canal ocupado, se encola el mensaje [%s]", buffer);
        m_pServer->Enqueue("queue2speech", buffer, inlen + 1);
      }
      else
      {
        /* si no estaba haciendo nada lo ocupo y paso el mensaje directamente */
        m_bOcupado = true;
        buffer[inlen] = '\0';
        m_pServer->m_pLog->Add(100, "Canal libre, se reproduce directamente el mensaje [%s]", buffer);
        SayText(buffer);
        SetTimer();
      }
      m_pServer->Resp(NULL, 0, GME_OK);
    }
    else if( !strcmp(fn, "queue2speech"))
    {
      m_pServer->Resp(NULL, 0, GME_OK);
      CheckQueue();
    }
    else
    {
      m_pServer->m_pLog->Add(1, "GME_SVC_NOTFOUND");
      m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
    }
  }
  m_pServer->m_pLog->Add(1, "ERROR en la espera de mensajes");
  OnClose(0);
  return 0;
}

void OnClose(int sig)
{
  m_pServer->m_pLog->Add(1, "Terminando server");
  m_pServer->UnSuscribe("text2speech", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("queue2speech", GM_MSG_TYPE_NOT);
  delete m_pServer;
  exit(0);
}

/*
    regula los intervalos de ejecuciòn de los mensajes
*/
void SayText(const char* text)
{
  m_pServer->m_pLog->Add(100, "SayText - festival_say_text");
  festival_say_text(text);
  m_pServer->m_pLog->Add(100, "SayText - festival_wait_for_spooler");
  festival_wait_for_spooler();
  m_pServer->m_pLog->Add(100, "SayText - fin");
}

void SetTimer(void)
{
  int rc;
  ST_STIMER timer_data;

  /* seteo un timer para el proximo mensaje */
  strcpy(timer_data.set_timer.servicio, "queue2speech");
  timer_data.set_timer.modo_servicio = GM_MSG_TYPE_NOT;
  timer_data.set_timer.delay = 5;
  timer_data.set_timer.at = 0;
  timer_data.set_timer.tipo_timer = 'U';
  timer_data.set_timer.len = 0;
  timer_data.set_timer.data[0] = '\0';
  m_pServer->m_pLog->Add(100, "Se levanta un timer para queue2speech");
  if((rc = m_pServer->Call(".set_timer",
        &timer_data, sizeof(ST_STIMER), NULL, 500)) != GME_OK)
  {
    /* no hay mas id's disponibles */
    m_pServer->m_pLog->Add(1, "ERROR: Error %i al setear el timer para queue2speech", rc);
  }
}

void CheckQueue(void)
{
  CGMServerBase::GMIOS qdata;

  /* saco un mensaje de la cola y lo paso, si no hay mas libero el canal */
  m_pServer->m_pLog->Add(100, "Busco un mensaje en la cola");
  if(m_pServer->Dequeue("queue2speech", &qdata) != GME_OK)
  {
    m_pServer->m_pLog->Add(100, "No hay mensajes en la cola, se libera el canal");
    m_bOcupado = false;
  }
  else if(qdata.len)
  {
    m_pServer->m_pLog->Add(100, "Se saca de la cola el mensaje [%s]", qdata.data);
    SayText((const char*)qdata.data);
    free(qdata.data);
    SetTimer();
  }
  else
  {
    m_pServer->m_pLog->Add(100, "No hay mensajes en la cola, se libera el canal");
    m_bOcupado = false;
  }
}

