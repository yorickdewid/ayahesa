window.onload = function() {

  // Get references to elements on the page.
  var form = document.getElementById('message-form');
  var messageField = document.getElementById('message');
  var messagesList = document.getElementById('messages');
  var socketStatus = document.getElementById('status');
  var sendBtn = document.getElementById('send');
  var closeBtn = document.getElementById('close');
  var userName = 'Guest';

  // Set the timer
  setInterval(function() {
    if (socket.readyState == 1) {
      socket.send(JSON.stringify({"msg":null}));
    }
  }, 30000);

  // Create a new WebSocket.
  var socket = new WebSocket('ws://' + window.location.hostname + ':8888/endpoint/msg/socket');

  // Handle any errors that occur.
  socket.onerror = function(error) {
    console.log('WebSocket Error: ' + error);
  };

  // Show a connected message when the WebSocket is opened.
  socket.onopen = function(event) {
    socketStatus.innerHTML = 'Connected';
    socketStatus.className = 'open';
  };

  // Handle messages sent by the server.
  socket.onmessage = function(event) {
    if (event.data[0] != '{')
      return;
    var message = JSON.parse(event.data);
    if (message.msg == null)
      return;
    messagesList.innerHTML += '<li class="received"><span>' + message.userName + ':</span>' +
                               message.msg + '</li>';
    messagesList.scrollTop = messagesList.scrollHeight;
  };

  // Show a disconnected message when the WebSocket is closed.
  socket.onclose = function(event) {
    socketStatus.innerHTML = 'Disconnected';
    socketStatus.className = 'closed';
    messageField.disabled = true;
    sendBtn.disabled = true;
    closeBtn.disabled = true;
  };

  // Send a message when the form is submitted.
  form.onsubmit = function(e) {
    e.preventDefault();

    if (socket.readyState != 1)
      return;

    // Retrieve the message from the textarea.
    var message = messageField.value;
    if (message[0] == '/') {
      var res = message.split(" ");
      userName = res[1];
      messageField.value = '';
      return;
    }

    // Send the message through the WebSocket.
    socket.send(JSON.stringify({"userName":userName,"msg":message}));

    // Add the message to the messages list.
    messagesList.innerHTML += '<li class="sent"><span>You:</span>' + message +
                              '</li>';
    messagesList.scrollTop = messagesList.scrollHeight;

    // Clear out the message field.
    messageField.value = '';

    return false;
  };

  // Close the WebSocket connection when the close button is clicked.
  closeBtn.onclick = function(e) {
    e.preventDefault();

    // Close the WebSocket.
    socket.close();

    return false;
  };

};
