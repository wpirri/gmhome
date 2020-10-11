<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>

<html>
<head>
  <title>SysHome</title>
  <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
</head>
<body background="images/bg_cesped.jpg">
  <table border="0" width="100%" height="100%">
  <tr><td>
  <center>
    <h1><font color="#FF0000"><b>Iniciando ...</b></font></h1>
  </center>
  </td></tr>
  </table>
</body>
</html>
<!-- POR AHORA VAMOS DIRECTAMENTE A CONTROL HOME, NO HAY OTRA COSA -->
<script language="JavaScript">
  setTimeout('window.location.replace("controlhome/")', 1);
</script>

