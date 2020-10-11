<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::AdminAuto::ABMTask")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Task</title>
  <script language="javascript">
    function closeWindow() { window.close(); }
  </script>
</head>

<script language="php">
  $db = new PPgSqlDB;
  $db->Open($dbserver, "home", $dbuser, $dbpass);

  $dia_semana = 0;
  if( isset($dia_semana_lunes)) $dia_semana += 1;
  if( isset($dia_semana_martes)) $dia_semana += 2;
  if( isset($dia_semana_miercoles)) $dia_semana += 4;
  if( isset($dia_semana_jueves)) $dia_semana += 8;
  if( isset($dia_semana_viernes)) $dia_semana += 16;
  if( isset($dia_semana_sabado)) $dia_semana += 32;
  if( isset($dia_semana_domingo)) $dia_semana += 64;

  if(isset($dst))
  {
    $dst_group =  substr($dst, 0, 5);
  }

  if(isset($cmd))
  {
    $dst_cmd =substr($cmd, 0, 3);
  }

  if(isset($flag_condicion))
  {
    if($flag_condicion == "igual") $flag_condicion = 1;
    else if($flag_condicion == "mayor") $flag_condicion = 2;
    else if($flag_condicion == "menor") $flag_condicion = 3;
    else if($flag_condicion == "mayor o igual") $flag_condicion = 4;
    else if($flag_condicion == "menor o igual") $flag_condicion = 5;
    else if($flag_condicion == "distinto") $flag_condicion = 6;
    else $flag_condicion = 0;
  }

  if( !isset($enable))
  {
    $enable = 0;
  }

  if( isset($actualizar))
  {
    $query_set = "dia_semana = $dia_semana, hora = $hora, minuto = $minuto, ".
                  "dst_cmd = '$dst_cmd', ".
                  "task_name = '$task_name', enable = $enable, dst_param1 = '$dst_param1', ".
                  "flag = '$flag', flag_condicion = $flag_condicion, ".
                  "flag_valor = '$flag_valor', dst_param2 = '$dst_param2'";
    if( $dst_group > 0)
    {
      $query_set .= ", dst_group = $dst_group";
    }
    if( !$db->Update("tarea_programada", $query_set, "id = $bkp_id"))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }
    exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport --change-config tarea_programada");
    ?>
      <script language="javascript">
        window.close();
      </script>
    <?
  }
  else if( isset($agregar))
  {
    $query_into = "dia_semana, hora, minuto, dst_cmd, task_name, enable, dst_param1, flag, flag_condicion, flag_valor, dst_param2";
    $query_values = "$dia_semana, $hora, $minuto, '$dst_cmd', '$task_name', $enable, '$dst_param1', '$flag', $flag_condicion, '$flag_valor', '$dst_param2'";
    if( $dst_group > 0)
    {
      $query_into .= ", dst_group";
      $query_values .= ", $dst_group";
    }

    if( !$db->Insert("tarea_programada", $query_into, $query_values))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }
    exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport --change-config tarea_programada");
    ?>
      <script language="javascript">
        window.close();
      </script>
    <?
  }
  else if( isset($eliminar))
  {
    if( !$db->Delete("tarea_programada", "id = $bkp_id"))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }
    exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport --change-config tarea_programada");
    ?>
      <script language="javascript">
        window.close();
      </script>
    <?
  }
  else if( isset($id))
  {
    if($db->Select("id, dia_semana, hora, minuto, dst_group, dst_cmd,".
                     " enable, task_name, dst_param1, flag, flag_condicion, ".
                     "flag_valor, dst_param2, last_exec", // SELECT
                   "tarea_programada",                         // FROM
                   "id = $id",                                 // WHERE
                   "hora, minuto",   // ORDER BY
                   ""))                                        // GROUP BY
    {
      $id         = $db->SelectData(1);
      $dia_semana = $db->SelectData(2);
      $hora       = $db->SelectData(3);
      $minuto     = $db->SelectData(4);
      $dst_group  = $db->SelectData(5);
      $dst_cmd    = $db->SelectData(6);
      $enable     = $db->SelectData(7);
      $task_name  = $db->SelectData(8);
      $dst_param1 = $db->SelectData(9);
      $flag       = $db->SelectData(10);
      if($flag == "siempre")
      {
        $flag_condicion = 0;
        $flag_valor = "";
      }
      else
      {
        $flag_condicion = $db->SelectData(11);
        $flag_valor = $db->SelectData(12);
      }
      $dst_param2 = $db->SelectData(13);
      $last_exec = $db->SelectData(14);
    }
  }
  else
  {
    $id         = "";
    $dia_semana = 127;
    $hora       = 12;
    $minuto     = 0;
    $dst_group  = "";
    $dst_cmd    = "";
    $enable     = 0;
    $task_name  = "";
    $dst_param1 = "";
    $dst_param2 = "";
    $flag       = "siempre";
    $flag_condicion = 0;
    $flag_valor = "";
    $last_exec = "NUNCA";
  }
  if(strlen($last_exec) == 0) $last_exec = "NUNCA";
