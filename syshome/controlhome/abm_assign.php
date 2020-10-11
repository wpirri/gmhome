<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::AdminEvent::ABMAssign")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Assign</title>
  <script language="javascript">
    function closeWindow() { window.close(); }
  </script>
</head>

<script language="javascript" type="text/javascript">
  function OpenABM( filename, param, title )
  {
    window.open(filename + '?' + param, title, 'tollbar=no,status=no,menubar=no,scrollbars=no,resizable=no,width=800,height=450');
  }
</script>

<script language="php">
  $db = new PPgSqlDB;
  $db->Open($dbserver, "home", $dbuser, $dbpass);

  if( isset($port))
  {
    if($port ==      "Entrada 1")  $port = "I1";
    else if($port == "Entrada 2")  $port = "I2";
    else if($port == "Entrada 3")  $port = "I3";
    else if($port == "Entrada 4")  $port = "I4";
    else if($port == "Entrada 5")  $port = "I5";
    else if($port == "Entrada 6")  $port = "I6";
    else if($port == "Salida 1")   $port = "O1";
    else if($port == "Salida 2")   $port = "O2";
    else if($port == "Salida 3")   $port = "O3";
    else if($port == "Salida 4")   $port = "O4";
    else if($port == "Salida 5")   $port = "O5";
    else if($port == "Salida 6")   $port = "O6";
    else if($port == "Analog 1")   $port = "A1";
    else if($port == "Analog 2")   $port = "A2";
    else if($port == "Analog 3")   $port = "A3";
    else if($port == "Analog 4")   $port = "A4";
    else if($port == "Analog 5")   $port = "A5";
    else if($port == "Analog 6")   $port = "A6";
  }

  if( isset($dev_name)) $id = substr($dev_name, 0, 3);

  if( isset($actualizar))
  {
    if( !$db->Update("assign", 
                    "id = $id, port = '$port', bit = $bit, assign_name = '$ass_name'",
                    "id = $bkp_id AND port = '$bkp_port' AND bit = $bkp_bit"))
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
    if( !$db->Insert("assign", 
                  "id, port, bit, assign_name",
                  "$id, '$port', $bit, '$ass_name'"))
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
    if( !$db->Delete("assign", "id = $bkp_id AND port = '$bkp_port' AND bit = $bkp_bit"))
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
  else if( isset($id) && isset($port) && isset($bit))
  {
    if($db->Select("id, port, bit, assign_name", // SELECT
                    "assign", // FROM
                    "id = $id AND port = '$port' AND bit = $bit", // WHERE
                    "id, port, bit", // ORDER BY
                    "" // GROUP BY
                  ))
    {
      $id =       $db->SelectData(1);
      $port =     $db->SelectData(2);
      $bit =      $db->SelectData(3);
      $ass_name = $db->SelectData(4);
    }
  }
  else
  {
    $id =       "";
    $port =     "";
    $bit =      "";
    $ass_name =  "";
  }
</script>
<body bgcolor="#F0F0F0">
<h1>Asignaci&oacute;n</h1>
<form name="frm_abm_assign" method=POST action="abm_assign.php">
<table border="0" width="90%" align="center">
<tr>
  <td>Dispositivo</td><td>Puerto</td><td>E/S</td>
</tr>
<tr>
  <!--<td><input type=text name="id" size=10 maxlength="3" value="<?echo "$id";?>"></td>-->
  <td>
  <select name="dev_name" size="1">
    <option>&nbsp;</option>
    <?
      if($db->Select("id, device_name",  // SELECT
                      "device", // FROM
                      "", // WHERE
                      "", // ORDER BY
                      "")) // GROUP BY
      {
        do
        {
          $dev_id =   $db->SelectData(1);
          $dev_name = $db->SelectData(2);
          if($dev_id == $id)
            printf("<option selected>%03d - %s</option>", $dev_id, $dev_name);
          else
            printf("<option >%03d - %s</option>", $dev_id, $dev_name);
        } while($db->SelectNext());
      }
    ?>
  </select>
  </td>
  <td>
    <select name="port" size="1">
      <option>&nbsp;</option>
      <option <?if($port == "I1")echo "selected";?> >Entrada 1</option>
      <option <?if($port == "I2")echo "selected";?> >Entrada 2</option>
      <option <?if($port == "I3")echo "selected";?> >Entrada 3</option>
      <option <?if($port == "I4")echo "selected";?> >Entrada 4</option>
      <option <?if($port == "I5")echo "selected";?> >Entrada 5</option>
      <option <?if($port == "I6")echo "selected";?> >Entrada 6</option>
      <option <?if($port == "O1")echo "selected";?> >Salida 1</option>
      <option <?if($port == "O2")echo "selected";?> >Salida 2</option>
      <option <?if($port == "O3")echo "selected";?> >Salida 3</option>
      <option <?if($port == "O4")echo "selected";?> >Salida 4</option>
      <option <?if($port == "O5")echo "selected";?> >Salida 5</option>
      <option <?if($port == "O6")echo "selected";?> >Salida 6</option>
      <option <?if($port == "A1")echo "selected";?> >Analog 1</option>
      <option <?if($port == "A2")echo "selected";?> >Analog 2</option>
      <option <?if($port == "A3")echo "selected";?> >Analog 3</option>
      <option <?if($port == "A4")echo "selected";?> >Analog 4</option>
      <option <?if($port == "A5")echo "selected";?> >Analog 5</option>
      <option <?if($port == "A6")echo "selected";?> >Analog 6</option>
    </select>
  </td>
  <td>
    <select name="bit" size="1">
      <option>&nbsp;</option>
      <option <?if($bit == 0)echo "selected";?> >0</option>
      <option <?if($bit == 1)echo "selected";?> >1</option>
      <option <?if($bit == 2)echo "selected";?> >2</option>
      <option <?if($bit == 3)echo "selected";?> >3</option>
      <option <?if($bit == 4)echo "selected";?> >4</option>
      <option <?if($bit == 5)echo "selected";?> >5</option>
      <option <?if($bit == 6)echo "selected";?> >6</option>
      <option <?if($bit == 7)echo "selected";?> >7</option>
    </select>
  </td>
</tr>
</table>
<table border="0" width="90%" align="center">
<tr>
  <td>Nombre</td>
</tr>
<tr>
  <td><input type=text name="ass_name" size="70%" maxlength="256" value="<?echo "$ass_name";?>"></td>
</tr>
<tr>
</table>
<br />
<table border="0" width="90%" align="center">
<tr>
  <td>Testigo/s:&nbsp;
    <?
    if($id != "")
    {
      if($db->Select("device.device_name,assign.assign_name", // SELECT
                     "testigo, device, assign", // FROM
                     "testigo.id_testigo = assign.id and ".
                     "testigo.port_testigo = assign.port and ".
                     "testigo.bit_testigo = assign.bit and ".
                     "assign.id = device.id and ".
                     "testigo.id = $id and testigo.port = '$port' and testigo.bit = $bit",  // WHERE
                     "", // ORDER BY
                     "")) // GROUP BY
      {
        $d1 = $db->SelectData(1);
        $d2 = $db->SelectData(2);
        printf("%s de %s", $d2, $d1);
        while($db->SelectNext())
        {
          printf(", %s de %s", $d2, $d1);
        }
      }
    }
    ?>
  </td>
  <td align="right" valign="center" size="10">
    <?
    if($id != "") { ?>
    <input type=submit name="modificar" value="Modificar" onClick="javascript:OpenABM('abm_testigos', 'id=<? echo "$id"; ?>&port=<? echo "$port"; ?>&bit=<? echo "$bit"; ?>')">
    <? } ?>
  </td>
</tr>
</table>

<br />
<br />
<br />
<table border="0" width="90%" align="center">
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
<input type=hidden name="bkp_id"   value="<? echo "$id"; ?>">
<input type=hidden name="bkp_port" value="<? echo "$port"; ?>">
<input type=hidden name="bkp_bit"  value="<? echo "$bit"; ?>">
</form>
</body>
</html>
<? $db->Close(); ?>
