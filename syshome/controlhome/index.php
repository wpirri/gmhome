<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Control Home</title>
</head>
<frameset rows="*,1">
  <frame name="main" src="main.php<?if(isset($view)){echo "?view=$view";}?>" scrolling="yes" noresize frameborder="no">
  <frame name="control" src="control.php" scrolling="no" noresize frameborder="no">
</frameset>
<noframes>
<p>Su navegador no soporta frames</p>
</noframes
</frameset>
</html>
