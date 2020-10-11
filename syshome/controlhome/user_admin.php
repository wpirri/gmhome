<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::UserAdmin")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Administracion de usuarios</title>
</head>
<body bgcolor="#FFFFFF">
<table width="80%" align=center>
<td align=left><h1>Administraci&oacute;n de usuarios</h1></td>
<td align=right><a href="index.php?view=0" target=_top><img src="images/back.png" border="0"></a></td>
</table>

<table width="80%" bgcolor="#FFFFFF" align=center>
<tr>
  <td bgcolor="#FFD0D0"><b>Usuario</b></td>
  <td bgcolor="#FFD0D0"><b>Nombre</b></td>
  <td bgcolor="#FFD0D0" align="center"><b>&nbsp;</b></td>
</tr>

<script language="php">
  $count = 0;
  $db = new PPgSqlDB;
  if($db->Open($dbserver, "home", $dbuser, $dbpass))
  {
    // ADMIN
    if( isset($action) && isset($ok) && isset($id))
    {
      if($action == "update")
      {
        if( !$db->Update("persona",
                    "user_name = '".$user_name."', ".
                    "nombre = '".$nombre."', apellido = '".$apellido."', ".
                    "dia = '".$dia."', mes = '".$mes."', anio = '".$anio."', ".
                    "mail = '".$mail."', postal = '".$postal."', ".
                    "sms = '".$sms."', telefono = '".$telefono."'",
                    "id = $id"))
        {
          echo "<script language=\"JavaScript\">alert('Error al actualizar datos');</script>";
        }
        else if($password != "unchanged")
        {
          $crypt_pass = crypt($password, "MH");
          if( !$db->Update("persona",
                    "password = '".$crypt_pass."'",
                    "id = $id")) echo "<script language=\"JavaScript\">alert('Error al actualizar clave');</script>";
        }
      }
      if($action == "delete")
      {
        if( !$db->Update("persona",
                    "user_name = NULL, password = NULL",
                    "id = $id"))
        {
          echo "<script language=\"JavaScript\">alert('Error al eliminar usuario');</script>";
        }
      }
    }
    // LISTADO
    if($db->Select("id, user_name, nombre, apellido",           // SELECT
            "persona",                                          // FROM
            (isset($list))?"":"user_name IS NOT NULL",          // WHERE
            "user_name",                                        // ORDER BY
            ""))                                                // GROUP BY
    {
      do
      {
        $id = $db->SelectData(1);
        $user_name = $db->SelectData(2);
        $nombre = $db->SelectData(3);
        $apellido = $db->SelectData(4);
        if($count%2)
          $color="#D0D0FF";
        else
          $color="#D0FFD0";
        if(isset($list))
        {
          if( !strlen($user_name))
          {
        ?>
          <tr>
            <td bgcolor="<? echo $color; ?>"><? echo "$user_name"; ?></td>
            <td bgcolor="<? echo $color; ?>"><? echo "$nombre $apellido"; ?></td>
            <td bgcolor="<? echo $color; ?>" align="center">
              <a href="<? echo "user_detail.php?item=editar&id=$id"; ?>">
                <img src="images/mini-hoja-lapiz.gif" border="0">
              </a>
            </td>
          </tr>
        <?
          }
        }
        else
        {
        ?>
          <tr>
            <td bgcolor="<? echo $color; ?>"><? echo "$user_name"; ?></td>
            <td bgcolor="<? echo $color; ?>"><? echo "$nombre $apellido"; ?></td>
            <td bgcolor="<? echo $color; ?>" align="center">

            <? if($user_name != $ADMIN_USER) { ?>
              <a href="<? echo "user_access.php?item=permisos&id=$id"; ?>">
              <img src="images/mini-opciones.gif" border="0" title="Cambiar permisos">
              </a>&nbsp;
            <? } ?>

              <a href="<? echo "user_detail.php?item=editar&id=$id"; ?>">
              <img src="images/mini-propiedades.gif" border="0" title="Modificar datos del usuario">
              </a>&nbsp;

            <? if($user_name == $SESSION_USER || $SESSION_USER == $ADMIN_USER) { ?>
              <a href="<? echo "user_key.php?id=$id"; ?>">
              <img src="images/password-mini.png" border="0" title="Cambiar Clave del usuario">
              </a>&nbsp;
            <? } ?>

            <? if($user_name != $SESSION_USER && $user_name != $ADMIN_USER) { ?>
              <a href="<? echo "user_detail.php?item=borrar&id=$id"; ?>">
              <img src="images/mini-hoja-tachada.gif" border="0" title="Eliminar usuario">              
              </a>&nbsp;
            <? } ?>

            </td>
          </tr>
        <?
        }
        $count++;
      } while($db->SelectNext());
    }
    $db->Close();
  }
</script>

<tr>
  <td bgcolor="#FFD0D0">&nbsp;</td>
  <td bgcolor="#FFD0D0">&nbsp;</td>
  <? if(isset($list)) { ?>
    <td bgcolor="#FFD0D0" align="center"><a href="?">Volver</a></td>
  <? } else { ?>
    <td bgcolor="#FFD0D0" align="center"><a href="?list=all">
    <img src="images/mini-hoja-lapiz.gif" border="0">&nbsp;Agregar</a></td>
  <? } ?>
</tr>

</table>
</body>
</html>
