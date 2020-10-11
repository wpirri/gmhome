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
#include "serdev.h"

#include <string>
#include <iostream>
#include <csignal>
using namespace std;

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <syslog.h>

#include <gmonitor/gmerror.h>

#define INFINITE_RESEND      0
#define DEFAULT_RESEND       50
#define DEFAULT_RESEND_TO    5

CSerDev::CSerDev(CGLog* pLog, CGMServerWait *pServer)
{
  m_pLog = pLog;
  m_pServer = pServer;
}

CSerDev::~CSerDev()
{
  Close();
}

int CSerDev::Open(const char* port)
{
  if(CSerComm::Open(port) != 0) return -1;
  m_cCnt = 0;
  memset(&m_hwStat, 0, sizeof(hw_stat)*256);
  return 0;
}

void CSerDev::Close()
{
  CSerComm::Close();
}

int CSerDev::Process()
{
  char typ, src, cnt, fcn;
  char data[10];
  EVENT_NOTIF   event_data;
  time_t t;
  struct tm *pstm;

  /* es puesto en false por si se hace una respuesta especifica, sino al final sale la genérica */
  bool responder = false;
  bool repetido;
  char rdata[10];

  /*m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Process()");*/
  if(CSerComm::Recv(&typ, &src, &cnt, &fcn, data))
  {
    memset(rdata, ' ', 10);
    m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: RECV typ: %c src: 0x%02X "
            "cnt: 0x%02X fcn: %c data: 0x%02X 0x%02X 0x%02X 0x%02X "
            "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
          typ, (int)(src & 0xFF),
          (int)(cnt & 0xFF),
          fcn,
          (int)(data[0] & 0xFF),
          (int)(data[1] & 0xFF),
          (int)(data[2] & 0xFF),
          (int)(data[3] & 0xFF),
          (int)(data[4] & 0xFF),
          (int)(data[5] & 0xFF),
          (int)(data[6] & 0xFF),
          (int)(data[7] & 0xFF),
          (int)(data[8] & 0xFF),
          (int)(data[9] & 0xFF));
    /* Validaciones generales */
  	if(src == (char)0x00 || src == (char)0xFF) return 0;

    t = time(&t);
    m_hwStat[((int)(src & 0xFF))].last_rcv = time(&m_hwStat[((int)(src & 0xFF))].last_rcv);
    repetido = false;

    if(typ == 'Q')                                              /* QUERY */
    {
      /* detecto los mensajes repetidos */
      if(m_hwStat[((int)(src & 0xFF))].last_cnt == cnt) repetido = true;

      switch(fcn)
      {
        case 'P':                                   /* PING */
          /* actualizo los datos del periferico */
          if( !m_hwStat[((int)(src & 0xFF))].on_line )
          {
            m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Dispositivo nuevo 0x%02X "
                  "(HW T:0x%02X V:0x%02X SW:0x%02X)",
                  (int)(src & 0xFF), data[0], data[1], data[2]);
          }
          else
          {
            if( m_hwStat[((int)(src & 0xFF))].hw_typ != data[0] ||
                m_hwStat[((int)(src & 0xFF))].hw_ver != data[1] ||
                m_hwStat[((int)(src & 0xFF))].sw_ver != data[2]  )
            {
              m_pLog->Add(GLOG_LEVEL_WARNING, "WARNING: Dispositivo cambiado: "
                    "viejo 0x%02X (HW T:0x%02X V:0x%02X SW:0x%02X)",
                    (int)(src & 0xFF), m_hwStat[((int)(src & 0xFF))].hw_typ, m_hwStat[((int)(src & 0xFF))].hw_ver, m_hwStat[((int)(src & 0xFF))].sw_ver);
              m_pLog->Add(GLOG_LEVEL_WARNING, "WARNING: Dispositivo cambiado: "
                    "nuevo 0x%02X (HW T:0x%02X V:0x%02X SW:0x%02X)",
                    (int)(src & 0xFF), data[0], data[1], data[2]);
            }
          }
          m_hwStat[((int)(src & 0xFF))].hw_typ = data[0];
          m_hwStat[((int)(src & 0xFF))].hw_ver = data[1];
          m_hwStat[((int)(src & 0xFF))].sw_ver = data[2];
          m_hwStat[((int)(src & 0xFF))].up_model = data[3];
          m_hwStat[((int)(src & 0xFF))].display = data[4];
          m_hwStat[((int)(src & 0xFF))].info_06 = data[5];
          m_hwStat[((int)(src & 0xFF))].info_07 = data[6];
          m_hwStat[((int)(src & 0xFF))].info_08 = data[7];
          m_hwStat[((int)(src & 0xFF))].info_09 = data[8];
          m_hwStat[((int)(src & 0xFF))].info_10 = data[9];
          if( !m_hwStat[((int)(src & 0xFF))].on_line)
          {
            m_pLog->Add(GLOG_LEVEL_WARNING, "INFO: Dispositivo 0x%02X ONLINE", (int)(src & 0xFF));
            /* marco los datos desactualizados para forzar el refrezco */
            if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x01) /* MIO-16X16 */
            {
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_e1 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_e2 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_s1 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_s2 = t + 10;
            }
            else if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x02) /* MIO-16x16/AD */
            {
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_e1 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_e2 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_s1 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_s2 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a1 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a2 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a3 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a4 = t + 10;
            }
          }
          /* a MPAD-6 le voy a actualizar los datos cada vez que me mande un ping */
          if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x03) /* MPAD-6 */
          {
            m_hwStat[((int)(src & 0xFF))].typ.hw3.next_update = t + 10;
          }
          m_hwStat[((int)(src & 0xFF))].on_line = 1;
          rdata[0] = 0;
          rdata[1] = 0;
          rdata[2] = 0;
          responder = true;
          break;
        case '0':

          break;
        case '1':

          break;
        case '2':

          break;
        case '3':

          break;
        case '4':

          break;
        case '5':

          break;
        case '6':
          if( repetido )
          {
            responder = true;
            memcpy(rdata, data, 10);
            break;
          }
          event_data.address = (int)(src & 0xFF);
          event_data.time = t;
          if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x01)  /* MIO-16x16 */
          {
            if(data[0] == '1')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de ENTRADA 1 de 0x%02X", (int)(src & 0xFF));
              /* pongo el timer para pedir refresh */
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_e1 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x01) != (data[1] & 0x01) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[0] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 0 [%s]", (int)(src & 0xFF), (data[1] & 0x01)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 0;
                  event_data.on_off = (data[1] & 0x01)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x02) != (data[1] & 0x02) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[1] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 1 [%s]", (int)(src & 0xFF), (data[1] & 0x02)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 1;
                  event_data.on_off = (data[1] & 0x02)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x04) != (data[1] & 0x04) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[2] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 2 [%s]", (int)(src & 0xFF), (data[1] & 0x04)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 2;
                  event_data.on_off = (data[1] & 0x04)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x08) != (data[1] & 0x08) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[3] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 3 [%s]", (int)(src & 0xFF), (data[1] & 0x08)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 3;
                  event_data.on_off = (data[1] & 0x08)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x10) != (data[1] & 0x10) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[4] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 4 [%s]", (int)(src & 0xFF), (data[1] & 0x10)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 4;
                  event_data.on_off = (data[1] & 0x10)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x20) != (data[1] & 0x20) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[5] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 5 [%s]", (int)(src & 0xFF), (data[1] & 0x20)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 5;
                  event_data.on_off = (data[1] & 0x20)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x40) != (data[1] & 0x40) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[6] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 6 [%s]", (int)(src & 0xFF), (data[1] & 0x40)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 6;
                  event_data.on_off = (data[1] & 0x40)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x80) != (data[1] & 0x80) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[7] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 7 [%s]", (int)(src & 0xFF), (data[1] & 0x80)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 7;
                  event_data.on_off = (data[1] & 0x80)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 = data[1];
            }
            else if(data[0] == '2')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de ENTRADA 2 de 0x%02X", (int)(src & 0xFF));
              /* pongo el timer para pedir refresh */
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_e2 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x01) != (data[1] & 0x01) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[0] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 0 [%s]", (int)(src & 0xFF), (data[1] & 0x01)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 0;
                  event_data.on_off = (data[1] & 0x01)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x02) != (data[1] & 0x02) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[1] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 1 [%s]", (int)(src & 0xFF), (data[1] & 0x02)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 1;
                  event_data.on_off = (data[1] & 0x02)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x04) != (data[1] & 0x04) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[2] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 2 [%s]", (int)(src & 0xFF), (data[1] & 0x04)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 2;
                  event_data.on_off = (data[1] & 0x04)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x08) != (data[1] & 0x08) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[3] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 3 [%s]", (int)(src & 0xFF), (data[1] & 0x08)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 3;
                  event_data.on_off = (data[1] & 0x08)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x10) != (data[1] & 0x10) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[4] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 4 [%s]", (int)(src & 0xFF), (data[1] & 0x10)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 4;
                  event_data.on_off = (data[1] & 0x10)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x20) != (data[1] & 0x20) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[5] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 5 [%s]", (int)(src & 0xFF), (data[1] & 0x20)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 5;
                  event_data.on_off = (data[1] & 0x20)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x40) != (data[1] & 0x40) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[6] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 6 [%s]", (int)(src & 0xFF), (data[1] & 0x40)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 6;
                  event_data.on_off = (data[1] & 0x40)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x80) != (data[1] & 0x80) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[7] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 7 [%s]", (int)(src & 0xFF), (data[1] & 0x80)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 7;
                  event_data.on_off = (data[1] & 0x80)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 = data[1];
            }
            responder = true;
            memcpy(rdata, data, 10);
          }
          else if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x02)  /* MIO-16x16/AD */
          {
            if(data[0] == '1')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de ENTRADA 1 de 0x%02X", (int)(src & 0xFF));
              /* pongo el timer para pedir refresh */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_e1 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x01) != (data[1] & 0x01) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[0] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 0 [%s]", (int)(src & 0xFF), (data[1] & 0x01)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 0;
                  event_data.on_off = (data[1] & 0x01)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x02) != (data[1] & 0x02) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[1] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 1 [%s]", (int)(src & 0xFF), (data[1] & 0x02)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 1;
                  event_data.on_off = (data[1] & 0x02)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x04) != (data[1] & 0x04) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[2] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 2 [%s]", (int)(src & 0xFF), (data[1] & 0x04)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 2;
                  event_data.on_off = (data[1] & 0x04)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x08) != (data[1] & 0x08) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[3] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 3 [%s]", (int)(src & 0xFF), (data[1] & 0x08)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 3;
                  event_data.on_off = (data[1] & 0x08)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x10) != (data[1] & 0x10) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[4] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 4 [%s]", (int)(src & 0xFF), (data[1] & 0x10)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 4;
                  event_data.on_off = (data[1] & 0x10)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x20) != (data[1] & 0x20) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[5] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 5 [%s]", (int)(src & 0xFF), (data[1] & 0x20)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 5;
                  event_data.on_off = (data[1] & 0x20)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x40) != (data[1] & 0x40) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[6] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 6 [%s]", (int)(src & 0xFF), (data[1] & 0x40)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 6;
                  event_data.on_off = (data[1] & 0x40)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x80) != (data[1] & 0x80) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[7] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 7 [%s]", (int)(src & 0xFF), (data[1] & 0x80)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 7;
                  event_data.on_off = (data[1] & 0x80)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 = data[1];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.mascara_notif_e1 = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.seguidor_e1_s1 = data[3];
            }
            else if(data[0] == '2')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de ENTRADA 2 de 0x%02X", (int)(src & 0xFF));
              /* pongo el timer para pedir refresh */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_e2 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x01) != (data[1] & 0x01) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[0] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 0 [%s]", (int)(src & 0xFF), (data[1] & 0x01)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 0;
                  event_data.on_off = (data[1] & 0x01)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x02) != (data[1] & 0x02) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[1] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 1 [%s]", (int)(src & 0xFF), (data[1] & 0x02)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 1;
                  event_data.on_off = (data[1] & 0x02)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x04) != (data[1] & 0x04) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[2] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 2 [%s]", (int)(src & 0xFF), (data[1] & 0x04)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 2;
                  event_data.on_off = (data[1] & 0x04)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x08) != (data[1] & 0x08) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[3] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 3 [%s]", (int)(src & 0xFF), (data[1] & 0x08)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 3;
                  event_data.on_off = (data[1] & 0x08)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x10) != (data[1] & 0x10) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[4] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 4 [%s]", (int)(src & 0xFF), (data[1] & 0x10)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 4;
                  event_data.on_off = (data[1] & 0x10)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x20) != (data[1] & 0x20) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[5] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 5 [%s]", (int)(src & 0xFF), (data[1] & 0x20)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 5;
                  event_data.on_off = (data[1] & 0x20)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x40) != (data[1] & 0x40) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[6] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 6 [%s]", (int)(src & 0xFF), (data[1] & 0x40)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 6;
                  event_data.on_off = (data[1] & 0x40)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x80) != (data[1] & 0x80) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[7] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 7 [%s]", (int)(src & 0xFF), (data[1] & 0x80)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 7;
                  event_data.on_off = (data[1] & 0x80)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 = data[1];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.mascara_notif_e2 = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.seguidor_e2_s2 = data[3];
            }
            responder = true;
            memcpy(rdata, data, 10);
          }
          else if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x03) /* MPAD-6 */
          {
            m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo aviso de PULSADOR %c de 0x%02X", data[0], (int)(src & 0xFF));
            if( ((int)(src & 0xFF)) != 254 )
            {
              /* comunico el evento */
              event_data.port[0] = 'I';
              event_data.port[1] = data[0];
              event_data.bit = 0;
              event_data.on_off = 1;
              m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
            }
            responder = true;
            memcpy(rdata, data, 10);
          }
          break;
        case '7':
          if( repetido )
          {
            responder = true;
            memcpy(rdata, data, 10);
            break;
          }
          event_data.address = (int)(src & 0xFF);
          event_data.time = t;
          if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x01) /* MIO-16x16 */
          {
            if(data[0] == '1')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de SALIDA 1 de 0x%02X", (int)(src & 0xFF));
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_s1 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x01) != (data[1] & 0x01) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[0] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x02) != (data[1] & 0x02) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[1] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x04) != (data[1] & 0x04) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[2] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x08) != (data[1] & 0x08) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[3] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x10) != (data[1] & 0x10) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[4] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x20) != (data[1] & 0x20) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[5] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x40) != (data[1] & 0x40) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[6] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x80) != (data[1] & 0x80) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[7] = t;
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 = data[1];
            }
            else if(data[0] == '2')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de SALIDA 2 de 0x%02X", (int)(src & 0xFF));
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_s2 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x01) != (data[1] & 0x01) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[0] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x02) != (data[1] & 0x02) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[1] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x04) != (data[1] & 0x04) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[2] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x08) != (data[1] & 0x08) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[3] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x10) != (data[1] & 0x10) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[4] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x20) != (data[1] & 0x20) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[5] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x40) != (data[1] & 0x40) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[6] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x80) != (data[1] & 0x80) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[7] = t;
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 = data[1];
            }
            responder = true;
            memcpy(rdata, data, 10);
          }
          if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x02) /* MIO-16x16/AD */
          {
            if(data[0] == '1')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de SALIDA 1 de 0x%02X", (int)(src & 0xFF));
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_s1 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x01) != (data[1] & 0x01) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[0] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 0 [%s]", (int)(src & 0xFF), (data[1] & 0x01)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 0;
                  event_data.on_off = (data[1] & 0x01)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x02) != (data[1] & 0x02) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[1] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 1 [%s]", (int)(src & 0xFF), (data[1] & 0x02)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 1;
                  event_data.on_off = (data[1] & 0x02)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x04) != (data[1] & 0x04) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[2] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 2 [%s]", (int)(src & 0xFF), (data[1] & 0x04)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 2;
                  event_data.on_off = (data[1] & 0x04)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x08) != (data[1] & 0x08) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[3] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 3 [%s]", (int)(src & 0xFF), (data[1] & 0x08)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 3;
                  event_data.on_off = (data[1] & 0x08)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x10) != (data[1] & 0x10) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[4] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 4 [%s]", (int)(src & 0xFF), (data[1] & 0x10)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 4;
                  event_data.on_off = (data[1] & 0x10)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x20) != (data[1] & 0x20) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[5] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 5 [%s]", (int)(src & 0xFF), (data[1] & 0x20)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 5;
                  event_data.on_off = (data[1] & 0x20)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x40) != (data[1] & 0x40) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[6] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 6 [%s]", (int)(src & 0xFF), (data[1] & 0x40)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 6;
                  event_data.on_off = (data[1] & 0x40)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x80) != (data[1] & 0x80) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[7] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 7 [%s]", (int)(src & 0xFF), (data[1] & 0x80)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 7;
                  event_data.on_off = (data[1] & 0x80)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 = data[1];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.mascara_notif_s1 = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.seguidor_e1_s1 = data[3];
            }
            else if(data[0] == '2')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de SALIDA 2 de 0x%02X", (int)(src & 0xFF));
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_s2 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x01) != (data[1] & 0x01) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[0] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 0 [%s]", (int)(src & 0xFF), (data[1] & 0x01)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 0;
                  event_data.on_off = (data[1] & 0x01)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x02) != (data[1] & 0x02) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[1] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 1 [%s]", (int)(src & 0xFF), (data[1] & 0x02)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 1;
                  event_data.on_off = (data[1] & 0x02)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x04) != (data[1] & 0x04) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[2] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 2 [%s]", (int)(src & 0xFF), (data[1] & 0x04)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 2;
                  event_data.on_off = (data[1] & 0x04)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x08) != (data[1] & 0x08) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[3] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 3 [%s]", (int)(src & 0xFF), (data[1] & 0x08)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 3;
                  event_data.on_off = (data[1] & 0x08)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x10) != (data[1] & 0x10) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[4] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 4 [%s]", (int)(src & 0xFF), (data[1] & 0x10)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 4;
                  event_data.on_off = (data[1] & 0x10)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x20) != (data[1] & 0x20) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[5] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 5 [%s]", (int)(src & 0xFF), (data[1] & 0x20)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 5;
                  event_data.on_off = (data[1] & 0x20)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x40) != (data[1] & 0x40) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[6] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 6 [%s]", (int)(src & 0xFF), (data[1] & 0x40)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 6;
                  event_data.on_off = (data[1] & 0x40)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x80) != (data[1] & 0x80) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[7] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 7 [%s]", (int)(src & 0xFF), (data[1] & 0x80)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 7;
                  event_data.on_off = (data[1] & 0x80)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 = data[1];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.mascara_notif_s2 = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.seguidor_e2_s2 = data[3];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.seguidor_an_s2 = data[4];
            }
            responder = true;
            memcpy(rdata, data, 10);
          }
          else if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x03) /* MPAD-6 */
          {

            responder = true;
            memcpy(rdata, data, 10);
          }
          break;
        case '9': /* Alarmas de A/D */
          if( repetido )
          {
            responder = true;
            memcpy(rdata, data, 10);
            break;
          }
          if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x02) /* MIO-16x16/AD */
          {
            if(data[0] == '1')
            {
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_alarm = data[1] - '0';
              /* actualizo los datos del A/D */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a1 = t + 300; /* 5 minutos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_h = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_l = data[3];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_maxh = data[4];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_maxl = data[5];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_minh  = data[6];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_minl  = data[7];
            }
            else if(data[0] == '2')
            {
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_alarm = data[1] - '0';
              /* actualizo los datos del A/D */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a2 = t + 300; /* 5 minutos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_h = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_l = data[3];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_maxh = data[4];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_maxl = data[5];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_minh  = data[6];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_minl  = data[7];
            }
            else if(data[0] == '3')
            {
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_alarm = data[1] - '0';
              /* actualizo los datos del A/D */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a3 = t + 300; /* 5 minutos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_h = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_l = data[3];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_maxh = data[4];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_maxl = data[5];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_minh  = data[6];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_minl  = data[7];
            }
            else if(data[0] == '4')
            {
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_alarm = data[1] - '0';
              /* actualizo los datos del A/D */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a4 = t + 300; /* 5 minutos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_h = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_l = data[3];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_maxh = data[4];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_maxl = data[5];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_minh  = data[6];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_minl  = data[7];
            }
            /* completo la info general */
            m_hwStat[((int)(src & 0xFF))].typ.hw2.mascara_notif_an = data[8];
            m_hwStat[((int)(src & 0xFF))].typ.hw2.seguidor_an_s2 = data[9];
            m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo alarma de ANALOG %c de 0x%02X [%s]",
              data[0], (int)(src & 0xFF), (data[1] == '1')?"off -> on":"on -> off");
            if( ((int)(src & 0xFF)) != 254 )
            {
              /* comunico el evento */
              event_data.address = (int)(src & 0xFF);
              event_data.time = t;
              event_data.port[0] = 'A';
              event_data.port[1] = data[0];
              event_data.bit = 0;
              event_data.on_off = data[1] - '0';
              m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
            }
            /* seteo la respuesta */
            responder = true;
            memcpy(rdata, data, 10);
          }
          break;
        case 'T':
          /* Hay que pasarle la hora */
          pstm =  localtime(&t);
          /* seteo la respuesta */
          responder = true;
          fcn = 'T';
          rdata[0] = (pstm->tm_hour & 0xFF);
          rdata[1] = (pstm->tm_min & 0xFF);
          rdata[2] = (pstm->tm_sec & 0xFF);
          rdata[3] = ((pstm->tm_wday == 0)?(0x06):(pstm->tm_wday - 1)) & 0xFF;
          rdata[4] = (pstm->tm_mday & 0xFF);
          rdata[5] = (pstm->tm_mon & 0xFF);
          rdata[6] = (pstm->tm_year & 0xFF);
          rdata[7] = ' ';
          rdata[8] = ' ';
          rdata[9] = ' ';
          break;
      }
      if(responder)
      {
        /* las respuestas mas básicas las armo directamente */
        m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: SEND typ: R dst: 0x%02X "
               "cnt: 0x%02X fcn: %c data: 0x%02X 0x%02X 0x%02X 0x%02X "
               "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
            (int)(src & 0xFF), (int)(cnt & 0xFF), fcn,
            (int)(rdata[0] & 0xFF), (int)(rdata[1] & 0xFF), (int)(rdata[2] & 0xFF),
            (int)(rdata[3] & 0xFF), (int)(rdata[4] & 0xFF), (int)(rdata[5] & 0xFF),
            (int)(rdata[6] & 0xFF), (int)(rdata[7] & 0xFF), (int)(rdata[8] & 0xFF),
            (int)(rdata[9] & 0xFF));
        CSerComm::Send('R', src/*destino*/, cnt, fcn, rdata);
      }
      /* para detectar mensajes repetidos */
      m_hwStat[((int)(src & 0xFF))].last_cnt = cnt;
    }
    else if(typ == 'R')                                         /* RESPONSE */
    {
      /* valido que sea un mensaje que estaba esperando y lo saco de la cola */
      if( !Dequeue(src, fcn, cnt)) return 0;
      /* proceso según el mensaje */
      switch(fcn)
      {
        case 'P':
          if( !m_hwStat[((int)(src & 0xFF))].on_line )
          {
            m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Dispositivo nuevo 0x%02X "
                  "(HW T:0x%02X V:0x%02X SW:0x%02X)",
                  (int)(src & 0xFF), data[0], data[1], data[2]);
          }
          else
          {
            if( m_hwStat[((int)(src & 0xFF))].hw_typ != data[0] ||
                m_hwStat[((int)(src & 0xFF))].hw_ver != data[1] ||
                m_hwStat[((int)(src & 0xFF))].sw_ver != data[2]  )
            {
              m_pLog->Add(GLOG_LEVEL_WARNING, "WARNING: Dispositivo cambiado: "
                    "viejo 0x%02X (HW T:0x%02X V:0x%02X SW:0x%02X)",
                    (int)(src & 0xFF), m_hwStat[((int)(src & 0xFF))].hw_typ, m_hwStat[((int)(src & 0xFF))].hw_ver, m_hwStat[((int)(src & 0xFF))].sw_ver);
              m_pLog->Add(GLOG_LEVEL_WARNING, "WARNING: Dispositivo cambiado: "
                    "nuevo 0x%02X (HW T:0x%02X V:0x%02X SW:0x%02X)",
                    (int)(src & 0xFF), data[0], data[1], data[2]);
            }
          }
          m_hwStat[((int)(src & 0xFF))].hw_typ = data[0];
          m_hwStat[((int)(src & 0xFF))].hw_ver = data[1];
          m_hwStat[((int)(src & 0xFF))].sw_ver = data[2];
          m_hwStat[((int)(src & 0xFF))].up_model = data[3];
          m_hwStat[((int)(src & 0xFF))].display = data[4];
          m_hwStat[((int)(src & 0xFF))].info_06 = data[5];
          m_hwStat[((int)(src & 0xFF))].info_07 = data[6];
          m_hwStat[((int)(src & 0xFF))].info_08 = data[7];
          m_hwStat[((int)(src & 0xFF))].info_09 = data[8];
          m_hwStat[((int)(src & 0xFF))].info_10 = data[9];
          if( !m_hwStat[((int)(src & 0xFF))].on_line)
          {
            m_pLog->Add(GLOG_LEVEL_WARNING, "INFO: Dispositivo 0x%02X ONLINE", (int)(src & 0xFF));
            /* marco los datos desactualizados para forzar el refrezco */
            if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x01) /* MIO-16x16 */
            {
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_e1 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_e2 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_s1 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_s2 = t + 10;
            }
            else if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x02) /* MIO-16x16/AD */
            {
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_e1 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_e2 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_s1 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_s2 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a1 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a2 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a3 = t + 10;
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a4 = t + 10;
            }
            else if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x03) /* MPAD-6 */
            {
              /* nada */
            }
          }
          m_hwStat[((int)(src & 0xFF))].on_line = 1;
          break;
        case '0':

          break;
        case '1':

          break;
        case '2':

          break;
        case '3':

          break;
        case '4':

          break;
        case '5':

          break;
        case '6':
          event_data.address = (int)(src & 0xFF);
          event_data.time = t;
          if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x01)  /* MIO-16x16 */
          {
            if(data[0] == '1')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de ENTRADA 1 de 0x%02X", (int)(src & 0xFF));
              /* pongo el timer para pedir refresh */
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_e1 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x01) != (data[1] & 0x01) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[0] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 0 [%s]", (int)(src & 0xFF), (data[1] & 0x01)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 0;
                  event_data.on_off = (data[1] & 0x01)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x02) != (data[1] & 0x02) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[1] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 1 [%s]", (int)(src & 0xFF), (data[1] & 0x02)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 1;
                  event_data.on_off = (data[1] & 0x02)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x04) != (data[1] & 0x04) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[2] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 2 [%s]", (int)(src & 0xFF), (data[1] & 0x04)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 2;
                  event_data.on_off = (data[1] & 0x04)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x08) != (data[1] & 0x08) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[3] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 3 [%s]", (int)(src & 0xFF), (data[1] & 0x08)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 3;
                  event_data.on_off = (data[1] & 0x08)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x10) != (data[1] & 0x10) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[4] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 4 [%s]", (int)(src & 0xFF), (data[1] & 0x10)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 4;
                  event_data.on_off = (data[1] & 0x10)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x20) != (data[1] & 0x20) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[5] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 5 [%s]", (int)(src & 0xFF), (data[1] & 0x20)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 5;
                  event_data.on_off = (data[1] & 0x20)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x40) != (data[1] & 0x40) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[6] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 6 [%s]", (int)(src & 0xFF), (data[1] & 0x40)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 6;
                  event_data.on_off = (data[1] & 0x40)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 & 0x80) != (data[1] & 0x80) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e1[7] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 7 [%s]", (int)(src & 0xFF), (data[1] & 0x80)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 7;
                  event_data.on_off = (data[1] & 0x80)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw1.e1 = data[1];
            }
            else if(data[0] == '2')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de ENTRADA 2 de 0x%02X", (int)(src & 0xFF));
              /* pongo el timer para pedir refresh */
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_e2 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x01) != (data[1] & 0x01) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[0] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 0 [%s]", (int)(src & 0xFF), (data[1] & 0x01)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 0;
                  event_data.on_off = (data[1] & 0x01)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x02) != (data[1] & 0x02) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[1] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 1 [%s]", (int)(src & 0xFF), (data[1] & 0x02)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 1;
                  event_data.on_off = (data[1] & 0x02)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x04) != (data[1] & 0x04) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[2] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 2 [%s]", (int)(src & 0xFF), (data[1] & 0x04)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 2;
                  event_data.on_off = (data[1] & 0x04)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x08) != (data[1] & 0x08) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[3] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 3 [%s]", (int)(src & 0xFF), (data[1] & 0x08)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 3;
                  event_data.on_off = (data[1] & 0x08)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x10) != (data[1] & 0x10) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[4] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 4 [%s]", (int)(src & 0xFF), (data[1] & 0x10)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 4;
                  event_data.on_off = (data[1] & 0x10)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x20) != (data[1] & 0x20) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[5] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 5 [%s]", (int)(src & 0xFF), (data[1] & 0x20)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 5;
                  event_data.on_off = (data[1] & 0x20)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x40) != (data[1] & 0x40) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[6] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 6 [%s]", (int)(src & 0xFF), (data[1] & 0x40)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 6;
                  event_data.on_off = (data[1] & 0x40)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 & 0x80) != (data[1] & 0x80) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_e2[7] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 7 [%s]", (int)(src & 0xFF), (data[1] & 0x80)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 7;
                  event_data.on_off = (data[1] & 0x80)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw1.e2 = data[1];
            }
          }
          else if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x02)  /* MIO-16x16/AD */
          {
            if(data[0] == '1')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de ENTRADA 1 de 0x%02X", (int)(src & 0xFF));
              /* pongo el timer para pedir refresh */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_e1 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x01) != (data[1] & 0x01) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[0] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 0 [%s]", (int)(src & 0xFF), (data[1] & 0x01)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 0;
                  event_data.on_off = (data[1] & 0x01)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x02) != (data[1] & 0x02) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[1] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 1 [%s]", (int)(src & 0xFF), (data[1] & 0x02)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 1;
                  event_data.on_off = (data[1] & 0x02)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x04) != (data[1] & 0x04) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[2] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 2 [%s]", (int)(src & 0xFF), (data[1] & 0x04)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 2;
                  event_data.on_off = (data[1] & 0x04)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x08) != (data[1] & 0x08) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[3] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 3 [%s]", (int)(src & 0xFF), (data[1] & 0x08)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 3;
                  event_data.on_off = (data[1] & 0x08)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x10) != (data[1] & 0x10) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[4] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 4 [%s]", (int)(src & 0xFF), (data[1] & 0x10)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 4;
                  event_data.on_off = (data[1] & 0x10)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x20) != (data[1] & 0x20) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[5] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 5 [%s]", (int)(src & 0xFF), (data[1] & 0x20)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 5;
                  event_data.on_off = (data[1] & 0x20)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x40) != (data[1] & 0x40) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[6] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 6 [%s]", (int)(src & 0xFF), (data[1] & 0x40)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 6;
                  event_data.on_off = (data[1] & 0x40)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 & 0x80) != (data[1] & 0x80) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e1[7] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I1 bit: 7 [%s]", (int)(src & 0xFF), (data[1] & 0x80)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I1", 2);
                  event_data.bit = 7;
                  event_data.on_off = (data[1] & 0x80)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.e1 = data[1];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.mascara_notif_e1 = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.seguidor_e1_s1 = data[3];
            }
            else if(data[0] == '2')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de ENTRADA 2 de 0x%02X", (int)(src & 0xFF));
              /* pongo el timer para pedir refresh */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_e2 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x01) != (data[1] & 0x01) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[0] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 0 [%s]", (int)(src & 0xFF), (data[1] & 0x01)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 0;
                  event_data.on_off = (data[1] & 0x01)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x02) != (data[1] & 0x02) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[1] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 1 [%s]", (int)(src & 0xFF), (data[1] & 0x02)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 1;
                  event_data.on_off = (data[1] & 0x02)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x04) != (data[1] & 0x04) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[2] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 2 [%s]", (int)(src & 0xFF), (data[1] & 0x04)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 2;
                  event_data.on_off = (data[1] & 0x04)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x08) != (data[1] & 0x08) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[3] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 3 [%s]", (int)(src & 0xFF), (data[1] & 0x08)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 3;
                  event_data.on_off = (data[1] & 0x08)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x10) != (data[1] & 0x10) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[4] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 4 [%s]", (int)(src & 0xFF), (data[1] & 0x10)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 4;
                  event_data.on_off = (data[1] & 0x10)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x20) != (data[1] & 0x20) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[5] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 5 [%s]", (int)(src & 0xFF), (data[1] & 0x20)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 5;
                  event_data.on_off = (data[1] & 0x20)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x40) != (data[1] & 0x40) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[6] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 6 [%s]", (int)(src & 0xFF), (data[1] & 0x40)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 6;
                  event_data.on_off = (data[1] & 0x40)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 & 0x80) != (data[1] & 0x80) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_e2[7] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: I2 bit: 7 [%s]", (int)(src & 0xFF), (data[1] & 0x80)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "I2", 2);
                  event_data.bit = 7;
                  event_data.on_off = (data[1] & 0x80)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.e2 = data[1];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.mascara_notif_e2 = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.seguidor_e2_s2 = data[3];
            }
          }
          else if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x03) /* MPAD-16 */
          {

          }
          break;
        case '7':
          event_data.address = (int)(src & 0xFF);
          event_data.time = t;
          if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x01) /* MIO-16x16 */
          {
            if(data[0] == '1')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de SALIDA 1 de 0x%02X", (int)(src & 0xFF));
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_s1 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x01) != (data[1] & 0x01) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[0] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x02) != (data[1] & 0x02) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[1] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x04) != (data[1] & 0x04) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[2] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x08) != (data[1] & 0x08) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[3] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x10) != (data[1] & 0x10) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[4] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x20) != (data[1] & 0x20) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[5] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x40) != (data[1] & 0x40) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[6] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 & 0x80) != (data[1] & 0x80) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s1[7] = t;
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw1.s1 = data[1];
            }
            else if(data[0] == '2')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de SALIDA 2 de 0x%02X", (int)(src & 0xFF));
              m_hwStat[((int)(src & 0xFF))].typ.hw1.next_update_s2 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x01) != (data[1] & 0x01) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[0] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x02) != (data[1] & 0x02) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[1] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x04) != (data[1] & 0x04) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[2] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x08) != (data[1] & 0x08) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[3] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x10) != (data[1] & 0x10) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[4] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x20) != (data[1] & 0x20) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[5] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x40) != (data[1] & 0x40) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[6] = t;
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 & 0x80) != (data[1] & 0x80) )
                m_hwStat[((int)(src & 0xFF))].typ.hw1.last_change_s2[7] = t;
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw1.s2 = data[1];
            }
          }
          if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x02) /* MIO-16x16/AD */
          {
            if(data[0] == '1')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de SALIDA 1 de 0x%02X", (int)(src & 0xFF));
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_s1 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x01) != (data[1] & 0x01) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[0] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 0 [%s]", (int)(src & 0xFF), (data[1] & 0x01)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 0;
                  event_data.on_off = (data[1] & 0x01)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x02) != (data[1] & 0x02) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[1] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 1 [%s]", (int)(src & 0xFF), (data[1] & 0x02)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 1;
                  event_data.on_off = (data[1] & 0x02)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x04) != (data[1] & 0x04) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[2] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 2 [%s]", (int)(src & 0xFF), (data[1] & 0x04)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 2;
                  event_data.on_off = (data[1] & 0x04)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x08) != (data[1] & 0x08) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[3] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 3 [%s]", (int)(src & 0xFF), (data[1] & 0x08)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 3;
                  event_data.on_off = (data[1] & 0x08)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x10) != (data[1] & 0x10) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[4] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 4 [%s]", (int)(src & 0xFF), (data[1] & 0x10)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 4;
                  event_data.on_off = (data[1] & 0x10)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x20) != (data[1] & 0x20) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[5] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 5 [%s]", (int)(src & 0xFF), (data[1] & 0x20)?"off -> on":"on -> off");
                /* comunico el evento */
                memcpy(event_data.port, "O1", 2);
                event_data.bit = 5;
                event_data.on_off = (data[1] & 0x20)?1:0;
                m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x40) != (data[1] & 0x40) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[6] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 6 [%s]", (int)(src & 0xFF), (data[1] & 0x40)?"off -> on":"on -> off");
                /* comunico el evento */
                memcpy(event_data.port, "O1", 2);
                event_data.bit = 6;
                event_data.on_off = (data[1] & 0x40)?1:0;
                m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 & 0x80) != (data[1] & 0x80) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s1[7] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O1 bit: 7 [%s]", (int)(src & 0xFF), (data[1] & 0x80)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O1", 2);
                  event_data.bit = 7;
                  event_data.on_off = (data[1] & 0x80)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.s1 = data[1];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.mascara_notif_s1 = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.seguidor_e1_s1 = data[3];
            }
            else if(data[0] == '2')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de SALIDA 2 de 0x%02X", (int)(src & 0xFF));
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_s2 = t + 720; /* 12 minutos */
              /* detecto los cambios */
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x01) != (data[1] & 0x01) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[0] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 0 [%s]", (int)(src & 0xFF), (data[1] & 0x01)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 0;
                  event_data.on_off = (data[1] & 0x01)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x02) != (data[1] & 0x02) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[1] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 1 [%s]", (int)(src & 0xFF), (data[1] & 0x02)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 1;
                  event_data.on_off = (data[1] & 0x02)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x04) != (data[1] & 0x04) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[2] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 2 [%s]", (int)(src & 0xFF), (data[1] & 0x04)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 2;
                  event_data.on_off = (data[1] & 0x04)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x08) != (data[1] & 0x08) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[3] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 3 [%s]", (int)(src & 0xFF), (data[1] & 0x08)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 3;
                  event_data.on_off = (data[1] & 0x05)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x10) != (data[1] & 0x10) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[4] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 4 [%s]", (int)(src & 0xFF), (data[1] & 0x10)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 4;
                  event_data.on_off = (data[1] & 0x10)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x20) != (data[1] & 0x20) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[5] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 5 [%s]", (int)(src & 0xFF), (data[1] & 0x20)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 5;
                  event_data.on_off = (data[1] & 0x20)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x40) != (data[1] & 0x40) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[6] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 6 [%s]", (int)(src & 0xFF), (data[1] & 0x40)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 6;
                  event_data.on_off = (data[1] & 0x40)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              if( (m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 & 0x80) != (data[1] & 0x80) )
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.last_change_s2[7] = t;
                m_pLog->Add(GLOG_LEVEL_DEBUG, "dev: 0x%02X port: O2 bit: 7 [%s]", (int)(src & 0xFF), (data[1] & 0x80)?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "O2", 2);
                  event_data.bit = 7;
                  event_data.on_off = (data[1] & 0x80)?1:0;
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
              /* actualizo los datos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.s2 = data[1];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.mascara_notif_s2 = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.seguidor_e2_s2 = data[3];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.seguidor_an_s2 = data[4];
            }
          }
          else if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x03) /* MPAD-16 */
          {

          }
          break;
        case '8':     /* Respuesta con datos de entradas analógicas */
          event_data.address = (int)(src & 0xFF);
          event_data.time = t;
          if(m_hwStat[((int)(src & 0xFF))].hw_typ == 0x02) /* MIO-16x16/AD */
          {
            if(data[0] == '1')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de ANALOG 1 de 0x%02X", (int)(src & 0xFF));
              /* pongo el timer para pedir refresh */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a1 = t + 300; /* 5 minutos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_h = data[1];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_l = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_maxh = data[3];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_maxl = data[4];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_minh  = data[5];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_minl  = data[6];
              if(m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_alarm != data[7])
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.a1_alarm = data[7];
                m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo alarma de ANALOG %c de 0x%02X [%s]",
                  data[0], (int)(src & 0xFF), (data[7] == '1')?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "A1", 2);
                  event_data.bit = 0;
                  event_data.on_off = data[7] - '0';
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
            }
            else if(data[0] == '2')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de ANALOG 2 de 0x%02X", (int)(src & 0xFF));
              /* pongo el timer para pedir refresh */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a2 = t + 300; /* 5 minutos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_h = data[1];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_l = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_maxh = data[3];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_maxl = data[4];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_minh  = data[5];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_minl  = data[6];
              if(m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_alarm != data[7])
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.a2_alarm = data[7];
                m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo alarma de ANALOG %c de 0x%02X [%s]",
                  data[0], (int)(src & 0xFF), (data[7] == '1')?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "A2", 2);
                  event_data.bit = 0;
                  event_data.on_off = data[7] - '0';
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
            }
            else if(data[0] == '3')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de ANALOG 3 de 0x%02X", (int)(src & 0xFF));
              /* pongo el timer para pedir refresh */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a3 = t + 300; /* 5 minutos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_h = data[1];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_l = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_maxh = data[3];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_maxl = data[4];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_minh  = data[5];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_minl  = data[6];
              if(m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_alarm != data[7])
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.a3_alarm = data[7];
                m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo alarma de ANALOG %c de 0x%02X [%s]",
                  data[0], (int)(src & 0xFF), (data[7] == '1')?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "A3", 2);
                  event_data.bit = 0;
                  event_data.on_off = data[7] - '0';
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
            }
            else if(data[0] == '4')
            {
              m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo datos de ANALOG 4 de 0x%02X", (int)(src & 0xFF));
              /* pongo el timer para pedir refresh */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.next_update_a4 = t + 300; /* 5 minutos */
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_h = data[1];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_l = data[2];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_maxh = data[3];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_maxl = data[4];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_minh  = data[5];
              m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_minl  = data[6];
              if(m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_alarm != data[7])
              {
                m_hwStat[((int)(src & 0xFF))].typ.hw2.a4_alarm = data[7];
                m_pLog->Add(GLOG_LEVEL_DEBUG, "Recibiendo alarma de ANALOG %c de 0x%02X [%s]",
                  data[0], (int)(src & 0xFF), (data[7] == '1')?"off -> on":"on -> off");
                if( ((int)(src & 0xFF)) != 254 )
                {
                  /* comunico el evento */
                  memcpy(event_data.port, "A4", 2);
                  event_data.bit = 0;
                  event_data.on_off = data[7] - '0';
                  m_pServer->Post("port-change", &event_data, sizeof(EVENT_NOTIF));
                }
              }
            }
            /* para todos los canales */
            m_hwStat[((int)(src & 0xFF))].typ.hw2.mascara_notif_an = data[8];
            m_hwStat[((int)(src & 0xFF))].typ.hw2.seguidor_an_s2 = data[9];
          }
          break;
      }
    }
    else if(typ == 'E')                                         /* ERROR */
    {
      m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR: Respuesta de error desde 0x%02X "
              "cnt: 0x%02X fcn: %c data: 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X "
              "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
          (int)(src & 0xFF),
          (int)(cnt & 0xFF),
          fcn,
          (int)(data[0] & 0xFF),
          (int)(data[1] & 0xFF),
          (int)(data[2] & 0xFF),
          (int)(data[3] & 0xFF),
          (int)(data[4] & 0xFF),
          (int)(data[5] & 0xFF),
          (int)(data[6] & 0xFF),
          (int)(data[7] & 0xFF),
          (int)(data[8] & 0xFF),
          (int)(data[9] & 0xFF));
      /* lo saco de la cola */
      Dequeue(src, fcn, cnt);
    }
    return 1;
  }
  return 0;
}

