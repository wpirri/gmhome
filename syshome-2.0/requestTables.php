<script language="php">

//$page = $_REQUEST['page'];
//if( empty($page) ) $page = 1; 

$cmd = "/usr/bin/gmh_syshome_cgi -h spawn.witchblade -s web_object page=0";
exec($cmd, $output);
$output = implode("", $output);


header("Content-Type: text/xml");
echo $output;

</script>