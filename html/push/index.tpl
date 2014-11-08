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



		var sourceIo = new EventSource('/push/io.push');

	  sourceIo.addEventListener('message', function(e) {
	  		$("#io").append(e.data + "<br /> \n\r");
		}, false);

		sourceIo.addEventListener('open', function(e) {
		  $("#io").append("Connection opened<br /> \n\r");
		}, false);

		sourceIo.addEventListener('error', function(e) {
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
@menu.include@ 
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
		<p>Raw Feedback from: /push/io.push </p> <br />
		<div id="io"></div>
		<p>Raw Feedback from: /push/listen.push </p> <br />
		<div id="messages"></div>
	</div>


</div>
</body>
</html>
