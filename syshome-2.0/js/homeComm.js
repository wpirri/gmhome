

function requestMainView() {
	if( $("trafic_lamp") ) { $("trafic_lamp").src = "images/yellow.gif"; }
	new Ajax.Request("requestData.php?page=10", { onSuccess: updateMainView, onFailure: updateMainViewError } );
}

function updateMainViewError(resp) {
	if( $("trafic_lamp") ) { $("trafic_lamp").src = "images/red.gif"; }
	// Para que lo vuelva a pedir pero en 30 segundos
	setTimeout("requestMainView()",30000);
}

function updateMainView(resp) {
	// resp es XMLDocument
	var xmlData = resp.responseXML;
	var hwData = xmlData.getElementsByTagName('device');
	var hwPort = new Array();
	var hwElement;
	var port = '';
	var i = 0;
	var j = 0;
	var k = 0;
	var l = 0;
	var x = 0;

	// Para que lo vuelva a pedir
	setTimeout("requestMainView()",2000);

	while (hwData[i]) {
		hwPort = hwData[i].getElementsByTagName('port');
		j = 0;
		while (hwPort[j]) {
			port = hwPort[j].getElementsByTagName('name')[0].textContent;
			if (port[0] == 'I' || port[0] == 'O') {
				l = 0;
				while ( (hwPort[j].getElementsByTagName('data'))[l] ) {
					hwElement = new Object();
					hwElement.disp = (hwData[i].getElementsByTagName('address'))[0].textContent;
					hwElement.port = port;
					hwElement.bit = ((hwPort[j].getElementsByTagName('data'))[l].getElementsByTagName('bit'))[0].textContent;
					hwElement.value = ((hwPort[j].getElementsByTagName('data'))[l].getElementsByTagName('value'))[0].textContent; 
					gHwArray[k] = hwElement;
					l++;
					k++;
				}
			} else if (port[0] == 'A') {
				hwElement = new Object();
				hwElement.disp = (hwData[i].getElementsByTagName('address'))[0].textContent;
				hwElement.port = port;
				hwElement.bit = 0;
				hwElement.value = ((hwPort[j].getElementsByTagName('data'))[0].getElementsByTagName('value'))[0].textContent; 
				gHwArray[k] = hwElement;
				k++;
			}
			j++;
		}
		i++;
	}
	// datos cargados en gHwArray
	i = 0;
	while( gImgArray[i] ) {
		j = 0;
		while( gHwArray[j] ) {
			if( gImgArray[i].disp == gHwArray[j].disp &&
			    gImgArray[i].port == gHwArray[j].port &&
			    gImgArray[i].bit == gHwArray[j].bit    ) {
				if(gImgArray[i].port[0] == 'I' || gImgArray[i].port[0] == 'O') {
					if(gHwArray[j].value == '1') {
						$(gImgArray[i].idtag).src = gImgArray[i].src_on;
					} else if(gHwArray[j].value == 0) {
						$(gImgArray[i].idtag).src = gImgArray[i].src_off;
					} else {
						$(gImgArray[i].idtag).src = gImgArray[i].src_unk;
					}
				} else if (gImgArray[i].port[0] == 'A') {
					x = gHwArray[j].value * 5;
					$(gImgArray[i].idtag + '_decimas').src = gImgNumber[x % 10];
					x = Math.floor(x / 10);
					$(gImgArray[i].idtag + '_unidades').src = gImgNumber[x % 10];
					x = Math.floor(x / 10);
					$(gImgArray[i].idtag + '_decenas').src = gImgNumber[x % 10];
				}
				break;
			}
			j++;
		}
		i++;
	}
	if( $("trafic_lamp") ) { $("trafic_lamp").src = "images/green.gif"; }
}

function commandOk(resp) {
//	alert(resp.responseText);
}

function commandSwitch(disp, port, bit) {
	new Ajax.Request("requestCommand.php?cmd=switch&disp=" + disp + "&port=" + port + "&bit=" + bit, { onSuccess: commandOk } );
}

function commandPulse(disp, port, bit, time) {
	new Ajax.Request("requestCommand.php?cmd=pulse&disp=" + disp + "&port=" + port + "&bit=" + bit + "&time=" + time, { onSuccess: commandOk } );
}

