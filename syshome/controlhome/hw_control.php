<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::AdminHW::Control")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);

  if(! isset($device)) return;

</script>
<html>
<head>
  <title>GmHome::Administrador de dispositivos</title>
<script language="javascript" type="text/javascript">
  var bit_status = new Array(2);
  bit_status[0] = new Image();
  bit_status[0].src = "images/red.gif";
  bit_status[1] = new Image();
  bit_status[1].src = "images/green.gif";

  function reloadFrame() { self.location.href="?device=<?echo $device;?>"; }
</script>
</head>
  <script language="php">
  $db = new PPgSqlDB;
  $db->Open($dbserver, "home", $dbuser, $dbpass);

  if(isset($device) && isset($port) && isset($bit) && isset($value))
  {
    if(control_permisos($SESSION_USER, "SysHome::ControlHome", $REMOTE_ADDR))
    {
      $cmd = "/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -h $homeserver -P $homeport -d $device -i -p $port -b $bit";
      exec($cmd, $output);
    }
    else
    {
      echo "<script language=\"JavaScript\">alert('Permisos insuficientes');</script>";
    }
  }
  if(isset($actualizar))
  {
    $Mascara_E1S1 = 0;
    if(isset($mascara1bit0)) $Mascara_E1S1 += pow(2,0);
    if(isset($mascara1bit1)) $Mascara_E1S1 += pow(2,1);
    if(isset($mascara1bit2)) $Mascara_E1S1 += pow(2,2);
    if(isset($mascara1bit3)) $Mascara_E1S1 += pow(2,3);
    if(isset($mascara1bit4)) $Mascara_E1S1 += pow(2,4);
    if(isset($mascara1bit5)) $Mascara_E1S1 += pow(2,5);
    if(isset($mascara1bit6)) $Mascara_E1S1 += pow(2,6);
    if(isset($mascara1bit7)) $Mascara_E1S1 += pow(2,7);
    $Mascara_E2S2 = 0;
    if(isset($mascara2bit0)) $Mascara_E2S2 += pow(2,0);
    if(isset($mascara2bit1)) $Mascara_E2S2 += pow(2,1);
    if(isset($mascara2bit2)) $Mascara_E2S2 += pow(2,2);
    if(isset($mascara2bit3)) $Mascara_E2S2 += pow(2,3);
    if(isset($mascara2bit4)) $Mascara_E2S2 += pow(2,4);
    if(isset($mascara2bit5)) $Mascara_E2S2 += pow(2,5);
    if(isset($mascara2bit6)) $Mascara_E2S2 += pow(2,6);
    if(isset($mascara2bit7)) $Mascara_E2S2 += pow(2,7);
    $Mascara_ANS2 = 0;
    if(isset($mascara3bit0)) $Mascara_ANS2 += pow(2,0);
    if(isset($mascara3bit1)) $Mascara_ANS2 += pow(2,1);
    if(isset($mascara3bit2)) $Mascara_ANS2 += pow(2,2);
    if(isset($mascara3bit3)) $Mascara_ANS2 += pow(2,3);
    if(isset($mascara3bit4)) $Mascara_ANS2 += pow(2,4);
    if(isset($mascara3bit5)) $Mascara_ANS2 += pow(2,5);
    if(isset($mascara3bit6)) $Mascara_ANS2 += pow(2,6);
    if(isset($mascara3bit7)) $Mascara_ANS2 += pow(2,7);
    $info_e1 = 0;
    if(isset($notife1bit0)) $info_e1 += pow(2,0);
    if(isset($notife1bit1)) $info_e1 += pow(2,1);
    if(isset($notife1bit2)) $info_e1 += pow(2,2);
    if(isset($notife1bit3)) $info_e1 += pow(2,3);
    if(isset($notife1bit4)) $info_e1 += pow(2,4);
    if(isset($notife1bit5)) $info_e1 += pow(2,5);
    if(isset($notife1bit6)) $info_e1 += pow(2,6);
    if(isset($notife1bit7)) $info_e1 += pow(2,7);
    $info_e2 = 0;
    if(isset($notife2bit0)) $info_e2 += pow(2,0);
    if(isset($notife2bit1)) $info_e2 += pow(2,1);
    if(isset($notife2bit2)) $info_e2 += pow(2,2);
    if(isset($notife2bit3)) $info_e2 += pow(2,3);
    if(isset($notife2bit4)) $info_e2 += pow(2,4);
    if(isset($notife2bit5)) $info_e2 += pow(2,5);
    if(isset($notife2bit6)) $info_e2 += pow(2,6);
    if(isset($notife2bit7)) $info_e2 += pow(2,7);
    $info_s1 = 0;
    if(isset($notifs1bit0)) $info_s1 += pow(2,0);
    if(isset($notifs1bit1)) $info_s1 += pow(2,1);
    if(isset($notifs1bit2)) $info_s1 += pow(2,2);
    if(isset($notifs1bit3)) $info_s1 += pow(2,3);
    if(isset($notifs1bit4)) $info_s1 += pow(2,4);
    if(isset($notifs1bit5)) $info_s1 += pow(2,5);
    if(isset($notifs1bit6)) $info_s1 += pow(2,6);
    if(isset($notifs1bit7)) $info_s1 += pow(2,7);
    $info_s2 = 0;
    if(isset($notifs2bit0)) $info_s2 += pow(2,0);
    if(isset($notifs2bit1)) $info_s2 += pow(2,1);
    if(isset($notifs2bit2)) $info_s2 += pow(2,2);
    if(isset($notifs2bit3)) $info_s2 += pow(2,3);
    if(isset($notifs2bit4)) $info_s2 += pow(2,4);
    if(isset($notifs2bit5)) $info_s2 += pow(2,5);
    if(isset($notifs2bit6)) $info_s2 += pow(2,6);
    if(isset($notifs2bit7)) $info_s2 += pow(2,7);
    $info_an = 0;
    if(isset($notifanbit0)) $info_an += pow(2,0);
    if(isset($notifanbit1)) $info_an += pow(2,1);
    if(isset($notifanbit2)) $info_an += pow(2,2);
    if(isset($notifanbit3)) $info_an += pow(2,3);
    if(isset($notifanbit4)) $info_an += pow(2,4);
    if(isset($notifanbit5)) $info_an += pow(2,5);
    if(isset($notifanbit6)) $info_an += pow(2,6);
    if(isset($notifanbit7)) $info_an += pow(2,7);

    $analog1_max += 0;
    $analog1_min += 0;
    $analog2_max += 0;
    $analog2_min += 0;
    $analog3_max += 0;
    $analog3_min += 0;
    $analog4_max += 0;
    $analog4_min += 0;

    if(control_permisos($SESSION_USER, "SysHome::ControlHome", $REMOTE_ADDR))
    {
      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -p 1 -f ".$Mascara_E1S1);
      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -p 2 -f ".$Mascara_E2S2);
      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -p 10 -f ".$Mascara_ANS2);

      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -p 1 -o ".$info_e1);
      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -p 2 -o ".$info_e2);
      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -p 3 -o ".$info_s1);
      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -p 4 -o ".$info_s2);

      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -p 10 -o ".$info_an);

      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -p 1 -a ".$analog1_max." ".$analog1_min);
      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -p 2 -a ".$analog2_max." ".$analog2_min);
      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -p 3 -a ".$analog3_max." ".$analog3_min);
      exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $device -p 4 -a ".$analog4_max." ".$analog4_min);

      if($mode == "update")
      {
        $db->Update("devconf", 
                    "Mascara_E1S1 = $Mascara_E1S1, Mascara_E2S2 = $Mascara_E2S2, ".
                    "Mascara_ANS2 = $Mascara_ANS2, ".
                    "info_e1 = $info_e1, info_e2 = $info_e2, ".
                    "info_s1 = $info_s1, info_s2 = $info_s2, ".
                    "info_an = $info_an, ".
                    "ANALOG1_MAX = $analog1_max, ANALOG1_MIN = $analog1_min, ".
                    "ANALOG2_MAX = $analog2_max, ANALOG2_MIN = $analog2_min, ".
                    "ANALOG3_MAX = $analog3_max, ANALOG3_MIN = $analog3_min, ".
                    "ANALOG4_MAX = $analog4_max, ANALOG4_MIN = $analog4_min",
                    "id = $device");
      }
      else if($mode == "insert")
      {
        $db->Insert( "devconf",
                     "id, Mascara_E1S1, Mascara_E2S2, Mascara_ANS2, ".
                     "info_e1, info_e2, info_s1, info_s2, info_an, ".
                     "ANALOG1_MAX, ANALOG1_MIN, ".
                     "ANALOG2_MAX, ANALOG2_MIN, ".
                     "ANALOG3_MAX, ANALOG3_MIN, ".
                     "ANALOG4_MAX, ANALOG4_MIN",                         
                     "$device, $Mascara_E1S1, $Mascara_E2S2, , $Mascara_ANS2, ".
                     "$info_e1, $info_e2, $info_s1, $info_s2, $info_an, ".
                     "$analog1_max, $analog1_min, ".
                     "$analog2_max, $analog2_min, ".
                     "$analog3_max, $analog3_min, ".
                     "$analog4_max, $analog4_min" );
      }
    }
    else
    {
      echo "<script language=\"JavaScript\">alert('Permisos insuficientes');</script>";
    }

  }
  </script>

  <script language="php">
  $data = array();
  $valid_data = false;
  /* consulta de estados de todos los ports */
  $cmd = "/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -g -d $device -e";
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
      $seguidores=0;
      $alarm_state=0;
      $alarm_max=0;
      $alarm_min=0;
      for( $i = 0; $i < count($vals);$i++ )
      {
        if($vals[$i]['type'] == "complete")
        {
          if($vals[$i]['tag'] == "name") $port = $vals[$i]['value'];
          else if($vals[$i]['tag'] == "bit") $bit = $vals[$i]['value'];
          else if($vals[$i]['tag'] == "value") $val = $vals[$i]['value'];
          else if($vals[$i]['tag'] == "seguidor") $seguidores = $vals[$i]['value'];
          else if($vals[$i]['tag'] == "alarm") $alarm_state = $vals[$i]['value'];
          else if($vals[$i]['tag'] == "max") $alarm_max = $vals[$i]['value'];
          else if($vals[$i]['tag'] == "min") $alarm_min = $vals[$i]['value'];
          else if($vals[$i]['tag'] == "info-mask") $info_mask = $vals[$i]['value'];
          else if($vals[$i]['tag'] == "last-change")
          {
            $last_change = $vals[$i]['value'];
            if( substr($port, 0, 1) == 'A' )
            {
              $data[$port]['value'] = $val;
              $data[$port]['alarm'] = $alarm_state;
              $data[$port]['max'] = $alarm_max;
              $data[$port]['min'] = $alarm_min;
              $data[$port]['seguidor'] = $seguidores;
              $data[$port]['info-mask'] = $info_mask;
              $data[$port]['last-change'] = $last_change;
            }
            else
            {
              $data[$port][$bit]['value'] = $val;
              $data[$port]['info-mask'] = $info_mask;
              if( substr($port, 0, 1) == 'I' )
              {
                $data[$port]['seguidor'] = $seguidores;
              }
              $data[$port][$bit]['last-change'] = $last_change;
            }
            $valid_data = true;
          }
        }
      }
    }
  }
  </script>

