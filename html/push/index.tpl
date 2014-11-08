<html><head><title>WiFi connection</title>
<link rel="stylesheet" type="text/css" href="/style.css">
<script type="text/javascript" src="/js/jquery-2.1.1.min.js"></script>
<script type="text/javascript">
	if (!!window.EventSource) {
	  var source = new EventSource('/push/listen.push');

	  source.addEventListener('message', function(e) {
	  		$("#messages").append(e.data + "<br /> \n\r");
		}, false);

		source.addEventListener('open', function(e) {
		  $("#messages").append("Connection opened<br /> \n\r");
		}, false);

		source.addEventListener('error', function(e) {
		  if (e.readyState == EventSource.CLOSED) {  }
		}, false);


		$(function() {
				$('form#msgform').submit(function(event) {
					event.preventDefault();
					$.post( '/push/index.cgi', $('form#msgform').serialize(), function(data) { });
				});
			});
		
	} else {
	  // Result to xhr polling :(
	}
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
		<h2>Push examples:</h2>

	<div class="section">
		
		<p>Send push message</p>
		<form name="msg" id="msgform" method="post">
		<p>
			<textarea name="text"></textarea> <br />
			<input id="submitbtn" type="submit" name="submitbtn" value="Connect!">
		</p> 
		</form> <br />
	</div>


	<div class="section">
		<p>Raw Feedback from: /push/listen.push </p> <br />
		<div id="messages"></div>
	</div>


</div>
</body>
</html>
