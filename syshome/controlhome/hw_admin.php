<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::AdminHW")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Admnistración de dispositivos</title>
  <link title="new" rel="stylesheet" href="css/event_admin.css" type="text/css">
<script language="javascript" type="text/javascript">
  function reloadFrame() { self.location.reload(); }

  function OpenAssign( filename, param, title )
  {
    window.open(filename + param, title, 'tollbar=no,status=no,menubar=no,scrollbars=no,resizable=no,width=640,height=200');
  }

  function OpenABM( filename, param, title )
  {
    window.open(filename + param, title, 'tollbar=no,status=no,menubar=no,scrollbars=no,resizable=no,width=640,height=350');
  }
</script>
</head>

<script language="php">
$data = array();
$valid_data = false;
/* consulta de estados de todos los ports */
$cmd = "/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -l -e";
exec($cmd, $output);
if(count($output) > 0)
{
  $output = implode("", $output);
  $p = xml_parser_create();
  xml_parser_set_option($p, XML_OPTION_CASE_FOLDING, 0);
  xml_parser_set_option($p, XML_OPTION_SKIP_WHITE, 1);
  $rc = xml_parse_into_struct($p, $output, $vals, $index);
  xml_parser_free($p);
  if($rc == 1)
  {
    for( $i = 0; $i < count($vals);$i++ )
    {
      if($vals[$i]['type'] == "complete")
      {
        if($vals[$i]['tag'] == "address") $address = $vals[$i]['value'] + 0;
        else if($vals[$i]['tag'] == "status") $status = $vals[$i]['value'];
        else if($vals[$i]['tag'] == "hardware-typ") $hardware_typ = $vals[$i]['value'];
        else if($vals[$i]['tag'] == "hardware-ver") $hardware_ver = $vals[$i]['value'];
        else if($vals[$i]['tag'] == "software-ver") $software_ver = $vals[$i]['value'];

        else if($vals[$i]['tag'] == "processor") $processor = $vals[$i]['value'];
        else if($vals[$i]['tag'] == "display")
 
       {
          $display = $vals[$i]['value'];         

          $data[$address]['status'] = $status;
          $data[$address]['hardware-typ'] = $hardware_typ;
          $data[$address]['hardware-ver'] = $hardware_ver;
          $data[$address]['software-ver'] = $software_ver;
          $data[$address]['processor'] = $processor;
          $data[$address]['display'] = $display;
          $data[$address]['new-device'] = true;
          $valid_data = true;
        }
      }
    }
  }
}
</script>

<script language="php">
$db = new PPgSqlDB;
$db->Open($dbserver, "home", $dbuser, $dbpass);
</script>

<body bgcolor="#FFFFFF" onload="setTimeout('reloadFrame()',60000);">
<table width="80%" align=center>
<td align=left><h1>Administraci&oacute;n de dispositivos</h1></td>
<td align=right><a href="index.php?view=0" target=_top><img src="images/back.png" border="0"></a></td>
</table>

