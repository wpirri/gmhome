<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::AdminEvent::ABMGroup")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Group</title>
  <script language="javascript">
    function closeWindow() { window.close(); }
  </script>
</head>
<script language="php">
  $en_grupo = array();
  $salidas = array();
  $en_grupo_key = array();
  $salidas_key = array();

  $db = new PPgSqlDB;
  $db->Open($dbserver, "home", $dbuser, $dbpass);

  if(isset($agregar) && isset($group_name))
  {
    if( !$db->Insert("group_list",
                "group_name",
                "'".$group_name."'"))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }
    $id = $db->LastId();
  }
  else if(isset($add) && isset($salidas_sel))
  {
    $dev =  substr($salidas_sel, 0, 3);
    $port = substr($salidas_sel, 3, 2);
    $bit =  substr($salidas_sel, 5, 1);

    if( !$db->Insert("group_rel",
                "group_id, id, port, bit",
                $id.", ".$dev.", '".$port."', ".$bit))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }

  }
  else if(isset($del) && isset($en_grupo_sel))
  {
    $dev =  substr($en_grupo_sel, 0, 3);
    $port = substr($en_grupo_sel, 3, 2);
    $bit =  substr($en_grupo_sel, 5, 1);

    if( !$db->Delete("group_rel",
                "group_id = ".$id." AND id = ".$dev." AND port = '".$port."' AND bit = ".$bit))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }
  }
  else if( isset($eliminar))
  {
    if( !$db->Delete("group_rel", "group_id = $id"))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }
    if( !$db->Delete("group_list", "id = $id"))
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
  else if( isset($actualizar))
  {
    if( !$db->Update("group_list", 
                "group_name = '$group_name'",
                "id = $id"))
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

  if( isset($id))
  {
    if($db->Select("id, group_name", // SELECT
                    "group_list", // FROM
                    "id = $id", // WHERE
                    "", // ORDER BY
                    "" // GROUP BY
                  ))
    {
      $id =         $db->SelectData(1);
      $group_name = $db->SelectData(2);
    }
    if($db->Select("assign.id, assign.port, assign.bit, assign.assign_name", // SELECT
                   "assign, group_rel", // FROM
                   "group_rel.group_id = $id AND assign.id = group_rel.id AND assign.port = group_rel.port AND assign.bit = group_rel.bit", // WHERE
                   "assign.id, assign.port, assign.bit", // ORDER BY
                   "")) // GROUP BY
    {
      do
      {
        $ass_id     = $db->SelectData(1);
        $ass_port   = $db->SelectData(2);
        $ass_bit    = $db->SelectData(3);
        $ass_name   = $db->SelectData(4);

        array_push($en_grupo, sprintf("%.50s", $ass_name));
        array_push($en_grupo_key, sprintf("%03d%2.2s%01d", $ass_id, $ass_port, $ass_bit));

      } while($db->SelectNext());
    }
  }
  else
  {
    ?>
    <script language="javascript">
      window.close();
    </script>
    <?
  }

  if($db->Select("id, port, bit, assign_name", // SELECT
                 "assign", // FROM
                 "substr(port, 1,1) = 'O'", // WHERE
                 "id, port, bit", // ORDER BY
                 "")) // GROUP BY
  {
    do
    {
      $ass_id     = $db->SelectData(1);
      $ass_port   = $db->SelectData(2);
      $ass_bit    = $db->SelectData(3);
      $ass_name   = $db->SelectData(4);

      array_push($salidas, sprintf("%.50s", $ass_name));
      array_push($salidas_key, sprintf("%03d%2.2s%01d", $ass_id, $ass_port, $ass_bit));

    } while($db->SelectNext());
  }

</script>
<body bgcolor="#F0F0F0">
<h1>Grupo</h1>
<form name="frm_abm_group" method=POST action="abm_group.php">
<table border="0" width="90%" align="center">
<tr>
  <td align="center"><input type=text name="group_name" value="<? echo "$group_name"; ?>" size="60%" maxlength="256"><td>
</tr>
</table>
<table border="0" width="90%" align="center">
<tr bgcolor="#FFD0D0">
  <td width="40%" align="right"><b>No asignados</b>&nbsp;</td>
  <td width="20%">&nbsp;</td>
  <td width="40%" align="left">&nbsp;<b>Asignados</b></td>
</tr>

<tr bgcolor="#D0D0FF">
  <td align="right" valign="top">
    <br />
    <select name=salidas_sel size="10">
      <?  for($i = 0; $i < count($salidas); $i++)
          {
            if( !in_array($salidas_key[$i], $en_grupo_key))
            {
              printf("<option value=\"%s\">%s</option>", $salidas_key[$i], $salidas[$i]);
            }
          }
      ?>
    </select>
    &nbsp;
    <br />
    <br />
  </td>

  <td align ="center" valign="top">
    <br />
    <input type=submit name="add" value="Agregar >>"><br /><br />
    <input type=submit name="del" value="  << Quitar  "><br />
  </td>

  <td align="left" valign="top">
    <br />
    <select name=en_grupo_sel size="10">
    <?
      for($i = 0; $i < count($en_grupo); $i++)
      {
        printf("<option value=\"%s\">%s</option>", $en_grupo_key[$i], $en_grupo[$i]);
      } 
    ?>
    </select>
    <br />
    <br />
  </td>
<tr bgcolor="#FFD0D0"><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>
</table>

<table border="0" width="90%" align="center">
<tr>
  <td align="right" bgcolor="#E0E0E0">
    <input type=submit name="actualizar"  value="Actualizar">&nbsp;&nbsp;
    <input type=submit name="eliminar"  value="Eliminar">&nbsp;&nbsp;&nbsp;&nbsp;
    <input type=submit name="cancelar" value="Salir" onClick="closeWindow()">
  </td>
</tr>
</table>

<input type=hidden name="id"   value="<? echo "$id"; ?>">
</form>
</body>
</html>
<? $db->Close(); ?>
