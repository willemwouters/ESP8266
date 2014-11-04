<html><head><title>WiFi connection</title>
<link rel="stylesheet" type="text/css" href="style.css">
<script type="text/javascript" src="/js/jquery-2.1.1.min.js"></script>
<script type="text/javascript">

var currAp="%currSsid%";

function scanAPs() {
	$.get( "wifiscan.cgi", function( data ) {
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
<div id="main">
<p>
Current WiFi mode: %WiFiMode%
</p>
<form name="wifiform" action="connect.cgi" method="post">
<p>
To connect to a WiFi network, please select one of the detected networks...<br>
<div id="aps">Scanning...</div>
<br>
WiFi password, if applicable: <br />
<input type="text" name="passwd" val="%WiFiPasswd%"> <br />
<input type="submit" name="connect" value="Connect!">
</p>
</div>
</body>
</html>
