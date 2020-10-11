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
#include "sercomm.h"

#include <string>
#include <iostream>
#include <csignal>
using namespace std;

#include <unistd.h>
#include <string.h>
#include <time.h>
#include <syslog.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>

#include <gmonitor/glog.h>

/*
extern CGLog* pLog;
*/

CSerComm::CSerComm()
{
  m_sfd = -1;
}

CSerComm::~CSerComm()
{
  Close();
}

int CSerComm::Open(int port)
{
  char dev_name[256];

  if(port < 1 || port > 4) return -1;
  /* aca hay que poner #IFDEF por SO */
  sprintf(dev_name, "/dev/ttyS%i", port - 1);

  return CSerComm::Open(dev_name);
}

int CSerComm::Open(const char* port)
{
  struct termios settings;

  if(m_sfd != -1) Close();

  m_rx_count = 0;
  m_last_rx_char = 0l;
  /* abro el puerto */
  m_sfd = open( port, O_RDWR|O_NOCTTY | O_NONBLOCK);
  if(m_sfd == -1) return -2;
  /* me guardo la configuracion que tiene asi se la devuelvo antes de cerrarlo */
  tcgetattr(m_sfd, &m_old_settings);
  /* seteo la nueva configuración */
  bzero(&settings, sizeof(struct termios));
  settings.c_cflag = (B9600|CS8|CLOCAL|CREAD); /* 1200,N,8,1 */
  settings.c_iflag = IGNPAR;
  settings.c_oflag = 0;
  settings.c_lflag = 0; /* no CANONICAL y sin eco */
  settings.c_cc[VTIME] = 0;
  settings.c_cc[VMIN] = 1; /* tamaño de los mensajes */

  tcflush(m_sfd, TCIFLUSH);
  tcsetattr(m_sfd, TCSANOW, &settings);

  return 0;
}

void CSerComm::Close()
{
  if(m_sfd == -1) return;
  tcsetattr(m_sfd, TCSANOW, &m_old_settings);
  close(m_sfd);
}

int CSerComm::Send(char typ, char dst, char cnt, char fcn, const char* data)
{
  char tx_buffer[16];
  int i;

  tx_buffer[0] = typ;
  tx_buffer[1] = dst;
  tx_buffer[2] = 0x00; /* src */
  tx_buffer[3] = cnt;
  tx_buffer[4] = fcn;
  memcpy(&tx_buffer[5], data, 10);
   tx_buffer[15] = 0;
  for(i = 0; i < 15; i++) tx_buffer[15] += tx_buffer[i];
/*
  pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: SEND typ: %c dst: 0x%02X "
              "cnt: 0x%02X fcn: %c data: 0x%02X 0x%02X 0x%02X 0x%02X "
              "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X sum: 0x%02X",
          tx_buffer[0],
          (int)(tx_buffer[1] & 0xFF),
          (int)(tx_buffer[3] & 0xFF),
          tx_buffer[4],
          (int)(tx_buffer[5] & 0xFF),
          (int)(tx_buffer[6] & 0xFF),
          (int)(tx_buffer[7] & 0xFF),
          (int)(tx_buffer[8] & 0xFF),
          (int)(tx_buffer[9] & 0xFF),
          (int)(tx_buffer[10] & 0xFF),
          (int)(tx_buffer[11] & 0xFF),
          (int)(tx_buffer[12] & 0xFF),
          (int)(tx_buffer[13] & 0xFF),
          (int)(tx_buffer[14] & 0xFF),
          (int)(tx_buffer[15] & 0xFF));
*/
  if(write(m_sfd, tx_buffer, 16) != 16)
  {
/*
    pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Error al escribir en el socket");
*/
    return -1;
  }
  return 0;
}

int CSerComm::Recv(char* typ, char* src, char* cnt, char* fcn, char* data)
{
  int i;
  long t;
  char ch;
  char  chksum;

  while(read(m_sfd, &ch, 1) > 0)
  {
/*
    pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: RECV count: %02i data: 0x%02X",
                                m_rx_count, (int)(ch&0xFF));
*/
    t = time(&t);
    /* si hace mas de tres segundos que no recibo nada descarto lo recibido antes */
    if((m_last_rx_char + 3l) < t ) m_rx_count = 0;
    m_last_rx_char = t;
    switch(m_rx_count)
    {
    case 0:
      /* control de start */
      if(ch == 'Q' || ch == 'R' || ch == 'E')
      {
        m_rx_buffer[m_rx_count] = ch;
        m_rx_count++;
      }
/*
      else
      {
        pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Error en start [%c - 0x%02X]", ch, (int)ch);
      }
*/
      break;
    case 1:
      /* control de destino */
      if(ch != 0x00)
      {
        m_rx_count = 0;
/*
        pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: Error destino invalido [0x%02X]", (int)ch);
*/
      }
      else
      {
        m_rx_count++;
      }
      break;
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
      /* del 2 al 14 pueden valer cualquier cosa entre 0 y 255 */
      m_rx_buffer[m_rx_count] = ch;
      m_rx_count++;
      break;
    case 15:
      /* control de checksum */
      m_rx_buffer[m_rx_count] = ch;
      m_rx_count = 0;
      chksum = 0;
/*
      pLog->Add(GLOG_LEVEL_DEBUG, "DEBUG: RECV typ: %c src: 0x%02X "
                  "cnt: 0x%02X fcn: %c data: 0x%02X 0x%02X 0x%02X 0x%02X "
                  "0x%02X 0x%02X 0x%02X 0x%02X 0x%02X 0x%02X sum: 0x%02X",
              m_rx_buffer[0],
              (int)(m_rx_buffer[2] & 0xFF),
              (int)(m_rx_buffer[3] & 0xFF),
              m_rx_buffer[4],
              (int)(m_rx_buffer[5] & 0xFF),
              (int)(m_rx_buffer[6] & 0xFF),
              (int)(m_rx_buffer[7] & 0xFF),
              (int)(m_rx_buffer[8] & 0xFF),
              (int)(m_rx_buffer[9] & 0xFF),
              (int)(m_rx_buffer[10] & 0xFF),
              (int)(m_rx_buffer[11] & 0xFF),
              (int)(m_rx_buffer[12] & 0xFF),
              (int)(m_rx_buffer[13] & 0xFF),
              (int)(m_rx_buffer[14] & 0xFF),
              (int)(m_rx_buffer[15] & 0xFF));
*/
      for(i = 0; i < 15; i++) chksum += m_rx_buffer[i];
      if(m_rx_buffer[15] == chksum)
      {
        /* mensaje válido */
        *typ = m_rx_buffer[0];
        *src = m_rx_buffer[2];
        *cnt = m_rx_buffer[3];
        *fcn = m_rx_buffer[4];
        memcpy(data, &m_rx_buffer[5], 10);
        return 1;
      }
/*
      else
      {
        pLog->Add(GLOG_LEVEL_DEBUG, "ERROR: CHECKSUM esperado: 0x%02X", chksum);
      }
*/
      break;
    }
  }
  return 0;
}

