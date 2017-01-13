// Import the Clay package
var Clay = require('pebble-clay');
// Load our Clay configuration file
var clayConfig = require('./config');
// Initialize Clay
var clay = new Clay(clayConfig);

function getPlaying(callback){
  var claySetting = JSON.parse(localStorage.getItem('clay-settings'));  
  var url = claySetting.host + '/list?token=' + claySetting.token + '&id=' + claySetting.id;
  var req = new XMLHttpRequest();  
  console.log("Get playing information from: " + url);  
  
  req.onreadystatechange = function(e) {
    if (req.readyState == 4 && req.status == 200) {
      var response = JSON.parse(req.responseText);
      if (response !== undefined && !response.error) {
        if (response.length > 0 && response[0] !== undefined)
          callback(response[0].current_playing);
        callback(null);
      } else {
        if (response.error){
          console.log("Error get playing. message: " + response.error.message);
          Pebble.showSimpleNotificationOnPebble("Set volume failed", response.error.message);
        } else {
          console.log("Error get playing. response text: " + req.responseText);
          Pebble.showSimpleNotificationOnPebble("Get playing failed", "Error on read playing information");
        }
      }
    } else if (req.status == 404 || req.status == 500) {
      console.log("Get playing failed. response code: " + req.status);
      Pebble.showSimpleNotificationOnPebble("Get playing failed", "Responsecode: " + req.status);
    }
  };
  
  req.timeout = 4000; // 4 seconds timeout
  req.open("GET", url);
  req.send();  
}

function sendPlaying(playing){
  console.log('Send playing: ' + playing);
  var dict;
  if (playing !== undefined && playing !== null){    
    dict = { 'ARTIST': playing.artist, 'TITLE': playing.title };
  } else {
    dict = { 'ARTIST': 'Nothing played', 'TITLE': '' };
  }
  // Send the object
  Pebble.sendAppMessage(dict, function() {
    console.log('Playing sent successfully: ' + JSON.stringify(dict));
  }, function(e) {
    console.log('Playing sent failed: ' + JSON.stringify(e));
  });
}

function update_volume(message) {
  var claySetting = JSON.parse(localStorage.getItem('clay-settings'));
  console.log('Update volume. Get current volume first');
  getPlaying(function(playing){
    var url = claySetting.host + '?token=' + claySetting.token + '&id=' + claySetting.id + '&player=' + claySetting.player;
    console.log('Update volume. Get current volume done.');
    if (message !== undefined && playing !== null && playing.current_playing !== null){
      var volume = playing.current_playing.volume;
      if (message.VOLUME === 0){      
        volume = volume + 5;
      } else {
        volume = volume - 5;
      }
      url = url + '&volume=' + volume;
      console.log('Send webrequest: ' + url);
      sendVolumeRequest(url);
    } else {
      console.log('No active player -> Cant update volume...');
    }
  });
}

function play_pause(){
  var claySetting = JSON.parse(localStorage.getItem('clay-settings'));  
  var url = claySetting.host + '/play_pause?token=' + claySetting.token + '&id=' + claySetting.id + '&player=' + claySetting.player;
  var req = new XMLHttpRequest();  
  console.log("Set play/pause: " + url);  
  
  req.onreadystatechange = function(e) {
    if (req.readyState == 4 && req.status == 200) {
      var response = JSON.parse(req.responseText);
      if (response !== undefined && !response.error) {
        sendPlaying(response);
      } else {
        if (response.error){
          console.log("Error play_pause. message: " + response.error.message);
          Pebble.showSimpleNotificationOnPebble("Play/Pause failed", response.error.message);
        } else {
          console.log("Error play_pause. response text: " + req.responseText);
          Pebble.showSimpleNotificationOnPebble("Play/Pause failed", "Error on read playing information");
        }
      }
    } else if (req.status == 404 || req.status == 500) {
      console.log("Play/Pause failed. response code: " + req.status);
      Pebble.showSimpleNotificationOnPebble("Play/Pause failed", "Responsecode: " + req.status);
    }
  };
  
  req.timeout = 4000; // 4 seconds timeout
  req.open("GET", url);
  req.send();
}

function sendVolumeRequest(url) {
  var req = new XMLHttpRequest();
 
  req.onreadystatechange = function(e) {
    if (req.readyState == 4 && req.status == 200) {
      var response = JSON.parse(req.responseText);
      if (response !== undefined && !response.error) {
        sendPlaying(response);
      } else {
        if (response.error){
          console.log("Error message: " + response.error.message);
          Pebble.showSimpleNotificationOnPebble("Set volume failed", response.error.message);
        } else {
          console.log("Unknown response:" + req.responseText);
          Pebble.showSimpleNotificationOnPebble("Set volume failed", "Error on setting volume");
        }
      }
    } else if (req.status == 404 || req.status == 500) {
      console.log("Error " + req.status);
      Pebble.showSimpleNotificationOnPebble("Set volume failed", "Responsecode: " + req.status);
    }
  };
  
  req.timeout = 4000; // 4 seconds timeout
  req.open("GET", url);
  req.send();
}

Pebble.addEventListener('ready', function() {
  getPlaying(function(playing){
    sendPlaying(playing);  
  });  
});

Pebble.addEventListener('appmessage', function(e) {
  if (e.payload.VOLUME === 2){
    play_pause();    
  } else {
    update_volume(e.payload);    
  }  
});