int CSerDev::MaxWait()
{
  int i;
  int delay, max_wait = -1;
  time_t t;
  ser_msg *msg;

  t = time(&t);
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: MaxWait()");
  for(i = 1; i <= 255; i++) /* Queda excluido el HOST addr=0 */
  {
    if( (msg = GetNext(i)) != NULL ) 
    {
      /* si hay un mensaje sin enviar se debe despachar ya */
      if( !msg->enviado) return 0;
      /* para el resto veo... */
      delay = msg->time_out - t;
      if(delay < 0) delay = 0;
      if(max_wait < 0 || delay < max_wait)
      {
        max_wait = delay;
      }
    }
    if(i < 255) /* en el buffer de broadcast no chequeo versiones */
    {
      /* calculo el minimo tiempo de refrezco */
      if(m_hwStat[i].hw_typ == 0x01) /* MIO-16x16 */
      {
        delay = m_hwStat[i].typ.hw1.next_update_e1 - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
        delay = m_hwStat[i].typ.hw1.next_update_e2 - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
        delay = m_hwStat[i].typ.hw1.next_update_s1 - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
        delay = m_hwStat[i].typ.hw1.next_update_s2 - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
      }
      else if(m_hwStat[i].hw_typ == 0x02) /* MIO-16x16/AD */
      {
        delay = m_hwStat[i].typ.hw2.next_update_e1 - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
        delay = m_hwStat[i].typ.hw2.next_update_e2 - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
        delay = m_hwStat[i].typ.hw2.next_update_s1 - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
        delay = m_hwStat[i].typ.hw2.next_update_s2 - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
        delay = m_hwStat[i].typ.hw2.next_update_a1 - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
        delay = m_hwStat[i].typ.hw2.next_update_a2 - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
        delay = m_hwStat[i].typ.hw2.next_update_a3 - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
        delay = m_hwStat[i].typ.hw2.next_update_a4 - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
      }
      else if(m_hwStat[i].hw_typ == 0x03) /* MPAD-6 */
      {
        delay = m_hwStat[i].typ.hw3.next_update - t;
        if(delay < 0) delay = 0;
        if(max_wait < 0 || delay < max_wait)
        {
          max_wait = delay;
        }
      }
    }
  }
  if(max_wait == 0) max_wait = 1;
  return ((max_wait >= 0)?max_wait:120); /* si no hay timers devuelve dos minutos */
}

