var SendConfig = function(dict){
  // Send the object
  Pebble.sendAppMessage(dict, function() {
    console.log('Message sent successfully: ' + JSON.stringify(dict));
  }, function(e) {
    console.log('Message failed: ' + JSON.stringify(e));
  }); 
};

Pebble.addEventListener('ready', function() {
  // PebbleKit JS is ready!
  console.log('PebbleKit JS ready!');
  var enemyName = localStorage.getItem('ENEMYNAME');
  if(enemyName){
    console.log(JSON.stringify(enemyName));
    var dict = { "ENEMYNAME": enemyName };    
    SendConfig(dict);
  }
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://birdhelloworld.herokuapp.com/';
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode the user's preferences
  var configData = JSON.parse(decodeURIComponent(e.response));  
  var dict = { 
    "ENEMYNAME": (configData.enemyName || "").toUpperCase(),
    "ALLYNAME": (configData.allyName || "").toUpperCase(),
    "FOCUSANIMATE": configData.focusAnimate || 0,
    "FLICKANIMATE": configData.flickAnimate || 0
  };
  
  localStorage.setItem('ENEMYNAME', dict.ENEMYNAME);
  
  console.log(JSON.stringify(dict));
  SendConfig(dict);
});

