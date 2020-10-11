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
#ifndef _CHOMETASK_H_
#define _CHOMETASK_H_

#define AUTOACTIVACION_DELAY   1800    /*30 minutos*/
#define AUTOREACTIVACION_DELAY  600    /*10 minutos*/

#include <string>
using namespace std;

#include <gmonitor/gmerror.h>
#include <gmonitor/gms.h>

#include <postgresql/libpq-fe.h>

class CHomeTask
{
public:
  CHomeTask();
  virtual ~CHomeTask();

  typedef struct _EVENT_NOTIF
  {
    int     address;
    char    port[2];
    int     bit;
    int     on_off;
    long    time;
  } EVENT_NOTIF;

  int Init(CGMServer *pserver, CGLog *plog);
  int Process(const char *funcion, char typ,
      void* in, unsigned long inlen,
      void** out, unsigned long *outlen);
protected:
  PGconn *m_pDB;
  char m_str_conn[256];
  int ReConnect(void);
  int ExecCmd(char *dst_cmd, char *dst_param1, char *dst_param2, char *flag,
              char *flag_condicion, char *flag_valor, char *dst_group);
  int MakeExecuted(long id);
  int LoadConfig();
  int LoadTimer(void);
  void KillTimerId(unsigned int id);
  int InitAutoActivacion();
  int CheckAutoActivacion(EVENT_NOTIF* ev);
  int ExecAutoActivacion();
  int CheckAutoReactivacion(EVENT_NOTIF* ev);
  int ExecAutoReactivacion();
  int EnviarComandoActivacion(int force = 0);
  /*
    Devuelve true si el dìa de la semana que se indica en tm_wday está encendido
    en los flags de db_wday
  */
  bool WDayExist(int tm_wday, int db_wday);

private:
  CGMServer* m_pServer;
  CGLog* m_pLog;
  int m_enable_autoactivacion;
  int m_enable_autoreactivacion;
  unsigned int m_timer1_id;
  unsigned int m_timer2_id;
  unsigned int m_timer3_id;
  time_t    m_desactivacion_time;
  time_t    m_startup_time;
  EVENT_NOTIF m_UltimoSensor;
  EVENT_NOTIF m_DemasSensores[256];
  /* variables para almacenar la salida de activación de alarma */
  char *m_act_dst_cmd;
  char *m_act_dst_param1;
  char *m_act_dst_param2;
  char *m_act_dst_group;
  char *m_act_flag;
  char *m_act_flag_condicion;
  char *m_act_flag_valor;

};
#endif /* _CHOMETASK_H_ */
