<script language="php">
  require('gmhome_config.php');
  require('gmhome_access_control.php');
  // si la peticion viene de la red local me saltro el login
	if( acceso_local($REMOTE_ADDR) ) 
	{ 
	?> 
    <script language="JavaScript">
      setTimeout('window.location.replace("<? echo $INIT_PAGE; ?>")', 1);
    </script>
	<? 
	} 
	else 
	{ 
	?> 
    <script language="JavaScript">
      setTimeout('window.location.replace("<? echo $LOGIN_PAGE; ?>")', 1);
    </script>
	<? 
	} 
</script>

