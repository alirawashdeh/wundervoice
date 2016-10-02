
var urlAddTask = "https://a.wunderlist.com/api/v1/tasks";
var client_id = "77e7a497276456d896a7";
var config_url = 'https://wunderdictate.herokuapp.com';

var access_token;
var list_name;
var list_id;

Pebble.addEventListener('ready', function() {
  console.log('PebbleKit JS ready!');

  		access_token = localStorage.access_token;
      list_name = localStorage.list_name;
      list_id = localStorage.list_id;

});

Pebble.addEventListener('showConfiguration', function() {
  console.log('Showing configuration page: ' + config_url);

  Pebble.openURL(config_url);
});

Pebble.addEventListener('appmessage',
  function(e) {
    console.log('Received message: ' + JSON.stringify(e.payload));
    console.log(list_id);
    var response = AddItem(e.payload.KEY_MESSAGE);
    console.log(response);

    var success = 0;
    if(response.indexOf('created') != -1)
    {
      success = 1;
    }
    var dict = {
       'KEY_SUCCESS': success,
       'KEY_LIST': list_name
     };

    // Send to watchapp
    Pebble.sendAppMessage(dict, function() {
      console.log('Send successful: ' + JSON.stringify(dict));
    }, function() {
      console.log('Send failed!');
    });


  }
);

function AddItem(text) {
    var req = new XMLHttpRequest();
    req.open("POST", urlAddTask, false);
    req.setRequestHeader("Content-Type", "application/json");
    req.setRequestHeader("X-Access-Token", access_token);
    req.setRequestHeader("X-Client-ID", client_id);
    var json = {
       'list_id': parseInt(list_id),
       'title':text
     };
     console.log(JSON.stringify(json));
    req.send(JSON.stringify(json));
    return req.responseText;
}

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));

   		localStorage.access_token = configData['access_token'];
      localStorage.list_name = configData['list_name'];
      localStorage.list_id = configData['list_id'];
        		access_token = localStorage.access_token;
            list_name = localStorage.list_name;
            list_id = localStorage.list_id;

  var dict = {
     'KEY_CONFIGURED': 1
   };

  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });


});
