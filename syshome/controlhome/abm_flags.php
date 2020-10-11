<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::AdminEvent::ABMFlags")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Flag</title>
  <script language="javascript">
    function closeWindow() { window.close(); }
  </script>
</head>

<script language="php">
  $db = new PPgSqlDB;
  $db->Open($dbserver, "home", $dbuser, $dbpass);

  if( isset($actualizar))
  {
    if( !$db->Update("home_flag", 
                    "valor = '$valor', descripcion = '$descripcion'",
                    "nombre = '$nombre'"))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }
    ?>
      <script language="javascript">
        window.close();
      </script>
    <?
  }
  else if( isset($agregar))
  {
    if( !$db->Insert("home_flag", 
                    "nombre, valor, descripcion",
                    "'$nombre', '$valor', '$descripcion'"))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }
    ?>
      <script language="javascript">
        window.close();
      </script>
    <?
  }
  else if( isset($eliminar))
  {
    if( !$db->Delete("home_flag", "nombre = '$nombre'"))
    {
      ?>
        <script language="javascript">
          alert('<? printf("ERROR: %s", $db->LastError()); ?>');
        </script>
      <?
    }
    ?>
      <script language="javascript">
        window.close();
      </script>
    <?
  }
  else if( isset($nombre))
  {
    if($db->Select("nombre, valor, descripcion",         // SELECT
                    "home_flag",                         // FROM
                    "nombre = '$nombre'",                // WHERE
                    "",                                  // ORDER BY
                    ""))                                 // GROUP BY
    {
      $nombre =      $db->SelectData(1);
      $valor =       $db->SelectData(2);
      $descripcion = $db->SelectData(3);
    }
  }
  else
  {
    $nombre =      "";
    $valor =       "";
    $descripcion = "";
  }
  $db->Close();
</script>
<body  bgcolor="#F0F0F0">
<h1>Flag</h1>
<form name="frm_abm_flag" method=POST action="abm_flags.php">
<table border="0" width="90%" align="center">
<tr>
  <td>Nombre</td>
</tr>
<tr>
  <td>
    <? if($nombre != "") {?>
      <p><?echo "$nombre";?></p>
      <input type=hidden name="nombre" size=10 maxlength="32" value="<?echo "$nombre";?>">
    <? }else{ ?>
      <input type=text name="nombre" size=10 maxlength="32" value="">
    <? } ?>
  </td>
</tr>
<tr>
  <td>Valor</td>
</tr>
<tr>
  <td><input type=text name="valor" size="80" maxlength="255" value="<?echo "$valor";?>"></td>
</tr>
<tr>
  <td>Descripcion</td>
</tr>
<tr>
  <td><input type=text name="descripcion" size="80" maxlength="255" value="<?echo "$descripcion";?>"></td>
</tr>
</table>
<br />
<table border="0" width="90%" align="center">
  <td align="right" bgcolor="#E0E0E0">
    <? if($nombre != "") {?>
      <input type=submit name="actualizar"  value="Actualizar">&nbsp;&nbsp;
      <input type=submit name="eliminar"  value="Eliminar">&nbsp;&nbsp;
    <? }else{ ?>
      <input type=submit name="agregar"  value="Agregar ">&nbsp;&nbsp;
    <? } ?>
    <input type=submit name="cancelar" value="Cancelar" onClick="closeWindow()">
  </td>
</tr>
</table>
</form>
</body>
</html>