<? if(isset($device) && isset($port) && isset($bit) && isset($value)) { ?>
  <body onload="setTimeout('reloadFrame()',3000);">
<? } else { ?>
  <body>
<? } ?>
<table width="80%" align=center>
<td align=left><h1>Administrador de dispositivos</h1></td>
<td align=right><a href="hw_admin.php" target=_top><img src="images/back.png" border="0"></a></td>
</table>

<table width="80%" align=center>
<td align=left><h2>Reportado por HW <? printf("%03d", $device); ?><h2></td><td>&nbsp;</td>
</table>
<table width="80%" bgcolor="#FFFFFF" align=center>
<tr>
  <td bgcolor="#FFD0D0">&nbsp;<b>Puerto</b></td>
  <td bgcolor="#FFD0D0">&nbsp;<b>Estado</b></td>
  <td bgcolor="#FFD0D0">&nbsp;<b>Configuraci&oacute;n</b></td>
</tr>
  <script language="php">
  $count = 0;
  while( $item = each($data) )
  {
    if($count%2)
      $color="#D0D0FF";
    else
      $color="#D0FFD0";
            
    echo "<tr>";
    echo "<td bgcolor=\"$color\">&nbsp;";
    if( substr($item[0], 0, 1) == 'I')      printf("Entrada %s", substr($item[0], 1, 1));
    else if( substr($item[0], 0, 1) == 'O') printf("Salida %s", substr($item[0], 1, 1));
    else if( substr($item[0], 0, 1) == 'A') printf("Analog %s", substr($item[0], 1, 1));
    ?>
    </td>
    <td bgcolor="<?echo "$color";?>">&nbsp;
      <? if(substr($item[0], 0, 1) == 'A') { ?>
          <b><?
              printf("%d,%02d V - %d", 
                      $item[1]['value']*0.005, ($item[1]['value']*0.5)%100,
                      $item[1]['value']);
             ?></b>
    </td>
    <td bgcolor="<?echo "$color";?>">&nbsp;
        <? printf("<b>noti:</b> %s  &nbsp;&nbsp;<b>seg:</b> %s &nbsp;&nbsp; <b>max:</b> %d &nbsp;&nbsp; <b>min:</b> %d &nbsp;&nbsp; <b>stat:</b> %s", ($item[1]['info-mask'] & (pow(2,(substr($item[0], 1, 1))-1)))?"On":"Off", ($item[1]['seguidor'] & (pow(2,(substr($item[0], 1, 1))-1)))?"On":"Off", $item[1]['max'], $item[1]['min'], ($item[1]['alarm'] == 0)?"Off":"On" ); ?>
      <? } else { ?>
        <?if(substr($item[0], 0, 1) == 'O'){?><a href="?device=<?print($device);?>&port=<? print(substr($item[0], 1, 1)); ?>&bit=7&value=<? print($item[1][7]['value']); ?>"><?}?>
          <img src="images/<? if($item[1][7]['value'] == 1){ echo "green.gif"; }else{ echo "red.gif"; }?>" name=line<?echo "$count";?>bit7 border="0" >
        <?if(substr($item[0], 0, 1) == 'O'){?></a><?}?>
     
        <?if(substr($item[0], 0, 1) == 'O'){?><a href="?device=<?print($device);?>&port=<? print(substr($item[0], 1, 1)); ?>&bit=6&value=<? print($item[1][6]['value']); ?>"><?}?>
          <img src="images/<? if($item[1][6]['value'] == 1){ echo "green.gif"; }else{ echo "red.gif"; }?>" name=line<?echo "$count";?>bit6 border="0" >
        <?if(substr($item[0], 0, 1) == 'O'){?></a><?}?>

        <?if(substr($item[0], 0, 1) == 'O'){?><a href="?device=<?print($device);?>&port=<? print(substr($item[0], 1, 1)); ?>&bit=5&value=<? print($item[1][5]['value']); ?>"><?}?>
          <img src="images/<? if($item[1][5]['value'] == 1){ echo "green.gif"; }else{ echo "red.gif"; }?>" name=line<?echo "$count";?>bit5 border="0" >
        <?if(substr($item[0], 0, 1) == 'O'){?></a><?}?>

        <?if(substr($item[0], 0, 1) == 'O'){?><a href="?device=<?print($device);?>&port=<? print(substr($item[0], 1, 1)); ?>&bit=4&value=<? print($item[1][4]['value']); ?>"><?}?>
          <img src="images/<? if($item[1][4]['value'] == 1){ echo "green.gif"; }else{ echo "red.gif"; }?>" name=line<?echo "$count";?>bit4 border="0" >
        <?if(substr($item[0], 0, 1) == 'O'){?></a><?}?>

        <?if(substr($item[0], 0, 1) == 'O'){?><a href="?device=<?print($device);?>&port=<? print(substr($item[0], 1, 1)); ?>&bit=3&value=<? print($item[1][3]['value']); ?>"><?}?>
          <img src="images/<? if($item[1][3]['value'] == 1){ echo "green.gif"; }else{ echo "red.gif"; }?>" name=line<?echo "$count";?>bit3 border="0" >
        <?if(substr($item[0], 0, 1) == 'O'){?></a><?}?>

        <?if(substr($item[0], 0, 1) == 'O'){?><a href="?device=<?print($device);?>&port=<? print(substr($item[0], 1, 1)); ?>&bit=2&value=<? print($item[1][2]['value']); ?>"><?}?>
          <img src="images/<? if($item[1][2]['value'] == 1){ echo "green.gif"; }else{ echo "red.gif"; }?>" name=line<?echo "$count";?>bit2 border="0" >
        <?if(substr($item[0], 0, 1) == 'O'){?></a><?}?>

        <?if(substr($item[0], 0, 1) == 'O'){?><a href="?device=<?print($device);?>&port=<? print(substr($item[0], 1, 1)); ?>&bit=1&value=<? print($item[1][1]['value']); ?>"><?}?>
          <img src="images/<? if($item[1][1]['value'] == 1){ echo "green.gif"; }else{ echo "red.gif"; }?>" name=line<?echo "$count";?>bit1 border="0" >
        <?if(substr($item[0], 0, 1) == 'O'){?></a><?}?>

        <?if(substr($item[0], 0, 1) == 'O'){?><a href="?device=<?print($device);?>&port=<? print(substr($item[0], 1, 1)); ?>&bit=0&value=<? print($item[1][0]['value']); ?>"><?}?>
          <img src="images/<? if($item[1][0]['value'] == 1){ echo "green.gif"; }else{ echo "red.gif"; }?>" name=line<?echo "$count";?>bit0 border="0" >
        <?if(substr($item[0], 0, 1) == 'O'){?></a><?}?>
    </td>
    <td bgcolor="<?echo "$color";?>">&nbsp;
        <? if(substr($item[0], 0, 1) == 'I') { ?>
          <? printf("<b>notificacion:</b> 0x%02X", $item[1]['info-mask']); ?>
          <? printf("<b>seguidores:</b> 0x%02X", $item[1]['seguidor']); ?>
        <? } ?>
        <? if(substr($item[0], 0, 1) == 'O') { ?>
          <? printf("<b>notificacion:</b> 0x%02X", $item[1]['info-mask']); ?>
        <? } ?>
      <? } ?>
    </td>
    </tr>
    <?
    $count++;
  }
  </script>
