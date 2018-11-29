var mosca = require('mosca');
 
var settings = {
  port: 1883
};

var admin = require("firebase-admin");

// var Ledcomment = '1';

var record_index = 0;

// setInterval(function() {
//   Ledcomment = (Ledcomment === '1')?'0':'1';
//   server.publish({topic: 'LEDToggle', payload: Ledcomment});
//   console.log('Message Sent');
// }, 5000);

var serviceAccount = require("./serviceAccountKey.json");

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://smart-conditioner.firebaseio.com"
});

var db = admin.database().ref();
// var messRef = db.child('message');

//here we start mosca
var server = new mosca.Server(settings);

server.on('ready', setup);
 
// fired when the mqtt server is ready
function setup() {
  console.log('Mosca server is up and running');
}
 
// fired when a client is connected
server.on('clientConnected', function(client) {
  console.log('client connected', client.id);
});
 
// fired when a message is received
server.on('published', function(packet, client) {
  if (packet.topic == "AirControl"){
    db.update({index: record_index.toString()});
    db.push({
      time: "unknown",
      remote: packet.payload.toString(),
      indoor: Ledcomment,
      outdoor: Ledcomment,
      index: record_index
    });
    record_index++;
  }
  
  console.log('Published : ', packet.topic, ' || ', packet.payload.toString());
});
 
// fired when a client subscribes to a topic
server.on('subscribed', function(topic, client) {
  console.log('subscribed : ', topic);
});
 
// fired when a client subscribes to a topic
server.on('unsubscribed', function(topic, client) {
  console.log('unsubscribed : ', topic);
});
 
// fired when a client is disconnecting
server.on('clientDisconnecting', function(client) {
  console.log('clientDisconnecting : ', client.id);
});
 
// fired when a client is disconnected
server.on('clientDisconnected', function(client) {
  console.log('clientDisconnected : ', client.id);
});