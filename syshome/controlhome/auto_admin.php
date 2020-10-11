<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::AdminAuto")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>

<head>
  <title>GmHome::Administracion de eventos</title>
  <link title="new" rel="stylesheet" href="css/event_admin.css" type="text/css">
</head>

<script language="javascript" type="text/javascript">
  function OpenABM( filename, param, title )
  {
    window.open(filename + param, title, 'tollbar=no,status=no,menubar=no,scrollbars=no,resizable=no,width=700,height=450');
  }
  function reloadWindow()
  {
    self.location.reload();
  }
  function OpenTempControl( disp, port )
  {
    if ( disp <= 0 ) return;
    if ( port <= 0 ) return;
    window.open('temp_control.php?sel_id=' + disp + '&sel_port=' + port, 'Control de temperatura', 'tollbar=no,status=no,menubar=no,scrollbars=no,resizable=no,width=800,height=450');
  }
</script>

<body bgcolor="#FFFFFF">
<table border="0" width="80%" align=center>
<td align=left><h1>Administraci&oacute;n de automatizaci&oacute;n</h1></td>
<td align=right><a href="index.php?view=0" target=_top><img src="images/back.png" border="0"></a></td>
</table>

<script language="php">
$db = new PPgSqlDB;
$db->Open($dbserver, "home", $dbuser, $dbpass);
if( !isset($folder)) $folder = 1;
</script>

<table border="0" width="80%" align=center bgcolor="#F0F0F0">
<tr>
  <td <? if($folder == 1) echo "bgcolor=\"FFD0D0\""; ?> align="center"><a class=tablelink href="?folder=1"><b>Alarma&nbsp;|&nbsp;</b></a></td>
  <td <? if($folder == 2) echo "bgcolor=\"FFD0D0\""; ?> align="center"><a class=tablelink href="?folder=2"><b>Climatizaci&oacute;n&nbsp;|&nbsp;</b></a></td>
  <td <? if($folder == 3) echo "bgcolor=\"FFD0D0\""; ?> align="center"><a class=tablelink href="?folder=3"><b>Riego&nbsp;|&nbsp;</b></a></td>
  <td <? if($folder == 4) echo "bgcolor=\"FFD0D0\""; ?> align="center"><a class=tablelink href="?folder=4"><b>Entretenimiento&nbsp;|&nbsp;</b></a></td>
  <td <? if($folder == 5) echo "bgcolor=\"FFD0D0\""; ?> align="center"><a class=tablelink href="?folder=5"><b>Reloj&nbsp;|&nbsp;</b></a></td>
  <td width="50%">&nbsp;</td>
</tr>
</table>
<br />

