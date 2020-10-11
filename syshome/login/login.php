<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
</script>

<script language="php">
  $SESSION_USER = "";
  $SESSION_DATA = "";

  if(empty($opcion) || $opcion != "Aceptar")
	{
    ?>
      </body>
      </html>
  		<script language="JavaScript">
    		setTimeout('window.location.replace("<? echo "$EXIT_PAGE"; ?>")', 1);
  		</script>
    <?
    return;
 	}
	?>
    <html>
    <head>
      <title>Login</title>
    </head>
    <body background="../images/bg_cesped.jpg">
      <table border="0" width="100%" height="100%">
      <tr><td>
      <center>
        <h1><font color="#FF0000"><b>Verificando ...</b></font></h1>
      </center>
      </td></tr>
      </table>
    </body>
    </html>
	<?
	if(empty($username) || empty($password))
	{
    ?>
  		<script language="JavaScript">
  		setTimeout('window.location.replace("index.php?login_error=1")', 1);
  		</script>
    <?
    return;
  }
  $db = new PPgSqlDB;
  if($db->Open($dbserver, "home", $dbuser, $dbpass))
  {
    if($db->Select("password",                // SELECT
            "persona",                        // FROM
            "user_name = '".$username."'",    // WHERE
            "",                               // ORDER BY
            ""))                              // GROUP BY
    {
      $crypt_pass = crypt($password, substr($db->SelectData(1), 0, 2));
      if($db->SelectData(1) == $crypt_pass)
      {
        // Autenticación OK
        $SESSION_USER = $username;
        $SESSION_DATA = string_acceso($username, $REMOTE_ADDR, $HTTP_USER_AGENT);
        ?>
      		<script language="JavaScript">
        		setTimeout('window.location.replace("<? echo "$INIT_PAGE"; ?>")', 1);
      		</script>
        <?
        return;
      }
    }
  }
  else
  {
    ?>
  		<script language="JavaScript">
    		alert('Error de conexion a la base de datos');
  		</script>
    <?
  }
  // si llego acá es porque falló algo
  ?>
		<script language="JavaScript">
		setTimeout('window.location.replace("index.php?login_error=2")', 1);
		</script>
  <?
  return;
</script>