int CSerDev::Check()
{
  int i;
  time_t t;
  ser_msg *msg;
  ser_msg new_msg;
  struct timespec ts;

  t = time(&t);

  for(i = 255; i > 0; i--) /* Queda excluido HOST */
  {
    if( (msg = GetNext(i)) != NULL ) 
    {
      if( !msg->enviado )
      {
        m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: SEND typ: %c dst: 0x%02X "
                    "cnt: 0x%02X fcn: %c data: 0x%02X 0x%02X 0x%02X 0x%02X "
                    "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
                    'Q', (int)(i & 0xFF),
                    (int)(msg->cnt & 0xFF),
                    msg->fcn,
                    (int)(msg->data[0] & 0xFF),
                    (int)(msg->data[1] & 0xFF),
                    (int)(msg->data[2] & 0xFF),
                    (int)(msg->data[3] & 0xFF),
                    (int)(msg->data[4] & 0xFF),
                    (int)(msg->data[5] & 0xFF),
                    (int)(msg->data[6] & 0xFF),
                    (int)(msg->data[7] & 0xFF),
                    (int)(msg->data[8] & 0xFF),
                    (int)(msg->data[9] & 0xFF));
        CSerComm::Send('Q', i, msg->cnt, msg->fcn, msg->data);
        msg->enviado = 1;

        /* hago una pausa para que no se me peguen todos los mensaje */
        ts.tv_sec = 0l;
        ts.tv_nsec = 100000000l;
        while( nanosleep(&ts, &ts)  == -1 );

        /* si hay algo de broadcast encolado primero se desagota toda esta cola */
        if(i == 255)
        {
          msg->time_out = 0;
          return 0;
        }
        else
        {
          msg->time_out = t + msg->resend_to;
        }
      }
      /* verifico time-out */
      else if(msg->time_out < t)
      {
        /* no me quejo de los time-out de broadcast */
        if(i != 255)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Time-Out de 0x%02X ", (int)(i & 0xFF));
          /* si no contesta el PING lo saco de linea */
          if(msg->fcn == 'P')
          {
            if(m_hwStat[i].on_line > 0)
            {
              m_pLog->Add(GLOG_LEVEL_WARNING, "INFO: Dispositivo 0x%02X OFFLINE", (int)(i & 0xFF));
            }
            m_hwStat[i].on_line = 0;
          }
        }
        if(msg->resend > 0)
        { /* cuando el resend llega a cero no mando mas y descarto el mensaje */
          msg->resend--;
          if(msg->resend == 0)
          {
            Dequeue(i, msg->fcn, msg->cnt);
            /* si el mensaje era de Broadcast no proceso el resto de las colas
               salgo para que vuelva a entrar y lo primero que haga sea procesar broadcast de nuevo */
            if(i == 255)
            {
              return 0;
            }
            else
            {
              m_pLog->Add(GLOG_LEVEL_WARNING,
                    "WARNING: Mensaje descartado para 0x%02X", (int)(i & 0xFF));
              m_pLog->Add(GLOG_LEVEL_WARNING,
                          "MSG: to: 0x%02X cnt: 0x%02X fcn: %c "
                          "data: 0x%02X 0x%02X 0x%02X 0x%02X "
                          "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
                          (int)(i & 0xFF),
                          (int)(msg->cnt & 0xFF),
                          msg->fcn,
                          (int)(msg->data[0] & 0xFF),
                          (int)(msg->data[1] & 0xFF),
                          (int)(msg->data[2] & 0xFF),
                          (int)(msg->data[3] & 0xFF),
                          (int)(msg->data[4] & 0xFF),
                          (int)(msg->data[5] & 0xFF),
                          (int)(msg->data[6] & 0xFF),
                          (int)(msg->data[7] & 0xFF),
                          (int)(msg->data[8] & 0xFF),
                          (int)(msg->data[9] & 0xFF));
              if(m_hwStat[i].on_line > 0)
              {
                m_pLog->Add(GLOG_LEVEL_WARNING, "INFO: Dispositivo 0x%02X OFFLINE", (int)(i & 0xFF));
              }
              m_hwStat[i].on_line = 0;
              continue;
            }
          }
        }
        /* reenvío el mensaje */
        m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: SEND typ: %c dst: 0x%02X "
                    "cnt: 0x%02X fcn: %c data: 0x%02X 0x%02X 0x%02X 0x%02X "
                    "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X",
                    'Q', (int)(i & 0xFF),
                    (int)(msg->cnt & 0xFF),
                    msg->fcn,
                    (int)(msg->data[0] & 0xFF),
                    (int)(msg->data[1] & 0xFF),
                    (int)(msg->data[2] & 0xFF),
                    (int)(msg->data[3] & 0xFF),
                    (int)(msg->data[4] & 0xFF),
                    (int)(msg->data[5] & 0xFF),
                    (int)(msg->data[6] & 0xFF),
                    (int)(msg->data[7] & 0xFF),
                    (int)(msg->data[8] & 0xFF),
                    (int)(msg->data[9] & 0xFF));
        CSerComm::Send('Q', i, msg->cnt, msg->fcn, msg->data);

        /* hago una pausa para que no se me peguen todos los mensaje */
        ts.tv_sec = 0l;
        ts.tv_nsec = 100000000l;
        while( nanosleep(&ts, &ts)  == -1 );

        if(i == 255)
        {
          msg->time_out = 0;
          return 0;
        }
        else
        {
          msg->time_out = t + msg->resend_to;
          msg->resend_to++;
        }
      }
    }
    else if(m_hwStat[i].on_line && i != 255)/* de if( GetNext(i, &msg) ) ... */
    {
      /* si no tiene nada pendiente pero existe me fijo si están desactualizados los estados */
      if(m_hwStat[i].hw_typ == 0x01) /* MIO-16x16 */
      {
        new_msg.fcn = 0;
        if(m_hwStat[i].typ.hw1.next_update_e1 < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Pidiendo estado de E1 a 0x%02X ",
                                      (int)(i & 0xFF));
          new_msg.fcn = '6';
          new_msg.data[0] = '1';
        }
        else if(m_hwStat[i].typ.hw1.next_update_e2 < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Pidiendo estado de E2 a 0x%02X ",
                                      (int)(i & 0xFF));
          new_msg.fcn = '6';
          new_msg.data[0] = '2';
        }
        else if(m_hwStat[i].typ.hw1.next_update_s1 < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Pidiendo estado de S1 a 0x%02X ",
                                      (int)(i & 0xFF));
          new_msg.fcn = '7';
          new_msg.data[0] = '1';
        }
        else if(m_hwStat[i].typ.hw1.next_update_s2 < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Pidiendo estado de S2 a 0x%02X ",
                                      (int)(i & 0xFF));
          new_msg.fcn = '7';
          new_msg.data[0] = '2';
        }
        /* si hay algo para mandar... */
        if(new_msg.fcn)
        {
          new_msg.cnt = ++m_cCnt;
          new_msg.data[1] = ' ';
          new_msg.data[2] = ' ';
          new_msg.data[3] = ' ';
          new_msg.data[4] = ' ';
          new_msg.data[5] = ' ';
          new_msg.data[6] = ' ';
          new_msg.data[7] = ' ';
          new_msg.data[8] = ' ';
          new_msg.data[9] = ' ';
          new_msg.resend_to = DEFAULT_RESEND_TO;
          new_msg.resend = DEFAULT_RESEND;

          Enqueue(i, &new_msg);
        }
        else if((m_hwStat[i].last_rcv + 600) < t) /* si no tengo nada pendiente y hace mucho que no me habla le mando un ping */
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Enviando ping a 0x%02X ",
                                      (int)(i & 0xFF));
          /* mando un ping para ver si está vivo */

          /* infinito, no sale nada mas hasta que conteste este */
          new_msg.resend = INFINITE_RESEND;
          new_msg.fcn = 'P';
          memcpy(new_msg.data, "\0\0\0       ", 10);
          new_msg.resend_to = 120;

          Enqueue(i, &new_msg);
        }
      }
      else if(m_hwStat[i].hw_typ == 0x02) /* MIO-16x16-AD */
      {
        new_msg.fcn = 0;
        if(m_hwStat[i].typ.hw2.next_update_e1 < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Pidiendo estado de E1 a 0x%02X ",
                                      (int)(i & 0xFF));
          new_msg.fcn = '6';
          new_msg.data[0] = '1';
        }
        else if(m_hwStat[i].typ.hw2.next_update_e2 < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Pidiendo estado de E2 a 0x%02X ",
                                      (int)(i & 0xFF));
          new_msg.fcn = '6';
          new_msg.data[0] = '2';
        }
        else if(m_hwStat[i].typ.hw2.next_update_s1 < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Pidiendo estado de S1 a 0x%02X ",
                                      (int)(i & 0xFF));
          new_msg.fcn = '7';
          new_msg.data[0] = '1';
        }
        else if(m_hwStat[i].typ.hw2.next_update_s2 < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Pidiendo estado de S2 a 0x%02X ",
                                      (int)(i & 0xFF));
          new_msg.fcn = '7';
          new_msg.data[0] = '2';
        }
        else if(m_hwStat[i].typ.hw2.next_update_a1 < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Pidiendo estado de A1 a 0x%02X ",
                                      (int)(i & 0xFF));
          new_msg.fcn = '8';
          new_msg.data[0] = '1';
        }
        else if(m_hwStat[i].typ.hw2.next_update_a2 < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Pidiendo estado de A2 a 0x%02X ",
                                      (int)(i & 0xFF));
          new_msg.fcn = '8';
          new_msg.data[0] = '2';
        }
        else if(m_hwStat[i].typ.hw2.next_update_a3 < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Pidiendo estado de A3 a 0x%02X ",
                                      (int)(i & 0xFF));
          new_msg.fcn = '8';
          new_msg.data[0] = '3';
        }
        else if(m_hwStat[i].typ.hw2.next_update_a4 < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Pidiendo estado de A4 a 0x%02X ",
                                      (int)(i & 0xFF));
          new_msg.fcn = '8';
          new_msg.data[0] = '4';
        }
        /* si hay algo para mandar... */
        if(new_msg.fcn)
        {
          /* armo el resto del mensaje */
          new_msg.data[1] = ' ';
          new_msg.data[2] = ' ';
          new_msg.data[3] = ' ';
          new_msg.data[4] = ' ';
          new_msg.data[5] = ' ';
          new_msg.data[6] = ' ';
          new_msg.data[7] = ' ';
          new_msg.data[8] = ' ';
          new_msg.data[9] = ' ';
          new_msg.resend_to = DEFAULT_RESEND_TO;
          new_msg.resend = DEFAULT_RESEND;

          Enqueue(i, &new_msg);
        }
        else if((m_hwStat[i].last_rcv + 600) < t) /* si no tengo nada pendiente y hace mucho que no me habla le mando un ping */
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Enviando ping a 0x%02X ",
                                      (int)(i & 0xFF));
          /* mando un ping para ver si está vivo */
          /* infinito, no sale nada mas hasta que conteste este */
          new_msg.resend = INFINITE_RESEND;
          new_msg.fcn = 'P';
          memcpy(new_msg.data, "\0\0\0       ", 10);
          new_msg.resend_to = 120;

          Enqueue(i, &new_msg);
        }
      }
      else if(m_hwStat[i].hw_typ == 0x03) /* MPAD-6 */
      {
        if(m_hwStat[i].typ.hw3.next_update < t)
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Actualizando estado de salidas a 0x%02X ",
                                      (int)(i & 0xFF));
          SetOut((int)(i & 0xFF), 1, m_hwStat[i].typ.hw3.s[0]);
          SetOut((int)(i & 0xFF), 2, m_hwStat[i].typ.hw3.s[1]);
          SetOut((int)(i & 0xFF), 3, m_hwStat[i].typ.hw3.s[2]);
          SetOut((int)(i & 0xFF), 4, m_hwStat[i].typ.hw3.s[3]);
          SetOut((int)(i & 0xFF), 5, m_hwStat[i].typ.hw3.s[4]);
          SetOut((int)(i & 0xFF), 6, m_hwStat[i].typ.hw3.s[5]);
        }
        else if((m_hwStat[i].last_rcv + 600) < t) /* si no tengo nada pendiente y hace mucho que no me habla le mando un ping */
        {
          m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Enviando ping a 0x%02X ",
                                      (int)(i & 0xFF));
          /* mando un ping para ver si está vivo */
          /* infinito, no sale nada mas hasta que conteste este */
          new_msg.resend = INFINITE_RESEND;
          new_msg.fcn = 'P';
          memcpy(new_msg.data, "\0\0\0       ", 10);
          new_msg.resend_to = 120;

          Enqueue(i, &new_msg);
        }
      }
    }
    else if(m_hwStat[i].hw_typ && m_hwStat[i].hw_ver && m_hwStat[i].sw_ver && i != 255)
    {
      /* si está offline pero alguna vez estuvo on ... */
      Ping(i, INFINITE_RESEND);
    }
  }
  return 0;  
}