<? if( $folder == 1) {

if( isset($update) && $update == "home_config" )
{
  if( !isset($autoactivacion)) $autoactivacion = 0;
  if( !isset($autoreactivacion)) $autoreactivacion = 0;
  $db->Update("home_config", "autoactivacion_alarma = $autoactivacion,autoreactivacion_alarma = $autoreactivacion", "id = 0");
  exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport --change-config tarea_programada");
}
?>
<form name ="alarma" method=POST action="auto_admin.php">
  <input type=hidden name="folder" value="1">
  <input type=hidden name="update" value="home_config">
  <?
  if($db->Select( "autoactivacion_alarma,autoreactivacion_alarma", // SELECT
                  "home_config",    // FROM
                  "id = 0",         // WHERE
                  "",               // ORDER BY
                  ""))              // GROUP BY
  {
    $autoactivacion = $db->SelectData(1);
    $autoreactivacion = $db->SelectData(2);
    ?>
    <table border="0" width="80%" bgcolor="#FFFFFF" align=center>
    <tr><td bgcolor="#FFD0D0"><b>Opciones</b></td></tr>
    <tr><td><input type=checkbox onchange="submit()" name=autoactivacion value="1" <? if($autoactivacion == 1){echo "checked";} ?> >&nbsp;&nbsp;&nbsp;&nbsp;Activaci&oacute;n autom&aacute;tica por inactividad</td></tr>
    <tr><td><input type=checkbox onchange="submit()" name=autoreactivacion value="1"<? if($autoreactivacion == 1){echo "checked";} ?> >&nbsp;&nbsp;&nbsp;&nbsp;Reactivaci&oacute;n autom&aacute;tica por inactividad</td></tr>
    </table>
    <?
  }
  ?>
  <br />
  <?
  if($db->Select( "assign.assign_name", // SELECT
                  "assign, home_config",    // FROM
                  "assign.id = home_config.activacion_dev AND assign.port = activacion_port AND assign.bit = activacion_bit",         // WHERE
                  "",               // ORDER BY
                  ""))              // GROUP BY
  {
    $activacion_name = $db->SelectData(1);
  }
  if($db->Select( "group_list.group_name", // SELECT
                  "group_list, auto_event",    // FROM
                  "group_list.id = auto_event.dst_group AND auto_event.id = 'autoactivacion'",         // WHERE
                  "",               // ORDER BY
                  ""))              // GROUP BY
  {
    $autoactivacion_name = $db->SelectData(1);
  }
  ?>
  <table border="0" width="80%" bgcolor="#FFFFFF" align=center>
  <tr><td bgcolor="#FFD0D0"><b>Par&aacute;metros</b></td></tr>
  <tr><td>Para monitorear la activacion: <b><? echo "$activacion_name"; ?></b></td></tr>
  <tr><td>Para activacion eviar pulso a: <b><? echo "$autoactivacion_name"; ?></b></td></tr>
  </table>
  <br />
  <table border="0" width="80%" bgcolor="#FFFFFF" align=center>
  <tr>
  <td bgcolor="#FFD0D0"><b>Sensores de alarma</b></td>
  <td bgcolor="#FFD0D0" align=center><b>Ultimo</b></td>
  </tr>
    <script language="php">
    // Listado de asignaciones
    $count = 0;
    if($db->Select( "autoactivacion.id, autoactivacion.port, autoactivacion.bit, autoactivacion.ultimo, assign.assign_name", // SELECT
                    "autoactivacion, assign", // FROM
                    "autoactivacion.id = assign.id AND autoactivacion.port = assign.port AND autoactivacion.bit = assign.bit", // WHERE
                    "autoactivacion.id, autoactivacion.port, autoactivacion.bit, autoactivacion.ultimo", // ORDER BY
                    "")) // GROUP BY
    {
      do
      {
        $id =       $db->SelectData(1);
        $port =     $db->SelectData(2);
        $bit =      $db->SelectData(3);
        $ultimo=    $db->SelectData(4);
        $ass_name = $db->SelectData(5);
        if($count%2)
          $color="#D0D0FF";
        else
          $color="#D0FFD0";
        ?>
          <tr>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_alarma.php', '?id=<? echo "$id"; ?>&port=<? echo "$port"; ?>&bit=<? echo "$bit"; ?>', 'Assign' )">
                <? printf("%s", $ass_name); ?>
              </a>
            </td>
            <td bgcolor="<? echo $color; ?>" align=center>
              <input type=radio name=ultimo value="<? printf("%03d%2.2s%01d", $id, $port, $bit); ?>" <? if($ultimo) echo "checked"; ?>>
            </td>
          </tr>
        <?
        $count++;
      } while($db->SelectNext());
    }
    </script>
    <tr>
      <td bgcolor="#FFD0D0">&nbsp;</td>
      <td bgcolor="#FFD0D0" align="right"><a href="javascript:OpenABM( 'abm_alarma.php', '', 'Alarma' )">Nuevo</a></td>
    </tr>
  </table>


</form>

<? } else if( $folder == 2) { ?>

<?
  $data1 = array();
  $valid_data1 = false;
  $data3 = array();
  $valid_data3 = false;

  if( !isset($seguidor1)) $seguidor1 = 0;
  if( !isset($seguidor3)) $seguidor3 = 0;

  if( isset($update) && $update == "seguidores" )
  {
    if(control_permisos($SESSION_USER, "SysHome::ControlHome", $REMOTE_ADDR))
    {
      if( isset($calefaccion_dorm_frente)) $seguidor1 |= 4;
      else $seguidor1 &= 11;
      if( isset($calefaccion_dorm_fondo)) $seguidor1 |= 2;
      else $seguidor1 &= 13;
      if( isset($calefaccion_comedor)) $seguidor1 |= 8;
      else $seguidor1 &= 7;
      if( isset($calefaccion_taller)) $seguidor3 |= 2;
      else $seguidor3 &= 13;

      /* actualizo el estado de los termostatos */
      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d 1 -p 10 -f $seguidor1");
      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d 3 -p 10 -f $seguidor3");

      /* apago los calefactores */
      if( !isset($calefaccion_dorm_frente))
        exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d 1 -p 2 -b 2 -s");
      if( !isset($calefaccion_dorm_fondo))
        exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d 1 -p 2 -b 1 -s");
      if( !isset($calefaccion_comedor))
        exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d 1 -p 2 -b 3 -s");
      if( !isset($calefaccion_taller))
        exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d 3 -p 2 -b 1 -s");
    }
  }
  else
  {
    /* consulta de estados de todos los ports de la interface 1 */
    $cmd = "/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -g -d 1 -e";
    exec($cmd, $output);
    if(count($output) > 0)
    {
      $output = implode("", $output);
      $p = xml_parser_create();
      xml_parser_set_option($p, XML_OPTION_CASE_FOLDING, 0);
      xml_parser_set_option($p, XML_OPTION_SKIP_WHITE, 1);
      $rc = xml_parse_into_struct($p, $output, $vals, $index);
      xml_parser_free($p);
      if($rc == 1)
      {
        for( $i = 0; $i < count($vals);$i++ )    
        {
          if($vals[$i]['type'] == "complete")
          {
            if($vals[$i]['tag'] == "name") $port = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "bit") $bit = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "value") $val = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "seguidor") $seguidores = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "alarm") $alarm_state = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "max") $alarm_max = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "min") $alarm_min = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "last-change")
            {
              $last_change = $vals[$i]['value'];
              if( substr($port, 0, 1) == 'A' )
              {
                $data1[$port]['value'] = $val;
                $data1[$port]['alarm'] = $alarm_state;
                $data1[$port]['max'] = $alarm_max;
                $data1[$port]['min'] = $alarm_min;
                $data1[$port]['seguidor'] = $seguidores;
                $data1[$port]['last-change'] = $last_change;
              }
              else
              {
                $data1[$port][$bit]['value'] = $val;
                if( substr($port, 0, 1) == 'I' )
                  $data1[$port]['seguidor'] = $seguidores;
                $data1[$port][$bit]['last-change'] = $last_change;
              }
              $valid_data1 = true;
              $bit = 0;
              $seguidor1 = $data1["A1"]['seguidor'];
            }
          }
        }
      }
    }
    if($valid_data1)
    {
      $calefaccion_dorm_frente = $seguidor1 & 4; 
      $calefaccion_dorm_fondo = $seguidor1 & 2; 
      $calefaccion_comedor = $seguidor1 & 8;
      $calefaccion_bano = 0;

      $refrigeracion_dorm_frente = 0; 
      $refrigeracion_dorm_fondo = 0; 
      $refrigeracion_comedor = 0;

      $humedad_dorm_frente = 0; 
      $humedad_dorm_fondo = 0; 
      $humedad_comedor = 0;
    }

    /* consulta de estados de todos los ports de la interface 3 */
    $cmd = "/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -g -d 3 -e";
    exec($cmd, $output);
    if(count($output) > 0)
    {
      $output = implode("", $output);
      $p = xml_parser_create();
      xml_parser_set_option($p, XML_OPTION_CASE_FOLDING, 0);
      xml_parser_set_option($p, XML_OPTION_SKIP_WHITE, 1);
      $rc = xml_parse_into_struct($p, $output, $vals, $index);
      xml_parser_free($p);
      if($rc == 1)
      {
        for( $i = 0; $i < count($vals);$i++ )    
        {
          if($vals[$i]['type'] == "complete")
          {
            if($vals[$i]['tag'] == "name") $port = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "bit") $bit = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "value") $val = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "seguidor") $seguidores = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "alarm") $alarm_state = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "max") $alarm_max = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "min") $alarm_min = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "last-change")
            {
              $last_change = $vals[$i]['value'];
              if( substr($port, 0, 1) == 'A' )
              {
                $data3[$port]['value'] = $val;
                $data3[$port]['alarm'] = $alarm_state;
                $data3[$port]['max'] = $alarm_max;
                $data3[$port]['min'] = $alarm_min;
                $data3[$port]['seguidor'] = $seguidores;
                $data3[$port]['last-change'] = $last_change;
              }
              else
              {
                $data3[$port][$bit]['value'] = $val;
                if( substr($port, 0, 1) == 'I' )
                  $data3[$port]['seguidor'] = $seguidores;
                $data3[$port][$bit]['last-change'] = $last_change;
              }
              $valid_data3 = true;
              $bit = 0;
              $seguidor3 = $data3["A1"]['seguidor'];
            }
          }
        }
      }
    }
    if($valid_data3)
    {
      $calefaccion_taller = $seguidor3 & 2;;

      $refrigeracion_taller = 0;

      $humedad_taller = 0;
    }


  }
