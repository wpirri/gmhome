
function openAbmAssign( resp ) {
	$('divPopup').innerHTML = resp.responseText;
	
	new Ajax.Updater('divPopupTop', 'abm_assign.php?title=1');
	new Ajax.Updater('divPopupContent', 'abm_assign.php?data=1');
}

function openPopup( index ) {
	if ( index == 0 ) {
		$('divPopup').innerHTML = '';
	} else if (index == 1) {
		new Ajax.Request('requestPopup.php', { onSuccess: openSysConfig } );
	} else if (index == 2) {
		new Ajax.Request('requestPopup.php', { onSuccess: openAbmUser } );
	} else if (index == 3) {
		new Ajax.Request('requestPopup.php', { onSuccess: openAccessControl } );
	} else if (index == 4) {
		new Ajax.Request('requestPopup.php', { onSuccess: openAbmHardware } );
	} else if (index == 5) {
		new Ajax.Request('requestPopup.php', { onSuccess: openAbmAssign } );
	} else if (index == 6) {
		new Ajax.Request('requestPopup.php', { onSuccess: openMediaControl } );
	} else if (index == 7) {
		new Ajax.Request('requestPopup.php', { onSuccess: openCCDControl } );
	}
}
