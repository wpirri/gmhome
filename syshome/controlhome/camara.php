<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::ViewCameras")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
  if( !isset($camara)) $camara="2";
</script>
<html>
<head>
  <title>GmHome::Camaras</title>
  <script language="javascript" type="text/javascript">
  <!--
  function reloadFrame() {
          self.location.reload();
  }
  -->
  </script>
</head>
<body bgcolor="#0000A0" onload="setTimeout('reloadFrame()',1000);">
<img src="/cgi-bin/nph-zms?mode=single&monitor=<? echo $camara; ?>&scale=100" border ="0">
</body>
</html>
