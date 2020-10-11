<script language="php">

include('displayTemp.php');

$page = $_REQUEST['page'];
if( empty($page) ) $page = 1; 

$cmd = "/usr/bin/gmh_syshome_cgi -h spawn.witchblade -s web_object page=$page";
exec($cmd, $output);
$output = implode("", $output);
$p = xml_parser_create();
xml_parser_set_option($p, XML_OPTION_CASE_FOLDING, 0);
xml_parser_set_option($p, XML_OPTION_SKIP_WHITE, 1);
$rc = xml_parse_into_struct($p, $output, $vals, $index);

if($rc == 1)
{
	for( $i = 0; $i < count($vals);$i++ )    
	{
		//echo "[$i] type: ".$vals[$i]['type']." - tag: ".$vals[$i]['tag']." - value: ".$vals[$i]['value']."\n";
		if($vals[$i]['type'] == "close")
		{
			if($vals[$i]['tag'] == "item")
			{
				// fin de un item
				// hay que ver si es un elemento digital o uno analógico
				if($port[0] == "A")
				{
					DisplayTemp($idtag);
				}
				else
				{
					echo '<div id="div'.$idtag.'">';
					if( !empty($atag) ) { echo '<a href="'.$atag.'">'; }
					echo '<img id="'.$idtag.'" src="'.$src.'" border="0"';
					if( !empty($name) ) { echo ' onmouseover="document.'.$idtag.'.title=\''.$name.'\'"'; }
					echo '>';
					if( !empty($atag) ) { echo '</a>'; }
					echo '</div>';
				}
				// Blanqueo los datos
				$idtag = "";
				$atag = "";
				$src = "";
				$name = "";
				$port = " ";
			}
		}
		else if($vals[$i]['type'] == "complete")
		{
			if($vals[$i]['tag'] == "idtag")
			{
				$idtag = $vals[$i]['value'];
			}
			else if($vals[$i]['tag'] == "atag")
			{
				$atag = $vals[$i]['value'];
			}
			else if($vals[$i]['tag'] == "src_unk")
			{
				$src = $vals[$i]['value'];
			}
			else if($vals[$i]['tag'] == "name")
			{
				$name = $vals[$i]['value'];
			}
			else if($vals[$i]['tag'] == "port")
			{
				$port = $vals[$i]['value'];
			}
		}
	}
}
xml_parser_free($p);
</script>
