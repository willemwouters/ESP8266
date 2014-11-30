<html><head><title>WiFi connection</title>
<link rel="stylesheet" type="text/css" href="/style.css">
<script type="text/javascript" src="/js/jquery-2.1.1.min.js"></script>
<script type="text/javascript">
var currAp="%currSsid%";
function scanAPs() {
	$.get( "WiFiScan.cgi", function( data ) {
		var innerText = "";
		var json = data; //JSON.parse(data);
		if(json.result.inProgress == "0") {
			$.each( json.result.APs, function( key, itemData ) {
				if(itemData.essid != "") {				
         			var tempData = "<input type=\"radio\" name=\"essid\" value=" + itemData.essid + ">" + itemData.essid + "<br>"; 
         			innerText += ( "<li id='" + itemData.essid + "'>" + tempData + "</li> \n" );
	   				$("#aps").html(innerText);
	       		}
			});
		}
		window.setTimeout(scanAPs, 10000);
	});
}

window.onload=function(e) {
	scanAPs();
};
</script>
</head>
<body>
<div id="menu">
@menu.include@ 
</div>

<div id="main">
	<div class="section">
		<h2>Wifi mode</h2>
		<p>Current WiFi mode: %WiFiMode%</p> <br />
	</div>

	<div class="section">
		<h2>Wifi config</h2>
		<p>Config Wifi Connection</p>
		<form name="wifiform" action="WiFiConnect.cgi" method="post">
		<p>
			To connect to a WiFi network, please select one of the detected networks...<br>
			<div id="aps">Scanning...</div>	<br>
			WiFi password, if applicable: <br />
			<input type="text" name="passwd" value="%WiFiPasswd%"> <br />
			<input type="submit" name="connect" value="Connect!">
		</p> 
		</form> <br />
	</div>

	<div class="section">
		<h2>AP</h2>
		<p>Config AP Settings</p>
		<form name="apform" action="WiFiApSave.cgi" method="post">
		<p>
			AP SSID: <input type="text" name="ssid" value="%ssid%"> <br />
			AP PASSWD: <input type="text" name="password" value="%password%"> <br />
			<input type="submit" name="Save" value="Save">
		</p> 
		</form> <br />
</div>
</body>
</html>
