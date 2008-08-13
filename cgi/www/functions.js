var configuration = new Object;

function setElementText(id, text) {
	document.getElementById(id).firstChild.nodeValue = text
}

function setElementClass(id, name) {
	document.getElementById(id).className = name
}

function getHTTPObject() {
	var xmlHttp;
	
	try {
		// Firefox, Opera 8.0+, Safari
		xmlHttp = new XMLHttpRequest();
		if (xmlHttp.overrideMimeType) {
			// set type accordingly to anticipated content type
			xmlHttp.overrideMimeType('text/html');
		}
	} catch (e) {
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
	
	return xmlHttp;
}

function onLoad() {
	initConfig();
	updateConfig();
}

function initConfig() {
	configuration.sort_color_0 = false;
	configuration.sort_color_1 = false;
	configuration.sort_color_2 = false;
	configuration.sort_color_3 = false;
	
	configuration.count_color_0 = 0;
	configuration.count_color_1 = 0;
	configuration.count_color_2 = 0;
	configuration.count_color_3 = 0;
	
	configuration.count_sorted = 0;
	configuration.count_total = 0;
}

function configBool_toggle(name) {
	configuration[name] = ! configuration[name];
	sendConfig(name, configuration[name]);
	
	readConfig();
}

function configUnit_insist(name) {
	sendConfig(name);
}

function sendConfig(name, value) {
	var message;
	var xmlHttp = getHTTPObject();
	
	if (value == null)
		message = name + "\n";
	else
		message = name + "=" + value + "\n";
	
//	xmlHttp.open('POST', 'http://localhost:1234/cgi-bin/config.cgi', true);
	xmlHttp.open('POST', '/cgi-bin/config.cgi', true);
	xmlHttp.setRequestHeader("Content-length", message.length);
	xmlHttp.send(message);
}

function updateConfig() {
	var xmlHttp = getHTTPObject();
	
	function process() {
		if (xmlHttp.readyState == 4) {
			var vars = xmlHttp.responseText.split("\n");
			
			for (var i in vars) {
				var ii = vars[i].split("=");
				
				configuration[ii[0]] = ii[1];
			}
			
			readConfig();
		}
	}
	
	xmlHttp.open('GET', '/statistics.txt', true);
	xmlHttp.onreadystatechange = process;
	xmlHttp.send("");
	
	setTimeout("updateConfig();", 500);
}

function readConfig() {
	var classes = new Object;
	classes[false] = "inaktiv";
	classes["false"] = "inaktiv";
	classes[true] = "aktiv";
	classes["true"] = "aktiv";
	
//	setElementText("title", configuration.sort_color_0);
	
	setElementClass("sort_color_0", classes[configuration.sort_color_0]);
	setElementClass("sort_color_1", classes[configuration.sort_color_1]);
	setElementClass("sort_color_2", classes[configuration.sort_color_2]);
	setElementClass("sort_color_3", classes[configuration.sort_color_3]);
	
	setElementText("count_color_0", configuration.count_color_0);
	setElementText("count_color_1", configuration.count_color_1);
	setElementText("count_color_2", configuration.count_color_2);
	setElementText("count_color_3", configuration.count_color_3);
	
	setElementText("count_sorted", configuration.count_sorted);
	setElementText("count_total", configuration.count_color_0 + configuration.count_color_1 + configuration.count_color_2 + configuration.count_color_3);
}
