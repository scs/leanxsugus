var configuration = new Object;
var statistics = new Object;

// This function replaces the text contained by the element referenced by the given id with the given text.
function setElementText(id, text) {
	document.getElementById(id).firstChild.nodeValue = text
}

// This function sets the class element of the element referenced by the given id.
function setElementClass(id, name) {
	document.getElementById(id).className = name
}

// This function loads the main page.
function loadIndex() {
	location.href = 'index.html';
}

// This function does some magic to get an XMLHttpRequest object even in some archaic browsers.
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
		// Internet Exploder
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

// This function tries to inhibit selection of text by the user. It's a bit of magic and does not work everywhere.
function disableSelectionEverywhere() {
	// This function returns a list of all nodes recursively contained by the given node.
	function allNodes(elem) {
		var nodes = new Array(elem);
		
		// Recursively collect all objects from all child-elements.
		for (var i in elem.childNodes)
			nodes = nodes.concat(allNodes(elem.childNodes[i]));
		
		return nodes;
	}
	
	// This function sets some attributes of an element so it's text may not be selected.
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
	
	// Loop over all HTML object and disable selection for them.
	for (var i in nodes)
		disableSelection(nodes[i]);
}

// This initializes the configuration object and sends all default values to the camera.
function initConfig() {	
	configUnit_insist("reset_counters");
	configBool_set("calibrating", false);
	configBool_set("sort_color_0", false);
	configBool_set("sort_color_1", false);
	configBool_set("sort_color_2", false);
	configBool_set("sort_color_3", false);
}


// This initializes the statistics objects so the interface has something reasonable to display.
function initStatistics() {
	statistics.count_color_0 = 0;
	statistics.count_color_1 = 0;
	statistics.count_color_2 = 0;
	statistics.count_color_3 = 0;
	statistics.count_sorted = 0;
}

// This function is used to toggle a boolean configuration variable.
function configBool_toggle(name) {
	configuration[name] = ! configuration[name];
	
	sendConfig(name, configuration[name]);
	updateInterface();
}

// This function is used to set a boolean configuration variables.
function configBool_set(name, value) {
	configuration[name] = value;
	
	sendConfig(name, value);
	updateInterface();
}

// This function is used to reset a unit configuration variable (ie. a trigger).
function configUnit_insist(name) {
	sendConfig(name);
}

// This function sends the given variable and value to the camera.
function sendConfig(name, value) {
	var message;
	var xmlHttp = getHTTPObject();
	
	// Construct the message.
	if (value == null)
		message = name + "\n";
	else
		message = name + "=" + value + "\n";
	
	// Send the message over an asynchronous HTTP POST request.
	xmlHttp.open('POST', '/cgi-bin/config.cgi', true);
	xmlHttp.setRequestHeader("Content-length", message.length);
	xmlHttp.send(message);
}

// This function calls itself to update the statistics object two times a second
function getStatistics() {
	var xmlHttp = getHTTPObject();
	
	// This function is called when the request completes.
	function process() {
		if (xmlHttp.readyState == 4) {
			var vars = xmlHttp.responseText.split("\n");
			
			// Read the variales in the recieved statistics file.
			for (var i in vars) {
				var ii = vars[i].split("=");
				
				statistics[ii[0]] = ii[1];
			}
			
			// Update the interface.
			updateInterface();
		}
	}
	
	// Send the request.
	xmlHttp.open('GET', '/statistics.txt', true);
	xmlHttp.onreadystatechange = process;
	xmlHttp.send("");
	
	// Recurse.
	setTimeout("getStatistics();", 500);
}

// Thos functio updates the interface according to the configuration and statistics file.
function updateInterface() {
	var classes = new Object;
	var count_total = 0;
	
	// This is used to set the CSS class of the buttons according to their state.
	classes[false] = "inaktiv";
	classes[true] = "aktiv";
	
	// Set button states.
	setElementClass("sort_color_0", classes[configuration.sort_color_0]);
	setElementClass("sort_color_1", classes[configuration.sort_color_1]);
	setElementClass("sort_color_2", classes[configuration.sort_color_2]);
	setElementClass("sort_color_3", classes[configuration.sort_color_3]);
	
	// Set the counters.
	setElementText("count_color_0", statistics.count_color_0);
	setElementText("count_color_1", statistics.count_color_1);
	setElementText("count_color_2", statistics.count_color_2);
	setElementText("count_color_3", statistics.count_color_3);
	
	// Compute the total of objects.
	count_total += parseInt(statistics.count_color_0, 10)
	count_total += parseInt(statistics.count_color_1, 10)
	count_total += parseInt(statistics.count_color_2, 10)
	count_total += parseInt(statistics.count_color_3, 10)
	
	// Set the total and sorted counter.
	setElementText("count_sorted", statistics.count_sorted);
	setElementText("count_total", count_total);
}