<table border="0" width="80%" bgcolor="#FFFFFF" align=center>
<tr>
<td bgcolor="#FFD0D0"><b>Direccion</b></td>
<td align="center" bgcolor="#FFD0D0"><b>Nombre</b></td>
<td align="center" bgcolor="#FFD0D0"><b>Stat</b></td>
<td bgcolor="#FFD0D0"><b>Tipo</b></td>
<td align="center" bgcolor="#FFD0D0"><b>HW</b></td>
<td align="center" bgcolor="#FFD0D0"><b>SW</b></td>
<td align="center" bgcolor="#FFD0D0"><b>Proc</b></td>
<td align="center" bgcolor="#FFD0D0"><b>Display</b></td>
<td align="center" bgcolor="#FFD0D0"><b>Editar</b></td>
</tr>
  <script language="php">
  // Listado de dispositivos
  $count = 0;
  if($db->Select("id, hw_typ, hw_ver, sw_ver, device_name",  // SELECT
                  "device",                                  // FROM
                  "",                                        // WHERE
                  "id",                                      // ORDER BY
                  ""))                                       // GROUP BY
  {
    do
    {
      $id =       $db->SelectData(1);
      $hw_typ =   $db->SelectData(2);
      $hw_ver =   $db->SelectData(3);
      $sw_ver =   $db->SelectData(4);
      $dev_name = $db->SelectData(5);

      $data[$id]['new-device'] = false;

      if($count%2)
        $color="#D0D0FF";
      else
        $color="#D0FFD0";
  </script>

        <tr>
          <!-- DIRECCION -->
          <td bgcolor="<? echo $color; ?>">
            <a class=tablelink href="javascript:OpenABM( 'abm_device.php', '?id=<? echo "$id"; ?>', 'Device' )">
              <? printf("%03d", $id); ?>
            </a>
          </td>

          <!-- NOMBRE -->
          <td bgcolor="<? echo $color; ?>">
            <a class=tablelink href="javascript:OpenABM( 'abm_device.php', '?id=<? echo "$id"; ?>', 'Device' )">
              <? echo "$dev_name"; ?>
            </a>
          </td>

          <!-- ESTADO -->
          <td align="center" bgcolor="<? echo $color; ?>">
            <a class=tablelink href="javascript:OpenABM( 'abm_device.php', '?id=<? echo "$id"; ?>', 'Device' )">
              <? printf("%s", ($data[$id]['status'] == 1)?"on":"off"); ?>
            </a>
          </td>

          <!-- TIPO -->
          <td bgcolor="<? echo $color; ?>">
            <a class=tablelink href="javascript:OpenABM( 'abm_device.php', '?id=<? echo "$id"; ?>', 'Device' )">
            <? 
              if($data[$id]['status'] == 1)
              {
                if($data[$id]['hardware-typ'] == 1)      echo "MIO-16x16";
                else if($data[$id]['hardware-typ'] == 2) echo "MIO-16x16/AD";
                else if($data[$id]['hardware-typ'] == 3) echo "MPAD-6";
                else                                     echo "nuevo";
              }
              else
              {
                if($hw_typ == 1)      echo "(MIO-16x16)";
                else if($hw_typ == 2) echo "(MIO-16x16/AD)";
                else if($hw_typ == 3) echo "(MPAD-6)";
                else                  echo "desconocido";
              }
            ?>
            </a>
          </td>

          <!-- VERSION HW -->
          <td align="center" bgcolor="<? echo $color; ?>">
            <a class=tablelink href="javascript:OpenABM( 'abm_device.php', '?id=<? echo "$id"; ?>', 'Device' )">
              <?
              if($data[$id]['status'] == 1)
              {
                printf("%d", $data[$id]['hardware-ver']);
              }
              else
              {
                echo "($hw_ver)";
              }
              ?>
            </a>
          </td>

          <!-- VERSION SW -->
          <td align="center" bgcolor="<? echo $color; ?>">
            <a class=tablelink href="javascript:OpenABM( 'abm_device.php', '?id=<? echo "$id"; ?>', 'Device' )">
              <?
              if($data[$id]['status'] == 1)
              {
                printf("%d", $data[$id]['software-ver']);
              }
              else
              {
                echo "($sw_ver)";
              }
              ?>
            </a>
          </td>

          <!-- PROCESADOR -->
          <td bgcolor="<? echo $color; ?>">
            <a class=tablelink href="javascript:OpenABM( 'abm_device.php', '?id=<? echo "$id"; ?>', 'Device' )">
              <?
              if($data[$id]['processor'] == 1) echo("16F877");
              else if($data[$id]['processor'] == 2) echo("16F877A");
              else if($data[$id]['processor'] == 3) echo("16F887");
              else if($data[$id]['processor'] == 17) echo("16F628");
              else if($data[$id]['processor'] == 18) echo("16F628A");
              ?>
            </a>
          </td>

          <!-- DISPLAY -->
          <td align="center" bgcolor="<? echo $color; ?>">
            <a class=tablelink href="javascript:OpenABM( 'abm_device.php', '?id=<? echo "$id"; ?>', 'Device' )">
              <?
              if($data[$id]['display'] == 40) echo("20x2");
              else if($data[$id]['display'] == 80) echo("20x4");
              ?>
            </a>
          </td>
          <td bgcolor="<? echo $color; ?>">
          <?if($data[$id]['status'] == 1) {?> 
            <a href="javascript:OpenAssign('hw_assign.php', '?device=<? echo "$id"; ?>', 'Asignacion de direccion');">
            <img src="images/mini-propiedades.gif" border="0" title="Cambiar direccion de hw"></a>
            <?if($data[$id]['hardware-typ'] == 1 || $data[$id]['hardware-typ'] == 2) {?>
            <a href="<? echo "hw_control.php?device=$id"; ?>">
            <img src="images/mini-opciones.gif" border="0" title="Ver / Modificar estado de salidas y entradas"></a>
            <? } ?>
          <? }else{ echo "&nbsp;"; }?>
          </td>

       </tr>

  <script language="php">
      $count++;
    } while($db->SelectNext());
  }

  /* listado de los dispositivos que no están en la base */

  for( $i = 0; $i < count($data);$i++ )
  {
    $datum = each($data);
    $key = $datum['key'];
    if($data[$key]['new-device'] == true && $data[$key]['status'] == 1)
    {
      $key = $datum['key'];


      if($count%2)
        $color="#D0D0FF";
      else
        $color="#D0FFD0";
  </script>

        <tr>
          <!-- DIRECCION -->
          <td bgcolor="<? echo $color; ?>">
            <? printf("%03d", $key); ?>
          </td>

          <!-- NOMBRE -->
          <td bgcolor="<? echo $color; ?>">
            <? echo "Dispositivo nuevo"; ?>
          </td>

          <!-- ESTADO -->
          <td align="center" bgcolor="<? echo $color; ?>">
            <? printf("%s", ($data[$key]['status'] == 1)?"on":"off"); ?>
          </td>

          <!-- TIPO -->
          <td bgcolor="<? echo $color; ?>">
            <? 
              if($data[$key]['hardware-typ'] == 1)      echo "MIO-16x16";
              else if($data[$key]['hardware-typ'] == 2) echo "MIO-16x16/AD";
              else if($data[$key]['hardware-typ'] == 3) echo "MPAD-6";
              else                                     echo "nuevo";
            ?>
          </td>

          <!-- VERSION HW -->
          <td align="center" bgcolor="<? echo $color; ?>">
            <?
              printf("%d", $data[$key]['hardware-ver']);
            ?>
          </td>

          <!-- VERSION SW -->
          <td align="center" bgcolor="<? echo $color; ?>">
            <?
              printf("%d", $data[$key]['software-ver']);
            ?>
          </td>

          <!-- PROCESADOR -->
          <td bgcolor="<? echo $color; ?>">
          <?
            if($data[$key]['processor'] == 1) echo("16F877");
            else if($data[$key]['processor'] == 2) echo("16F877A");
            else if($data[$key]['processor'] == 3) echo("16F887");
            else if($data[$key]['processor'] == 17) echo("16F628");
            else if($data[$key]['processor'] == 18) echo("16F628A");
          ?>
          </td>

          <!-- DISPLAY -->
          <td align="center" bgcolor="<? echo $color; ?>">
          <?
            if($data[$key]['display'] == 40) echo("20x2");
            else if($data[$key]['display'] == 80) echo("20x4");
          ?>
          </td>
          <td bgcolor="<? echo $color; ?>">
            <? if($key == 254) { ?>
            <a href="javascript:OpenAssign('hw_assign.php', '?device=254', 'Asignacion de direccion');">
            <img src="images/mini-propiedades.gif" border="0" title="Cambiar direccion de hw"></a>
            <? } else { ?>
            <a href="javascript:OpenABM( 'abm_device.php', '', 'Device' )">Agregar</a>
            <? } ?>
          </td>

       </tr>

  <script language="php">
      $count++;





    }
  }
  </script>
  <tr>
    <td bgcolor="#FFD0D0">&nbsp;</td>
    <td bgcolor="#FFD0D0">&nbsp;</td>
    <td bgcolor="#FFD0D0">&nbsp;</td>
    <td bgcolor="#FFD0D0">&nbsp;</td>
    <td bgcolor="#FFD0D0">&nbsp;</td>
    <td bgcolor="#FFD0D0">&nbsp;</td>
    <td bgcolor="#FFD0D0">&nbsp;</td>
    <td bgcolor="#FFD0D0">&nbsp;</td>
    <td bgcolor="#FFD0D0">&nbsp;</td>
<!--    <td bgcolor="#FFD0D0" align="center"><a href="javascript:OpenABM( 'abm_device.php', '', 'Device' )">Nuevo</a></td>-->
  </tr>
</table>

</body>
</html>

