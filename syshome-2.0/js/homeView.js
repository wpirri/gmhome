// Cargo las librerias que necesito
document.write('<script type="text/javascript" src="/js/prototype.js"></script>');
document.write('<script type="text/javascript" src="js/homeAbm.js"></script>');
document.write('<script type="text/javascript" src="js/homeComm.js"></script>');
//document.write('<script type="text/javascript" src="js/homeModel.js"></script>');
//document.write('<link type="text/css" rel="stylesheet" href="css/plantaFrente.css" />');
// pongo la inicialización en el onload del body para que se ejecute
// una vez que se terminó de cargar la página
window.onload=initHomeView;

var gHwArray = new Array();
var gImgArray = new Array();
var gImgNumber = new Array();

//// MENU FLOTANTE
//numero de pixels de separacion con la parte superior de la ventana  
var menuPos = 30;  
var old = menuPos;  

function setMainView(resp) {
	$("divMainView").innerHTML = resp.responseText;
}

function setMainTables(resp) {
	// resp es XMLDocument
	var xmlData = resp.responseXML;
	var itemList = xmlData.getElementsByTagName('item');
	var i = 0;
	var item;

	while ( itemList[i] ) {
		item = new Object();
		item.idtag = itemList[i].getElementsByTagName('idtag')[0].textContent;
		if(itemList[i].getElementsByTagName('disp')[0])
			item.disp = itemList[i].getElementsByTagName('disp')[0].textContent;
		else
			item.disp = 0;
		if(itemList[i].getElementsByTagName('port')[0])
			item.port = itemList[i].getElementsByTagName('port')[0].textContent;
		else
			item.port = 'xx';
		if(itemList[i].getElementsByTagName('bit')[0])
			item.bit = itemList[i].getElementsByTagName('bit')[0].textContent;
		else
			item.bit = 0;
		if(itemList[i].getElementsByTagName('src_unk')[0])
			item.src_unk = itemList[i].getElementsByTagName('src_unk')[0].textContent;
		else
			item.src_unk = 'none';
		if(itemList[i].getElementsByTagName('src_on')[0])
			item.src_on = itemList[i].getElementsByTagName('src_on')[0].textContent;
		else
			item.src_on = 'none';
		if(itemList[i].getElementsByTagName('src_off')[0])
			item.src_off = itemList[i].getElementsByTagName('src_off')[0].textContent;
		else
			item.src_off = 'none';
		
		gImgArray[i] = item;
		
		i++;
	}
	gImgNumber[0] = 'images/c0.gif';
	gImgNumber[1] = 'images/c1.gif';
	gImgNumber[2] = 'images/c2.gif';
	gImgNumber[3] = 'images/c3.gif';
	gImgNumber[4] = 'images/c4.gif';
	gImgNumber[5] = 'images/c5.gif';
	gImgNumber[6] = 'images/c6.gif';
	gImgNumber[7] = 'images/c7.gif';
	gImgNumber[8] = 'images/c8.gif';
	gImgNumber[9] = 'images/c9.gif';
	// Pido datos cada 5 segundos
	//setInterval("requestMainView()",5000);
	setTimeout("requestMainView()",500);
}

function initHomeView() {
	new Ajax.Updater("divMenu", "mainMenu.html");
	new Ajax.Request("mainView.php?page=10", { onSuccess: setMainView } );
	new Ajax.Updater("divDinObj", "dinamicObjects.php?page=10");
	new Ajax.Request("requestTables.php", { onSuccess: setMainTables } );

	// MENU FLOTANTE
    // inicia el proceso que mantiene la posicion  
    movemenu();  
}

// MENU FLOTANTE
// MOVIMIENTO  
function movemenu() {  
    if (window.innerHeight) {  
    	pos = window.pageXOffset;  
    } else if (document.documentElement && document.documentElement.scrollLeft) {  
		pos = document.documentElement.scrollLeft;  
    } else if (document.body) {  
        pos = document.body.scrollLeft;  
    }  
      
    if (pos < menuPos)  
        pos = menuPos;  
    else  
        pos += 10;  

    if (pos == old) {
    	$('divMenu').style.left = pos + 'px';
    	$('divPopup').style.left = (pos + 10) + 'px';
    }
  
    old = pos;
      
    setTimeout('movemenu()',200);  
}  
