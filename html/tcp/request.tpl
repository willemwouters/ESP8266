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
	<ul>
		<li><a href="/wifi">WIFI - Config</a></li>
		<li><a href="/wifi/status.tpl">WIFI - Status</a></li>
		<li><a href="/tcp/request.tpl">TCP Server - HTTP Request</a></li>
		<li><a href="/tcp/serverconfig.tpl">TCP Server - Config</a></li>
		<li><a href="/tcp/serverstatus.tpl">TCP Server - Status</a></li>
		<li><a href="/push/index.tpl">HTML5 Push examples IO</a></li>
	</ul>
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
</div>

<div id="console">
	
</div>


</body></html>