int CSerDev::Discover()
{
  int i;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Discover()");
  for(i = 1; i < /*255*/11; i++) /* Queda excluido HOST y BROADCAST */ /* escaneo solo los primeros 10 */
  {
    if( !m_hwStat[i].on_line)
    {
      Ping(i, DEFAULT_RESEND);
    }
    while(Process());
  }
  return 0;  
}

int CSerDev::Ping(int dst, int resend)
{
  time_t t;
  ser_msg msg;

  t = time(&t);
  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Ping(0x%02X)", (int)(dst & 0xFF));
	if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR;
  m_pLog->Add(GLOG_LEVEL_INFO, "INFO: Enviando ping a 0x%02X ", (int)(dst & 0xFF));
  /* mando un ping */
  msg.fcn = 'P';
  memcpy(msg.data, "\0\0\0       ", 10);
  msg.resend_to = 45;
  msg.resend = resend;

  Enqueue(dst, &msg);
  return GME_OK;  
}

int CSerDev::SetBit(int dst, int sal, int bit, int on_off)
{
  ser_msg msg;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: SetBit(0x%02X, 0x%02X, 0x%02X, 0x%02X)",
              (int)(dst & 0xFF), (int)(sal & 0xFF), (int)(bit & 0xFF), (int)(on_off & 0xFF));
	if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR;
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (SetBit) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
  }

  msg.fcn = '3';
  msg.data[0] = '0' + sal;
  msg.data[1] = '0' + bit;
  msg.data[2] = '0' + on_off;
  msg.data[3] = ' ';
  msg.data[4] = ' ';
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  if(m_hwStat[((int)dst&0xFF)].hw_typ == 0x01) /* MIO-16x16 */
  {
    /* marco como desactualizada la salida correspondiente */
/* ya no es necesario porque las  salidas informan cuando cambian
    if(sal == 1)
      m_hwStat[((int)dst&0xFF)].typ.hw1.next_update_s1 = 0;
    else if(sal == 2)
      m_hwStat[((int)dst&0xFF)].typ.hw1.next_update_s2 = 0;
*/
  }
  else if(m_hwStat[((int)dst&0xFF)].hw_typ == 0x02) /* MIO-16x16/AD */
  {
    /* marco como desactualizada la salida correspondiente */
/* ya no es necesario porque las  salidas informan cuando cambian
    if(sal == 1)
      m_hwStat[((int)dst&0xFF)].typ.hw2.next_update_s1 = 0;
    else if(sal == 2)
      m_hwStat[((int)dst&0xFF)].typ.hw2.next_update_s2 = 0;
*/
  }
  return GME_OK;  
}

