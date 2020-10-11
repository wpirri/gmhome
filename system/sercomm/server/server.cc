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
#include <string>
#include <iostream>
#include <csignal>
#include <vector>
using namespace std;

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/wait.h> 

#include <gmonitor/gmerror.h>
#include <gmonitor/gmswaited.h>

#include "serdev.h"

/*
int  child_pid;
*/

void OnClose(int sig);
/*
void OnChildExit(int sig);
void OnAlarm(int sig);
*/

CGMServerWait *m_pServer;
CSerDev *m_pSerDev;

int main(int argc, char** argv, char** env)
{
  int i, rc = 0;
  char fn[33];
  char typ;
  char in[256];
  unsigned long in_len;
  char data[10240];
  char tmp[256];
  int data_len = 0;
  int d1, d2, d3, d4, d5;
  char port[256];
  char  *s1;
  struct tm *s_tm;
  vector <CSerDev::HWINFO> v_hw_info;
  vector <CSerDev::HWLIST> v_hw_list;

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
/*
  signal(SIGCHLD, OnChildExit);
*/
  strcpy(port, "/dev/ttyS0");

  for(i = 1; i < argc; i++)
  {
    if( !strcmp(argv[i], "-p") )
    {
      i++;
      strncpy(port, argv[i], 255);
    }
  }

  m_pServer = new CGMServerWait;
  m_pServer->Init("gmh_sercomm");
  m_pServer->m_pLog->Add(1, "Iniciando server");

  m_pSerDev = new CSerDev(m_pServer->m_pLog, m_pServer);
  m_pSerDev->Open(port);

  if(m_pServer->Suscribe("sercmd-s1-bit-on-off", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a sercmd-s1-bit-on-off");
  if(m_pServer->Suscribe("sercmd-s2-bit-on-off", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a sercmd-s2-bit-on-off");
  if(m_pServer->Suscribe("serinf", GM_MSG_TYPE_CR) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serinf");
  if(m_pServer->Suscribe("serinf-ex", GM_MSG_TYPE_CR) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serinf-ex CR");
  if(m_pServer->Suscribe("serinf-ex", GM_MSG_TYPE_INT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serinf-ex INT");
  if(m_pServer->Suscribe("serpulse-s1-bit", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serpulse-s1-bit");
  if(m_pServer->Suscribe("serpulse-s2-bit", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serpulse-s2-bit");
  if(m_pServer->Suscribe("serinvert-s1-bit", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serinvert-s1-bit");
  if(m_pServer->Suscribe("serinvert-s2-bit", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serinvert-s2-bit");
  if(m_pServer->Suscribe("serlist", GM_MSG_TYPE_CR) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serlist CR");
  if(m_pServer->Suscribe("serlist", GM_MSG_TYPE_INT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serlist INT");
  if(m_pServer->Suscribe("serlist-ex", GM_MSG_TYPE_CR) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serlist-ex CR");
  if(m_pServer->Suscribe("serlist-ex", GM_MSG_TYPE_INT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serlist-ex INT");
  if(m_pServer->Suscribe("serchange-address", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serchange-address");
  if(m_pServer->Suscribe("serchange-baudrate", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serchange-baudrate");
  if(m_pServer->Suscribe("serreset", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serreset");
  if(m_pServer->Suscribe("sercmd-seguidores", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a sercmd-seguidores");
  if(m_pServer->Suscribe("sercmd-mask", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a sercmd-mask");
  if(m_pServer->Suscribe("serset-ad-alarm", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serset-ad-alarm");
  if(m_pServer->Suscribe("sermessage", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a sermessage");
  if(m_pServer->Suscribe("serbuzzer", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a serbuzzer");
  if(m_pServer->Suscribe("update-datetime", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a update-datetime");
  if(m_pServer->Suscribe("set-out", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a set-out");
  if(m_pServer->Suscribe("display-config", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a display-config");
  if(m_pServer->Suscribe("send-ir", GM_MSG_TYPE_NOT) != GME_OK)
    m_pServer->m_pLog->Add(1, "ERROR al suscribir a send-ir");
  
  m_pServer->m_pLog->Add(1, "Inicializacion OK");

  m_pSerDev->Discover();

  while(rc >= 0)
  {

    while(m_pSerDev->Process());

    while((rc = m_pServer->Wait(fn, &typ, in, 256, &in_len,
                                /*m_pSerDev->MaxWait()*100*/10)) > 0)
    {
      if( !strcmp(fn, "sercmd-s1-bit-on-off"))
      {
        in[3] = '\0';
        in[7] = '\0';
        in[11] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* bit */
        d3 = atoi(&in[8]);   /* on / off */
        m_pSerDev->SetBit(d1, 1, d2, d3); /* para la salida 1 */

        if(m_pServer->Resp(NULL, 0, GME_OK) != GME_OK)
        {
          m_pServer->m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR al responder mensaje");
        }
      }
      else if( !strcmp(fn, "sercmd-s2-bit-on-off"))
      {
        in[3] = '\0';
        in[7] = '\0';
        in[11] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* bit */
        d3 = atoi(&in[8]);   /* on / off */
        m_pSerDev->SetBit(d1, 2, d2, d3); /* para la salida 2 */
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "serpulse-s1-bit"))
      {
        in[3] = '\0';
        in[7] = '\0';
        in[11] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* bit */
        d3 = atoi(&in[8]);   /* tiempo del pulso */
        m_pSerDev->PulseBit(d1, 1, d2, d3); /* para la salida 2 */
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "serpulse-s2-bit"))
      {
        in[3] = '\0';
        in[7] = '\0';
        in[11] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* bit */
        d3 = atoi(&in[8]);   /* tiempo del pulso */
        m_pSerDev->PulseBit(d1, 2, d2, d3); /* para la salida 2 */
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "serinvert-s1-bit"))
      {
        in[3] = '\0';
        in[7] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* bit */
        m_pSerDev->InvertBit(d1, 1, d2); /* para la salida 2 */
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "serinvert-s2-bit"))
      {
        in[3] = '\0';
        in[7] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* bit */
        m_pSerDev->InvertBit(d1, 2, d2); /* para la salida 2 */
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "serinf"))
      {
        data[0] = '\0';
        in[3] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        v_hw_info = m_pSerDev->GetInfo(d1);
        if(v_hw_info.size())
        {
          for(i = 0; i < (int)v_hw_info.size(); i++)
          {
            if(v_hw_info[i].port_typ == 'A')
            {
              sprintf(tmp, "%c%c:%03i %03i ", v_hw_info[i].port_typ, v_hw_info[i].port_num,
                      (int)(v_hw_info[i].high_port_val & 0xFF), (int)(v_hw_info[i].port_val & 0xFF));
            }
            else
            {
              sprintf(tmp, "%c%c:%03i ", v_hw_info[i].port_typ, v_hw_info[i].port_num,
                      (int)(v_hw_info[i].port_val & 0xFF));
            }
            strcat(data, tmp);
          }
          data_len = strlen(data);
        }
        if(m_pServer->Resp(data, data_len, GME_OK) != GME_OK)
        {
          m_pServer->m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR al responder mensaje");
        }
      }
      else if( !strcmp(fn, "serinf-ex")) /* devuelvo en formato XML */
      {
        data[0] = '\0';
        in[3] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        m_pServer->m_pLog->Add(GLOG_LEVEL_DEBUG, "serinf-ex(%03i)", d1);
        v_hw_info = m_pSerDev->GetInfo(d1);
        m_pServer->m_pLog->Add(GLOG_LEVEL_DEBUG, "  v_hw_info.size() = %i", v_hw_info.size());
        if(v_hw_info.size())
        {
          sprintf(&data[strlen(data)], "<device>\n");
          sprintf(&data[strlen(data)], "  <address>%i</address>\n", d1);
          for(i = 0; i < (int)v_hw_info.size(); i++)
          {
            sprintf(&data[strlen(data)], "  <port>\n");
            sprintf(&data[strlen(data)], "    <number>%i</number>\n", i);
            sprintf(&data[strlen(data)], "    <name>%c%c</name>\n",
                                        v_hw_info[i].port_typ, v_hw_info[i].port_num);
            if(v_hw_info[i].port_typ == 'A')
            {
              sprintf(&data[strlen(data)], "    <info-mask>%i</info-mask>\n", (int)(v_hw_info[i].info_mask & 0xFF));
              sprintf(&data[strlen(data)], "    <seguidor>%i</seguidor>\n", (int)(v_hw_info[i].seguidor_val & 0xFF));
              sprintf(&data[strlen(data)], "    <data>\n");
              sprintf(&data[strlen(data)], "      <value>%i</value>\n", 
                      ((int)(v_hw_info[i].port_val & 0xFF)) +
                        ( ((int)(v_hw_info[i].high_port_val & 0x0F)) * 256 ));
              sprintf(&data[strlen(data)], "      <alarm>%i</alarm>\n", 
                      ((int)(v_hw_info[i].alarm_state & 0xFF)));
              sprintf(&data[strlen(data)], "      <max>%i</max>\n", 
                      ((int)(v_hw_info[i].alarm_maxl & 0xFF)) +
                        ( ((int)(v_hw_info[i].alarm_maxh & 0x0F)) * 256 ));
              sprintf(&data[strlen(data)], "      <min>%i</min>\n", 
                      ((int)(v_hw_info[i].alarm_minl & 0xFF)) +
                        ( ((int)(v_hw_info[i].alarm_minh & 0x0F)) * 256 ));
              s_tm = localtime(&v_hw_info[i].port_change[0]);
              sprintf(&data[strlen(data)], "      <last-change>%02i/%02i/%04i %02i:%02i:%02i</last-change>\n",
                                            s_tm->tm_mday, s_tm->tm_mon+1, s_tm->tm_year+1900,
                                            s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec);
              sprintf(&data[strlen(data)], "    </data>\n");
            }
            else
            {
              sprintf(&data[strlen(data)], "    <info-mask>%i</info-mask>\n", (int)(v_hw_info[i].info_mask & 0xFF));
              if(v_hw_info[i].port_typ == 'I')
              {
                sprintf(&data[strlen(data)], "    <seguidor>%i</seguidor>\n", (int)(v_hw_info[i].seguidor_val & 0xFF));
              }
              sprintf(&data[strlen(data)], "    <data>\n");
              sprintf(&data[strlen(data)], "      <bit>0</bit>\n");
              sprintf(&data[strlen(data)], "      <value>%i</value>\n", (int)((v_hw_info[i].port_val & 0x01)?1:0));
              s_tm = localtime(&v_hw_info[i].port_change[0]);
              sprintf(&data[strlen(data)], "      <last-change>%02i/%02i/%04i %02i:%02i:%02i</last-change>\n",
                                            s_tm->tm_mday, s_tm->tm_mon+1, s_tm->tm_year+1900,
                                            s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec);
              sprintf(&data[strlen(data)], "    </data>\n");
              sprintf(&data[strlen(data)], "    <data>\n");
              sprintf(&data[strlen(data)], "      <bit>1</bit>\n");
              sprintf(&data[strlen(data)], "      <value>%i</value>\n", (int)((v_hw_info[i].port_val & 0x02)?1:0));
              s_tm = localtime(&v_hw_info[i].port_change[1]);
              sprintf(&data[strlen(data)], "      <last-change>%02i/%02i/%04i %02i:%02i:%02i</last-change>\n",
                                            s_tm->tm_mday, s_tm->tm_mon+1, s_tm->tm_year+1900,
                                            s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec);
              sprintf(&data[strlen(data)], "    <data>\n");
              sprintf(&data[strlen(data)], "      <bit>2</bit>\n");
              sprintf(&data[strlen(data)], "      <value>%i</value>\n", (int)((v_hw_info[i].port_val & 0x04)?1:0));
              s_tm = localtime(&v_hw_info[i].port_change[2]);
              sprintf(&data[strlen(data)], "      <last-change>%02i/%02i/%04i %02i:%02i:%02i</last-change>\n",
                                            s_tm->tm_mday, s_tm->tm_mon+1, s_tm->tm_year+1900,
                                            s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec);
              sprintf(&data[strlen(data)], "    </data>\n");
              sprintf(&data[strlen(data)], "    </data>\n");
              sprintf(&data[strlen(data)], "    <data>\n");
              sprintf(&data[strlen(data)], "      <bit>3</bit>\n");
              sprintf(&data[strlen(data)], "      <value>%i</value>\n", (int)((v_hw_info[i].port_val & 0x08)?1:0));
              s_tm = localtime(&v_hw_info[i].port_change[3]);
              sprintf(&data[strlen(data)], "      <last-change>%02i/%02i/%04i %02i:%02i:%02i</last-change>\n",
                                            s_tm->tm_mday, s_tm->tm_mon+1, s_tm->tm_year+1900,
                                            s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec);
              sprintf(&data[strlen(data)], "    </data>\n");
              sprintf(&data[strlen(data)], "    <data>\n");
              sprintf(&data[strlen(data)], "      <bit>4</bit>\n");
              sprintf(&data[strlen(data)], "      <value>%i</value>\n", (int)((v_hw_info[i].port_val & 0x10)?1:0));
              s_tm = localtime(&v_hw_info[i].port_change[4]);
              sprintf(&data[strlen(data)], "      <last-change>%02i/%02i/%04i %02i:%02i:%02i</last-change>\n",
                                            s_tm->tm_mday, s_tm->tm_mon+1, s_tm->tm_year+1900,
                                            s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec);
              sprintf(&data[strlen(data)], "    </data>\n");
              sprintf(&data[strlen(data)], "    <data>\n");
              sprintf(&data[strlen(data)], "      <bit>5</bit>\n");
              sprintf(&data[strlen(data)], "      <value>%i</value>\n", (int)((v_hw_info[i].port_val & 0x20)?1:0));
              s_tm = localtime(&v_hw_info[i].port_change[5]);
              sprintf(&data[strlen(data)], "      <last-change>%02i/%02i/%04i %02i:%02i:%02i</last-change>\n",
                                            s_tm->tm_mday, s_tm->tm_mon+1, s_tm->tm_year+1900,
                                            s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec);
              sprintf(&data[strlen(data)], "    </data>\n");
              sprintf(&data[strlen(data)], "    <data>\n");
              sprintf(&data[strlen(data)], "      <bit>6</bit>\n");
              sprintf(&data[strlen(data)], "      <value>%i</value>\n", (int)((v_hw_info[i].port_val & 0x40)?1:0));
              s_tm = localtime(&v_hw_info[i].port_change[6]);
              sprintf(&data[strlen(data)], "      <last-change>%02i/%02i/%04i %02i:%02i:%02i</last-change>\n",
                                            s_tm->tm_mday, s_tm->tm_mon+1, s_tm->tm_year+1900,
                                            s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec);
              sprintf(&data[strlen(data)], "    </data>\n");
              sprintf(&data[strlen(data)], "    <data>\n");
              sprintf(&data[strlen(data)], "      <bit>7</bit>\n");
              sprintf(&data[strlen(data)], "      <value>%i</value>\n", (int)((v_hw_info[i].port_val & 0x80)?1:0));
              s_tm = localtime(&v_hw_info[i].port_change[7]);
              sprintf(&data[strlen(data)], "      <last-change>%02i/%02i/%04i %02i:%02i:%02i</last-change>\n",
                                            s_tm->tm_mday, s_tm->tm_mon+1, s_tm->tm_year+1900,
                                            s_tm->tm_hour, s_tm->tm_min, s_tm->tm_sec);
              sprintf(&data[strlen(data)], "    </data>\n");
            }
            sprintf(&data[strlen(data)], "  </port>\n");
          }
          sprintf(&data[strlen(data)], "</device>\n");
        }
        data_len = strlen(data);
        m_pServer->m_pLog->Add(GLOG_LEVEL_DEBUG, "  respondiendo %i bytes", data_len);
        if(m_pServer->Resp(data, data_len, GME_OK) != GME_OK)
        {
          m_pServer->m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR al responder mensaje");
        }
      }
      else if( !strcmp(fn, "serlist")) /* listado de dispositivos activos */
      {
        data[0] = '\0';
        v_hw_list = m_pSerDev->GetList();
        if(v_hw_list.size())
        {
          for(i = 0; i < (int)v_hw_list.size(); i++)
          {
            sprintf(tmp, "DISP: %03i STAT: %i HW-TYP: %i HW-VER: %i SW-TYP: %i\n",
                (int)(v_hw_list[i].hw_addr & 0xFF),
                v_hw_list[i].on_line,
                (int)(v_hw_list[i].hw_typ & 0xFF),
                (int)(v_hw_list[i].hw_ver & 0xFF),
                (int)(v_hw_list[i].sw_ver & 0xFF));
            strcat(data, tmp);
          }
        }
        data_len = strlen(data);
        if(m_pServer->Resp(data, data_len, GME_OK) != GME_OK)
        {
          m_pServer->m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR al responder mensaje");
        }
      }
      else if( !strcmp(fn, "serlist-ex")) /* listado de dispositivos activos XML */
      {
        data[0] = '\0';
        v_hw_list = m_pSerDev->GetList();
        if(v_hw_list.size())
        {
          for(i = 0; i < (int)v_hw_list.size(); i++)
          {
            sprintf(&data[strlen(data)], "<device>\n");
            sprintf(&data[strlen(data)], "  <address>%03i</address>\n", (int)(v_hw_list[i].hw_addr & 0xFF));
            sprintf(&data[strlen(data)], "  <status>%i</status>\n", v_hw_list[i].on_line);
            sprintf(&data[strlen(data)], "  <hardware-typ>%i</hardware-typ>\n", (int)(v_hw_list[i].hw_typ & 0xFF));
            sprintf(&data[strlen(data)], "  <hardware-ver>%i</hardware-ver>\n", (int)(v_hw_list[i].hw_ver & 0xFF));
            sprintf(&data[strlen(data)], "  <software-ver>%i</software-ver>\n", (int)(v_hw_list[i].sw_ver & 0xFF));
            sprintf(&data[strlen(data)], "  <processor>%i</processor>\n", (int)(v_hw_list[i].up_model & 0xFF));
            sprintf(&data[strlen(data)], "  <display>%i</display>\n", (int)(v_hw_list[i].display & 0xFF));
            sprintf(&data[strlen(data)], "</device>\n");
          }
        }
        data_len = strlen(data);
        if(m_pServer->Resp(data, data_len, GME_OK) != GME_OK)
        {
          m_pServer->m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR al responder mensaje");
        }
      }
      else if( !strcmp(fn, "serchange-address")) /* pide el cambio de la dirección del dispositivo */
      {
        in[3] = '\0';
        in[7] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* newaddress */
        m_pSerDev->ChangeAddress(d1, d2);
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "serchange-baudrate")) /* pide el cambio de velocidad de comunicacion de un dispositivo */
      {
        in[3] = '\0';
        in[7] = '\0';
        d1 = atoi(&in[0]);         /* dispositivo */
        d2 = (atoi(&in[4])*100);   /* baudrate */
        m_pSerDev->ChangeBaudRate(d1, d2);
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "serreset")) /* reset de dispositivo */
      {
        in[3] = '\0';
        d1 = atoi(&in[0]);         /* dispositivo */
        m_pSerDev->Reset(d1);
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "sercmd-seguidores"))
      {
        in[3] = '\0';
        in[7] = '\0';
        in[11] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* salida */
        d3 = atoi(&in[8]);   /* mascara */
        m_pSerDev->SetSeguidores(d1, d2, d3);
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "sercmd-mask"))
      {
        in[3] = '\0';
        in[7] = '\0';
        in[11] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* salida */
        d3 = atoi(&in[8]);   /* mascara */
        m_pSerDev->SetMask(d1, d2, d3);
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "serset-ad-alarm"))
      {
        in[3] = '\0';
        in[7] = '\0';
        in[13] = '\0';
        in[19] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* entrada A/D */
        d3 = atoi(&in[8]);   /* max */
        d4 = atoi(&in[14]);  /* min */
        m_pSerDev->SetADAlarm(d1, d2, d3, d4);
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "sermessage"))
      {
        in[3] = '\0';
        in[7] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* linea */
        s1 = &in[8];
        d3 = strlen(s1);
        m_pSerDev->SendMessage(d1, d2, s1, d3);
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "serbuzzer"))
      {
        in[3] = '\0';
        in[7] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* num beeps */
        m_pSerDev->PlayBeep(d1, d2);
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "update-datetime"))
      {
        in[3] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        m_pSerDev->RefreshDatetime(d1);
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "set-out"))
      {
        in[3] = '\0';
        in[7] = '\0';
        in[11] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* salida */
        d3 = atoi(&in[8]);   /* estado */
        m_pSerDev->SetOut(d1, d2, d3);
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "display-config"))
      {
        in[3] = '\0';
        in[7] = '\0';
        in[11] = '\0';
        in[15] = '\0';
        in[19] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        d2 = atoi(&in[4]);   /* enable / disable */
        d3 = atoi(&in[8]);   /* tipo */
        d4 = atoi(&in[12]);  /* pantalla por defecto */
        d5 = atoi(&in[16]);  /* time out */
        m_pSerDev->SetDisplay(d1, d2, d3, d4, d5);
        m_pServer->Resp(NULL, 0, GME_OK);
      }
      else if( !strcmp(fn, "send-ir"))
      {
        in[3] = '\0';
        in[14] = '\0';
        d1 = atoi(&in[0]);   /* dispositivo */
        s1 = &in[4];         /* Codigo infra-rojo */
        m_pSerDev->SendIR(d1, s1);
        m_pServer->Resp(NULL, 0, GME_OK);
      }

      else
      {
        m_pServer->m_pLog->Add(GLOG_LEVEL_WARNING, "GME_SVC_NOTFOUND: %s", fn);
        m_pServer->Resp(NULL, 0, GME_SVC_NOTFOUND);
      }
    }
    /* manejo de timers */
    m_pSerDev->Check();
  }
  m_pServer->m_pLog->Add(GLOG_LEVEL_ERROR, "ERROR en la espera de mensajes");
  OnClose(0);
  return 0;
}

void OnClose(int sig)
{
  m_pServer->m_pLog->Add(GLOG_LEVEL_DEBUG, "Exit on signal %i", sig);
  m_pServer->m_pLog->Add(GLOG_LEVEL_INFO, "Terminando server");

  m_pServer->UnSuscribe("sercmd-s1-bit-on-off", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("sercmd-s2-bit-on-off", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("serinf", GM_MSG_TYPE_CR);
  m_pServer->UnSuscribe("serpulse-s1-bit", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("serpulse-s2-bit", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("serinvert-s1-bit", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("serinvert-s2-bit", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("serinf-ex", GM_MSG_TYPE_CR);
  m_pServer->UnSuscribe("serinf-ex", GM_MSG_TYPE_INT);
  m_pServer->UnSuscribe("serlist", GM_MSG_TYPE_CR);
  m_pServer->UnSuscribe("serlist", GM_MSG_TYPE_INT);
  m_pServer->UnSuscribe("serlist-ex", GM_MSG_TYPE_CR);
  m_pServer->UnSuscribe("serlist-ex", GM_MSG_TYPE_INT);
  m_pServer->UnSuscribe("change-address", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("serchange-baudrate", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("serreset", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("sercmd-seguidores", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("sercmd-mask", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("serset-ad-alarm", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("sermessage", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("serbuzzer", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("update-datetime", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("set-out", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("display-config", GM_MSG_TYPE_NOT);
  m_pServer->UnSuscribe("send-ir", GM_MSG_TYPE_NOT);
  delete m_pServer;

  exit(0);
}

