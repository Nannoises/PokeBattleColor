var ConfigData = {
  "EnemyName": "BLASTOISE",
  "AllyName" : "CHARIZARD",
  "AllySpriteUrl" : "",
  "AllyShinySpriteUrl" : "",
  "EnemySpriteUrl" : ""
};
var BUFFER_SIZE = 8000;

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
  var url = 'http://birdhelloworld.herokuapp.com/custom?';
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
function processImage(responseData) {
  // Convert to a array
  var byteArray = new Uint8Array(responseData);
  var array = [];
  for(var i = 0; i < byteArray.byteLength; i++) {
    array.push(byteArray[i]);
  }
    
  // Send chunks to Pebble
  transmitImage(array);
};
function downloadImage(url) {    
  console.log('Requesting: ' + url);
  var request = new XMLHttpRequest();
  request.onload = function() {
    console.log(url + ' loaded successfully!');
    processImage(this.response);
  };
  request.responseType = "arraybuffer";
  request.open("GET", url);
  request.send();
};
function transmitImage(array) {
  var index = 0;
  var arrayLength = array.length;

  // Transmit the length for array allocation
  console.log('Sending image metadata');
  Pebble.sendAppMessage({'AppKeyDataLength': arrayLength }, function(e) {
    // Success, begin sending chunks
    sendChunk(array, index, arrayLength);
  }, function(e) {
    console.log('Failed to initiate image transfer!');
  });
};
function sendChunk(array, index, arrayLength) {
  // Determine the next chunk size
  var chunkSize = BUFFER_SIZE;
  if(arrayLength - index < BUFFER_SIZE) {
    // Resize to fit just the remaining data items
    chunkSize = arrayLength - index;
  }

  // Prepare the dictionary
  var dict = {
    'AppKeyDataChunk': array.slice(index, index + chunkSize),
    'AppKeyChunkSize': chunkSize,
    'AppKeyIndex': index,
    'AppKeyComplete': 0
  };
  console.log('Sending chunk ' + index + '. Chunk size: ' + chunkSize + ' Chunk data: ' + array.slice(index, index + chunkSize));
  // Send the chunk
  Pebble.sendAppMessage(dict, function() {
    console.log('Successfully sent chunk!');    
    // Success
    index += chunkSize;

    if(index < arrayLength) {
      // Send the next chunk
      sendChunk(array, index, arrayLength);
    } else {
      // Complete!
      Pebble.sendAppMessage({'AppKeyComplete': 0});      
    }
  }, function(e) {
    console.log('Failed to send chunk with index ' + index);
  });
};