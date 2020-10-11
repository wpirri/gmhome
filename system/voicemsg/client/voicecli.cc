/***************************************************************************
 * Copyright (C) 2003 Walter Pirri
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


int main(int argc, char** argv)
{
  int i;
  char buffer[4096];
  CGMInitData gminit;
  CGMClient *pClient;

  signal(SIGALRM,         SIG_IGN);
  signal(SIGPIPE,         SIG_IGN);

  buffer[0] = '\0';

  for(i = 1; i < argc; i++)
  {
    if(!strcmp(argv[i], "-h"))
    {
      i++;
      gminit.m_host = argv[i];
    }
    else
    {
      strcpy(buffer, argv[i]);
    }
  }

  if( !strlen(buffer))
  {
    fprintf(stderr, "Uso: %s [-h host] mensaje\n", argv[0]);
    fprintf(stderr, "     donde 'host':      nombre o dirección del server\n");
    exit(1);
  }

  pClient = new CGMClient(&gminit);
  pClient->Notify("text2speech", buffer, strlen(buffer)+1);
  delete pClient;
  return 0;
}
