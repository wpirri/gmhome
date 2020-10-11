<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::AdminEvent::ABMDevice")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Dispositivo</title>
  <script language="javascript">
    function closeWindow() { window.close(); }
  </script>
</head>

<script language="php">
  $db = new PPgSqlDB;
  $db->Open($dbserver, "home", $dbuser, $dbpass);

  if(isset($hw_typ))
  {
    if($hw_typ == "MIO-16x16")         $hw_typ = 1;
    else if($hw_typ == "MIO-16x16/AD") $hw_typ = 2;
    else if($hw_typ == "MPAD-6")      $hw_typ = 3;
  }

  if( isset($actualizar))
  {
    if( !$db->Update("device", 
                    "id = $id, hw_typ = $hw_typ, hw_ver = $hw_ver, sw_ver = $sw_ver, device_name = '$dev_name'",
                    "id = $bkp_id"))
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
    if( !$db->Insert("device", 
                    "id, hw_typ, hw_ver, sw_ver, device_name",
                    "$id, $hw_typ, $hw_ver, $sw_ver, '$dev_name'"))
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
    $db->Delete("devconf", "id = $bkp_id");
    if( !$db->Delete("device", "id = $bkp_id"))
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
    if($db->Select("id, hw_typ, hw_ver, sw_ver, device_name",         // SELECT
                    "device",                                  // FROM
                    "id = $id",                                // WHERE
                    "",                                        // ORDER BY
                    ""))                                       // GROUP BY
    {
      $id =       $db->SelectData(1);
      $hw_typ =   $db->SelectData(2);
      $hw_ver =   $db->SelectData(3);
      $sw_ver =   $db->SelectData(4);
      $dev_name = $db->SelectData(5);
    }
  }
  else
  {
    $id =       "";
    $hw_typ =   "";
    $hw_ver =   "";
    $sw_ver =   "";
    $dev_name = "";
  }
  $db->Close();
</script>
<body  bgcolor="#F0F0F0">
<h1>Dispositivo</h1>
<form name="frm_abm_device" method=POST action="abm_device.php">
<table border="0" width="90%" align="center">
<tr>
  <td>Direcci&oacute;n</td><td>Tipo de Hardware</td>
</tr>
<tr>
  <td><input type=text name="id" size=10 maxlength="3" value="<?echo "$id";?>"></td>
  <td>
    <select name="hw_typ" size="1">
      <option>&nbsp;</option>
      <option <?if($hw_typ == 1)echo "selected";?> >MIO-16x16</option>
      <option <?if($hw_typ == 2)echo "selected";?> >MIO-16x16/AD</option>
      <option <?if($hw_typ == 3)echo "selected";?> >MPAD-6</option>
    </select>
  </td>
</tr>
<tr>
  <td>Versi&oacute;n de hardware</td><td>Versi&oacute;n de software</td>
</tr>
<tr>
  <td><input type=text name="hw_ver" size="10" maxlength="3" value="<?echo "$hw_ver";?>"></td>
  <td><input type=text name="sw_ver" size="10" maxlength="3" value="<?echo "$sw_ver";?>"></td>
</tr>
</table>
<table border="0" width="90%" align="center">
<tr>
  <td>Nombre</td>
</tr>
<tr>
  <td><input type=text name="dev_name" size="70%" maxlength="256" value="<?echo "$dev_name";?>"></td>
</tr>
<tr>
</table>
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
<input type=hidden name="bkp_id" value="<? echo "$id"; ?>">
</form>
</body>
</html>