</script>
<body bgcolor="#F0F0F0">
<h1>Tarea</h1>
<form name="frm_abm_task" method=POST action="abm_task.php">
<table border="0" width="90%" align="center" bgcolor="#F0F0F0">
<tr>
  <td>Hora</td><td>&nbsp;</td>
</tr>
<tr>
  <td>
    <select name="hora" size="1">
      <option>&nbsp;</option>
      <?
      for($i=0; $i < 24;$i++)
      {
        if($i == $hora)
          printf("<option selected>%02d</option>", $i);
        else
          printf("<option>%02d</option>", $i);
      }
      ?>
    </select>
    &nbsp;:&nbsp;
    <select name="minuto" size="1">
      <option>&nbsp;</option>
      <?
      for($i=0; $i < 60;$i++)
      {
        if($i == $minuto)
          printf("<option selected>%02d</option>", $i);
        else
          printf("<option>%02d</option>", $i);
      }
      ?>
    </select>
  </td>
  <td>&nbsp;</td>
</tr>
<tr>
  <td>
    Dias:&nbsp;&nbsp;Lu<input type=checkbox name=dia_semana_lunes value="1" <? if($dia_semana & 1) echo "checked";?> >
    &nbsp;Ma<input type=checkbox name=dia_semana_martes value="1" <? if($dia_semana & 2) echo "checked";?> >
    &nbsp;Mi<input type=checkbox name=dia_semana_miercoles value="1" <? if($dia_semana & 4) echo "checked";?> >
    &nbsp;Ju<input type=checkbox name=dia_semana_jueves value="1" <? if($dia_semana & 8) echo "checked";?> >
    &nbsp;Vi<input type=checkbox name=dia_semana_viernes value="1" <? if($dia_semana & 16) echo "checked";?> >
    &nbsp;Sa<input type=checkbox name=dia_semana_sabado value="1" <? if($dia_semana & 32) echo "checked";?> >
    &nbsp;Do<input type=checkbox name=dia_semana_domingo value="1" <? if($dia_semana & 64) echo "checked";?> >
  </td>
  <td><b>&Uacute;ltima: <? if($last_exec == "NUNCA"){ echo "NUNCA"; }else{ print( strftime("%d/%m/%Y %H:%M:%S", $last_exec)); } ?></b></td>
</tr>
<tr>
  <td>Comando</td><td>Par&aacute;metros</td>
</tr>
<tr>
  <td>
    <select name="cmd" size="1">
      <option>&nbsp;</option>
      <?
      if($db->Select("id, cmd_name", // SELECT
                     "comando", // FROM
                     "", // WHERE
                     "id", // ORDER BY
                     "")) // GROUP BY
      {
        do
        {
          $cmd_id     = $db->SelectData(1);
          $cmd_name   = $db->SelectData(2);
          if($id != "")
          {
            if($cmd_id == $dst_cmd)
              printf("<option selected>%03d - %s</option>", $cmd_id, $cmd_name);
            else
              printf("<option>%03d - %s</option>", $cmd_id, $cmd_name);
          }
          else printf("<option>%03d - %s</option>", $cmd_id, $cmd_name);
        } while($db->SelectNext());
      }
      ?>
      </select>
  </td>
  <td>
    1: <input type=text name="dst_param1" size="10" maxlength="256" value="<?echo "$dst_param1";?>">&nbsp;&nbsp;
    2: <input type=text name="dst_param2" size="10" maxlength="256" value="<?echo "$dst_param2";?>">
  </td>