<tr>
  <td bgcolor="#FFD0D0">&nbsp;</td>
  <td bgcolor="#FFD0D0">&nbsp;</td>
  <td bgcolor="#FFD0D0">&nbsp;</td>
</tr>
</table>
<br />

<script language="php">
if($db->Select("Mascara_E1S1, Mascara_E2S2, Mascara_ANS2, ANALOG1_MAX, ANALOG1_MIN, ANALOG2_MAX, ANALOG2_MIN, ANALOG3_MAX, ANALOG3_MIN, ANALOG4_MAX, ANALOG4_MIN, info_e1, info_e2, info_s1, info_s2, info_an", // SELECT
               "devconf", // FROM
               "id = $device", // WHERE
               "", // ORDER BY
               "")) // GROUP BY
{
  $Mascara_E1S1    = $db->SelectData(1);
  $Mascara_E2S2    = $db->SelectData(2);
  $Mascara_ANS2    = $db->SelectData(3);
  $analog1_max     = $db->SelectData(4);
  $analog1_min     = $db->SelectData(5);
  $analog2_max     = $db->SelectData(6);
  $analog2_min     = $db->SelectData(7);
  $analog3_max     = $db->SelectData(8);
  $analog3_min     = $db->SelectData(9);
  $analog4_max     = $db->SelectData(10);
  $analog4_min     = $db->SelectData(11);
  $info_e1         = $db->SelectData(12);
  $info_e2         = $db->SelectData(13);
  $info_s1         = $db->SelectData(14);
  $info_s2         = $db->SelectData(15);
  $info_an         = $db->SelectData(16);
  $mode = "update";
}
else
{
  $Mascara_E1S1    = 0;
  $Mascara_E2S2    = 0;
  $analog1_max     = 0;
  $analog1_min     = 0;
  $analog2_max     = 0;
  $analog2_min     = 0;
  $analog3_max     = 0;
  $analog3_min     = 0;
  $analog4_max     = 0;
  $analog4_min     = 0;
  $Mascara_ANS2    = 0;
  $info_e1         = 0;
  $info_e2         = 0;
  $info_s1         = 0;
  $info_s2         = 0;
  $info_an         = 0;
  $mode = "insert";
}
</script>

