<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::ControlHome")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Temperatura</title>
  <script language="javascript">
    function closeWindow() { window.close(); }
  </script>
</head>

<script language="php">
  $data1 = array();
  $valid_data1 = false;

  if( !isset($sel_id) || !isset($sel_port))
  {
    // error en parametros de llamada
    ?>
      <script language="javascript">
        alert('error en parametros de llamada');
        window.close();
      </script>
    <?
    return;
  }
  if( isset($actualizar))
  {
    if( !isset($max_temp) || !isset($min_temp) )
    {
        $max_temp = 1024;
        $min_temp = 0;
    }
    else
    {
      $max_temp = $max_temp * 10 / 5;
      $min_temp = $min_temp * 10 / 5;
    }
    exec("/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -d $sel_id -p $sel_port -a $max_temp $min_temp");
    ?>
      <script language="javascript">
        window.close();
      </script>
    <?
    return;
  }
  else
  {
    /* consulta de estados de todos los ports de la interface 1 */
    $cmd = "/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -g -d ".$sel_id." -e";
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
            if($vals[$i]['tag'] == "name") $port = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "bit") $bit = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "value") $val = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "seguidor") $seguidores = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "alarm") $alarm_state = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "max") $alarm_max = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "min") $alarm_min = $vals[$i]['value'];
            else if($vals[$i]['tag'] == "last-change")
            {
              $last_change = $vals[$i]['value'];
              if( substr($port, 0, 1) == 'A' )
              {
                $data1[$port]['value'] = $val;
                $data1[$port]['alarm'] = $alarm_state;
                $data1[$port]['max'] = $alarm_max;
                $data1[$port]['min'] = $alarm_min;
                $data1[$port]['last-change'] = $last_change;
              }
              else
              {
                $data1[$port][$bit]['value'] = $val;
                if( substr($port, 0, 1) == 'I' )
                  $data1[$port]['seguidor'] = $seguidores;
                $data1[$port][$bit]['last-change'] = $last_change;
              }
              $valid_data1 = true;
              $bit = 0;
            }
          }
        }
      }
    }
  }
</script>
<body bgcolor="#F0F0F0">
<h1>Temperatura</h1>
<? if($valid_data1) { ?>
<form name="frm_temp_control" method=POST action="temp_control.php">
<table border="0" width="600" align="center" bgcolor="#F0F0F0">
<tr>
  <td align="right" width="40%">Temperatura actual:</td>
  <td width="20%">&nbsp;</td>
  <td width="40%"><?printf("%d.%d", ($data1["A".$sel_port]['value']*0.5), ($data1["A".$sel_port]['value']*5)%10);?>&nbsp;&deg;C</td>
<tr>
<tr>
  <td>&nbsp;</td><td>&nbsp;</td>
</tr>
<tr>
  <td align="right">Maxima:</td>
  <td>&nbsp;</td>
  <td><input type=text name="max_temp" size="6" maxlength="4" value="<?printf("%d.%d", ($data1["A".$sel_port]['max']*0.5), ($data1["A".$sel_port]['max']*5)%10);?>">&nbsp;&deg;C</td>
</tr>
<tr>
  <td align="right">Minima:</td>
  <td>&nbsp;</td>
  <td><input type=text name="min_temp" size="6" maxlength="4" value="<?printf("%d.%d", ($data1["A".$sel_port]['min']*0.5), ($data1["A".$sel_port]['min']*5)%10);?>">&nbsp;&deg;C</td>
</tr>
</table>

<table border="0" width="90%" align="center">
<tr>
  <td>&nbsp;</td>
</tr>
<tr>
  <td align="right" bgcolor="#E0E0E0">
    <input type=submit name="actualizar"  value="Actualizar">&nbsp;&nbsp;
    <input type=submit name="cancelar" value="Cancelar" onClick="closeWindow()">
  </td>
</tr>
</table>
<input type=hidden name="sel_id" value="<? echo "$sel_id"; ?>">
<input type=hidden name="sel_port" value="<? echo "$sel_port"; ?>">
</form>
<? } ?>
</body>
</html>

