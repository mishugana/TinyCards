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
                  console.log("poo");
                  cardDict.SIDE1=fact.text;
                }
              var cardSide1 = card.sides[0];
              fact = cardSide1.concepts[0].fact;
              if (fact.type == 'TEXT')
                {   
                  cardDict.SIDE2=fact.text;
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



Pebble.addEventListener('ready', function (e) {
  console.log('connect!' + e.ready);
  fetchDeck("b4fdea0f-0baf-4e8d-977c-70704f7bad29");
  console.log(e.type);
});

Pebble.addEventListener('appmessage', function (e) {
 fetchDeck("b4fdea0f-0baf-4e8d-977c-70704f7bad29");
  console.log('message!');
});

Pebble.addEventListener('webviewclosed', function (e) {
  console.log('webview closed');
  console.log(e.type);
  console.log(e.response);
});