<form name="frm_parametros" method=POST action="hw_control.php">
<table width="80%" align=center>
<td align=left><h2>Parametros de configuraci&oacute;n<h2></td><td>&nbsp;</td>
</table>
<table width="80%" bgcolor="#FFFFFF" align=center>
<tr>
  <td bgcolor="#FFD0D0">&nbsp;</td>
  <td bgcolor="#FFD0D0">&nbsp;</td>
</tr>
<tr>
  <td bgcolor="#D0D0FF">&nbsp;Seguidores I1 -> O1</td>
  <td bgcolor="#D0D0FF">&nbsp;M&aacute;scara (7..0)&nbsp;
    <input type=checkbox name=mascara1bit7 value="1" <? if($Mascara_E1S1 & 0x80) echo "checked";?> >
    <input type=checkbox name=mascara1bit6 value="1" <? if($Mascara_E1S1 & 0x40) echo "checked";?> >
    <input type=checkbox name=mascara1bit5 value="1" <? if($Mascara_E1S1 & 0x20) echo "checked";?> >
    <input type=checkbox name=mascara1bit4 value="1" <? if($Mascara_E1S1 & 0x10) echo "checked";?> >
    <input type=checkbox name=mascara1bit3 value="1" <? if($Mascara_E1S1 & 0x08) echo "checked";?> >
    <input type=checkbox name=mascara1bit2 value="1" <? if($Mascara_E1S1 & 0x04) echo "checked";?> >
    <input type=checkbox name=mascara1bit1 value="1" <? if($Mascara_E1S1 & 0x02) echo "checked";?> >
    <input type=checkbox name=mascara1bit0 value="1" <? if($Mascara_E1S1 & 0x01) echo "checked";?> >
  </td>
