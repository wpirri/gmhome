<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>
<head>
  <title>GmHome::Control Home</title>

<script language="javascript" type="text/javascript">
var pir = new Array(2);
pir[0] = new Image();
pir[0].src = "images/pir-off.gif";
pir[1] = new Image();
pir[1].src = "images/pir-on.gif";
var notif = new Array(11);
notif[0] = new Image();
notif[0].src = "images/_blank.png";
notif[1] = new Image();
notif[1].src = "images/system.png";
notif[2] = new Image();
notif[2].src = "images/stock_lock-open.png";
notif[3] = new Image();
notif[3].src = "images/stock_lock.png";
notif[4] = new Image();
notif[4].src = "images/pipe.png";
notif[5] = new Image();
notif[5].src = "images/stock_people.png";
notif[6] = new Image();
notif[6].src = "images/_blank.png";
notif[7] = new Image();
notif[7].src = "images/_blank.png";
notif[8] = new Image();
notif[8].src = "images/_blank.png";
notif[9] = new Image();
notif[9].src = "images/_blank.png";
notif[10] = new Image();
notif[10].src = "images/_blank.png";

var lamp = new Array(2);
lamp[0] = new Image();
lamp[0].src = "images/lampara-off.png";
lamp[1] = new Image();
lamp[1].src = "images/lampara-on.png";

var number = new Array(10);
number[0] = new Image();
number[0].src = "images/c0.gif"
number[1] = new Image();
number[1].src = "images/c1.gif"
number[2] = new Image();
number[2].src = "images/c2.gif"
number[3] = new Image();
number[3].src = "images/c3.gif"
number[4] = new Image();
number[4].src = "images/c4.gif"
number[5] = new Image();
number[5].src = "images/c5.gif"
number[6] = new Image();
number[6].src = "images/c6.gif"
number[7] = new Image();
number[7].src = "images/c7.gif"
number[8] = new Image();
number[8].src = "images/c8.gif"
number[9] = new Image();
number[9].src = "images/c9.gif"

var icon_info = new Array(2)
icon_info[0] = new Image();
icon_info[0].src = "images/documentacion_icono.gif"
icon_info[1] = new Image();
icon_info[1].src = "images/calefaccion_icono.gif"
icon_info[2] = new Image();
icon_info[2].src = "images/aire_acondicionado_icono.gif"

var info_sensor_comedor = 'sin info';
var info_sensor_living = 'sin info';
var info_sensor_dorm_fte = 'sin info';
var info_sensor_dorm_fdo = 'sin info';
var info_activacion_alarma = 'sin info';

function OpenCamara( camara )
{
  window.open('camara.php?camara='+camara, 'Camaras', 'tollbar=no,status=no,menubar=no,scrollbars=no,resizable=no,width=370,height=310');

}

function OpenTempControl( disp, port )
{
  if ( disp <= 0 ) return;
  if ( port <= 0 ) return;
  window.open('temp_control.php?sel_id=' + disp + '&sel_port=' + port, 'Control de temperatura', 'tollbar=no,status=no,menubar=no,scrollbars=no,resizable=no,width=800,height=450');
}

function displayBlock(idDiv){
        var theDiv = document.getElementById(idDiv);
        if (theDiv != null) {
                theDiv.style.display="block";
        }
}


function removeBlock(idDiv){
        var theDiv = document.getElementById(idDiv);
        if (theDiv != null) {
                theDiv.style.display="none";
        }
}

function setMsgText(sensor)
{
  if( sensor == 'COM' )
  {
    //document.msg_area.msg_text.value = 'Ult. detec.: ' + info_sensor_comedor;
    document.sensor_comedor.title = 'Ultima deteccion: ' + info_sensor_comedor;
  }
  else if( sensor == 'COC' )
  {
    //document.msg_area.msg_text.value = 'Ult. detec.: ' + info_sensor_cocina;
    document.sensor_cocina.title = 'Ultima deteccion: ' + info_sensor_cocina;
  }
  else if( sensor == 'DORM_FTE' )
  {
    //document.msg_area.msg_text.value = 'Ult. detec.: ' + info_sensor_dorm_fte;
    document.sensor_dorm_fte.title = 'Ultima deteccion: ' + info_sensor_dorm_fte;
  }
  else if( sensor == 'DORM_FDO' )
  {
    //document.msg_area.msg_text.value = 'Ult. detec.: ' + info_sensor_dorm_fdo;
    document.sensor_dorm_fdo.title = 'Ultima deteccion: ' + info_sensor_dorm_fdo;
  }
  else if( sensor == 'ACTIV' )
  {
    //document.msg_area.msg_text.value = 'Ult. cambio: ' + info_activacion_alarma;
    document.info_1.title = 'Ultimo cambio de estado: ' + info_activacion_alarma;
  }
}

