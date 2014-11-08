<html>
<head><title>Esp8266 web server</title>
<link rel="stylesheet" type="text/css" href="/style.css">
<script type="text/javascript" src="/js/jquery-2.1.1.min.js"></script>
<script type="text/javascript">

function getConsoleData() {
	$.get( "TcpResponse.cgi", function( data ) {
		var innerText = "";
	   	$("#console").append(data);
	});
	window.setTimeout(getConsoleData, 10000);
}

window.onload=function(e) {
	getConsoleData();
};
</script>
</head>
<body>
<div id="menu">
@menu.include@ 
</div>

<div id="main">
	<h2>Tcp Request</h2>
		<form name="apform" action="TcpRequest.cgi" method="post">
		<p>
		 Ip: <input type="text" name="ip" value=""> <br />
		 Port: <input type="text" name="port" value=""> <br />
		 Cmd: <textarea name="cmd">%cmd%</textarea> <br />
			<input type="submit" name="submit" value="Send">
		</p> 
		</form> <br />


<div id="console">
	
</div>

</div>
</body></html>