</tr>
<tr>
  <td bgcolor="#D0FFD0">&nbsp;Seguidores I2 -> O2</td>
  <td bgcolor="#D0FFD0">&nbsp;M&aacute;scara (7..0)&nbsp;
    <input type=checkbox name=mascara2bit7 value="1" <? if($Mascara_E2S2 & 0x80) echo "checked";?> >
    <input type=checkbox name=mascara2bit6 value="1" <? if($Mascara_E2S2 & 0x40) echo "checked";?> >
    <input type=checkbox name=mascara2bit5 value="1" <? if($Mascara_E2S2 & 0x20) echo "checked";?> >
    <input type=checkbox name=mascara2bit4 value="1" <? if($Mascara_E2S2 & 0x10) echo "checked";?> >
    <input type=checkbox name=mascara2bit3 value="1" <? if($Mascara_E2S2 & 0x08) echo "checked";?> >
    <input type=checkbox name=mascara2bit2 value="1" <? if($Mascara_E2S2 & 0x04) echo "checked";?> >
    <input type=checkbox name=mascara2bit1 value="1" <? if($Mascara_E2S2 & 0x02) echo "checked";?> >
    <input type=checkbox name=mascara2bit0 value="1" <? if($Mascara_E2S2 & 0x01) echo "checked";?> >
  </td>
