<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::ViewHome")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title></title>
  <script language="javascript" type="text/javascript">
  <!--
  function reloadFrame() {
          self.location.reload();
  }
  -->
  </script>
</head>
  <script language="php">
  $data1 = array();
  $data2 = array();
  $data3 = array();
  $valid_data1 = false;
  $valid_data2 = false;
  $valid_data3 = false;

  /* consulta de estados de todos los ports de la interface 1 */
  $cmd = "/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -g -d 1 -e";
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
          else if($vals[$i]['tag'] == "last-change")
          {
            $last_change = $vals[$i]['value'];

            $data1[$port][$bit]['value'] = $val;
            $data1[$port][$bit]['last-change'] = $last_change;

            $valid_data1 = true;
            $bit = 0;
          }
        }
      }
    }
  }

  /* consulta de estados de todos los ports de la interface 2 */
  $cmd = "/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -g -d 2 -e";
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
          else if($vals[$i]['tag'] == "last-change")
          {
            $last_change = $vals[$i]['value'];

            $data2[$port][$bit]['value'] = $val;
            $data2[$port][$bit]['last-change'] = $last_change;

            $valid_data2 = true;
            $bit = 0;
          }
        }
      }
    }
  }

  /* consulta de estados de todos los ports de la interface 3 */
  $cmd = "/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -g -d 3 -e";
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
          else if($vals[$i]['tag'] == "last-change")
          {
            $last_change = $vals[$i]['value'];

            $data3[$port][$bit]['value'] = $val;
            $data3[$port][$bit]['last-change'] = $last_change;

            $valid_data3 = true;
            $bit = 0;
          }
        }
      }
    }
  }

  </script>
<body onload="setTimeout('reloadFrame()',3000);">

<!-- INTERFACE 1 -->
<? if($valid_data1) { ?>
<script language="javascript" type="text/javascript">
  //// TEMPERATURAS - INICIO ////
  if(parent.main.document.temp_dorm_fondo_decenas != null)
  {
    // Informacion del sensor en el dormitorio del fondo
    parent.main.document.temp_dorm_fondo_decenas.src = parent.main.number[<?printf("%d", ($data1['A2'][0]['value']*0.05)%10);?>].src;
    parent.main.document.temp_dorm_fondo_unidades.src = parent.main.number[<?printf("%d", ($data1['A2'][0]['value']*0.5)%10);?>].src;
    parent.main.document.temp_dorm_fondo_decimas.src = parent.main.number[<?printf("%d", ($data1['A2'][0]['value']*5)%10);?>].src;
  }
  if(parent.main.document.temp_dorm_frente_decenas != null)
  {
    // Informacion del sensor en el dormitorio del frente
    parent.main.document.temp_dorm_frente_decenas.src = parent.main.number[<?printf("%d", ($data1['A3'][0]['value']*0.05)%10);?>].src;
    parent.main.document.temp_dorm_frente_unidades.src = parent.main.number[<?printf("%d", ($data1['A3'][0]['value']*0.5)%10);?>].src;
    parent.main.document.temp_dorm_frente_decimas.src = parent.main.number[<?printf("%d", ($data1['A3'][0]['value']*5)%10);?>].src;
  }
  //// TEMPERATURAS - FIN ////

  //// INFO - INICIO ////
  if(parent.main.document.icon_info_dorm_fdo != null)
  {
    <? if( $data1['O2'][1]['value'] == 0 ) { ?>   
      parent.main.document.icon_info_dorm_fdo.src = parent.main.icon_info[1].src;
    <? } else { ?>
      parent.main.document.icon_info_dorm_fdo.src = parent.main.icon_info[0].src;
    <? } ?>
  }
  if(parent.main.document.icon_info_dorm_fte != null)
  {
    <? if( $data1['O2'][2]['value'] == 0 ) { ?>   
      parent.main.document.icon_info_dorm_fte.src = parent.main.icon_info[1].src;
    <? } else { ?>
      parent.main.document.icon_info_dorm_fte.src = parent.main.icon_info[0].src;
    <? } ?>
  }
  if(parent.main.document.icon_info_taller != null)
  {
    <? if( $data3['O2'][1]['value'] == 0 ) { ?>   
      parent.main.document.icon_info_taller.src = parent.main.icon_info[1].src;
    <? } else { ?>
      parent.main.document.icon_info_taller.src = parent.main.icon_info[0].src;
    <? } ?>
  }
  //// INFO - FIN ////

  if(parent.main.document.sensor_dorm_fte != null)
  {
    parent.main.document.sensor_dorm_fte.src = parent.main.pir[<? if($data1['I1'][3]['value']){ print("1"); }else{ print("0"); } ?>].src;
    parent.main.info_sensor_dorm_fte = '<? echo $data1['I1'][3]['last-change']; ?>';
  }
  if(parent.main.document.sensor_dorm_fdo != null)
  {
    parent.main.document.sensor_dorm_fdo.src = parent.main.pir[<? if($data1['I1'][2]['value']){ print("1"); }else{ print("0"); } ?>].src;
    parent.main.info_sensor_dorm_fdo = '<? echo $data1['I1'][2]['last-change']; ?>';
  }
  if(parent.main.document.sensor_cocina != null)
  {
    parent.main.document.sensor_cocina.src = parent.main.pir[<? if($data1['I1'][0]['value']){ print("1"); }else{ print("0"); } ?>].src;
    parent.main.info_sensor_cocina = '<? echo $data1['I1'][0]['last-change']; ?>';
  }
  if(parent.main.document.sensor_comedor != null)
  {
    parent.main.document.sensor_comedor.src = parent.main.pir[<? if($data1['I1'][1]['value']){ print("1"); }else{ print("0"); } ?>].src;
    parent.main.info_sensor_comedor = '<? echo $data1['I1'][1]['last-change']; ?>';
  }
  if(parent.main.document.info_0 != null)
  {
    parent.main.document.info_0.src = parent.main.notif[1].src;
  }
  if(parent.main.document.info_1 != null)
  {
    parent.main.document.info_1.src = parent.main.notif[<? if($data1['I1'][4]['value']){ print("2"); }else{ print("3"); } ?>].src;
    parent.main.info_activacion_alarma = '<? echo $data1['I1'][4]['last-change']; ?>';
  }
  if(parent.main.document.info_2 != null)
  {
    parent.main.document.info_2.src = parent.main.notif[<? if($data1['I1'][5]['value']){ print("4"); }else{ print("0"); } ?>].src;
  }
  if(parent.main.document.info_3 != null)
  {
    parent.main.document.info_3.src = parent.main.notif[<? if($data1['I1'][6]['value']){ print("0"); }else{ print("5"); } ?>].src;
  }
  // // Luces de la casa // //
  if(parent.main.document.luz_pasillo_central != null)
  {
    parent.main.document.luz_pasillo_central.src = parent.main.lamp[<? if($data1['O2'][5]['value']){ print("1"); }else{ print("0"); } ?>].src;
  }
  // // Luces del fondo // //
  if(parent.main.document.reflector_fondo != null)
  {
    parent.main.document.reflector_fondo.src = parent.main.lamp[<? if($data1['O1'][6]['value']){ print("1"); }else{ print("0"); } ?>].src;
  }
</script>
<? } ?>

