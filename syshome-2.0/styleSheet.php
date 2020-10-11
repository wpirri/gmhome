<style type="text/css">

#divMenuBg {  
    position: absolute;  
    top: 10px;  
    left: 20px;  
    font: 12px verdana,arial,helvetica;  
		/*color: #0000cc;*/  
    background: #e0e0e0;  
    border: 0;
    z-index: 1;
	width:1510px;
	height:50px;
}  

#divMenu {  
    position: absolute;  
    top: 7px;  
    left: 10px;  
    font: 12px verdana,arial,helvetica;  
		/*color: #0000cc;*/  
    /*background: #55ee55;*/  
    border: 0;
    z-index: 20;  
}  
  
#divMainView {  
    position: absolute;  
    top: 60px;  
    left: 10px;  
    border: none;  
    z-index: 1;  
}  

#divPopupWin {  
	position: absolute;
	background:none repeat scroll 0 0 #FFFFFF;
	border:2px solid #9BA9C6;
	top: 100px;  
	left: 50px;  
	width:600px;
	height:400px;
	z-index: 100;  
}  

#divPopupTop {
	border-bottom:1px solid #9BA9C6;
	height:30px;
	margin-bottom:5px;
}

#divPopupContent {
	clear:both;
	overflow:auto;
	width:600px;
	height:360px;
    font: 12px verdana,arial,helvetica;  
}

<script language="php">
	$cmd = "/usr/bin/gmh_syshome_cgi -h spawn.witchblade -s web_object page=0";
	exec($cmd, $output);
	$output = implode("", $output);
	$p = xml_parser_create();
	xml_parser_set_option($p, XML_OPTION_CASE_FOLDING, 0);
	xml_parser_set_option($p, XML_OPTION_SKIP_WHITE, 1);
	$rc = xml_parse_into_struct($p, $output, $vals, $index);

	if($rc == 1)
	{
//	$css = fopen("css/plantaFrente.css", "w+");
		for( $i = 0; $i < count($vals);$i++ )    
		{
			if($vals[$i]['type'] == "close")
			{
				if($vals[$i]['tag'] == "item")
				{
					// fin de un item
//				fwrite($css, '#'.$object.' { position: absolute; left: '.$xpos.'px; top: '.$ypos.'px; z-index: 10 }'."\n");
					echo '#'.$idtag.' { position: absolute; left: '.$xpos.'px; top: '.$ypos.'px; z-index: 10 }'."\n";
					$idtag = " ";
					$xpos = " ";
					$ypos = " ";
				}
			}
			else if($vals[$i]['type'] == "complete")
			{
				if($vals[$i]['tag'] == "idtag")
				{
					$idtag = $vals[$i]['value'];
				}
				else if($vals[$i]['tag'] == "xpos")
				{
					$xpos = $vals[$i]['value'];
				}
				else if($vals[$i]['tag'] == "ypos")
				{
					$ypos = $vals[$i]['value'];
				}
			}
		}
//	fclose($css);
	}
	xml_parser_free($p);
</script>
</style>