</tr>
<tr>
  <td bgcolor="#D0D0FF">&nbsp;Seguidores AN -> O2</td>
  <td bgcolor="#D0D0FF">&nbsp;M&aacute;scara (7..0)&nbsp;
    <input type=checkbox name=mascara3bit7 value="1" <? if($Mascara_ANS2 & 0x80) echo "checked";?> >
    <input type=checkbox name=mascara3bit6 value="1" <? if($Mascara_ANS2 & 0x40) echo "checked";?> >
    <input type=checkbox name=mascara3bit5 value="1" <? if($Mascara_ANS2 & 0x20) echo "checked";?> >
    <input type=checkbox name=mascara3bit4 value="1" <? if($Mascara_ANS2 & 0x10) echo "checked";?> >
    <input type=checkbox name=mascara3bit3 value="1" <? if($Mascara_ANS2 & 0x08) echo "checked";?> >
    <input type=checkbox name=mascara3bit2 value="1" <? if($Mascara_ANS2 & 0x04) echo "checked";?> >
    <input type=checkbox name=mascara3bit1 value="1" <? if($Mascara_ANS2 & 0x02) echo "checked";?> >
    <input type=checkbox name=mascara3bit0 value="1" <? if($Mascara_ANS2 & 0x01) echo "checked";?> >
  </td>
</tr>

