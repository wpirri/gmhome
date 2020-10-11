<script language="php">

$cmd = $_REQUEST['cmd'];
if( empty($cmd) ) return;

$disp = $_REQUEST['disp'];
if( empty($disp) ) $disp = 0;
$port = $_REQUEST['port'];
if( empty($port) ) $port = 0;
$bit = $_REQUEST['bit'];
if( empty($bit) ) $bit = 0;
$time = $_REQUEST['time'];
if( empty($time) ) $time = 1;

if( $cmd == "switch" )
	$arg = sprintf("web_object_switch dev=%d port=%d bit=%d", $disp, $port, $bit);
else if( $cmd == "pulse" )
	$arg = sprintf("web_object_pulse dev=%d port=%d bit=%d time=%d", $disp, $port, $bit, $time);

$cmd_exec = "/usr/bin/gmh_syshome_cgi -h spawn.witchblade -s ".$arg;

exec($cmd_exec, $output);

echo "$cmd_exec";

</script>