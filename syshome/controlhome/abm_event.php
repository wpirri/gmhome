<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::AdminEvent::ABMEvent")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Event</title>
  <script language="javascript">
    function closeWindow() { window.close(); }
  </script>
</head>

<script language="php">
  $db = new PPgSqlDB;
  $db->Open($dbserver, "home", $dbuser, $dbpass);

  if(isset($src))
  {
    $src_dev =  substr($src, 0, 3);
    $src_port = substr($src, 3, 2);
    $src_bit =  substr($src, 5, 1);
  }

  if(isset($dst))
  {
    $dst_group =  substr($dst, 0, 5);
  }

  if(isset($src_on_off))
  {
    if($src_on_off == "On")       $src_on_off = 1;
    else if($src_on_off == "Off") $src_on_off = 0;
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
    $query_set = "src_dev = $src_dev, src_port = '$src_port', src_bit = $src_bit, ".
                  "src_on_off = $src_on_off, dst_cmd = '$dst_cmd', ".
                  "event_name = '$ev_name', enable = $enable, dst_param1 = '$dst_param1', ".
                  "flag = '$flag', flag_condicion = $flag_condicion, ".
                  "flag_valor = '$flag_valor', dst_param2 = '$dst_param2'";
    if( $dst_group > 0)
    {
      $query_set .= ", dst_group = $dst_group";
    }
    if( !$db->Update("event", $query_set, "id = $bkp_id"))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }
    ?>
      <script language="javascript">
        window.close();
      </script>
    <?
  }
  else if( isset($agregar))
  {
    $query_into = "src_dev, src_port, src_bit, src_on_off, dst_cmd, event_name, enable, dst_param1, flag, flag_condicion, flag_valor, dst_param2";
    $query_values = "$src_dev, '$src_port', $src_bit, $src_on_off, '$dst_cmd', '$ev_name', $enable, '$dst_param1', '$flag', $flag_condicion, '$flag_valor', '$dst_param2'";
    if( $dst_group > 0)
    {
      $query_into .= ", dst_group";
      $query_values .= ", $dst_group";
    }

    if( !$db->Insert("event", $query_into, $query_values))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }
    ?>
      <script language="javascript">
        window.close();
      </script>
    <?
  }
  else if( isset($eliminar))
  {
    if( !$db->Delete("event", "id = $bkp_id"))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }
    ?>
      <script language="javascript">
        window.close();
      </script>
    <?
  }
  else if( isset($id))
  {
    if($db->Select("id, src_dev, src_port, src_bit, src_on_off, dst_group, dst_cmd,".
                   " enable, event_name, dst_param1, flag, flag_condicion, flag_valor, dst_param2", // SELECT
                   "event",                                    // FROM
                   "id = $id",                                 // WHERE
                   "src_dev, src_port, src_bit, src_on_off",   // ORDER BY
                   ""))                                        // GROUP BY
    {
      $id         = $db->SelectData(1);
      $src_dev    = $db->SelectData(2);
      $src_port   = $db->SelectData(3);
      $src_bit    = $db->SelectData(4);
      $src_on_off = $db->SelectData(5);
      $dst_group  = $db->SelectData(6);
      $dst_cmd    = $db->SelectData(7);
      $enable     = $db->SelectData(8);
      $ev_name    = $db->SelectData(9);
      $dst_param1 = $db->SelectData(10);
      $flag       = $db->SelectData(11);
      if($flag == "siempre")
      {
        $flag_condicion = 0;
        $flag_valor = "";
      }
      else
      {
        $flag_condicion = $db->SelectData(12);
        $flag_valor = $db->SelectData(13);
      }
      $dst_param2 = $db->SelectData(14);
    }
  }
  else
  {
    $id         = "";
    $src_dev    = "";
    $src_port   = "";
    $src_bit    = "";
    $src_on_off = "";
    $dst_group  = "";
    $dst_cmd    = "";
    $enable     = 0;
    $ev_name    = "";
    $dst_param1 = "";
    $dst_param2 = "";
    $flag       = "siempre";
    $flag_condicion = 0;
    $flag_valor = "";
  }
</script>
<body bgcolor="#F0F0F0">
<h1>Evento</h1>
<form name="frm_abm_event" method=POST action="abm_event.php">
<table border="0" width="90%" align="center" bgcolor="#F0F0F0">
<tr>
  <td>Origen</td><td>On/Off</td>
</tr>
<tr>
  <td>
  <select name="src" size="1">
    <option>&nbsp;</option>
    <?
    if($db->Select("id, port, bit, assign_name", // SELECT
                   "assign", // FROM
                   "substr(port,1,1) <> 'O'", // WHERE
                   "id, port, bit", // ORDER BY
                   "")) // GROUP BY
    {
      do
      {
        $ass_id     = $db->SelectData(1);
        $ass_port   = $db->SelectData(2);
        $ass_bit    = $db->SelectData(3);
        $ass_name   = $db->SelectData(4);
        if($id != "")
        {
          if($ass_id == $src_dev && $ass_port == $src_port && $ass_bit == $src_bit)
            printf("<option selected>%03d%2.2s%01d - %s</option>", $ass_id, $ass_port, $ass_bit, $ass_name);
          else
            printf("<option>%03d%2.2s%01d - %s</option>", $ass_id, $ass_port, $ass_bit, $ass_name);
        }
        else printf("<option>%03d%2.2s%01d - %s</option>", $ass_id, $ass_port, $ass_bit, $ass_name);
      } while($db->SelectNext());
    }
    ?>
    </select>
  </td>
  <td>
    <select name="src_on_off" size="1">
      <option>&nbsp;</option>
      <option <?if($src_on_off == 0)echo "selected";?> >Off</option>
      <option <?if($src_on_off == 1)echo "selected";?> >On</option>
    </select>
  </td>
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
</tr>
</table>
<table border="0" width="90%" align="center" bgcolor="#F0F0F0">
<tr>
  <td>Nombre</td>
</tr>
<tr>
  <td><input type=text name="ev_name" size="70%" maxlength="256" value="<?echo "$ev_name";?>"></td>
</tr>
<tr>
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
