<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::UserAdmin::Detail")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Detalle de usuario</title>
</head>
<body bgcolor="#FFFFFF">
<h1 align=center>Administraci&oacute;n de usuarios</h1>

<script language="php">
  if( isset($item))
  {
    $db = new PPgSqlDB;
    if($db->Open($dbserver, "home", $dbuser, $dbpass))
    {
      if($db->Select( "id, user_name, password, ".
                      "nombre, apellido, dia, mes, anio, ".
                      "mail, postal, sms, telefono, ".
                      "pais_nacimiento, pais_radicacion",
              "persona",                                          // FROM
              "id = '".$id."'",                                   // WHERE
              "",                                                 // ORDER BY
              ""))                                                // GROUP BY
      {
        $id = $db->SelectData(1);
        $user_name = $db->SelectData(2);
        //$password = $db->SelectData(3);
        $nombre = $db->SelectData(4);
        $apellido = $db->SelectData(5);
        $dia = $db->SelectData(6);
        $mes = $db->SelectData(7);
        $anio = $db->SelectData(8);
        $mail = $db->SelectData(9);
        $postal = $db->SelectData(10);
        $sms = $db->SelectData(11);
        $telefono = $db->SelectData(12);
        $pais_nacimiento = $db->SelectData(13);
        $pais_radicacion = $db->SelectData(14);

        if($item == "editar")
        {
        ?>
          <form name="edit_form" method=POST action="user_admin.php">
          <table width="80%" bgcolor="#D0D0FF" align=center>
          <tr><td>&nbsp;</td><td>&nbsp;</td></tr>
          <tr><td>Nombre:</td>
              <td>
                  <input type=text name=nombre   value="<? echo $nombre; ?>"   size="20" maxlength="64">&nbsp;
                  <input type=text name=apellido value="<? echo $apellido; ?>" size="20" maxlength="32">
              </td>
          </tr>

          <tr><td>Nacimiento:</td>
            <td>
              <input type=text name=dia  value="<? echo $dia; ?>"  size="2" maxlength="2">/
              <input type=text name=mes  value="<? echo $mes; ?>"  size="2" maxlength="2">/
              <input type=text name=anio value="<? echo $anio; ?>" size="4" maxlength="4">
            </td>
          </tr>

          <tr><td>Contacto:</td>
            <td>
              mail: <input type=text name=mail     value="<? echo $mail; ?>"     size="30" maxlength="32">&nbsp;
              postal: <input type=text name=postal   value="<? echo $postal; ?>"   size="30" maxlength="64">
            </td>
          </tr>

          <tr><td>&nbsp;</td>
            <td>
              sms: <input type=text name=sms      value="<? echo $sms; ?>"      size="20" maxlength="32">&nbsp;
              tel: <input type=text name=telefono value="<? echo $telefono; ?>" size="20" maxlength="32">
            </td>
          </tr>


          <tr><td>Usuario:</td>
            <td>
              <input type=text name=user_name value="<? echo $user_name ?>" size="16" maxlength="16">
            </td>
          </tr>

          <tr><td>Clave:</td>
            <td>
              <input type=password name=password value="unchanged" size="16" maxlength="16">
            </td>
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
          <input type=hidden name=action value="update">
          </form>
        <?
        }
        else if($item == "borrar")
        {
          ?>
          <form name="delete_form" method=POST action="user_admin.php">
          <table width="80%" bgcolor="#D0D0FF" align=center>
          <tr><td>&nbsp;</td><td>&nbsp;</td></tr>
          <tr><td>Nombre:</td>
              <td>
                  <? echo $nombre; ?>&nbsp;
                  <? echo $apellido; ?>
              </td>
          </tr>

          <tr><td>Nacimiento:</td>
            <td>
              <? echo $dia; ?>/
              <? echo $mes; ?>/
              <? echo $anio; ?>
            </td>
          </tr>

          <tr><td>Contacto:</td>
            <td>
              mail: <? echo $mail; ?>&nbsp;
              postal: <? echo $postal; ?>
            </td>
          </tr>

          <tr><td>&nbsp;</td>
            <td>
              sms: <? echo $sms; ?>&nbsp;
              tel: <? echo $telefono; ?>
            </td>
          </tr>


          <tr><td>Usuario:</td>
            <td>
              <? echo $user_name ?>
            </td>
          </tr>

          <tr><td>Clave:</td>
            <td>
              ********
            </td>
          </tr>

          <tr>
            <td>&nbsp;</td>
            <td align="right">
              <input type=submit name="ok"     value="Eliminar"> &nbsp;
              <input type=submit name="cancel" value="Cancelar"> &nbsp;&nbsp;&nbsp;&nbsp;
            </td>
          </tr>
          <tr>
            <td>&nbsp;</td>
            <td>&nbsp;</td>
          </tr>
          </table>
          <input type=hidden name=id value="<? echo $id ?>">
          <input type=hidden name=action value="delete">
          </form>
          <?
        }
      }
      $db->Close();
    }
  }
</script>
</body>
</html>
