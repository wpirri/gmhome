<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
</script>

<html>
<head>
<title>Login</title>
<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">
</head>

<body bgcolor="#000000" text="#0000A0" link="#0000FF" vlink="#0000FF" alink="#0000FF" background="../images/bg_cesped.jpg">
<form method=POST name="login" action="login.php" >
<table width="90%" height="90%" border="0">
<tr><td>
<table width="100%" border="0" align="center" vspace="5" hspace="5">
  <tr>
    <td width="67%">
        <p align="center"><b><font size="+3">
          <i>Acceso al sistema SysHome.</i>
        </font></b></p>
    </td>
    <td align="center">
      <table width="300" border="2" bordercolor="#666666" bgcolor="#999999">
        <tr bgcolor="#0000CC">
          <td><p><img src="../images/mini_home.gif" width="16" height="16">
	        <font color="#FFFFFF"><b> Login</b></font>
		<script language="php">
      $SESSION_USER = "guest";
      $SESSION_DATA = "invalid data";
      if(!empty($login_error))
			{
			?>
			        <font color="#FF0000"><b>Error Usuario/Clave</b></font>
			<?
			}
		</script>
          </p></td>
        </tr>
        <tr bgcolor="#CCCCCC">

	<!-- dialogo de logon - inicio -->

            <table width="300" border="1" bgcolor="#999999">
		<tr><td>
		<table width="100%" border="0">
              	<tr>
			<td width="62%">
				<br />
				&nbsp;<font color="#000000">Usuario </font><br />
				&nbsp;<input type="text" name="username" size="20"><br />
				&nbsp;<font color="#000000">Clave </font><br />			
				&nbsp;<input type="password" name="password" size="20"><br />
				<br />
			</td>
			<td align="right">
				<br />
				<input type="submit" name="opcion" value="Aceptar">&nbsp;<br />
				<br />
				<input type="submit" name="opcion" value="Cancelar">&nbsp;<br />
			</td>
		</tr>
            	</table>
		</td></tr>
            </table>

	<!-- dialogo de logon -  fin -->

          </td>
        </tr>
      </table>
    </td>
  </tr>
</table>
</td></tr>
</table>
</body>
<script language="JavaScript">
        document.login.username.focus();
</script>

</html>