int CSerDev::PulseBit(int dst, int sal, int bit, int seg)
{
  ser_msg msg;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: PulseBit(0x%02X, 0x%02X, 0x%02X)", (int)(dst & 0xFF), (int)(sal & 0xFF), (int)(bit & 0xFF));
	if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR;
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (PulseBit) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
  }

  msg.fcn = '5';
  msg.data[0] = '0' + sal;
  msg.data[1] = '0' + bit;
  msg.data[2] = (char)(seg & 0xFF);
  msg.data[3] = ' ';
  msg.data[4] = ' ';
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  return GME_OK;  
}

int CSerDev::InvertBit(int dst, int sal, int bit)
{
  ser_msg msg;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: InvertBit(0x%02X, 0x%02X, 0x%02X)",
              (int)(dst & 0xFF), (int)(sal & 0xFF), (int)(bit & 0xFF));
	if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR;
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (InvertBit) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
  }

  msg.fcn = 'I';
  msg.data[0] = '0' + sal;
  msg.data[1] = '0' + bit;
  msg.data[2] = ' ';
  msg.data[3] = ' ';
  msg.data[4] = ' ';
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);

  if(m_hwStat[((int)dst&0xFF)].hw_typ == 0x01) /* MIO-16x16 */
  {
    /* marco como desactualizada la salida correspondiente */
/* ya no es necesario porque las  salidas informan cuando cambian
    if(sal == 1)
      m_hwStat[((int)dst&0xFF)].typ.hw1.next_update_s1 = 0;
    else if(sal == 2)
      m_hwStat[((int)dst&0xFF)].typ.hw1.next_update_s2 = 0;
*/
  }
  else if(m_hwStat[((int)dst&0xFF)].hw_typ == 0x02) /* MIO-16x16/AD */
  {
    /* marco como desactualizada la salida correspondiente */
/* ya no es necesario porque las  salidas informan cuando cambian
    if(sal == 1)
      m_hwStat[((int)dst&0xFF)].typ.hw2.next_update_s1 = 0;
    else if(sal == 2)
      m_hwStat[((int)dst&0xFF)].typ.hw2.next_update_s2 = 0;
*/
  }
  return GME_OK;  
}

