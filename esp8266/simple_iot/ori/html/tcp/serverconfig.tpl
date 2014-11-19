<html><head><title>WiFi connection</title>
<link rel="stylesheet" type="text/css" href="/style.css">
</head>
<body>
<div id="menu">
@menu.include@ 
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
