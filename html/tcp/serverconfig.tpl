<html><head><title>WiFi connection</title>
<link rel="stylesheet" type="text/css" href="/style.css">
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


	<div class="section">
		<h2>Tcp server state</h2>
		<p>Config TCP server state</p>
		<form name="apform" action="TcpServerEnable.cgi" method="post">
		<p>
			State: <br />
			<input type="radio" name="state" value="1">Enable<br />
			<input type="radio" name="state" value="0">Disable<br />
			<br />
			<input type="submit" name="submit" value="Save" />
		</p> 
		</form> <br />
	</div>

	<div class="section">
		<h2>Tcp server settings</h2>
		<p>Config Tcp server Settings</p>
		<form name="apform" action="TcpServerSave.cgi" method="post">
		<p>
			Type: <br />
			<input type="radio" name="type" value="TCP">TCP<br />
			<input type="radio" name="type" value="UDP">UDP<br />
			<input type="radio" name="type" value="BOTH">Both<br />  <br />
			Port: 
			<input type="text" name="port" value="333" />
			<br />
			<input type="submit" name="submit" value="Save" />
		</p> 
		</form> <br />
	</div>
</div>
</body>
</html>
