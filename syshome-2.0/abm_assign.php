<script language="php">

$title = $_REQUEST['title'];
if( empty($title) ) $title = 0; 
$data = $_REQUEST['data'];
if( empty($data) ) $data = 0; 

header("Content-Type: text/html");

if($title)
{
	echo '<table border="0" width="100%"><tr>';
	echo '<td align="left" width="50%"><marquee behavior="slide" scrolldelay=1 vspace=5 width="100%">&nbsp;&nbsp;Asignaci&oacute;n de dispositivos</marquee></td>';
	echo '<td align="right"><a href="javascript:openPopup(0)"><img src="images/close.png" border="0"></a></td>';
	echo '</tr></table>';
}
else if($data)
{
	echo '<table border="0" width="100%" align="center">';
	$cmd = "/usr/bin/gmh_syshome_cgi -h spawn.witchblade -s list_assign";
	exec($cmd, $output);
	$output = implode("", $output);
	$p = xml_parser_create();
	xml_parser_set_option($p, XML_OPTION_CASE_FOLDING, 0);
	xml_parser_set_option($p, XML_OPTION_SKIP_WHITE, 1);
	$rc = xml_parse_into_struct($p, $output, $vals, $index);
	if($rc == 1)
	{
		$count = 0;
		for( $i = 0; $i < count($vals);$i++ )    
		{
			if($vals[$i]['type'] == "close")
			{
				if($vals[$i]['tag'] == "item")
				{
					// fin de un item
					if($count % 2) $color = '#FFFFFF';
					else $color = '#C0C0C0'; 
					echo '<tr><td bgcolor="'.$color.'">&nbsp;<a href="javascript:editAssign('.$id.','.$port.','.$bit.')"><img border="0" src="images/mini-propiedades.gif"></a>&nbsp;&nbsp;'.$device_name.'</td><td bgcolor="'.$color.'">';
					if( $port[0] == 'I' )
						echo 'Entrada ';
					else if( $port[0] == 'O' )
						echo 'Salida ';
					else if( $port[0] == 'A' )
						echo 'Analogica ';
					else
						echo 'Desconocida ';
					echo $port[1].'</td><td bgcolor="'.$color.'">';
					if( $port[0] == 'A' )
						echo '&nbsp;';
					else
						echo $bit;
					echo '</td><td bgcolor="'.$color.'">'.$assign_name.'</td></tr>';
					// Blanqueo los datos
					$device_name = "";
					$id = "";
					$port = "";
					$bit = "";	
					$assign_name = "";
					$count++;
				}
			}
			else if($vals[$i]['type'] == "complete")
			{
				if($vals[$i]['tag'] == "device_name")
				{
					$device_name = $vals[$i]['value'];
				}
				else if($vals[$i]['tag'] == "id")
				{
					$id = $vals[$i]['value'];
				}
				else if($vals[$i]['tag'] == "port")
				{
					$port = $vals[$i]['value'];
				}
				else if($vals[$i]['tag'] == "bit")
				{
					$bit = $vals[$i]['value'];
				}
				else if($vals[$i]['tag'] == "assign_name")
				{
					$assign_name = $vals[$i]['value'];
				}
			}
		}
	}
	xml_parser_free($p);
	echo '</table>';
}


</script>