vector <CSerDev::HWLIST> CSerDev::GetList()
{
  int i;
  HWLIST  hw_list;
  vector <HWLIST> v_hw_list;

  v_hw_list.clear();

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: GetList()");
  for(i = 1; i < 255; i++) /* Queda excluido HOST y BROADCAST */
  {
    /* los dispositivos tiene que tener algo en las versiones, sino es basura */
    if(m_hwStat[i].hw_typ && m_hwStat[i].hw_ver && m_hwStat[i].sw_ver)
    {
      hw_list.hw_addr = (char)i&0xFF;
      hw_list.on_line = m_hwStat[i].on_line;
      hw_list.hw_typ = m_hwStat[i].hw_typ;
    	hw_list.hw_ver = m_hwStat[i].hw_ver;
      hw_list.sw_ver = m_hwStat[i].sw_ver;
      hw_list.up_model = m_hwStat[i].up_model;
      hw_list.display = m_hwStat[i].display;
      hw_list.info_06 = m_hwStat[i].info_06;
      hw_list.info_07 = m_hwStat[i].info_07;
      hw_list.info_08 = m_hwStat[i].info_08;
      hw_list.info_09 = m_hwStat[i].info_09;
      hw_list.info_10 = m_hwStat[i].info_10;
      v_hw_list.push_back(hw_list);
    }
  }
  return v_hw_list;
}