//function clearMsgText() {
//    document.msg_area.msg_text.value = '';
//}

</script>
<script language="php">
if( isset($disp) && isset($port) && isset($bit) && isset($cmd))
{
  if(control_permisos($SESSION_USER, "SysHome::ControlHome", $REMOTE_ADDR))
  {
    $exe = "/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -$cmd -d $disp -p $port -b $bit";
    system($exe);
    if( isset($otro_bit) )
    {
      $exe = "/usr/bin/gmh_clisercomm -h $homeserver -P $homeport -$cmd -d $disp -p $port -b $otro_bit";
      system($exe);
    }
  }
  else
  {
    echo "<script language=\"JavaScript\">alert('Permisos insuficientes');</script>";
  }
}
</script>

</head>
<? if( !isset($view)) $view=2; ?>
<body background="../images/bg_cesped.jpg">
<!--<h1 align=center>Control Home</h1>-->
<table width="100%" bgcolor="#FFFFFF" align=center>
<tr>
  <td>
    <table width="100%" bgcolor="#D0D0FF">
    <tr>
      <td align="center" <? if($view == 0) {?> bgcolor="#A0A0F0"<?}?> ><a href="?view=0"><img src="images/identity.png" border="0"></a></td>
      <td align="center" <? if($view == 1) {?> bgcolor="#A0A0F0"<?}?> ><a href="?view=1"><img src="images/back.png" border="0"></a></td>
      <td align="center" <? if($view == 2) {?> bgcolor="#A0A0F0"<?}?> ><a href="?view=2"><img src="images/gohome.png" border="0"></a></td>
      <td align="center" <? if($view == 3) {?> bgcolor="#A0A0F0"<?}?> ><a href="?view=3"><img src="images/forward.png" border="0"></a></td>
      <td width="40%" align="center">
        <!--<form name=msg_area ><input type=text name=msg_text size="30" value=""></form>-->
      </td>
    </tr>
    </table>
  </td>
  <td bgcolor="#FFFFFF" align="center"><a href="<? echo $EXIT_PAGE; ?>" target=_parent><img src="images/exit.png" border="0"></a></td>
</tr>
<tr><td height="400" valign="top" align="center">
<form name=mainform>
  <?
    if($view == 1)
    {
      include('frente.php');
    }
    else if($view == 2)
    {
      include('casa.php');
    }
    else if($view == 3)
    {
      include('fondo.php');
    }
    else
    {
      include('inicio.php');
    }
  ?>
</form>
</td>
<td width="50" align="center" valign="top" bgcolor="#D0FFD0">
  <br />
  <img src="images/_blank.png" name=info_0 border="0" alt="Sistema on-line">
<? if( ! empty($SESSION_USER) ) { ?>
  <a href="?view=<?echo $view;?>&cmd=x 2&disp=1&port=1&bit=7">
<? } ?>
    <img src="images/_blank.png" name=info_1 border="0" alt="Alarma"  onmouseout="document.info_1.title=''" onmouseover="setMsgText('ACTIV')">
  </a>
  <a href="/zm" target=_blank>
    <img src="images/camara.png" name=camara_1 border="0">
  </a>
  <img src="images/_blank.png" name=info_2 border="0" alt="Bombeador">
  <img src="images/_blank.png" name=info_3 border="0" alt="Intrusos">
  <img src="images/_blank.png" name=info_4 border="0">
  <img src="images/_blank.png" name=info_5 border="0">
<!--
  <img src="images/_blank.png" name=info_6 border="0">
  <img src="images/_blank.png" name=info_7 border="0">
  <img src="images/_blank.png" name=info_8 border="0">
  <img src="images/_blank.png" name=info_9 border="0">
-->
</td>
</tr>
</table>
</body>
</html>

<script language="javascript" type="text/javascript">
  //setTimeout("parent.control.location.reload()", 3000);
</script>
