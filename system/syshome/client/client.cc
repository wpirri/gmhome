/***************************************************************************
 * Copyright (C) 2010 Walter Pirri
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 ***************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>
#include <syslog.h>
#include <time.h>

#include <gmonitor/gmc.h>

#ifndef min
#  define min(a,b)      ((a) < (b) ? (a) : (b))
#endif

int main(int argc, char** argv)
{
  CGMInitData gminit;
  CGMClient *pClient;
  CGMError gmerror;

  int i, rc;

  char service[256];
  char data[4096];
  CGMClient::GMIOS resp_data;

  data[0] = '\0';
  service[0] = '\0';

  for(i = 1; i < argc; i++)
  {
    if(!strcmp(argv[i], "-h"))
    {
      i++;
      gminit.m_host = argv[i];
    }
    else if(!strcmp(argv[i], "-p"))
    {
      i++;
      gminit.m_port = atol(argv[i]);
    }
    else if(!strcmp(argv[i], "-s"))
    {
      i++;
      sprintf(service, "syshome_%s", argv[i]);
    }
    else 
    {
      strcat(data, argv[i]);
      strcat(data, " ");
    }

  }

  if( !strlen(service) ) return -1;

  /*fprintf(stderr, "service: [%s] - data: [%s]\n", service, data);*/
  
  pClient = new CGMClient(&gminit);

  if((rc = pClient->Connect(service, 1024)) != GME_OK)
  {
    delete pClient;
    return -1;
  }
  if((rc = pClient->Send(data, strlen(data)+1)) != GME_OK)
  {
    pClient->Discon();
    delete pClient;
    return -1;
  }
  while((rc = pClient->Recv(&resp_data, 300)) == GME_MORE_DATA)
  {
    fprintf(stdout, "%*.*s", (int)resp_data.len, (int)resp_data.len, (char*)resp_data.data);
    pClient->Free(resp_data);
  }
  if(rc == GME_OK)
  {
    fprintf(stdout, "%*.*s\n", (int)resp_data.len, (int)resp_data.len, (char*)resp_data.data);
    pClient->Free(resp_data);
  }
  else
  {
    pClient->Discon();
    delete pClient;
    return -1;
  }
  pClient->Discon();
  delete pClient;
  return 0;
}
