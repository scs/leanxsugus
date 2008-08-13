var xmlHttp = null;
var isIE = false;
var counter = 0;

function elem(elemIdentString) {
	return document.getElementById(elemIdentString);
}

function getHTTPObject() {
	try {
		// Firefox, Opera 8.0+, Safari
		xmlHttp = new XMLHttpRequest();
		if (xmlHttp.overrideMimeType) {
			// set type accordingly to anticipated content type
			xmlHttp.overrideMimeType('text/html');
		}
	} catch (e) {
		// Internet Explorer
		isIE = true;
		
		var msxmlhttp = new Array('Msxml2.XMLHTTP.5.0', 'Msxml2.XMLHTTP.4.0', 'Msxml2.XMLHTTP.3.0', 'Msxml2.XMLHTTP', 'Microsoft.XMLHTTP');
		for (var i = 0; i < msxmlhttp.length; i += 1) {
			try {
				xmlHttp = new ActiveXObject(msxmlhttp[i]);
			} catch (e) {
				xmlHttp = null;
			}
		}
		
		if (xmlHttp == null)
			alert("Your browser does not support AJAX!");
	}
}

function onLoad() {
//	updateData();
	refreshImage();
}

function refreshImage() {
	counter += 1;
	elem("camimage").src = "/image.bmp?" + counter;
	
	setTimeout("refreshImage()", 500);
}

function updateData() {
	var parameters = "";
	
	getHTTPObject();
	
	if (elem("captureColor"))
		parameters = "DoCaptureColor=" + elem("captureColor").checked;
	
	xmlHttp.open('POST', '/cgi-bin/leanxsugus.cgi', true);
	xmlHttp.onreadystatechange = useHttpResponse;
	xmlHttp.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
//	xmlHttp.setRequestHeader("Content-length", parameters.length);
//	xmlHttp.setRequestHeader("Connection", "close");
	xmlHttp.send(parameters);
}

function useHttpResponse() {
	if (xmlHttp.readyState == 4) {
		var response = xmlHttp.responseText.split('\n');
		var i = 0;
		var arg;
		var argSplit;
		
		// Separate the different parameters of the response.
		while (response[i]) {
			// Separate parameter name and parameter value
			arg = response[i];
			argSplit = arg.split('=');
			
			// Depending on the parameter name, invoke a different action.
			switch (argSplit[0]) {
			case "imgTS":
				elem("camimage").src = "/image.bmp?" + argSplit[1];
			/*	if (elem("camimage"))
					elem("camimage").src = "/img.bmp?" + argSplit[1];
				else
					window.location="index.html";
				break; */
			default:
				// Something unexpected received. This may happen if the application has shut down. So redirect to status off.
			//	window.location = "index.html";
				setTimeout("updateData()", 500);
				return;
			}
			
			i += 1;
		}
		setTimeout("updateData()", 1);
	}
}