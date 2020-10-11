<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::UserAdmin::Access")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Administracion de accesos</title>
</head>
<body bgcolor="#FFFFFF">
<h1 align=center>Administraci&oacute;n de accesos</h1>

<script language="php">
  $accesos = array();
  $permisos = array();
  $db = new PPgSqlDB;
  if($db->Open($dbserver, "home", $dbuser, $dbpass))
  {
    // Administracion
    if( isset($exit))
    {
      ?>
      <script language="JavaScript">
        setTimeout('window.location.replace("user_admin.php")', 1);
      </script>
      <?
      return;
    }
    else if(isset($add) && isset($acceso_sel))
    {
      if($db->Select("user_name", "persona", "id = ".$id, "", ""))
      {
        $db->Insert("product_access",
                    "id, producto, usuario",
                    "'".$id."', '".$acceso_sel."', '".$db->SelectData(1)."'");
      }
    }
    else if(isset($del) && isset($permiso_sel))
    {
      $db->Delete("product_access",
                  "id = ".$id." AND producto = '".$permiso_sel."'");
    }
    // Listado
    // Listado de permisos posibles
    if($db->Select("acceso",        // SELECT
            "access_string",        // FROM
            "sistema = 'SysHome'",  // WHERE
            "acceso",               // ORDER BY
            ""))                    // GROUP BY
    {
      do
      {
        array_push($accesos, $db->SelectData(1));
      } while($db->SelectNext());
    }
    // Listado de permisos vigentes
    if($db->Select("producto",    // SELECT
            "product_access",     // FROM
            "id = '".$id."'",     // WHERE
            "",                   // ORDER BY
            ""))                  // GROUP BY
    {
      do
      {
        array_push($permisos, $db->SelectData(1));
      } while($db->SelectNext());
    }
    $db->Close();
  }
  else return;
</script>

<form name="access_form" method=POST action="user_access.php">
<table width="80%" align=center>
<tr bgcolor="#FFD0D0">
  <td width="40%" align="right"><b>No asignados</b>&nbsp;</td>
  <td width="20%">&nbsp;</td>
  <td width="40%" align="left">&nbsp;<b>Asignados</b></td>
</tr>

<tr bgcolor="#D0D0FF">
  <td align="right" valign="top">
    <br />
    <select name=acceso_sel size="10">
      <?  for($i = 0; $i < count($accesos); $i++)
          {
            if( !in_array($accesos[$i], $permisos))
            {
      ?>
              <option><? echo $accesos[$i]; ?></option>
      <?
            }
          }
      ?>
    </select>
    &nbsp;
    <br />
    <br />
  </td>

  <td align ="center" valign="bottom">
    <input type=submit name="add" value="Agregar >>"><br /><br />
    <input type=submit name="del" value="  << Quitar  "><br />
    <br /><br />
    <input type=submit name="exit" value="Salir">
    <br />
    <br />
  </td>

  <td align="left" valign="top">
    <br />
    &nbsp;
    <select name=permiso_sel size="10">
    <? for($i = 0; $i < count($permisos); $i++) { ?>
      <option><? echo $permisos[$i]; ?></option>
    <? } ?>
    </select>
    <br />
    <br />
  </td>
<tr bgcolor="#FFD0D0"><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>
</table>
<input type=hidden name="id" value="<? echo $id; ?>">
<input type=hidden name="item" value="<? echo $item; ?>">
</form>

</body>
</html>
