var ConfigData = {
  "EnemyName": "BLASTOISE",
  "AllyName" : "CHARIZARD",
  "FocusAnimate" : 0,
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
    //console.log('Key: ' + key + ' RetrievedValue: ' + retrieved.toString());
    if(retrieved){
      if(typeof retrieved == 'string' && retrieved.toLowerCase() == 'true')
        ConfigData[key] = 1;
      else if(typeof retrieved == 'string' && retrieved.toLowerCase() == 'false')
        ConfigData[key] = 0;
      else
        ConfigData[key] = retrieved;
    }
  }
};
var StoreConfigData = function(){
  for(var key in ConfigData){         
    console.log('Key: ' + key + ' Value to Store: ' + ConfigData[key]);    
    localStorage.setItem(key.toString(), ConfigData[key]);    
  }
};
Pebble.addEventListener('ready', function() {
  // PebbleKit JS is ready!
  console.log('PebbleKit JS ready!');
  RetrieveConfigData();
  //SendConfig(); Reduce messages to pebble since config is stored on watch now.
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://birdhelloworld.herokuapp.com/?';
  for(var key in ConfigData){
    if(ConfigData[key] !== undefined)
      url += (key.toString() + '=' + ConfigData[key].toString() + '&');
  }
  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode the user's preferences
  var responseData = JSON.parse(decodeURIComponent(e.response));  
  for(var key in ConfigData){
    if(typeof responseData[key] == 'string')
      ConfigData[key] = responseData[key].toUpperCase();
    else
      ConfigData[key] = responseData[key] !== undefined;
  }
  
  StoreConfigData();  
  //console.log(JSON.stringify(ConfigData));
  SendConfig();
});

