var transferInProgress = false;

function fetchDeck(deckID) {
  var req = new XMLHttpRequest();
  req.open('GET', 'https://tinycards.duolingo.com/api/1/decks/' + deckID + '?expand=true', true);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        var response = JSON.parse(req.responseText);
        for (var i=0; i < response.cardCount; i++)
          {   
              var card = response.cards[i];
            
              var cardDict = {
                "NUMBER":i,
                "SIDE1":"IMAGE", 
                "SIDE2":"IMAGEN"
              };
            
              var cardSide2 = card.sides[1];
              var fact = cardSide2.concepts[0].fact;
              if (fact.type == 'TEXT')
                {   
                  cardDict.SIDE1=fact.text;
                }
              if (fact.type == 'IMAGE')
                {   
                  
                  cardDict.SIDE1= fact.imageUrl.substring(fact.imageUrl.lastIndexOf("/") );
                }
              var cardSide1 = card.sides[0];
              fact = cardSide1.concepts[0].fact;
              if (fact.type == 'TEXT')
                {   
                  cardDict.SIDE2=fact.text;
                }
              if (fact.type == 'IMAGE')
                {   
                  console.log(fact.imageUrl.substring(fact.imageUrl.lastIndexOf("/")));
                  cardDict.SIDE2=fact.imageUrl.substring(fact.imageUrl.lastIndexOf("/") );
                }
              console.log("cardDict: "+cardDict);
              Pebble.sendAppMessage(cardDict);
          }

      } else {
        console.log('Error');
      }
    }
  };
  req.send(null);
}

function downloadImage(imageUrl) {
    //imageUrl = "https://thawing-everglades-85022.herokuapp.com/?op=resize&tc=""
    console.log("Got message: " + JSON.stringify(imageUrl));

    console.log("https://thawing-everglades-85022.herokuapp.com/?op=resize&tc=" + imageUrl.payload.NETDL_URL.substring(1));
    if (transferInProgress === false) {
      transferInProgress = true;
      downloadBinaryResource("https://thawing-everglades-85022.herokuapp.com/?op=resize&tc=" + imageUrl.payload.NETDL_URL.substring(1), function(bytes) {
        transferImageBytes(bytes, imageUrl.payload.NETDL_CHUNK_SIZE,
                           function() { console.log("Done!"); transferInProgress = false; },
                           function(imageUrl) { console.log("Failed! " + imageUrl); transferInProgress = false; }
                          );
      },
                             function(imageUrl) {
                               console.log("Download failed: " + imageUrl); transferInProgress = false;
                             });
    }
    else {
      console.log("Ignoring request to download " + imageUrl.payload.NETDL_URL + " because another download is in progress.");
    }
    
}

function downloadBinaryResource(imageURL, callback, errorCallback) {
    var req = new XMLHttpRequest();
    req.open("GET", imageURL,true);
    req.responseType = "arraybuffer";
    req.onload = function(e) {
        console.log("loaded");
        var buf = req.response;
        if(req.status == 200 && buf) {
            var byteArray = new Uint8Array(buf);
            var arr = [];
            for(var i=0; i<byteArray.byteLength; i++) {
                arr.push(byteArray[i]);
            }

            console.log("Downloaded file with " + byteArray.length + " bytes.");
            callback(arr);
        }
        else {
          errorCallback("Request status is " + req.status);
        }
    };
    req.onerror = function(e) {
      errorCallback(e);
    };
    req.send(null);
}
function transferImageBytes(bytes, chunkSize, successCb, failureCb) {
  var retries = 0;

  success = function() {
    console.log("Success cb=" + successCb);
    if (successCb !== undefined) {
      successCb();
    }
  };
  failure = function(e) {
    console.log("Failure cb=" + failureCb);
    if (failureCb !== undefined) {
      failureCb(e);
    }
  };

  // This function sends chunks of data.
  sendChunk = function(start) {
    var txbuf = bytes.slice(start, start + chunkSize);
    console.log("Sending " + txbuf.length + " bytes - starting at offset " + start);
    Pebble.sendAppMessage({ "NETDL_DATA": txbuf },
      function(e) {
        // If there is more data to send - send it.
        if (bytes.length > start + chunkSize) {
          sendChunk(start + chunkSize);
        }
        // Otherwise we are done sending. Send closing message.
        else {
          Pebble.sendAppMessage({"NETDL_END": "done" }, success, failure);
        }
      },
      // Failed to send message - Retry a few times.
      function (e) {
        if (retries++ < 3) {
          console.log("Got a nack for chunk #" + start + " - Retry...");
          sendChunk(start);
        }
        else {
          failure(e);
        }
      }
    );
  };

  // Let the pebble app know how much data we want to send.
  console.log("started");
  Pebble.sendAppMessage({"NETDL_BEGIN": bytes.length },
    function (e) {
      // success - start sending
      sendChunk(0);
    }, failure);

}

Pebble.addEventListener('ready', function (e) {
  console.log('connect!' + e.ready);
  fetchDeck("b4fdea0f-0baf-4e8d-977c-70704f7bad29");
  console.log(e.type);
});

Pebble.addEventListener('appmessage', function (e) {
 
  if ('NETDL_URL' in e.payload) 
  {
    console.log("gotmsg");
    downloadImage(e);
  }
  else
    {
      fetchDeck("b4fdea0f-0baf-4e8d-977c-70704f7bad29");
    }
  console.log('message!');
});

Pebble.addEventListener('webviewclosed', function (e) {
  console.log('webview closed');
  console.log(e.type);
  console.log(e.response);
});