vector <CSerDev::HWINFO> CSerDev::GetInfo(int dst)
{
  HWINFO  hw_info;
  vector <HWINFO> v_hw_info;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: GetInfo()");
	if(dst <= 0x00 || dst >= 0xFF)
  {
    return v_hw_info;
  }
  v_hw_info.clear();
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (GetInfo) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
    return v_hw_info;
  }
  if(m_hwStat[((int)dst&0xFF)].hw_typ == 0x01) /* MIO-16x16 */
  {
    hw_info.high_port_val = 0;
    hw_info.alarm_maxh = 0;
    hw_info.alarm_maxl = 0;
    hw_info.alarm_minh = 0;
    hw_info.alarm_minl = 0;
    hw_info.alarm_state = 0;
    hw_info.seguidor_val = 0;

    hw_info.port_typ = 'I';
    hw_info.port_num = '1';
    hw_info.port_val = m_hwStat[((int)dst&0xFF)].typ.hw1.e1;
    memcpy( &hw_info.port_change, m_hwStat[((int)dst&0xFF)].typ.hw1.last_change_e1, sizeof(long[8]));
    v_hw_info.push_back(hw_info);

    hw_info.port_typ = 'I';
    hw_info.port_num = '2';
    hw_info.port_val = m_hwStat[((int)dst&0xFF)].typ.hw1.e2;
    memcpy( &hw_info.port_change, m_hwStat[((int)dst&0xFF)].typ.hw1.last_change_e2, sizeof(long[8]));
    v_hw_info.push_back(hw_info);

    hw_info.port_typ = 'O';
    hw_info.port_num = '1';
    hw_info.port_val = m_hwStat[((int)dst&0xFF)].typ.hw1.s1;
    memcpy( &hw_info.port_change, m_hwStat[((int)dst&0xFF)].typ.hw1.last_change_s1, sizeof(long[8]));
    v_hw_info.push_back(hw_info);

    hw_info.port_typ = 'O';
    hw_info.port_num = '2';
    hw_info.port_val = m_hwStat[((int)dst&0xFF)].typ.hw1.s2;
    memcpy( &hw_info.port_change, m_hwStat[((int)dst&0xFF)].typ.hw1.last_change_s2, sizeof(long[8]));
    v_hw_info.push_back(hw_info);
  }
  if(m_hwStat[((int)dst&0xFF)].hw_typ == 0x02) /* MIO-16x16/AD */
  {
    hw_info.high_port_val = 0;
    hw_info.alarm_maxh = 0;
    hw_info.alarm_maxl = 0;
    hw_info.alarm_minh = 0;
    hw_info.alarm_minl = 0;
    hw_info.alarm_state = 0;

    hw_info.port_typ = 'I';
    hw_info.port_num = '1';
    hw_info.port_val = m_hwStat[((int)dst&0xFF)].typ.hw2.e1;
    hw_info.info_mask = m_hwStat[((int)dst&0xFF)].typ.hw2.mascara_notif_e1;
    hw_info.seguidor_val = m_hwStat[((int)dst&0xFF)].typ.hw2.seguidor_e1_s1;
    memcpy( &hw_info.port_change, m_hwStat[((int)dst&0xFF)].typ.hw2.last_change_e1, sizeof(long[8]));
    v_hw_info.push_back(hw_info);

    hw_info.port_typ = 'I';
    hw_info.port_num = '2';
    hw_info.port_val = m_hwStat[((int)dst&0xFF)].typ.hw2.e2;
    hw_info.info_mask = m_hwStat[((int)dst&0xFF)].typ.hw2.mascara_notif_e2;
    hw_info.seguidor_val = m_hwStat[((int)dst&0xFF)].typ.hw2.seguidor_e2_s2;
    memcpy( &hw_info.port_change, m_hwStat[((int)dst&0xFF)].typ.hw2.last_change_e2, sizeof(long[8]));
    v_hw_info.push_back(hw_info);

    hw_info.seguidor_val = 0;

    hw_info.port_typ = 'O';
    hw_info.port_num = '1';
    hw_info.port_val = m_hwStat[((int)dst&0xFF)].typ.hw2.s1;
    hw_info.info_mask = m_hwStat[((int)dst&0xFF)].typ.hw2.mascara_notif_s1;
    memcpy( &hw_info.port_change, m_hwStat[((int)dst&0xFF)].typ.hw2.last_change_s1, sizeof(long[8]));
    v_hw_info.push_back(hw_info);

    hw_info.port_typ = 'O';
    hw_info.port_num = '2';
    hw_info.port_val = m_hwStat[((int)dst&0xFF)].typ.hw2.s2;
    hw_info.info_mask = m_hwStat[((int)dst&0xFF)].typ.hw2.mascara_notif_s2;
    memcpy( &hw_info.port_change, m_hwStat[((int)dst&0xFF)].typ.hw2.last_change_s2, sizeof(long[8]));
    v_hw_info.push_back(hw_info);

    hw_info.port_typ = 'A';
    hw_info.port_num = '1';
    hw_info.high_port_val = m_hwStat[((int)dst&0xFF)].typ.hw2.a1_h;
    hw_info.port_val = m_hwStat[((int)dst&0xFF)].typ.hw2.a1_l;
    hw_info.alarm_state = m_hwStat[((int)dst&0xFF)].typ.hw2.a1_alarm;
    hw_info.alarm_maxh = m_hwStat[((int)dst&0xFF)].typ.hw2.a1_maxh;
    hw_info.alarm_maxl = m_hwStat[((int)dst&0xFF)].typ.hw2.a1_maxl;
    hw_info.alarm_minh = m_hwStat[((int)dst&0xFF)].typ.hw2.a1_minh;
    hw_info.alarm_minl = m_hwStat[((int)dst&0xFF)].typ.hw2.a1_minl;
    memset(&hw_info.port_change, 0, sizeof(long[8]));
    hw_info.info_mask = m_hwStat[((int)dst&0xFF)].typ.hw2.mascara_notif_an;
    hw_info.seguidor_val = m_hwStat[((int)dst&0xFF)].typ.hw2.seguidor_an_s2;
    hw_info.port_change[0] = m_hwStat[((int)dst&0xFF)].typ.hw2.last_change_a1;
    v_hw_info.push_back(hw_info);

    hw_info.port_typ = 'A';
    hw_info.port_num = '2';
    hw_info.high_port_val = m_hwStat[((int)dst&0xFF)].typ.hw2.a2_h;
    hw_info.port_val = m_hwStat[((int)dst&0xFF)].typ.hw2.a2_l;
    hw_info.alarm_state = m_hwStat[((int)dst&0xFF)].typ.hw2.a2_alarm;
    hw_info.alarm_maxh = m_hwStat[((int)dst&0xFF)].typ.hw2.a2_maxh;
    hw_info.alarm_maxl = m_hwStat[((int)dst&0xFF)].typ.hw2.a2_maxl;
    hw_info.alarm_minh = m_hwStat[((int)dst&0xFF)].typ.hw2.a2_minh;
    hw_info.alarm_minl = m_hwStat[((int)dst&0xFF)].typ.hw2.a2_minl;
    memset(&hw_info.port_change, 0, sizeof(long[8]));
    hw_info.info_mask = m_hwStat[((int)dst&0xFF)].typ.hw2.mascara_notif_an;
    hw_info.seguidor_val = m_hwStat[((int)dst&0xFF)].typ.hw2.seguidor_an_s2;
    hw_info.port_change[0] = m_hwStat[((int)dst&0xFF)].typ.hw2.last_change_a2;
    v_hw_info.push_back(hw_info);

    hw_info.port_typ = 'A';
    hw_info.port_num = '3';
    hw_info.high_port_val = m_hwStat[((int)dst&0xFF)].typ.hw2.a3_h;
    hw_info.port_val = m_hwStat[((int)dst&0xFF)].typ.hw2.a3_l;
    hw_info.alarm_state = m_hwStat[((int)dst&0xFF)].typ.hw2.a3_alarm;
    hw_info.alarm_maxh = m_hwStat[((int)dst&0xFF)].typ.hw2.a3_maxh;
    hw_info.alarm_maxl = m_hwStat[((int)dst&0xFF)].typ.hw2.a3_maxl;
    hw_info.alarm_minh = m_hwStat[((int)dst&0xFF)].typ.hw2.a3_minh;
    hw_info.alarm_minl = m_hwStat[((int)dst&0xFF)].typ.hw2.a3_minl;
    memset(&hw_info.port_change, 0, sizeof(long[8]));
    hw_info.info_mask = m_hwStat[((int)dst&0xFF)].typ.hw2.mascara_notif_an;
    hw_info.seguidor_val = m_hwStat[((int)dst&0xFF)].typ.hw2.seguidor_an_s2;
    hw_info.port_change[0] = m_hwStat[((int)dst&0xFF)].typ.hw2.last_change_a3;
    v_hw_info.push_back(hw_info);

    hw_info.port_typ = 'A';
    hw_info.port_num = '4';
    hw_info.high_port_val = m_hwStat[((int)dst&0xFF)].typ.hw2.a4_h;
    hw_info.port_val = m_hwStat[((int)dst&0xFF)].typ.hw2.a4_l;
    hw_info.alarm_state = m_hwStat[((int)dst&0xFF)].typ.hw2.a4_alarm;
    hw_info.alarm_maxh = m_hwStat[((int)dst&0xFF)].typ.hw2.a4_maxh;
    hw_info.alarm_maxl = m_hwStat[((int)dst&0xFF)].typ.hw2.a4_maxl;
    hw_info.alarm_minh = m_hwStat[((int)dst&0xFF)].typ.hw2.a4_minh;
    hw_info.alarm_minl = m_hwStat[((int)dst&0xFF)].typ.hw2.a4_minl;
    memset(&hw_info.port_change, 0, sizeof(long[8]));
    hw_info.info_mask = m_hwStat[((int)dst&0xFF)].typ.hw2.mascara_notif_an;
    hw_info.seguidor_val = m_hwStat[((int)dst&0xFF)].typ.hw2.seguidor_an_s2;
    hw_info.port_change[0] = m_hwStat[((int)dst&0xFF)].typ.hw2.last_change_a4;
    v_hw_info.push_back(hw_info);
  }
  return v_hw_info;
}

int CSerDev::ChangeAddress(int dst, int newaddress)
{
  ser_msg msg;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: ChangeAddress(0x%02X, 0x%02X)", (int)(dst & 0xFF), (int)(newaddress & 0xFF));
	if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR; /* excluyo a broadcast */
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (ChangeAddress) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
  }
  if(m_hwStat[((int)dst&0xFF)].hw_typ < 0x02 )
  {
    m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (ChangeAddress) El dispositivo 0x%02X no soporta esta funcion.", (int)(dst & 0xFF));
    return GME_USER_ERROR;
  }

  msg.fcn = 'L';
  msg.data[0] = (char)(newaddress & 0xFF);
  msg.data[1] = ' ';
  msg.data[2] = ' ';
  msg.data[3] = ' ';
  msg.data[4] = ' ';
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  return GME_OK;  
}

int CSerDev::ChangeBaudRate(int dst, int baudrate)
{
  ser_msg msg;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: ChangeBaudRate(0x%02X, 0x%02X)", (int)(dst & 0xFF), (int)(baudrate & 0xFF));
	if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR; /* excluyo a broadcast */
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (ChangeBaudRate) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
  }
  if(m_hwStat[((int)dst&0xFF)].hw_typ < 0x02 )
  {
    m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (ChangeBaudRate) El dispositivo 0x%02X no soporta esta funcion.", (int)(dst & 0xFF));
    return GME_USER_ERROR;
  }
  switch(baudrate)
  {
    case 1200:
      baudrate = 0xBF;
      break;
    case 2400:
      baudrate = 0x5F;
      break;
    case 9600:
      baudrate = 0x17;
      break;
    case 19200:
      baudrate = 0x0B;
      break;
    default:
      m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR: (ChangeBaudRate) BaudRate %i inválido.", baudrate);
      return GME_USER_ERROR;
  }
  msg.fcn = 'B';
  msg.data[0] = (char)(baudrate & 0xFF);
  msg.data[1] = ' ';
  msg.data[2] = ' ';
  msg.data[3] = ' ';
  msg.data[4] = ' ';
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  return GME_OK;  
}

int CSerDev::Reset(int dst)
{
  ser_msg msg;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Reset(0x%02X)", (int)(dst & 0xFF));
	if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR; /* excluyo a broadcast */
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (Reset) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
  }
  if(m_hwStat[((int)dst&0xFF)].hw_typ < 0x02 )
  {
    m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (Reset) El dispositivo 0x%02X no soporta esta funcion.", (int)(dst & 0xFF));
    return GME_USER_ERROR;
  }
  msg.fcn = 'R';
  msg.data[0] = ' ';
  msg.data[1] = ' ';
  msg.data[2] = ' ';
  msg.data[3] = ' ';
  msg.data[4] = ' ';
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  return GME_OK;  
}

int CSerDev::SetSeguidores(int dst, int ent, int mascara)
{
  ser_msg msg;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: SetSeguidores(0x%02X, 0x%02X, 0x%02X)", (int)(dst & 0xFF), (int)(ent & 0xFF), (int)(mascara & 0xFF));
	if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR; /* excluyo a broadcast */
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (SetSeguidores) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
  }
  if(m_hwStat[((int)dst&0xFF)].hw_typ < 0x02 )
  {
    m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (SetSeguidores) El dispositivo 0x%02X no soporta esta funcion.", (int)(dst & 0xFF));
    return GME_USER_ERROR;
  }
  msg.fcn = 'F';
  if(ent < 10)
    msg.data[0] = '0' + ent;
  else
    msg.data[0] = 'A' + ent - 10;
  msg.data[1] = (char)(mascara & 0xFF);
  msg.data[2] = ' ';
  msg.data[3] = ' ';
  msg.data[4] = ' ';
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  return GME_OK;  
}

int CSerDev::SetMask(int dst, int ent, int mascara)
{
  ser_msg msg;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: SetMask(0x%02X, 0x%02X, 0x%02X)", (int)(dst & 0xFF), (int)(ent & 0xFF), (int)(mascara & 0xFF));
	if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR; /* excluyo a broadcast */
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (SetMask) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
  }
  if(m_hwStat[((int)dst&0xFF)].hw_typ < 0x02 )
  {
    m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (SetMask) El dispositivo 0x%02X no soporta esta funcion.", (int)(dst & 0xFF));
    return GME_USER_ERROR;
  }
  msg.fcn = 'O';
  if(ent < 10)
    msg.data[0] = '0' + ent;
  else
    msg.data[0] = 'A' + ent - 10;
  msg.data[1] = (char)(mascara & 0xFF);
  msg.data[2] = ' ';
  msg.data[3] = ' ';
  msg.data[4] = ' ';
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  return GME_OK;  
}

