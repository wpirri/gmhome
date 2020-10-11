<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::AdminHW::Assign")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Assign</title>
  <script language="javascript">
    function closeWindow() { window.close(); }
  </script>
</head>

<script language="php">
  if( isset($new_address))
  {
    exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -n $new_address");
    ?>
      <script language="javascript">
        //parent.reloadWindow();
        window.close();
      </script>
    <?
  }
</script>
<body bgcolor="#FFFFFF">
<h1>Cambiar Direcci&oacute;n de dispositivo</h1>
<form name="frm_hw_assign" method=POST action="hw_assign.php">
<table border="0" width="90%" align="center">
<tr>
  <td>Direcci&oacute;n actual:</td><td><?echo "$device";?></td>
</tr>
<tr>
  <td>Direcci&oacute;n nueva:</td><td><input type=text name="new_address" size="10" maxlength="3" value="<?echo "$device";?>"></td>
</tr>
</table>
<table border="0" width="90%" align="center">
  <td align="right">
    <input type=submit name="actualizar"  value="Actualizar">&nbsp;&nbsp;
    <input type=submit name="cancelar" value="Cancelar" onClick="closeWindow()">
  </td>
</tr>
</table>
<input type=hidden name="device" value="<? echo "$device"; ?>">
</form>
</body>
</html>
