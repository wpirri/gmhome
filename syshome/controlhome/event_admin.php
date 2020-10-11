<script language="php">
  require('gmhome_config.php');
  require('gmhome_dbconnect.php');
  require('gmhome_access_control.php');
  if( !control_acceso($SESSION_DATA, $SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT, "SysHome::AdminEvent")) return;
  $SESSION_DATA = string_acceso($SESSION_USER, $REMOTE_ADDR, $HTTP_USER_AGENT);
</script>
<html>

<head>
  <title>GmHome::Administracion de eventos</title>
  <link title="new" rel="stylesheet" href="css/event_admin.css" type="text/css">
</head>

<script language="javascript" type="text/javascript">
  function OpenABM( filename, param, title )
  {
    if( title == 'Group' )
    {
      window.open(filename + param, title, 'tollbar=no,status=no,menubar=no,scrollbars=no,resizable=no,width=800,height=450');
    }
    else if( title == 'Event' )
    {
      window.open(filename + param, title, 'tollbar=no,status=no,menubar=no,scrollbars=no,resizable=no,width=800,height=450');
    }
    else
    {
      window.open(filename + param, title, 'tollbar=no,status=no,menubar=no,scrollbars=no,resizable=no,width=640,height=350');
    }
  }
  function reloadWindow()
  {
    self.location.reload();
  }
</script>

<body bgcolor="#FFFFFF">
<table border="0" width="80%" align=center>
<td align=left><h1>Administraci&oacute;n de eventos</h1></td>
<td align=right><a href="index.php?view=0" target=_top><img src="images/back.png" border="0"></a></td>
</table>

<script language="php">
$db = new PPgSqlDB;
$db->Open($dbserver, "home", $dbuser, $dbpass);
$db2 = new PPgSqlDB;
$db2->Open($dbserver, "home", $dbuser, $dbpass);
if( !isset($folder)) $folder = 4;
</script>

<table border="0" width="80%" align=center bgcolor="#F0F0F0">
<tr>
  <td <? if($folder == 1) echo "bgcolor=\"FFD0D0\""; ?> align="center"><a class=tablelink href="?folder=1"><b>Flags&nbsp;|&nbsp;</b></a></td>
  <td <? if($folder == 2) echo "bgcolor=\"FFD0D0\""; ?> align="center"><a class=tablelink href="?folder=2"><b>Asignaciones&nbsp;|&nbsp;</b></a></td>
  <td <? if($folder == 3) echo "bgcolor=\"FFD0D0\""; ?> align="center"><a class=tablelink href="?folder=3"><b>Grupos&nbsp;|&nbsp;</b></a></td>
  <td <? if($folder == 4) echo "bgcolor=\"FFD0D0\""; ?> align="center"><a class=tablelink href="?folder=4"><b>Eventos&nbsp;|&nbsp;</b></a></td>
  <td width="50%">&nbsp;</td>
</tr>
</table>
<br />

