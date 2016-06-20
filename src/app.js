var BUFFER_SIZE = 8000;
var IMAGE_COUNT = 50;
var ImageArray = [];
var SendConfig = function(dict){
  // Send the object
  Pebble.sendAppMessage(dict, function() {
    console.log('Message sent successfully: ' + JSON.stringify(dict));
  }, function(e) {
    console.log('Message failed: ' + JSON.stringify(e));
  }); 
};
function processImage(responseData, imageNumber) {
  // Convert to a array
  var byteArray = new Uint8Array(responseData);
  var array = [];
  for(var i = 0; i < byteArray.byteLength; i++) {
    array.push(byteArray[i]);
  }
  
  ImageArray[imageNumber] = array;
  // Send chunks to Pebble
  //transmitImage(array, imageNumber);
  
  if(imageNumber == IMAGE_COUNT){
//    for(var j=1;j<IMAGE_COUNT + 1;j++){
//      transmitImage(ImageArray[j],j);    
    transmitImage(ImageArray[1],1);
   }
};
function downloadImage(imageNumber) {
  var url = 'http://birdhelloworld.herokuapp.com/frame-';  
  var imageString = imageNumber.toString();
  var paddingZeros = 3 - imageString.length;
  for(var i=0;i<paddingZeros;i++){
    url += "0";
  }
  url += imageString + ".png";  
  
  console.log('Requesting: ' + url);

  var request = new XMLHttpRequest();
  request.onload = function() {
    console.log('Image #' + imageNumber + ' loaded successfully!');
    processImage(this.response, imageNumber);
    if(imageNumber < IMAGE_COUNT)
      downloadImage(imageNumber + 1);
  };
  request.responseType = "arraybuffer";
  request.open("GET", url);
  request.send();
};
function transmitImage(array, imageNumber) {
  var index = 0;
  var arrayLength = array.length;

  // Transmit the length for array allocation
  console.log('Sending image metadata');
  Pebble.sendAppMessage({'AppKeyDataLength': arrayLength, 'AppKeyImageNumber': imageNumber}, function(e) {
    // Success, begin sending chunks
    sendChunk(array, index, arrayLength, imageNumber);
  }, function(e) {
    console.log('Failed to initiate image transfer!');
  });
};
function sendChunk(array, index, arrayLength, imageNumber) {
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
    'AppKeyImageNumber': imageNumber,
    'AppKeyComplete': 0
  };
  console.log('Sending chunk ' + index + ' for image ' + imageNumber + '. Chunk size: ' + chunkSize + ' Chunk data: ' + array.slice(index, index + chunkSize));
  // Send the chunk
  Pebble.sendAppMessage(dict, function() {
    console.log('Successfully sent chunk!');
    if(imageNumber < IMAGE_COUNT){
      transmitImage(ImageArray[imageNumber + 1], imageNumber + 1);
    } else {
      //transmitImage(ImageArray[1],1); //INFINITE LOOP
    }
    // Success
    //index += chunkSize;

    /*if(index < arrayLength) {
      // Send the next chunk
      sendChunk(array, index, arrayLength, imageNumber);
    } else {
      // Complete!
      Pebble.sendAppMessage({'AppKeyComplete': 0, 'AppKeyImageNumber': imageNumber});
      if(imageNumber < IMAGE_COUNT){
        transmitImage(ImageArray[imageNumber + 1], imageNumber + 1);
        //setTimeout(transmitImage(ImageArray[imageNumber + 1], imageNumber + 1), 100);
      }
    }*/
  }, function(e) {
    console.log('Failed to send chunk with index ' + index);
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
//  for(var i=0;i<IMAGE_COUNT;i++){
//    downloadImage(i+1);
//  }
  downloadImage(1)   
});

