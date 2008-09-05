var configuration = new Object;
var statistics = new Object;

function setElementText(id, text) {
	document.getElementById(id).firstChild.nodeValue = text
}

function setElementClass(id, name) {
	document.getElementById(id).className = name
}

/* This function does some magic to get an XMLHttpRequest object even in some archaic browsers. */
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

function disableSelectionEverywhere() {
	function allNodes(elem) {
		var nodes = new Array(elem);
		
		for (var i in elem.childNodes)
			nodes = nodes.concat(allNodes(elem.childNodes[i]));
		
		return nodes;
	}
	
	function disableSelection(element) {
		try {
			element.onselectstart = function() {
				return false;
			}
			element.unselectable = "on";
			element.style.MozUserSelect = "none";
		} catch (e) { }
	}
	
	var nodes = allNodes(document);
	
	for (var i in nodes)
		disableSelection(nodes[i]);
}

function onLoad() {
	disableSelectionEverywhere();
	
	initConfig();
	initStatistics();
	
	updateInterface();
	getStatistics();
}

function initConfig() {
	configuration.sort_color_0 = false;
	configuration.sort_color_1 = false;
	configuration.sort_color_2 = false;
	configuration.sort_color_3 = false;
	
	sendConfig("reset_counter");
	sendConfig("sort_color_0", false);
	sendConfig("sort_color_1", false);
	sendConfig("sort_color_2", false);
	sendConfig("sort_color_3", false);
}

function initStatistics() {
	statistics.count_color_0 = 0;
	statistics.count_color_1 = 0;
	statistics.count_color_2 = 0;
	statistics.count_color_3 = 0;
	statistics.count_sorted = 0;
}

function configBool_toggle(name) {
	configuration[name] = ! configuration[name];
	
	sendConfig(name, configuration[name]);
	updateInterface();
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
	
	xmlHttp.open('POST', '/cgi-bin/config.cgi', true);
	xmlHttp.setRequestHeader("Content-length", message.length);
	xmlHttp.send(message);
}

function getStatistics() {
	var xmlHttp = getHTTPObject();
	function process() {
		if (xmlHttp.readyState == 4) {
			var vars = xmlHttp.responseText.split("\n");
			
			for (var i in vars) {
				var ii = vars[i].split("=");
				
				statistics[ii[0]] = ii[1];
			}
			
			updateInterface();
		}
	}
	
	xmlHttp.open('GET', '/statistics.txt', true);
	xmlHttp.onreadystatechange = process;
	xmlHttp.send("");
	
	setTimeout("getStatistics();", 500);
}

function updateInterface() {
	var classes = new Object;
	var count_total = 0;
	
	classes[false] = "inaktiv";
	classes[true] = "aktiv";
	
	setElementClass("sort_color_0", classes[configuration.sort_color_0]);
	setElementClass("sort_color_1", classes[configuration.sort_color_1]);
	setElementClass("sort_color_2", classes[configuration.sort_color_2]);
	setElementClass("sort_color_3", classes[configuration.sort_color_3]);
	
	setElementText("count_color_0", statistics.count_color_0);
	setElementText("count_color_1", statistics.count_color_1);
	setElementText("count_color_2", statistics.count_color_2);
	setElementText("count_color_3", statistics.count_color_3);
	
	count_total += parseInt(statistics.count_color_0, 10)
	count_total += parseInt(statistics.count_color_1, 10)
	count_total += parseInt(statistics.count_color_2, 10)
	count_total += parseInt(statistics.count_color_3, 10)
	
	setElementText("count_sorted", statistics.count_sorted);
	setElementText("count_total", count_total);
}
