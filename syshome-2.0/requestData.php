<script language="php">

$page = $_REQUEST['page'];
if( empty($page) ) $page = 1; 

$cmd = "/usr/bin/gmh_syshome_cgi -h spawn.witchblade -s web_object_info $page";
exec($cmd, $output);
$output = implode("", $output);


header("Content-Type: text/xml");
echo $output;

</script>
