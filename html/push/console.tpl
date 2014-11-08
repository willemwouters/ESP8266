<html><head><title>WiFi connection</title>
<link rel="stylesheet" type="text/css" href="/style.css">
<script type="text/javascript" src="/js/jquery-2.1.1.min.js"></script>
<script type="text/javascript">
	if (!!window.EventSource) {
	  var source = new EventSource('/push/console.push');

	  source.addEventListener('message', function(e) {
	  		$("#messages").prepend(e.data + "<br /> \n\r");
		}, false);

		source.addEventListener('open', function(e) {
		  $("#messages").prepend("Connection opened<br /> \n\r");
		}, false);

		source.addEventListener('error', function(e) {
		  if (e.readyState == EventSource.CLOSED) {  }
		}, false);


	} else {
	  // Result to xhr polling :(
	}
</script>
</head>
<body>
<div id="menu">
@menu.include@ 
</div>

<div id="main">
		<h2>Console output:</h2>


	<div class="section">
		<p>Raw Feedback from: /push/console.push </p> <br />
		<div id="messages"></div>
	</div>


</div>
</body>
</html>
