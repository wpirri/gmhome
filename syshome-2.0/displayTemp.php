<script language="php">
function DisplayTemp($imageId)
{
	if( empty($imageId) ) $imageId = 'default'; 
	$imagePath = 'images/'; 
	?>
	<div id="<? echo $imageId ?>">
	<img id="<? echo $imageId ?>_decenas" src="<? echo $imagePath ?>c0.gif" border="0"><img id="<? echo $imageId ?>_unidades" src="<? echo $imagePath ?>c0.gif" border="0"><img src="<? echo $imagePath ?>dot.gif" border="0"><img id="<? echo $imageId ?>_decimas" src="<? echo $imagePath ?>c0.gif" border="0"><img src="<? echo $imagePath ?>cgrados.gif" border="0">
	</div>
	<?
}
</script>