int CSerDev::SetADAlarm(int dst, int ent, int max, int min)
{
  ser_msg msg;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: SetADAlarm(0x%02X, 0x%04X, 0x%04X)", (int)(dst & 0xFF), (int)(max & 0xFFFF), (int)(min & 0xFFFF));
	if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR; /* excluyo a broadcast */
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (SetADAlarm) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
  }
  if(m_hwStat[((int)dst&0xFF)].hw_typ < 0x02 )
  {
    m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (SetADAlarm) El dispositivo 0x%02X no soporta esta funcion.", (int)(dst & 0xFF));
    return GME_USER_ERROR;
  }
  msg.fcn = 'A';
  msg.data[0] = '0' + ent;
  msg.data[1] = (char)(max / 0xFF);
  msg.data[2] = (char)(max & 0xFF);
  msg.data[3] = (char)(min / 0xFF);
  msg.data[4] = (char)(min & 0xFF);
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  return GME_OK;  
}

int CSerDev::SendMessage(int dst, int line, const char* msg_text, int msg_len)
{
  ser_msg msg;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: SendMessage(0x%02X, %i, [%*.*s])", (int)(dst & 0xFF), line, msg_len, msg_len, msg_text);
	if(dst <= 0x00 || dst > 0xFF) return GME_USER_ERROR;
  if(dst != 0xFF) /* si es la dirección de broadcast algunos controles no tienen sentido */
  {
    if( !m_hwStat[((int)dst&0xFF)].on_line)
    {
      m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (SendMessage) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
      /* trato de despertarlo */
      Ping(dst, 1);
    }
    if(m_hwStat[((int)dst&0xFF)].hw_typ < 0x02 )
    {
      m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (SendMessage) El dispositivo 0x%02X no soporta esta funcion.", (int)(dst & 0xFF));
      return GME_USER_ERROR;
    }
  }
  switch (line)
  {
  case 1:
    msg.fcn = 'D';
    break;
  case 2:
    msg.fcn = 'G';
    break;
  case 3:
    msg.fcn = 'J';
    break;
  case 4:
    msg.fcn = 'M';
    break;
  default:
    return GME_USER_ERROR; /* linea invalida */
  }
  memset(&msg.data[0], ' ', 10);
  memcpy(&msg.data[0], msg_text, min(msg_len, 10));
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;
  Enqueue(dst, &msg);
  /* envío la segunda mitad de la línea */
  switch (line)
  {
  case 1:
    msg.fcn = 'E';
    break;
  case 2:
    msg.fcn = 'H';
    break;
  case 3:
    msg.fcn = 'K';
    break;
  case 4:
    msg.fcn = 'N';
    break;
  default:
    return GME_USER_ERROR; /* linea invalida */
  }
  memset(&msg.data[0], ' ', 10);
  /* copio solamente si es necesario */
  if(msg_len > 10)
  {
    memcpy(&msg.data[0], &msg_text[10], min((msg_len - 10), 10));
  }
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;
  Enqueue(dst, &msg);
  return GME_OK;  
}

int CSerDev::PlayBeep(int dst, int count)
{
  ser_msg msg;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: PlayBeep(0x%02X, %i)", (int)(dst & 0xFF), count);
	if(dst <= 0x00 || dst > 0xFF) return GME_USER_ERROR;
  if(dst != 0xFF) /* si es la dirección de broadcast algunos controles no tienen sentido */
  {
    if( dst != 0xFF && !(m_hwStat[((int)dst&0xFF)].on_line) )
    {
      m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (PlayBeep) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
      /* trato de despertarlo */
      Ping(dst, 1);
    }
    if(m_hwStat[((int)dst&0xFF)].hw_typ < 0x02 )
    {
      m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (PlayBeep) El dispositivo 0x%02X no soporta esta funcion.", (int)(dst & 0xFF));
      return GME_USER_ERROR;
    }
  }
  msg.fcn = 'C';
  msg.data[0] = '0' + count;
  msg.data[1] = ' ';
  msg.data[2] = ' ';
  msg.data[3] = ' ';
  msg.data[4] = ' ';
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  return GME_OK;  
}

int CSerDev::RefreshDatetime(int dst)
{
  ser_msg msg;
  time_t  t;
  struct tm *pstm;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: RefreshDatetime(0x%02X)", (int)(dst & 0xFF));
	if(dst <= 0x00 || dst > 0xFF) return GME_USER_ERROR;
  if(dst != 0xFF) /* si es la dirección de broadcast algunos controles no tienen sentido */
  {
    if( dst != 0xFF && !(m_hwStat[((int)dst&0xFF)].on_line) )
    {
      m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (RefreshDatetime) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
      /* trato de despertarlo */
      Ping(dst, 1);
    }
    if(m_hwStat[((int)dst&0xFF)].hw_typ < 0x02 )
    {
      m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (RefreshDatetime) El dispositivo 0x%02X no soporta esta funcion.", (int)(dst & 0xFF));
      return GME_USER_ERROR;
    }
  }
  t = time(&t);
  pstm =  localtime(&t); 

  msg.fcn = 'T';
  msg.data[0] = (pstm->tm_hour & 0xFF);
  msg.data[1] = (pstm->tm_min & 0xFF);
  msg.data[2] = (pstm->tm_sec & 0xFF);
  msg.data[3] = ((pstm->tm_wday == 0)?(0x06):(pstm->tm_wday - 1)) & 0xFF;
  msg.data[4] = (pstm->tm_mday & 0xFF);
  msg.data[5] = (pstm->tm_mon & 0xFF);
  msg.data[6] = (pstm->tm_year & 0xFF);
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  return GME_OK;  
}

/* encola un mensaje */
void CSerDev::Enqueue(int id, ser_msg *msg)
{
  ser_msg m;
  memcpy(&m, msg, sizeof(ser_msg));
  m.enviado = 0;
  m.cnt = ++m_cCnt;
  /* agrega el mensaje a la cola del dispositivo */
  m_hwStat[((int)id&0xFF)].pending.push_back(m);
}

/* obtiene el mensaje a enviar */
CSerDev::ser_msg* CSerDev::GetNext(int id)
{
  if( m_hwStat[((int)id&0xFF)].pending.empty() ) return NULL;
  return &m_hwStat[((int)id&0xFF)].pending[ 0 ];  
}

/* borra un mensaje de la cola */
int CSerDev::Dequeue(int id, char fcn, int count)
{
  ser_msg *m;
  if( m_hwStat[((int)id&0xFF)].pending.empty() ) return 0;
  /* verifico que el ultimo corresponda con el contador del mensaje */
  m = GetNext(id);
  if( !m) return 0;
  if( m->cnt != (char)(count & 0xFF) ) return 0;
  if( m->fcn != fcn ) return 0;
  /* lo borro del vector */
  m_hwStat[((int)id&0xFF)].pending.erase( m_hwStat[((int)id&0xFF)].pending.begin() );
  return 1;  
}

int CSerDev::SetOut(int dst, int sal, int val)
{
  ser_msg msg;
  time_t t;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: SetOut(0x%02X, %i)", (int)(dst & 0xFF), sal);
	if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR; /* excluyo a broadcast */
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (SetLed) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
  }
  if(m_hwStat[((int)dst&0xFF)].hw_typ < 0x03 )
  {
    m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (SetOut) El dispositivo 0x%02X no soporta esta funcion.", (int)(dst & 0xFF));
    return GME_USER_ERROR;
  }
  if(m_hwStat[((int)dst&0xFF)].hw_typ == 0x03 )
  {
    if(sal < 1 || sal > 6)
    {
      m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (SetOut) Salida %i fuera de rango para dispositivo 0x%02X.", sal, (int)(dst & 0xFF));
      return GME_USER_ERROR;
    }
    if(val < 0 || val > 4)
    {
      m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (SetOut) Valor %i fuera de rango para dispositivo 0x%02X.", val, (int)(dst & 0xFF));
      return GME_USER_ERROR;
    }
  }
  /* mantengo los estados para actualizaciones */
  t = time(&t);
  m_hwStat[dst].typ.hw3.s[sal - 1] = val; 
  m_hwStat[dst].typ.hw3.next_update = t + 2400; /* 1 hora */ 
  /* armo el mensaje */
  msg.fcn = '2';
  msg.data[0] = '0' + sal;
  msg.data[1] = '0' + val;
  msg.data[2] = ' ';
  msg.data[3] = ' ';
  msg.data[4] = ' ';
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  return GME_OK;  
}

int CSerDev::SetDisplay(int dst, int enh, int typ, int dfl, int to)
{
  ser_msg msg;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: SetDisplay(0x%02X, %s)", (int)(dst & 0xFF), (enh)?"on":"off");
  if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR; /* excluyo a broadcast */
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (SetDisplay) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
  }
  if(m_hwStat[((int)dst&0xFF)].hw_typ != 0x02 )
  {
    m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (SetDisplay) El dispositivo 0x%02X no soporta esta funcion.", (int)(dst & 0xFF));
    return GME_USER_ERROR;
  }
  /* armo el mensaje */
  msg.fcn = 'Q';
  msg.data[0] = '0' + enh;
  msg.data[1] = '0' + typ;
  msg.data[2] = '0' + dfl;
  msg.data[3] = to;
  msg.data[4] = ' ';
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  return GME_OK;  
}

int CSerDev::SendIR(int dst, const char* ircode)
{
  ser_msg msg;
  int i;

  m_pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: SendIR(0x%02X, %s)", (int)(dst & 0xFF), ircode);
  if(dst <= 0x00 || dst >= 0xFF) return GME_USER_ERROR; /* excluyo a broadcast */
  if( !m_hwStat[((int)dst&0xFF)].on_line)
  {
    m_pLog->Add(GLOG_LEVEL_INFO, "INFO: (SendIR) El dispositivo 0x%02X no esta en linea.", (int)(dst & 0xFF));
    /* trato de despertarlo */
    Ping(dst, 1);
  }
  if(m_hwStat[((int)dst&0xFF)].hw_typ != 0x02 )
  {
    m_pLog->Add(GLOG_LEVEL_WARNING, "WARN: (SendIR) El dispositivo 0x%02X no soporta esta funcion.", (int)(dst & 0xFF));
    return GME_USER_ERROR;
  }
  /* armo el mensaje */
  i = 0;
  msg.fcn = '4';
  for(i = 0; i < 10; i+=2)
  {
    msg.data[i/2] = ( (ircode[i+1] >= '0' && ircode[i+1] <= '9')?(ircode[i+1]-'0'):( (ircode[i+1] >= 'A' && ircode[i+1] <= 'F')?(ircode[i+1]-'A'+10):0 ) ) +
                  ( ( (ircode[i] >= '0' && ircode[i] <= '9')?(ircode[i]-'0'):( (ircode[i] >= 'A' && ircode[i] <= 'F')?(ircode[i]-'A'+10):0 ) ) * 16 );
  }
  msg.data[5] = ' ';
  msg.data[6] = ' ';
  msg.data[7] = ' ';
  msg.data[8] = ' ';
  msg.data[9] = ' ';
  msg.resend_to = DEFAULT_RESEND_TO;
  msg.resend = DEFAULT_RESEND;

  Enqueue(dst, &msg);
  return GME_OK;  
}