<tr>
  <td bgcolor="#D0FFD0">&nbsp;Notificaci&oacute;n -> I1</td>
  <td bgcolor="#D0FFD0">&nbsp;M&aacute;scara (7..0)&nbsp;
    <input type=checkbox name=notife1bit7 value="1" <? if($info_e1 & 0x80) echo "checked";?> >
    <input type=checkbox name=notife1bit6 value="1" <? if($info_e1 & 0x40) echo "checked";?> >
    <input type=checkbox name=notife1bit5 value="1" <? if($info_e1 & 0x20) echo "checked";?> >
    <input type=checkbox name=notife1bit4 value="1" <? if($info_e1 & 0x10) echo "checked";?> >
    <input type=checkbox name=notife1bit3 value="1" <? if($info_e1 & 0x08) echo "checked";?> >
    <input type=checkbox name=notife1bit2 value="1" <? if($info_e1 & 0x04) echo "checked";?> >
    <input type=checkbox name=notife1bit1 value="1" <? if($info_e1 & 0x02) echo "checked";?> >
    <input type=checkbox name=notife1bit0 value="1" <? if($info_e1 & 0x01) echo "checked";?> >
  </td>
</tr>
<tr>
  <td bgcolor="#D0D0FF">&nbsp;Notificaci&oacute;n -> I2</td>
  <td bgcolor="#D0D0FF">&nbsp;M&aacute;scara (7..0)&nbsp;
    <input type=checkbox name=notife2bit7 value="1" <? if($info_e2 & 0x80) echo "checked";?> >
    <input type=checkbox name=notife2bit6 value="1" <? if($info_e2 & 0x40) echo "checked";?> >
    <input type=checkbox name=notife2bit5 value="1" <? if($info_e2 & 0x20) echo "checked";?> >
    <input type=checkbox name=notife2bit4 value="1" <? if($info_e2 & 0x10) echo "checked";?> >
    <input type=checkbox name=notife2bit3 value="1" <? if($info_e2 & 0x08) echo "checked";?> >
    <input type=checkbox name=notife2bit2 value="1" <? if($info_e2 & 0x04) echo "checked";?> >
    <input type=checkbox name=notife2bit1 value="1" <? if($info_e2 & 0x02) echo "checked";?> >
    <input type=checkbox name=notife2bit0 value="1" <? if($info_e2 & 0x01) echo "checked";?> >
  </td>
</tr>
<tr>
  <td bgcolor="#D0FFD0">&nbsp;Notificaci&oacute;n -> O1</td>
  <td bgcolor="#D0FFD0">&nbsp;M&aacute;scara (7..0)&nbsp;
    <input type=checkbox name=notifs1bit7 value="1" <? if($info_s1 & 0x80) echo "checked";?> >
    <input type=checkbox name=notifs1bit6 value="1" <? if($info_s1 & 0x40) echo "checked";?> >
    <input type=checkbox name=notifs1bit5 value="1" <? if($info_s1 & 0x20) echo "checked";?> >
    <input type=checkbox name=notifs1bit4 value="1" <? if($info_s1 & 0x10) echo "checked";?> >
    <input type=checkbox name=notifs1bit3 value="1" <? if($info_s1 & 0x08) echo "checked";?> >
    <input type=checkbox name=notifs1bit2 value="1" <? if($info_s1 & 0x04) echo "checked";?> >
    <input type=checkbox name=notifs1bit1 value="1" <? if($info_s1 & 0x02) echo "checked";?> >
    <input type=checkbox name=notifs1bit0 value="1" <? if($info_s1 & 0x01) echo "checked";?> >
  </td>
</tr>
<tr>
  <td bgcolor="#D0D0FF">&nbsp;Notificaci&oacute;n -> O2</td>
  <td bgcolor="#D0D0FF">&nbsp;M&aacute;scara (7..0)&nbsp;
    <input type=checkbox name=notifs2bit7 value="1" <? if($info_s2 & 0x80) echo "checked";?> >
    <input type=checkbox name=notifs2bit6 value="1" <? if($info_s2 & 0x40) echo "checked";?> >
    <input type=checkbox name=notifs2bit5 value="1" <? if($info_s2 & 0x20) echo "checked";?> >
    <input type=checkbox name=notifs2bit4 value="1" <? if($info_s2 & 0x10) echo "checked";?> >
    <input type=checkbox name=notifs2bit3 value="1" <? if($info_s2 & 0x08) echo "checked";?> >
    <input type=checkbox name=notifs2bit2 value="1" <? if($info_s2 & 0x04) echo "checked";?> >
    <input type=checkbox name=notifs2bit1 value="1" <? if($info_s2 & 0x02) echo "checked";?> >
    <input type=checkbox name=notifs2bit0 value="1" <? if($info_s2 & 0x01) echo "checked";?> >
  </td>
