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
#ifndef _CSERDEV_H_
#define _CSERDEV_H_

#include "sercomm.h"

#include <string>
#include <vector>
using namespace std;

#include <gmonitor/glog.h>
#include <gmonitor/sincmem.h>
#include <gmonitor/gmswaited.h>

#define GMH_SCOMM_KEY	0x231313

class CSerDev : protected CSerComm
{
public:
  typedef struct _HWLIST
  {
    int on_line;
    char hw_addr;
    char hw_typ;
  	char hw_ver;
    char sw_ver;
    char up_model;
    char display;
    char info_06;
    char info_07;
    char info_08;
    char info_09;
    char info_10;
  } HWLIST;

  typedef struct _HWINFO
  {
    char port_typ; /* 'I' / 'O' / 'A' */
    char port_num; /* '0' .. '9' */
    char high_port_val; /* 0 .. 255 Bits mas significativos en puertos de dos bytes (analogico) */
    char port_val; /* 0 .. 255 */
    char alarm_state; /* 0 .. 1 indica el estado de alarma (activado / desactivado en un puerto analogico) */
    char alarm_maxh;  /* indican los valores de configuración de las alarmas en un puerto analogico */
    char alarm_maxl;
    char alarm_minh;
    char alarm_minl;
    char info_mask; /* contiene el valor de configuracion de la mascara de notificacion */
    char seguidor_val; /* contiene el valor de configuración de la mascara de seguidores */
    long port_change[8]; /* 8 timestamp uno por bit, en los analogicos vale solo el primero */
  } HWINFO;

  typedef struct _EVENT_NOTIF
  {
    int     address;
    char    port[2];
    int     bit;
    int     on_off;
    long    time;
  } EVENT_NOTIF;

  CSerDev(CGLog* pLog, CGMServerWait *pServer = NULL);
  virtual ~CSerDev();

  int Open(const char* port);
  void Close();

  int Process();
  int MaxWait();
  int Check();
  int Discover();
  int ChangeAddress(int dst, int newaddress);
  int ChangeBaudRate(int dst, int baudrate);
  int Ping(int dst, int resend);
  int Reset(int dst);
  int SetBit(int dst, int sal, int bit, int on_off);
  int PulseBit(int dst, int sal, int bit, int seg);
  int InvertBit(int dst, int sal, int bit);
  int SetSeguidores(int dst, int ent, int mascara);
  int SetMask(int dst, int ent, int mascara);
  int SetADAlarm(int dst, int ent, int max, int min);
  int SendMessage(int dst, int line, const char* msg_text, int msg_len);
  int PlayBeep(int dst, int count);
  int RefreshDatetime(int dst);
  int SetOut(int dst, int sal, int val);
  int SetDisplay(int dst, int enh, int typ, int dfl, int to);
  int SendIR(int dst, const char* ircode);
  vector <HWLIST>  GetList();
  vector <HWINFO> GetInfo(int dst);

protected:
  typedef struct _ser_msg
  {
    int enviado;
    long time_out;
    int resend;
    int resend_to;
    char cnt;
    char fcn;
    char data[10];
  } ser_msg;

  /* si al llegar la respuesta hay que enviar un aviso se completa esta
     estructura, el mensaje van con src, fcn y data como datos */
  typedef struct _fcn_ack
  {
    char msg_typ;
    char fcn[32];
  } fcn_ack;

  typedef struct _hw_stat
  {
    int on_line;
    long last_rcv;
    char last_cnt;
    char hw_typ;    /* 1: MIO-16x16  2: MIO-16x16/AD */
    char hw_ver;
    char sw_ver;
    char up_model;  /* 1: 16F877  2: 16F877A  3: 16F887 */
    char display;
    char info_06;
    char info_07;
    char info_08;
    char info_09;
    char info_10;
    union  /* estructuras según HW_TYP */
    {
      struct /* MIO-16x16 */
      {
        long next_update_e1;
        long last_change_e1[8];
        char e1;
        long next_update_e2;
        long last_change_e2[8];
        char e2;
        long next_update_s1;
        long last_change_s1[8];
        char s1;
        long next_update_s2;
        long last_change_s2[8];
        char s2;
      } hw1;
      struct  /* MIO-16x16/AD */
      {
        long next_update_e1;
        long last_change_e1[8];
        char e1;
        char seguidor_e1_s1;
        char mascara_notif_e1;

        long next_update_e2;
        long last_change_e2[8];
        char e2;
        char seguidor_e2_s2;
        char mascara_notif_e2;

        long next_update_s1;
        long last_change_s1[8];
        char s1;
        char mascara_notif_s1;

        long next_update_s2;
        long last_change_s2[8];
        char s2;
        char mascara_notif_s2;

        char seguidor_an_s2;
        char mascara_notif_an;

        long next_update_a1;
        long last_change_a1;
        char a1_alarm;
        char a1_h;
        char a1_l;
        char a1_maxh;
        char a1_maxl;
        char a1_minh;
        char a1_minl;

        long next_update_a2;
        long last_change_a2;
        char a2_alarm;
        char a2_h;
        char a2_l;
        char a2_maxh;
        char a2_maxl;
        char a2_minh;
        char a2_minl;

        long next_update_a3;
        long last_change_a3;
        char a3_alarm;
        char a3_h;
        char a3_l;
        char a3_maxh;
        char a3_maxl;
        char a3_minh;
        char a3_minl;

        long next_update_a4;
        long last_change_a4;
        char a4_alarm;
        char a4_h;
        char a4_l;
        char a4_maxh;
        char a4_maxl;
        char a4_minh;
        char a4_minl;
      } hw2;
      struct  /* MPAD-16 */
      {
        long next_update;
        int s[6];
      } hw3;
    } typ;
    vector <ser_msg> pending;
  } hw_stat;

  CGLog *m_pLog;

  void Enqueue(int id, ser_msg *msg);
  ser_msg *GetNext(int id);
  int Dequeue(int id, char fcn, int count);

private:
  /* alojamiento para mantener el estado de los 256 dispositivos */
/*
  CSincMem      *m_shMem;
*/
  hw_stat       m_hwStat[256];
  char          m_cCnt;
  CGMServerWait *m_pServer;

};

#endif /* _CSERDEV_H_ */

