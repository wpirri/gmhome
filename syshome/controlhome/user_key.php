<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::UserAdmin")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Cambio de clave</title>
</head>
<body bgcolor="#FFFFFF">
<h1 align=center>Cambio de clave de usuario</h1>

<script language="php">
  if(isset($cancel))
  {
	echo "<script language=\"JavaScript\">setTimeout('self.location.href=\"user_admin.php\"', 1);</script>";
	return;
  }
  if( isset($id))
  {
    $db = new PPgSqlDB;
    if($db->Open($dbserver, "home", $dbuser, $dbpass))
    {
      if($db->Select( "id, user_name, password, ".
                      "nombre, apellido",
              "persona",                                          // FROM
              "id = '".$id."'",                                   // WHERE
              "",                                                 // ORDER BY
              ""))                                                // GROUP BY
      {
        $id = $db->SelectData(1);
        $user_name = $db->SelectData(2);
        $password = $db->SelectData(3);
        $nombre = $db->SelectData(4);
        $apellido = $db->SelectData(5);

	/* Valido el usuario */
	if($user_name != $SESSION_USER) return;

	if(isset($ok))
	{ /* es un submit */
		$crypt_old_password = crypt($old_password, substr($password, 0, 2));
		if(!isset($old_password) || !isset($new_password_1) || !isset($new_password_2))
		{
			echo "<script language=\"JavaScript\">alert('Datos erroneos en cambio de clave');</script>";
			echo "<script language=\"JavaScript\">setTimeout('self.location.href=\"user_admin.php\"', 1);</script>";
			return;
		}
		if($crypt_old_password != $password)
		{
			echo "<script language=\"JavaScript\">alert('La clave declarada no coincide con la original');</script>";
			echo "<script language=\"JavaScript\">setTimeout('self.location.href=\"user_admin.php\"', 1);</script>";
			return;
		}
		if(strlen($new_password_1) < 8)
		{
			echo "<script language=\"JavaScript\">alert('La clave nueva debe tener por lo menos 8 caracteres de logitud');</script>";
			echo "<script language=\"JavaScript\">setTimeout('self.location.href=\"user_admin.php\"', 1);</script>";
			return;
		}
		if($new_password_1 != $new_password_2)
		{
			echo "<script language=\"JavaScript\">alert('La clave nueva no coincide con su repeticion');</script>";
			echo "<script language=\"JavaScript\">setTimeout('self.location.href=\"user_admin.php\"', 1);</script>";
			return;
		}
		if($new_password_1 == $old_password)
		{
			echo "<script language=\"JavaScript\">alert('No se puede realizar un cambio por la miama clave');</script>";
			echo "<script language=\"JavaScript\">setTimeout('self.location.href=\"user_admin.php\"', 1);</script>";
			return;
		}
		$crypt_new_password = crypt($new_password_1, substr($new_password_1, 0, 2));
		if( $db->Update("persona", "password = '".$crypt_new_password."'","id = $id") )
		{
			echo "<script language=\"JavaScript\">alert('Cambio de clave realizado correctamente');</script>";
		} else {
			echo "<script language=\"JavaScript\">alert('Ocurrio un error al cambiar la clave');</script>";
		}
		echo "<script language=\"JavaScript\">setTimeout('self.location.href=\"user_admin.php\"', 1);</script>";
		return;
	}
	?>
        <form name="edit_form" method=POST action="user_key.php">
        <table width="80%" bgcolor="#D0D0FF" align=center>
        <tr><td>&nbsp;</td><td>&nbsp;</td></tr>
        <tr>
            <td>Cambio de clave para:</td>
            <td><? echo "$nombre $apellido"; ?></td>
        </tr>
        <tr>
            <td>Clave actual:</td>
            <td><input type=password name=old_password value="" size="16" maxlength="16"></td>
        </tr>
        <tr>
            <td>Clave nueva:</td>
            <td><input type=password name=new_password_1 value="" size="16" maxlength="16"></td>
        </tr>
        <tr>
            <td>Repetir nueva:</td>
            <td><input type=password name=new_password_2 value="" size="16" maxlength="16"></td>
        </tr>
        <tr>
            <td>&nbsp;</td>
            <td align="right">
              <input type=submit name="ok"     value="Aceptar "> &nbsp;
              <input type=submit name="cancel" value="Cancelar"> &nbsp;&nbsp;&nbsp;&nbsp;
            </td>
        </tr>
        <tr>
            <td>&nbsp;</td>
            <td>&nbsp;</td>
        </tr>
        </table>
        <input type=hidden name=id value="<? echo $id ?>">
        </form>
	<?
        }
      }
      $db->Close();
    }
</script>
</body>
</html>
