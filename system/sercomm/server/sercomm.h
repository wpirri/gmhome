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
#ifndef _CSERCOMM_H_
#define _CSERCOMM_H_

#include <string>
using namespace std;

#include <termios.h>

class CSerComm
{
public:
  CSerComm();
  virtual ~CSerComm();

  int Open(int port);
  int Open(const char* port);
  void Close();

  int Send(char typ, char dst, char cnt, char fcn, const char* data);
  int Recv(char* typ, char* src, char* cnt, char* fcn, char* data);

protected:
  int m_sfd;

private:
  struct termios m_old_settings;
  char m_rx_buffer[64];
  int m_rx_count;
  long m_last_rx_char;
#ifdef _DEBUG_
  void Display(const char* dir, const char* str);
#endif /*_DEBUG_*/

};
#endif /* _CSERCOMM_H_ */
