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

#ifndef min
#  define min(a,b)      ((a) < (b) ? (a) : (b))
#endif

int main(int argc, char** argv)
{
  CGMInitData gminit;
  CGMClient *pClient;
  CGMError gmerror;

  int i;
  int help;
  int rc;

  char data[256];
  CGMClient::GMIOS resp_data;

  int cmd_count = 0;         /* contador de comando que debe haber por lo menos uno */
  int cmd_excl_count = 0;    /* contador de mensajes que no pueden compartir el comando */
  int cmd_req_address = 0;
  int cmd_req_port = 0;
  int cmd_req_bit = 0;

  int disp = -1;
  int set_reset = -1;
  int get = -1;
  int port = -1;
  int bit = -1;
  int ex = -1;
  int list = -1;
  int address = -1;
  int new_addr = -1;
  int baudrate = -1;
  int new_baudrate = -1;
  int reboot = -1;
  int update_datetime = -1;
  int set_follow = -1;
  int set_mask = -1;
  int set_display = -1;
  int num_beeps = -1;
  int set_ad_alarm = -1, ad_alarm_max = -1, ad_alarm_min = -1;
  int tiempo_seg = 0;
  int change_config = 0;
  int message_line = 0;
  int out_val = -1;
  char change_config_name[256];
  char msg_text[21];
  int ircode = 0;

  help = 0;

  for(i = 1; i < argc; i++)
  {
    if(!strcmp(argv[i], "-h"))
    {
      i++;
      gminit.m_host = argv[i];
    }
    else if(!strcmp(argv[i], "-P"))
    {
      i++;
      gminit.m_port = atol(argv[i]);
    }
    else if( !strcmp(argv[i], "-d") || !strcmp(argv[i], "--disp"))
    {
      i++;
      disp = atoi(argv[i]);
    }
    else if( !strcmp(argv[i], "-s") || !strcmp(argv[i], "--set"))
    {
      set_reset = 1;
      cmd_excl_count++;
      cmd_count++;
      cmd_req_address = 1;
      cmd_req_port = 1;
      cmd_req_bit = 1;
    }
    else if( !strcmp(argv[i], "-r") || !strcmp(argv[i], "--reset"))
    {
      set_reset = 0;
      cmd_excl_count++;
      cmd_count++;
      cmd_req_address = 1;
      cmd_req_port = 1;
      cmd_req_bit = 1;
    }
    else if( !strcmp(argv[i], "-u") || !strcmp(argv[i], "--updt"))
    {
      update_datetime = 1;
      cmd_count++;
      cmd_req_address = 1;
    }
    else if( !strcmp(argv[i], "-x") || !strcmp(argv[i], "--pulse"))
    {
      set_reset = 2;
      i++;
      tiempo_seg = atoi(argv[i]);
      cmd_excl_count++;
      cmd_count++;
      cmd_req_address = 1;
      cmd_req_port = 1;
      cmd_req_bit = 1;
    }
    else if( !strcmp(argv[i], "-i") || !strcmp(argv[i], "--invert"))
    {
      set_reset = 3;
      cmd_excl_count++;
      cmd_count++;
      cmd_req_address = 1;
      cmd_req_port = 1;
      cmd_req_bit = 1;
    }
    else if( !strcmp(argv[i], "-f") || !strcmp(argv[i], "--follow"))
    {
      i++;
      set_follow = atoi(argv[i]);
      cmd_excl_count++;
      cmd_count++;
    }
    else if( !strcmp(argv[i], "-o") || !strcmp(argv[i], "--notify"))
    {
      i++;
      set_mask = atoi(argv[i]);
      cmd_excl_count++;
      cmd_count++;
    }
    else if( !strcmp(argv[i], "-p") || !strcmp(argv[i], "--port"))
    {
      i++;
      port = atoi(argv[i]);
    }
    else if( !strcmp(argv[i], "-b") || !strcmp(argv[i], "--bit"))
    {
      i++;
      bit = atoi(argv[i]);
    }
    else if( !strcmp(argv[i], "-g") || !strcmp(argv[i], "--get"))
    {
      get = 1;
      cmd_excl_count++;
      cmd_count++;
      cmd_req_address = 1;
    }
    else if( !strcmp(argv[i], "-e") || !strcmp(argv[i], "--ex"))
    {
      ex = 1;
    }
    else if( !strcmp(argv[i], "-l") || !strcmp(argv[i], "--list"))
    {
      list = 1;
      cmd_excl_count++;
      cmd_count++;
    }
    else if( !strcmp(argv[i], "-n") || !strcmp(argv[i], "--new-addr"))
    {
      i++;
      address = 1;
      new_addr = atoi(argv[i]);
      cmd_excl_count++;
      cmd_count++;
      cmd_req_address = 1;
    }
    else if( !strcmp(argv[i], "-br") || !strcmp(argv[i], "--baudrate"))
    {
      i++;
      baudrate = 1;
      new_baudrate = atoi(argv[i]);
      cmd_excl_count++;
      cmd_count++;
      cmd_req_address = 1;
    }
    else if( !strcmp(argv[i], "-rb") || !strcmp(argv[i], "--reboot"))
    {
      reboot = 1;
      cmd_excl_count++;
      cmd_count++;
      cmd_req_address = 1;
    }
    else if( !strcmp(argv[i], "-a") || !strcmp(argv[i], "--ad"))
    {
      set_ad_alarm = 1;
      i++;
      ad_alarm_max = atoi(argv[i]);
      i++;
      ad_alarm_min = atoi(argv[i]);
      cmd_excl_count++;
      cmd_count++;
      cmd_req_address = 1;
      cmd_req_port = 1;
    }
    else if( !strcmp(argv[i], "--change-config"))
    {
      change_config = 1;
      i++;
      strcpy(change_config_name, argv[i]);
      cmd_excl_count++;
      cmd_count++;
    }
    else if( !strcmp(argv[i], "-m") || !strcmp(argv[i], "--msg") || !strcmp(argv[i], "--message"))
    {
      i++;
      message_line = atoi(argv[i]);
      i++;
      memset(msg_text, 0, 21);
      memcpy(msg_text, argv[i], min(20, strlen(argv[i])));
      cmd_excl_count++;
      cmd_count++;
      cmd_req_address = 1;
    }
    else if( !strcmp(argv[i], "-k"))
    {
      i++;
      num_beeps = atoi(argv[i]);
      cmd_count++;
      cmd_req_address = 1;
    }
    else if( !strcmp(argv[i], "-so") || !strcmp(argv[i], "--set-out"))
    {
      i++;
      out_val = atoi(argv[i]);
      cmd_excl_count++;
      cmd_count++;
      cmd_req_address = 1;
      cmd_req_port = 1;
    }
    else if( !strcmp(argv[i], "--display"))
    {
      i++;
      set_display = atoi(argv[i]);
      cmd_count++;
      cmd_req_address = 1;
    }
    else if( !strcmp(argv[i], "-ir"))
    {
      i++;
      memcpy(msg_text, argv[i], 10);
      ircode = 1;
      cmd_count++;
      cmd_excl_count++;
      cmd_req_address = 1;
    }
    else
    {
      help = 1;
    }
  }

  if( !help)
  {
    /* controles de parametros */
    if( cmd_excl_count > 1 )
    {
      fprintf(stderr, "No se pueden pasar -l, -r, -s, -n, -g, -br, -rb, -a o -m al mismo tiempo.\n");
      help = 1;
    }
    else if( cmd_count == 0 )
    {
      fprintf(stderr, "Debe pedirse al menos un comando -l, -r, -s, -n, -g, -br, -rb, -f o -a.\n");
      help = 1;
    }
    if(cmd_req_address)
    {
      if(disp < 1 || disp > 255)
      {
        fprintf(stderr, "Parametro -d debe ser valido.\n");
        help = 1;
      }
    }
    if(cmd_req_port)
    {
      if(port < 1 || port > 6)
      {
        fprintf(stderr, "Parametro -p debe ser valido.\n");
        help = 1;
      }
    }
    if(cmd_req_bit)
    {
      if(bit < 0 || bit > 7)
      {
        fprintf(stderr, "Parametro -b debe ser valido.\n");
        help = 1;
      }
    }
    if(set_ad_alarm != -1)
    {
      if(ad_alarm_max <= 0 || ad_alarm_max > 1024)
      {
        fprintf(stderr, "parametro max debe estar entre 1 y 1023 (1024 para desactivar).\n");
        help = 1;
      }
      if(ad_alarm_min < 0 || ad_alarm_min >=1024)
      {
        fprintf(stderr, "parametro min debe estar entre 1 y 1023\n");
        help = 1;
      }
    }
  }

  if(help)
  {
    fprintf(stderr, "Uso: %s [-h host][-P port] -l [-e] | -d ( -g [-e] | ( -s/r | -x seg) (-p #port -b #bit) | "
                    "-n new-address | -rb | -br baudrate | -a max min) | -m #line msg | -k #beeps | -so # | -u "
                    "| --display 0/1 | -ir value\n", argv[0]);
    fprintf(stderr, "     donde 'host': nombre o direccion del server.\n");
    fprintf(stderr, "     Comandos:\n");
    fprintf(stderr, "           -l(--list): Lista los dispositivos conectados.\n");
    fprintf(stderr, "           -g(--get): Devolver informacion de todos los puertos (se debe indicar dispositivo).\n");
    fprintf(stderr, "           -s(--set): Set -r(--reset): Reset (se debe indicar dispositivo, bit y port).\n");
    fprintf(stderr, "           -x(--pulse): Pulso (se debe indicar dispositivo, bit, port y cantidad de segundos del pulso).\n");
    fprintf(stderr, "           -i(--invert): Invierte (se debe indicar dispositivo, bit y port).\n");
    fprintf(stderr, "           -rb(--reboot): Resetea el dispositivo (se debe indicar dispositivo).\n");
    fprintf(stderr, "           -m # msg: Imprime msg en la linea # del display (se debe indicar dispositivo).\n");
    fprintf(stderr, "           -k #: Hace sonar # veces el buzzer del dispositivo correspondiente (se debe indicar dispositivo).\n");
    fprintf(stderr, "           -so(--set-out) #: Setea el valor para una salida (se debe indicar dispositivo y port).\n");
    fprintf(stderr, "           -ir value: Envía el codigo value en exadecimal por el puerto infra rojo (se debe indicar dispositivo).\n");
    fprintf(stderr, "     Configuracion:\n");
    fprintf(stderr, "           -f(--follow): Configuracion de seguidores (se debe indicar dispositivo y port). Con port = 10 se\n"
                    "                         setean los seguidores de los canales analogicos para la salida 2.\n");
    fprintf(stderr, "           -o(--notify): Configuracion de notificacion (se debe indicar dispositivo y port). Con port = 10 se\n"
                    "                         setean la notificacion de los canales analogicos, port 1 y 2 corresponden a las entradas 1 y 2\n"
                    "                         y port 3 y 4 corresponden a las salidas 1 y 2.\n");
    fprintf(stderr, "           -n(--new-addr) #: Cambia la direccion del dispositivo a # (se debe indicar dispositivo).\n");
    fprintf(stderr, "           -br(--baudrate) #: Cambia la velocidad serial del dispositivo a # (se debe indicar dispositivo).\n");
    fprintf(stderr, "           -a(--ad): Setea niveles de alarma A/D (se debe indicar dispositivo, port, max 0..1023 o 1024 y min 0..1023).\n");
    fprintf(stderr, "           -u (--updt): Actualiza fecha y hora del dispositivo correspondiente (se debe indicar dispositivo).\n");
    fprintf(stderr, "           --display #: Habilita (#=1) o inhabilita (#=0) el soporte para display (se debe indicar dispositivo).\n");
    fprintf(stderr, "     Parametros:\n");
    fprintf(stderr, "           -d(--disp): Numero de dispositivo (1 a 255) donde 255 es la direccion de broadcast (no es para todos los comandos).\n");
    fprintf(stderr, "           -p(--port): Numero de puerto (1 a 4).\n");
    fprintf(stderr, "           -b(--bit): Numero de bit (0 a 7).\n");
    fprintf(stderr, "     Opciones:\n");
    fprintf(stderr, "           -e(--ex): Devuelve informacion con respuesta extendida en XML, se usa junto con -g o -l.\n");
    exit(1);
  }
  
  pClient = new CGMClient(&gminit);

  if(change_config)
  {
    pClient->Post("hometask-change-config", change_config_name, strlen(change_config_name)+1);
  }
  if(set_reset != -1)
  {
    if(set_reset == 0 || set_reset == 1)
    {
      sprintf(data, "%03i %03i %03i", disp, bit, set_reset);
    }
    else if(set_reset == 2)
    {
      sprintf(data, "%03i %03i %03i", disp, bit, tiempo_seg);
    }
    else if(set_reset == 3)
    {
      sprintf(data, "%03i %03i", disp, bit);
    }
    if(port == 1)
    {
      if(set_reset == 0 || set_reset == 1)
        pClient->Notify("sercmd-s1-bit-on-off", data, strlen(data)+1);
      else if(set_reset == 2)
        pClient->Notify("serpulse-s1-bit", data, strlen(data)+1);
      else if(set_reset == 3)
        pClient->Notify("serinvert-s1-bit", data, strlen(data)+1);
    }
    else
    {
      if(set_reset == 0 || set_reset == 1)
        pClient->Notify("sercmd-s2-bit-on-off", data, strlen(data)+1);
      else if(set_reset == 2)
        pClient->Notify("serpulse-s2-bit", data, strlen(data)+1);
      else if(set_reset == 3)
        pClient->Notify("serinvert-s2-bit", data, strlen(data)+1);
    }
  }
  else if(get == 1)
  {
    sprintf(data, "%03i", disp);
    if(ex == 1)
    {
      fprintf(stdout, "<?xml version=\"1.0\" standalone=\"yes\"?>\n");
      fprintf(stdout, "<serinf-ex>\n");
      if((rc = pClient->Connect("serinf-ex", 1024)) != GME_OK)
      {
        fprintf(stderr, "ERROR: Connect() - %s\n", gmerror.Message(rc).c_str());
        fprintf(stdout, "<resp-code>%i</resp-code>\n", rc);
        fprintf(stdout, "</serinf-ex>\n");
      }
      if((rc = pClient->Send(data, strlen(data)+1)) != GME_OK)
      {
        fprintf(stderr, "ERROR: Send() - %s\n", gmerror.Message(rc).c_str());
        fprintf(stdout, "<resp-code>%i</resp-code>\n", rc);
        fprintf(stdout, "</serinf-ex>\n");
        pClient->Discon();
      }
      fprintf(stdout, "<resp-code>0</resp-code>\n");
      while((rc = pClient->Recv(&resp_data, 300)) == GME_MORE_DATA)
      {
        fprintf(stdout, "%*.*s", (int)resp_data.len, (int)resp_data.len, (char*)resp_data.data);
        pClient->Free(resp_data);
      }
      if(rc == GME_OK)
      {
        fprintf(stdout, "%*.*s", (int)resp_data.len, (int)resp_data.len, (char*)resp_data.data);
        fprintf(stdout, "</serinf-ex>\n");
        pClient->Free(resp_data);
      }
      else
      {
        fprintf(stderr, "ERROR: Recv() - %s\n", gmerror.Message(rc).c_str());
        fprintf(stdout, "</serinf-ex>\n");
      }
      pClient->Discon();
    }
    else
    {
      if(pClient->Call("serinf", data, strlen(data)+1, &resp_data, 3000) == GME_OK)
      {
        fprintf(stdout, "%*.*s\n", (int)resp_data.len, (int)resp_data.len, (char*)resp_data.data);
        pClient->Free(resp_data);
      }
    }
  }
  else if(address == 1)
  {
    sprintf(data, "%03i %03i", disp, new_addr);
    pClient->Notify("serchange-address", data, strlen(data)+1);
  }
  else if(baudrate == 1)
  {
    sprintf(data, "%03i %03i", disp, (new_baudrate/100));
    pClient->Notify("serchange-baudrate", data, strlen(data)+1);
  }
  else if(reboot == 1)
  {
    sprintf(data, "%03i", disp);
    pClient->Notify("serreset", data, strlen(data)+1);
  }
  else if(update_datetime == 1)
  {
    sprintf(data, "%03i", disp);
    pClient->Notify("update-datetime", data, strlen(data)+1);
  }
  else if(list == 1)
  {
    if(ex == 1)
    {
      fprintf(stdout, "<?xml version=\"1.0\" standalone=\"yes\"?>\n");
      fprintf(stdout, "<serlist-ex>\n");
      if((rc = pClient->Call("serlist-ex", NULL, 0, &resp_data, 3000)) == GME_OK)
      {
        fprintf(stdout, "<resp-code>0</resp-code>\n");
        fprintf(stdout, "%*.*s\n", (int)resp_data.len, (int)resp_data.len, (char*)resp_data.data);
        pClient->Free(resp_data);
      }
      else
      {
        fprintf(stdout, "<resp-code>%i</resp-code>\n", rc);
      }
      fprintf(stdout, "</serlist-ex>\n");
    }
    else
    {
      if(pClient->Call("serlist", NULL, 0, &resp_data, 3000) == GME_OK)
      {
        fprintf(stdout, "%*.*s\n", (int)resp_data.len, (int)resp_data.len, (char*)resp_data.data);
        pClient->Free(resp_data);
      }
    }
  }
  else if(set_follow != -1)
  {
    sprintf(data, "%03i %03i %03i", disp, port, set_follow);
    pClient->Notify("sercmd-seguidores", data, strlen(data)+1);
  }
  else if(set_mask != -1)
  {
    sprintf(data, "%03i %03i %03i", disp, port, set_mask);
    pClient->Notify("sercmd-mask", data, strlen(data)+1);
  }
  else if(set_ad_alarm != -1)
  {
    sprintf(data, "%03i %03i %05i %05i", disp, port, ad_alarm_max, ad_alarm_min);
    pClient->Notify("serset-ad-alarm", data, strlen(data)+1);
  }
  else if(message_line != 0)
  {
    sprintf(data, "%03i %03i %s", disp, message_line, msg_text);
    pClient->Notify("sermessage", data, strlen(data)+1);
  }
  else if(out_val != -1)
  {
    sprintf(data, "%03i %03i %03i", disp, port, out_val);
    pClient->Notify("set-out", data, strlen(data)+1);
  }
  else if(ircode != 0)
  {
    sprintf(data, "%03i %-10.10s", disp, msg_text);
    pClient->Notify("send-ir", data, strlen(data)+1);
  }



  /* estos comando se pueden combinar con otros */
  if(num_beeps != -1)
  {
    sprintf(data, "%03i %03i", disp, num_beeps);
    pClient->Notify("serbuzzer", data, strlen(data)+1);
  }
  if(set_display != -1)
  {
    sprintf(data, "%03i %03i %03i %03i %03i", disp, set_display, 0, 0, 0);
    pClient->Notify("display-config", data, strlen(data)+1);
  }

  delete pClient;
  return 0;
}
