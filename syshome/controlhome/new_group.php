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

  $db = new PPgSqlDB;
  $db->Open($dbserver, "home", $dbuser, $dbpass);


</script>
<body bgcolor="#F0F0F0">
<h1>Grupo nuevo</h1>
<form name="frm_new_group" method=POST action="abm_group.php">
<table border="0" width="90%" align="center">
<tr>
  <td align="center"><input type=text name="group_name" value="" size="60%" maxlength="256"><td>
</tr>
</table>

<table border="0" width="90%" align="center">
<tr>
  <td align="right" bgcolor="#E0E0E0">
    <input type=submit name="agregar"  value="Agregar ">&nbsp;&nbsp;
    <input type=submit name="cancelar" value="Cancelar" onClick="closeWindow()">
  </td>
</tr>
</table>

</form>
</body>
</html>
<? $db->Close(); ?>
