var ConfigData = {
  "EnemyName": "BLASTOISE",
  "AllyName" : "CHARIZARD",
  "FocusAnimate" : 1,
  "FlickAnimate" : 0
};

var SendConfig = function(){
  // Send the object
  Pebble.sendAppMessage(ConfigData, function() {
    console.log('Message sent successfully: ' + JSON.stringify(ConfigData));
  }, function(e) {
    console.log('Message failed: ' + JSON.stringify(e));
  }); 
};
var RetrieveConfigData = function(){
  for(var key in ConfigData){ 
    var retrieved = localStorage.getItem(key.toString());
    if(retrieved){
      ConfigData[key] = retrieved;
    }
  }
};
var StoreConfigData = function(){
  for(var key in ConfigData){     
    if(ConfigData[key]){
      localStorage.setItem(key.toString(), ConfigData[key]);
    }
  }
};
Pebble.addEventListener('ready', function() {
  // PebbleKit JS is ready!
  console.log('PebbleKit JS ready!');
  RetrieveConfigData();
  SendConfig();
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://birdhelloworld.herokuapp.com/?';
  for(var key in ConfigData){
    if(ConfigData[key])
      url += (key.toString() + '=' + ConfigData[key].toString() + '&');
  }
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode the user's preferences
  var responseData = JSON.parse(decodeURIComponent(e.response));  
  for(var key in responseData){
    if(ConfigData[key] !== undefined){
      if(typeof responseData[key] == 'string')
        ConfigData[key] = responseData[key].toUpperCase();
      else
        ConfigData[key] = responseData[key];
    }
  }
  
  StoreConfigData();  
  console.log(JSON.stringify(ConfigData));
  SendConfig();
});