?>

<form name ="clima" method=POST action="auto_admin.php">
  <input type=hidden name="folder" value="2">
  <input type=hidden name="update" value="seguidores">
  <input type=hidden name="seguidor1" value="<? printf("%d", $seguidor1); ?>">

  <table border="0" width="80%" bgcolor="#FFFFFF" align=center>
  <tr><td bgcolor="#FFD0D0"><b>Calefacci&oacute;n</b></td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=calefaccion_dorm_frente value="1"
      <? if($calefaccion_dorm_frente){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Dormitorio frente&nbsp;
      <a href="javascript:OpenTempControl('1','3');"><img src="images/termometro-mini-16-rojo.gif" border="0"></a>
  </td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=calefaccion_dorm_fondo value="1"
      <? if($calefaccion_dorm_fondo){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Dormitorio fondo&nbsp;
      <a href="javascript:OpenTempControl('1','2');"><img src="images/termometro-mini-16-rojo.gif" border="0"></a>
  </td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=calefaccion_comedor value="1"
      <? if($calefaccion_comedor){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Comedor&nbsp;
      <a href="javascript:OpenTempControl('1','4');"><img src="images/termometro-mini-16-rojo.gif" border="0"></a>
  </td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=calefaccion_bano value="1"
      <? if($calefaccion_bano){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Ba&ntilde;o&nbsp;
      <a href="javascript:OpenTempControl('0','0');"><img src="images/termometro-mini-16-rojo.gif" border="0"></a>
  </td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=calefaccion_taller value="1"
      <? if($calefaccion_taller){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Taller&nbsp;
      <a href="javascript:OpenTempControl('3','2');"><img src="images/termometro-mini-16-rojo.gif" border="0"></a>
  </td></tr>
  </table>
  <br />
  <table border="0" width="80%" bgcolor="#FFFFFF" align=center>
  <tr><td bgcolor="#FFD0D0"><b>Refrigeraci&oacute;n</b></td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=refrigeracion_dorm_frente value="1"
      <? if($refrigeracion_dorm_frente){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Dormitorio frente&nbsp;
      <a href="javascript:OpenTempControl('0','0');"><img src="images/termometro-mini-16.gif" border="0"></a>
  </td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=refrigeracion_dorm_fondo value="1"
      <? if($refrigeracion_dorm_fondo){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Dormitorio fondo&nbsp;
      <a href="javascript:OpenTempControl('0','0');"><img src="images/termometro-mini-16.gif" border="0"></a>
  </td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=refrigeracion_comedor value="1"
      <? if($refrigeracion_comedor){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Comedor&nbsp;
      <a href="javascript:OpenTempControl('0','0');"><img src="images/termometro-mini-16.gif" border="0"></a>
  </td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=refrigeracion_taller value="1"
      <? if($refrigeracion_taller){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Taller&nbsp;
      <a href="javascript:OpenTempControl('0','0');"><img src="images/termometro-mini-16.gif" border="0"></a>
  </td></tr>
  </table>
  <br />
  <table border="0" width="80%" bgcolor="#FFFFFF" align=center>
  <tr><td bgcolor="#FFD0D0"><b>Humedad</b></td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=humedad_dorm_frente value="1"
      <? if($humedad_dorm_frente){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Dormitorio frente&nbsp;
      <a href="javascript:OpenTempControl('0','0');"><img src="images/gota-mini-16.gif" border="0"></a>
  </td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=humedad_dorm_fondo value="1"
      <? if($humedad_dorm_fondo){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Dormitorio fondo&nbsp;
      <a href="javascript:OpenTempControl('0','0');"><img src="images/gota-mini-16.gif" border="0"></a>
  </td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=humedad_comedor value="1"
      <? if($humedad_comedor){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Comedor&nbsp;
      <a href="javascript:OpenTempControl('0','0');"><img src="images/gota-mini-16.gif" border="0"></a>
  </td></tr>
  <tr><td>
    <input type=checkbox onchange="submit()" name=humedad_taller value="1"
      <? if($humedad_taller){echo "checked";} ?> >
      &nbsp;&nbsp;&nbsp;&nbsp;Taller&nbsp;
      <a href="javascript:OpenTempControl('0','0');"><img src="images/gota-mini-16.gif" border="0"></a>
  </td></tr>
  </table>
  <br />

</form>

<? } else if( $folder == 3) { ?>

<form name ="riego" method=POST action="abm_riego.php">

</form>

<? } else if( $folder == 4) { ?>

<form name ="entret" method=POST action="abm_entret.php">

</form>

<? } else if( $folder == 5) { ?>

<form name ="reloj" method=POST action="abm_reloj.php">
  <table border="0" width="80%" bgcolor="#FFFFFF" align=center>
  <tr>
  <td bgcolor="#FFD0D0"><b>Hora</b></td>
  <td bgcolor="#FFD0D0"><b>Dia semana</b></td>
  <td bgcolor="#FFD0D0"><b>Comando</b></td>
  <td bgcolor="#FFD0D0"><b>Descripcion</b></td>
  </tr>
    <script language="php">
    $count = 0;
    if($db->Select( "task.id, task.dia_semana, task.hora, task.minuto, cmd.cmd_name, task.enable, task.task_name", // SELECT
                    "tarea_programada as task, comando as cmd", // FROM
                    "task.dst_cmd = cmd.id", // WHERE
                    "hora, minuto", // ORDER BY
                    "")) // GROUP BY
    {
      do
      {
        $id =         $db->SelectData(1);
        $dia_semana = $db->SelectData(2);
        $hora =       $db->SelectData(3);
        $minuto =     $db->SelectData(4);
        $cmd_name =   $db->SelectData(5);
        $enable =     $db->SelectData(6);
        $task_name =  $db->SelectData(7);
        if($count%2)
          $color="#D0D0FF";
        else
          $color="#D0FFD0";
        ?>
          <tr>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_task.php', '?id=<? echo "$id"; ?>', 'Task' )">
                <? 
                if($enable == 0) echo "<font color=\"#F00000\">";
                printf("%2d:%02d", $hora, $minuto);
                if($enable == 0) echo "</font>";
                ?>
              </a>
            </td>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_task.php', '?id=<? echo "$id"; ?>', 'Task' )">
                <? if($dia_semana&1)  { echo "Lu"; } else {echo "__";} ?>
                <? if($dia_semana&2)  { echo "Ma"; } else {echo "__";} ?>
                <? if($dia_semana&4)  { echo "Mi"; } else {echo "__";} ?>
                <? if($dia_semana&8)  { echo "Ju"; } else {echo "__";} ?>
                <? if($dia_semana&16) { echo "Vi"; } else {echo "__";} ?>
                <? if($dia_semana&32) { echo "Sa"; } else {echo "__";} ?>
                <font color="#FF0000"><b><? if($dia_semana&64) { echo "Do"; } else {echo "__";} ?></b></font>
              </a>
            </td>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_task.php', '?id=<? echo "$id"; ?>', 'Task' )">
                <? 
                if($enable == 0) echo "<font color=\"#F00000\">";
                printf("%s", $cmd_name);
                if($enable == 0) echo "</font>";
                ?>
              </a>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_task.php', '?id=<? echo "$id"; ?>', 'Task' )">
                <? 
                if($enable == 0) echo "<font color=\"#F00000\">";
                printf("%s", $task_name);
                if($enable == 0) echo "</font>";
                ?>
              </a>
            </td>
          </tr>
        <?
        $count++;
      } while($db->SelectNext());
    }
    </script>
    <tr>
      <td bgcolor="#FFD0D0">&nbsp;</td>
      <td bgcolor="#FFD0D0">&nbsp;</td>
      <td bgcolor="#FFD0D0">&nbsp;</td>
      <td bgcolor="#FFD0D0" align="right"><a href="javascript:OpenABM( 'abm_task.php', '', 'Task' )">Nuevo</a></td>
    </tr>
  </table>

</form>

<? } ?>

<script language="php">
  if( $db->IsOpen() ) $db->Close();
</script>

</body>
</html>