</tr>
<tr>
  <td>Condicion</td><td>&nbsp;</td>
</tr>
<tr>
  <td>
    <select name="flag" size="1">
      <option>&nbsp;</option>
      <?
      if($db->Select("nombre",       // SELECT
                     "home_flag",    // FROM
                     "",             // WHERE
                     "nombre",       // ORDER BY
                     ""))            // GROUP BY
      {
        do
        {
          $flag_nombre = $db->SelectData(1);
          if($id != "")
          {
            if($flag_nombre == $flag)
              printf("<option selected>%s</option>", $flag_nombre);
            else
              printf("<option>%s</option>", $flag_nombre);
          }
          else 
          {
            if($flag_nombre == "siempre")
              printf("<option selected>%s</option>", $flag_nombre);
            else
              printf("<option>%s</option>", $flag_nombre);
          }
        } while($db->SelectNext());
      }
      ?>
    </select>
    &nbsp;
    <select name="flag_condicion" size="1">
      <option>&nbsp;</option>
      <option <? if($flag_condicion == 1) echo "selected"; ?>>igual</option>
      <option <? if($flag_condicion == 2) echo "selected"; ?>>mayor</option>
      <option <? if($flag_condicion == 3) echo "selected"; ?>>menor</option>
      <option <? if($flag_condicion == 4) echo "selected"; ?>>mayor o igual</option>
      <option <? if($flag_condicion == 5) echo "selected"; ?>>menor o igual</option>
      <option <? if($flag_condicion == 6) echo "selected"; ?>>distinto</option>
    </select>
  </td>
  <td><input type=text name="flag_valor" size="20%" maxlength="256" value="<?echo "$flag_valor";?>"></td>
</tr>
<tr>
  <td>Destino</td><td>&nbsp;</td>
</tr>
<tr>
  <td>
    <select name="dst" size="1">
      <option>&nbsp;</option>
      <?
      if($db->Select("id, group_name", // SELECT
                     "group_list",     // FROM
                     "",               // WHERE
                     "id",             // ORDER BY
                     ""))              // GROUP BY
      {
        do
        {
          $group_id     = $db->SelectData(1);
          $group_name = $db->SelectData(2);
          if($id != "")
          {
            if($group_id == $dst_group)
              printf("<option selected>%05d - %s</option>", $group_id, $group_name);
            else
              printf("<option>%05d - %s</option>", $group_id, $group_name);
          }
          else printf("<option>%05d - %s</option>", $group_id, $group_name);
        } while($db->SelectNext());
      }
      ?>
      </select>
  </td>
  <td>&nbsp;</td>
</tr>
</table>
<table border="0" width="90%" align="center" bgcolor="#F0F0F0">
<tr>
  <td>Nombre</td>
</tr>
<tr>
  <td><input type=text name="task_name" size="70%" maxlength="256" value="<?echo "$task_name";?>"></td>
</tr>
</table>

<table border="0" width="90%" align="center">
<tr>
  <td>&nbsp;</td>
  <td>&nbsp;</td>
</tr>
<tr>
  <td>
    <input type=checkbox name=enable value="1" <? if($enable != 0) echo "checked";?> > - Habilitado
  </td>
  <td align="right" bgcolor="#E0E0E0">
    <? if($id != "") {?>
      <input type=submit name="actualizar"  value="Actualizar">&nbsp;&nbsp;
      <input type=submit name="eliminar"  value="Eliminar">&nbsp;&nbsp;
    <? }else{ ?>
      <input type=submit name="agregar"  value="Agregar ">&nbsp;&nbsp;
    <? } ?>
    <input type=submit name="cancelar" value="Cancelar" onClick="closeWindow()">
  </td>
</tr>
</table>
<input type=hidden name="bkp_id" value="<? echo "$id"; ?>">
</form>
</body>
</html>
<? $db->Close(); ?>
