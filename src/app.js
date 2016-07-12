var ConfigData = {
  "EnemyName": "BLASTOISE",
  "AllyName" : "CHARIZARD",
  "AllySpriteUrl" : "",
  "AllyShinySpriteUrl" : "",
  "EnemySpriteUrl" : "",
  "Dither" : false
};
var BUFFER_SIZE = 8000;
var IMAGE_TYPE_ALLY_SPRITE = 0;
var IMAGE_TYPE_ALLY_SHINY_SPRITE = 1;
var IMAGE_TYPE_ENEMY_SPRITE = 2;

var SendConfig = function(callback){
  // Send the object
  var dict = {
    "EnemyName": (ConfigData.EnemyName.toUpperCase()),
    "AllyName" : (ConfigData.AllyName.toUpperCase())
  };
  Pebble.sendAppMessage(dict, function() {
    console.log('Message sent successfully: ' + JSON.stringify(dict));
    if(callback && callback instanceof Function){
      callback();
    }
  }, function(e) {
    console.log('Message failed: ' + JSON.stringify(e));
  }); 
};
var RetrieveConfigData = function(){
  for(var key in ConfigData){ 
    var retrieved = localStorage.getItem(key.toString());
    if(retrieved === undefined || retrieved === null){
      continue;
    }
    console.log('Key: ' + key + ' RetrievedValue: ' + retrieved.toString());
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
  
  //getAndTransmitImage('http://www.pokestadium.com/sprites/black-white/mewtwo.png', IMAGE_TYPE_ENEMY_SPRITE, null);
  SendSprites();
  //SendConfig(); Reduce messages to pebble since config is stored on watch now.
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://birdhelloworld.herokuapp.com/custom?';
  for(var key in ConfigData){
    if(ConfigData[key] !== undefined)
      url += (key.toString() + '=' + ConfigData[key].toString() + '&');
  }
  Pebble.openURL(url);
});
function SendSprites(callback){
  getAndTransmitImage(ConfigData.AllySpriteUrl, IMAGE_TYPE_ALLY_SPRITE, function(){
    getAndTransmitImage(ConfigData.AllyShinySpriteUrl, IMAGE_TYPE_ALLY_SHINY_SPRITE, function(){
      getAndTransmitImage(ConfigData.EnemySpriteUrl, IMAGE_TYPE_ENEMY_SPRITE, function(){
        if(callback && callback instanceof Function){
          callback();
        }
      });    
    });    
  });   
};
Pebble.addEventListener('webviewclosed', function(e) {
  // Decode the user's preferences
  var responseData = JSON.parse(decodeURIComponent(e.response));  
  for(var key in ConfigData){    
      ConfigData[key] = responseData[key];
  }  
  console.log(JSON.stringify(ConfigData));  
  StoreConfigData();
  //SendConfig(SendSprites);
  //SendConfig(SendSprites);
  //SendSprites(SendConfig);
  SendConfig();
  SendSprites();
});
function getAndTransmitImage(url, imageType, callback) {
  if(!url || url === ''){
    if(callback && callback instanceof Function){
      callback();
    }
    return;
  }
  //Call format route to format image for Pebble.
  url = 'http://birdhelloworld.herokuapp.com/formatImage?ImageUrl=' + url;
  if(ConfigData.Dither){
    url += '&Dither=true';
  }
  var retrieved = JSON.parse(localStorage.getItem(url));      
  if(retrieved){
    console.log(url + ' retrieved from local storage. Length:' + retrieved.length);
    //var sliced = retrieved.slice(0, length);
    //console.log("original: " + retrieved);
    //console.log("sliced: " + sliced);
    //console.log('Legnth: ' + sliced.length);
    transmitImage(retrieved, imageType, callback);
    return;
  }
  console.log('Requesting: ' + url);
  var request = new XMLHttpRequest();
  request.onload = function() {
    console.log(url + ' loaded successfully!');
    // Convert to a array
    var byteArray = new Uint8Array(this.response);
    var array = [];
    for(var i = 0; i < byteArray.byteLength; i++) {
      array.push(byteArray[i]);
    }
    localStorage.setItem(url, JSON.stringify(array));
    transmitImage(array, imageType, callback);
  };
  request.responseType = "arraybuffer";
  request.open("GET", url);
  request.send();
};
function transmitImage(array, imageType, callback) {
  var index = 0;
  var arrayLength = array.length;

  // Transmit the length for array allocation
  console.log('Sending image metadata');
  Pebble.sendAppMessage({'AppKeyDataLength': arrayLength, 'ImageType': imageType }, function(e) {
    // Success, begin sending chunks
    sendChunk(array, index, arrayLength, imageType, callback);
  }, function(e) {
    console.log('Failed to initiate image transfer!');
  });
};
function sendChunk(array, index, arrayLength, imageType, callback) {
  // Determine the next chunk size
  var chunkSize = BUFFER_SIZE;
  var lastChunk = arrayLength - index < BUFFER_SIZE;
  if(lastChunk) {
    // Resize to fit just the remaining data items
    chunkSize = arrayLength - index;
  }

  // Prepare the dictionary
  var dict = {
    'AppKeyDataChunk': array.slice(index, index + chunkSize),
    'AppKeyChunkSize': chunkSize,
    'AppKeyIndex': index,
    'ImageType' : imageType
  };
  if(lastChunk){
    dict.AppKeyComplete = 0;
  }
  //console.log('Sending chunk ' + index + '. Chunk size: ' + chunkSize + ' Chunk data: ' + array.slice(index, index + chunkSize));
  // Send the chunk
  Pebble.sendAppMessage(dict, function() {
    console.log('Successfully sent chunk!');
    // Success
    index += chunkSize;

    if(index < arrayLength) {
      // Send the next chunk
      sendChunk(array, index, arrayLength, imageType, callback);
    } else{
      if(callback && callback instanceof Function){
        callback();
      }
    }
  }, function(e) {
    console.log('Failed to send chunk with index ' + index);
  });
};