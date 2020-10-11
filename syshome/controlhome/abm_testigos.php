<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::AdminEvent::ABMAssign")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Administracion de testigos</title>
</head>
<body bgcolor="#FFFFFF">
<h1 align=center>Administraci&oacute;n de testigos</h1>

<script language="php">
  $testigos_disponibles = array();
  $testigos_disponibles_key = array();
  $testigos_asignados = array();
  $testigos_asignados_key = array();

  $db = new PPgSqlDB;
  if($db->Open($dbserver, "home", $dbuser, $dbpass))
  {
    if(isset($add) && isset($testigos_disponibles_sel))
    {
      list($id_testigo, $port_testigo, $bit_testigo) = sscanf($testigos_disponibles_sel,"%d,%2s,%d");
      $db->Insert("testigo",
                  "id, port, bit, id_testigo, port_testigo, bit_testigo, valor_on, valor_off, invertir",
                  "$id, '$port', $bit, $id_testigo, '$port_testigo', $bit_testigo, 1, 0, 0");
    }
    else if(isset($del) && isset($testigos_asignados_sel))
    {
      list($id_testigo, $port_testigo, $bit_testigo) = sscanf($testigos_asignados_sel,"%d,%2s,%d");
      $db->Delete("testigo",
                  "id_testigo = $id_testigo and port_testigo = '$port_testigo' and bit_testigo = $bit_testigo");
    }
    // Listado de testigos disponibles
    if($db->Select("device.device_name,assign.assign_name,assign.id,assign.port,assign.bit", // SELECT
                   "device, assign", // FROM
                   "assign.id = device.id and device.hw_typ = 3 and substr(assign.port,1,1) = 'O'",  // WHERE
                   "", // ORDER BY
                   "")) // GROUP BY
    {
      do
      {
        $linea = sprintf("%s de %s", $db->SelectData(2), $db->SelectData(1));
        array_push($testigos_disponibles, $linea);
        $linea = sprintf("%s,%s,%s", $db->SelectData(3), $db->SelectData(4), $db->SelectData(5));
        array_push($testigos_disponibles_key, $linea);
      } while($db->SelectNext());
    }
    // Listado de testigos asignados
    if($db->Select("device.device_name,assign.assign_name,testigo.id_testigo,testigo.port_testigo,testigo.bit_testigo", // SELECT
                   "testigo, device, assign", // FROM
                   "testigo.id_testigo = assign.id and ".
                   "testigo.port_testigo = assign.port and ".
                   "testigo.bit_testigo = assign.bit and ".
                   "assign.id = device.id and ".
                   "testigo.id = $id and testigo.port = '$port' and testigo.bit = $bit",  // WHERE
                   "", // ORDER BY
                   "")) // GROUP BY
    {
      do
      {
        $linea = sprintf("%s de %s", $db->SelectData(2), $db->SelectData(1));
        array_push($testigos_asignados, $linea);
        $linea = sprintf("%s,%s,%s", $db->SelectData(3), $db->SelectData(4), $db->SelectData(5));
        array_push($testigos_asignados_key, $linea);
      } while($db->SelectNext());
    }
  }
  else return; // no se pudo abrir la base de datos
</script>

<form name="testigos_form" method=POST action="abm_testigos.php">
<table width="80%" align=center>
<tr bgcolor="#FFD0D0">
  <td width="40%" align="right"><b>Disponibles</b>&nbsp;</td>
  <td width="20%">&nbsp;</td>
  <td width="40%" align="left">&nbsp;<b>Asignados</b></td>
</tr>

<tr bgcolor="#D0D0FF">
  <td align="right" valign="top">
    <br />
    <select name="testigos_disponibles_sel" size="10">
    <?  
      for($i = 0; $i < count($testigos_disponibles); $i++)
      {
        if( !in_array($testigos_disponibles[$i], $testigos_asignados))
        {
          printf("<option value=\"%s\">%s&nbsp;&nbsp;</option>", $testigos_disponibles_key[$i], $testigos_disponibles[$i]);
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
    <input type=submit name="exit" value="Salir" onClick="javascript:window.close()">
    <br />
    <br />
  </td>

  <td align="left" valign="top">
    <br />
    &nbsp;
    <select name="testigos_asignados_sel" size="10">
    <? 
    for($i = 0; $i < count($testigos_asignados); $i++)
    {
      printf("<option value=\"%s\">%s&nbsp;&nbsp;</option>", $testigos_asignados_key[$i], $testigos_asignados[$i]);
    }
    ?>
    </select>
    <br />
    <br />
  </td>
<tr bgcolor="#FFD0D0"><td>&nbsp;</td><td>&nbsp;</td><td>&nbsp;</td></tr>
</table>
<input type=hidden name="id"   value="<? echo "$id"; ?>">
<input type=hidden name="port" value="<? echo "$port"; ?>">
<input type=hidden name="bit"  value="<? echo "$bit"; ?>">
</form>

</body>
</html>