<? if( $folder == 1) { ?>

<form name ="flags" method=POST action="abm_flags.php">
  <table border="0" width="80%" bgcolor="#FFFFFF" align=center>
    <tr>
    <td bgcolor="#FFD0D0"><b>Nombre</b></td>
    <td bgcolor="#FFD0D0"><b>Valor</b></td>
    <td bgcolor="#FFD0D0"><b>Descripcion</b></td>
    </tr>
    <script language="php">
    // Listado de flags
    $count = 0;
    if($db->Select( "nombre, valor, descripcion", // SELECT
                    "home_flag", // FROM
                    "", // WHERE
                    "nombre", // ORDER BY
                    "")) // GROUP BY
    {
      do
      {
        $flag_nombre =      $db->SelectData(1);
        $flag_valor =       $db->SelectData(2);
        $flag_descripcion = $db->SelectData(3);
        if($count%2)
          $color="#D0D0FF";
        else
          $color="#D0FFD0";

        ?>
          <tr>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_flags.php', '?nombre=<? echo "$flag_nombre"; ?>', 'Flag' )">
                <? printf("%s", $flag_nombre); ?>
              </a>
            </td>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_flags.php', '?nombre=<? echo "$flag_nombre"; ?>', 'Flag' )">
                <? printf("%s", $flag_valor); ?>
              </a>
            </td>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_flags.php', '?nombre=<? echo "$flag_nombre"; ?>', 'Flag' )">
                <? printf("%s", $flag_descripcion); ?>
              </a>
            </td>
          </tr>
        <?
        $count++;
      } while($db->SelectNext());
    }
    </script>
    <tr>
      <td bgcolor="#FFD0D0">&nbsp;</td>
      <td bgcolor="#FFD0D0">&nbsp;</td>
      <td bgcolor="#FFD0D0" align="right"><a href="javascript:OpenABM( 'abm_flags.php', '', 'Flag' )">Nuevo</a></td>
    </tr>
  </table>
</form>

<? } else if( $folder == 2) { ?>

<form name ="assigns" method=POST action="abm_assign.php">
  <table border="0" width="80%" bgcolor="#FFFFFF" align=center>
  <tr>
  <td bgcolor="#FFD0D0"><b>Dispositivo</b></td>
  <td bgcolor="#FFD0D0"><b>Puerto</b></td>
  <td align="center" bgcolor="#FFD0D0"><b>E/S</b></td>
  <td align="center" bgcolor="#FFD0D0"><b>Nombre</b></td>
  </tr>
    <script language="php">
    // Listado de asignaciones
    $count = 0;
    if($db->Select( "assign.id, assign.port, assign.bit, assign.assign_name, device.device_name", // SELECT
                    "assign, device", // FROM
                    "assign.id = device.id", // WHERE
                    "assign.id, assign.port, assign.bit", // ORDER BY
                    "")) // GROUP BY
    {
      do
      {
        $id =       $db->SelectData(1);
        $port =     $db->SelectData(2);
        $bit =      $db->SelectData(3);
        $ass_name = $db->SelectData(4);
        $dev_name = $db->SelectData(5);
        if($count%2)
          $color="#D0D0FF";
        else
          $color="#D0FFD0";
        ?>
          <tr>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_assign.php', '?id=<? echo "$id"; ?>&port=<? echo "$port"; ?>&bit=<? echo "$bit"; ?>', 'Assign' )">
                <? printf("%s", $dev_name); ?>
              </a>
            </td>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_assign.php', '?id=<? echo "$id"; ?>&port=<? echo "$port"; ?>&bit=<? echo "$bit"; ?>', 'Assign' )">
                <?
                  if( substr($port, 0, 1) == 'I')      printf("Entrada %s", substr($port, 1, 1));
                  else if( substr($port, 0, 1) == 'O') printf("Salida %s", substr($port, 1, 1));
                  else if( substr($port, 0, 1) == 'A') printf("Analog %s", substr($port, 1, 1));
                ?>
              </a>
            </td>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_assign.php', '?id=<? echo "$id"; ?>&port=<? echo "$port"; ?>&bit=<? echo "$bit"; ?>', 'Assign' )">
                <? 
                  if(substr($port, 0, 1) == 'A')  echo "&nbsp";
                  else                            echo "$bit";
                ?>
              </a>
            </td>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_assign.php', '?id=<? echo "$id"; ?>&port=<? echo "$port"; ?>&bit=<? echo "$bit"; ?>', 'Assign' )">
                <? echo "$ass_name"; ?>
              </a>
            </td>
          </tr>
        <?
        $count++;
      } while($db->SelectNext());
    }
    </script>
    <tr>
      <td bgcolor="#FFD0D0">&nbsp;</td>
      <td bgcolor="#FFD0D0">&nbsp;</td>
      <td bgcolor="#FFD0D0">&nbsp;</td>
      <td bgcolor="#FFD0D0" align="right"><a href="javascript:OpenABM( 'abm_assign.php', '', 'Assign' )">Nuevo</a></td>
    </tr>
  </table>
</form>

<? } else if( $folder == 3) { ?>

<form name ="groups" method=POST action="abm_group.php">
  <table border="0" width="80%" bgcolor="#FFFFFF" align=center>
  <tr>
  <td bgcolor="#FFD0D0"><b>Grupo</b></td>
  <td bgcolor="#FFD0D0" align=center><b>Miembros</b></td>
  </tr>
    <script language="php">
    // Listado de asignaciones
    $count = 0;
    if($db->Select( "id, group_name", // SELECT
                    "group_list", // FROM
                    "", // WHERE
                    "group_list.group_name", // ORDER BY
                    "")) // GROUP BY
    {
      do
      {
        $id =         $db->SelectData(1);
        $group_name = $db->SelectData(2);
        if($db2->Select( "count(*)", // SELECT
                      "group_rel", // FROM
                      "group_rel.group_id = ".$id, // WHERE
                      "", // ORDER BY
                      "group_rel.group_id")) // GROUP BY
        {
          $members =    $db2->SelectData(1);
        }
        else
        {
          $members = 0;
        }
        if($count%2)
          $color="#D0D0FF";
        else
          $color="#D0FFD0";
        ?>
          <tr>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_group.php', '?id=<? echo "$id"; ?>', 'Group' )">
                <? printf("%s", $group_name); ?>
              </a>
            </td>
            <td bgcolor="<? echo $color; ?>" align="center">
              <a class=tablelink  href="javascript:OpenABM( 'abm_group.php', '?id=<? echo "$id"; ?>', 'Group' )">
                <? printf("%d", $members); ?>
              </a>
            </td>
          </tr>
        <?
        $count++;
      } while($db->SelectNext());
    }
    </script>
    <tr>
      <td bgcolor="#FFD0D0">&nbsp;</td>
      <td bgcolor="#FFD0D0" align="right"><a href="javascript:OpenABM( 'new_group.php', '', 'Group' )">Nuevo</a></td>
    </tr>
  </table>
</form>

<? } else if( $folder == 4) { ?>

<form name ="events" method=POST action="abm_event.php">
  <table border="0" width="80%" bgcolor="#FFFFFF" align=center>
  <tr>
  <td bgcolor="#FFD0D0"><b>Evento</b></td>
  <td bgcolor="#FFD0D0">&nbsp;</td>
  <td bgcolor="#FFD0D0">&nbsp;</td>

  <td bgcolor="#FFD0D0"><b>Acci&oacute;n</b></td>
  <td bgcolor="#FFD0D0">&nbsp;</td>
  </tr>
  <tr>
  <td bgcolor="#FFD0D0"><b>Nombre</b></td>
  <td bgcolor="#FFD0D0"><b>On/Off</b></td>

  <td bgcolor="#FFD0D0">&nbsp;</td>

  <td bgcolor="#FFD0D0"><b>Cmd</b></td>
  <td bgcolor="#FFD0D0"><b>Descripci&oacute;n</b></td>
  </tr>
    <script language="php">
    // Listado de eventos
    $count = 0;
    if($db->Select("ev.id, ev.src_dev, ev.src_port, ev.src_bit, ev.src_on_off, ev.dst_cmd, ev.event_name, ev.enable, ass.assign_name, cmd.cmd_name", // SELECT
                   "event as ev, assign as ass, comando as cmd", // FROM
                   "ev.src_dev = ass.id AND ev.src_port = ass.port AND ev.src_bit = ass.bit AND ev.dst_cmd = cmd.id", // WHERE
                   "ev.src_dev, ev.src_port, ev.src_bit, ev.src_on_off",   // ORDER BY
                   ""))                                        // GROUP BY
    {
      do
      {
        $id         = $db->SelectData(1);
        $src_dev    = $db->SelectData(2);
        $src_port   = $db->SelectData(3);
        $src_bit    = $db->SelectData(4);
        $src_on_off = $db->SelectData(5);
        $dst_cmd    = $db->SelectData(6);
        $ev_name    = $db->SelectData(7);
        $enable     = $db->SelectData(8);
        $src_name   = $db->SelectData(9);
        $cmd_name   = $db->SelectData(10);
        if($count%2)
          $color="#D0D0FF";
        else
          $color="#D0FFD0";
        ?>
          <tr>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_event.php', '?id=<? echo "$id"; ?>', 'Event' )">
                <?
                  if($enable == 0) echo "<font color=\"#F00000\">";
                  echo "$src_name";
                  if($enable == 0) echo "</font>";
                ?>
              </a>
            </td>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_event.php', '?id=<? echo "$id"; ?>', 'Event' )">
                <?
                  if($enable == 0) echo "<font color=\"#F00000\">";
                  if($src_on_off == 1){echo "On";}else{echo "Off";}
                  if($enable == 0) echo "</font>";
                ?>
              </a>
            </td>

            <td bgcolor="#FFD0D0">&nbsp;</td>

            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_event.php', '?id=<? echo "$id"; ?>', 'Event' )">
                <?
                  if($enable == 0) echo "<font color=\"#F00000\">";
                  echo "$cmd_name";
                  if($enable == 0) echo "</font>";
                ?>
              </a>
            </td>
            <td bgcolor="<? echo $color; ?>">
              <a class=tablelink  href="javascript:OpenABM( 'abm_event.php', '?id=<? echo "$id"; ?>', 'Event' )">
                <?
                  if($enable == 0) echo "<font color=\"#F00000\">";
                  echo "$ev_name";
                  if($enable == 0) echo "</font>";
                ?>
              </a>
            </td>
          </tr>
        <?
        $count++;
      } while($db->SelectNext());
    }
    </script>
    <tr>
      <td bgcolor="#FFD0D0">&nbsp;</td>
      <td bgcolor="#FFD0D0">&nbsp;</td>

      <td bgcolor="#FFD0D0">&nbsp;</td>

      <td bgcolor="#FFD0D0">&nbsp;</td>
      <td bgcolor="#FFD0D0" align="right"><a href="javascript:OpenABM( 'abm_event.php', '', 'Event' )">Nuevo</a></td>
    </tr>
  </table>
</form>

<? } ?>

<script language="php">
  if( $db->IsOpen() ) $db->Close();
  if( $db2->IsOpen() ) $db2->Close();
</script>

</body>
</html>
