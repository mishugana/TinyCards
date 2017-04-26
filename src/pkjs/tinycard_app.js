function fetchDeck(deckID) {
  var req = new XMLHttpRequest();
  req.open('GET', 'https://tinycards.duolingo.com/api/1/decks/' + deckID + '?expand=true', true);
  req.onload = function () {
    if (req.readyState === 4) {
      if (req.status === 200) {
        var j=0;
        var response = JSON.parse(req.responseText);
        for (var i=0; i < response.cardCount; i++)
          {   
              var card = response.cards[i];
              var cardSide2 = card.sides[1];
              var fact = cardSide2.concepts[0].fact;
            console.log(fact.type);
              if (fact.type == 'TEXT')
                {   
                    var fact_text = fact.text;
                    Pebble.sendAppMessage({
                      'FACT': fact_text,
                      'NUMBER': j
                    });
                    j++;
                }
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
 //fetchDeck("b4fdea0f-0baf-4e8d-977c-70704f7bad29");
  console.log('message!');
});

Pebble.addEventListener('webviewclosed', function (e) {
  console.log('webview closed');
  console.log(e.type);
  console.log(e.response);
});