</tr>
<tr>
  <td bgcolor="#D0FFD0">&nbsp;Notificaci&oacute;n -> AN</td>
  <td bgcolor="#D0FFD0">&nbsp;M&aacute;scara (7..0)&nbsp;
    <input type=checkbox name=notifanbit7 value="1" <? if($info_an & 0x80) echo "checked";?> >
    <input type=checkbox name=notifanbit6 value="1" <? if($info_an & 0x40) echo "checked";?> >
    <input type=checkbox name=notifanbit5 value="1" <? if($info_an & 0x20) echo "checked";?> >
    <input type=checkbox name=notifanbit4 value="1" <? if($info_an & 0x10) echo "checked";?> >
    <input type=checkbox name=notifanbit3 value="1" <? if($info_an & 0x08) echo "checked";?> >
    <input type=checkbox name=notifanbit2 value="1" <? if($info_an & 0x04) echo "checked";?> >
    <input type=checkbox name=notifanbit1 value="1" <? if($info_an & 0x02) echo "checked";?> >
    <input type=checkbox name=notifanbit0 value="1" <? if($info_an & 0x01) echo "checked";?> >
  </td>
</tr>

<tr>
  <td bgcolor="#D0D0FF">&nbsp;Alarma anal&oacute;gica 1</td>
  <td bgcolor="#D0D0FF">&nbsp;
    min: <input type=text name="analog1_min" size="5" maxlength="4" value="<? printf("%d", $analog1_min); ?>">
    &nbsp;&nbsp;
    max: <input type=text name="analog1_max" size="5" maxlength="4" value="<? printf("%d", $analog1_max); ?>">
  </td>
</tr>
<tr>
  <td bgcolor="#D0FFD0">&nbsp;Alarma anal&oacute;gica 2</td>
  <td bgcolor="#D0FFD0">&nbsp;
    min: <input type=text name="analog2_min" size="5" maxlength="4" value="<? printf("%d", $analog2_min); ?>">
    &nbsp;&nbsp;
    max: <input type=text name="analog2_max" size="5" maxlength="4" value="<? printf("%d", $analog2_max); ?>">
  </td>
</tr>
<tr>
  <td bgcolor="#D0D0FF">&nbsp;Alarma anal&oacute;gica 3</td>
  <td bgcolor="#D0D0FF">&nbsp;
    min: <input type=text name="analog3_min" size="5" maxlength="4" value="<? printf("%d", $analog3_min); ?>">
    &nbsp;&nbsp;
    max: <input type=text name="analog3_max" size="5" maxlength="4" value="<? printf("%d", $analog3_max); ?>">
  </td>
</tr>
<tr>
  <td bgcolor="#D0FFD0">&nbsp;Alarma anal&oacute;gica 4</td>
  <td bgcolor="#D0FFD0">&nbsp;
    min: <input type=text name="analog4_min" size="5" maxlength="4" value="<? printf("%d", $analog4_min); ?>">
    &nbsp;&nbsp;
    max: <input type=text name="analog4_max" size="5" maxlength="4" value="<? printf("%d", $analog4_max); ?>">
  </td>
</tr>
<tr>
  <td bgcolor="#FFD0D0">&nbsp;</td>
  <td bgcolor="#FFD0D0">&nbsp;</td>
</tr>
</table>

<table border="0" width="80%" align="center" cellpadding=10>
<tr>
  <td align="right" bgcolor="#E0E0E0">
    <input type=submit name="actualizar"  value="Actualizar">&nbsp;&nbsp;
  </td>
</tr>
</table>
<input type=hidden name="device"  value="<? echo "$device"; ?>">
<input type=hidden name="mode"  value="<? echo "$mode"; ?>">
</form>

</body>
</html>