<!-- INTERFACE 2 -->
<? if($valid_data2) { ?>
<script language="javascript" type="text/javascript">
  //// TEMPERATURAS - INICIO ////
  if(parent.main.document.temp_garage_decenas != null)
  {
    // Informacion del sensor on-board
    parent.main.document.temp_garage_decenas.src = parent.main.number[<?printf("%d", ($data2['A1'][0]['value']*0.05)%10);?>].src;
    parent.main.document.temp_garage_unidades.src = parent.main.number[<?printf("%d", ($data2['A1'][0]['value']*0.5)%10);?>].src;
    parent.main.document.temp_garage_decimas.src = parent.main.number[<?printf("%d", ($data2['A1'][0]['value']*5)%10);?>].src;
  }
  //// TEMPERATURAS - FIN ////
  // // Luces del jardin del frente // //
  if(parent.main.document.luz_garage != null)
  {
    parent.main.document.luz_garage.src = parent.main.lamp[<? if($data2['O2'][1]['value']){ print("1"); }else{ print("0"); } ?>].src;
  }
  if(parent.main.document.luz_jardin_fte_puerta_calle != null)
  {
    parent.main.document.luz_jardin_fte_puerta_calle.src = parent.main.lamp[<? if($data2['O2'][3]['value']){ print("1"); }else{ print("0"); } ?>].src;
  }
  if(parent.main.document.luz_jardin_fte_ficus != null)
  {
    parent.main.document.luz_jardin_fte_ficus.src = parent.main.lamp[<? if($data2['O2'][2]['value']){ print("1"); }else{ print("0"); } ?>].src;
  }
  if(parent.main.document.luz_pasillo_lateral != null)
  {
    parent.main.document.luz_pasillo_lateral.src = parent.main.lamp[<? if($data2['O2'][5]['value']){ print("1"); }else{ print("0"); } ?>].src;
  }
  if(parent.main.document.luz_jardin_fte_palmera != null)
  {
    parent.main.document.luz_jardin_fte_palmera.src = parent.main.lamp[<? if($data2['O2'][4]['value']){ print("1"); }else{ print("0"); } ?>].src;
  }
</script>
<? } ?>

<!-- INTERFACE 3 -->
<? if($valid_data3) { ?>
<script language="javascript" type="text/javascript">
  //// TEMPERATURAS - INICIO ////
  if(parent.main.document.temp_taller_decenas != null)
  {
    // Informacion del sensor en el dormitorio del fondo
    parent.main.document.temp_taller_decenas.src = parent.main.number[<?printf("%d", ($data3['A2'][0]['value']*0.05)%10);?>].src;
    parent.main.document.temp_taller_unidades.src = parent.main.number[<?printf("%d", ($data3['A2'][0]['value']*0.5)%10);?>].src;
    parent.main.document.temp_taller_decimas.src = parent.main.number[<?printf("%d", ($data3['A2'][0]['value']*5)%10);?>].src;
  }
  //// TEMPERATURAS - FIN ////
  // // Luces del jardin del fondo // //
  if(parent.main.document.luz_taller != null)
  {
    parent.main.document.luz_taller.src = parent.main.lamp[<? if($data3['O2'][4]['value']){ print("1"); }else{ print("0"); } ?>].src;
  }
  if(parent.main.document.luz_deposito != null)
  {
    parent.main.document.luz_deposito.src = parent.main.lamp[<? if($data3['O2'][5]['value']){ print("1"); }else{ print("0"); } ?>].src;
  }
  if(parent.main.document.luz_terraja != null)
  {
    parent.main.document.luz_terraja.src = parent.main.lamp[<? if($data3['O2'][6]['value']){ print("1"); }else{ print("0"); } ?>].src;
  }
</script>
<? } ?>



</body>
</html>
