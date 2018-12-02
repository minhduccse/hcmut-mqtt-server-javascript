var mosca = require('mosca');
 
var settings = {
  port: 1883
};

var admin = require("firebase-admin");

var record_index = 0;

var serviceAccount = require("./serviceAccountKey.json");

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://smart-conditioner.firebaseio.com"
});

var db = admin.database().ref();
const numberRecordsRef = db.child("index");
const remoteRef = db.child("remote");

//here we start mosca
var server = new mosca.Server(settings);

server.on('ready', setup);
 
// fired when the mqtt server is ready
function setup() {
  console.log('Mosca server is up and running');
  numberRecordsRef.once("value",  function(snapshot){
    if(isNaN(snapshot.val())) record_index = 0;
    else record_index = Number(snapshot.val());
  });
}

remoteRef.on("value", function(snapshot) {
  var message = {
    topic: "ServerControl",
    payload: snapshot.val()
  };
  
  server.publish(message, function(){});
});

// fired when a client is connected
server.on('clientConnected', function(client) {
  console.log('client connected', client.id);
});
 
// fired when a message is received
server.on('published', function(packet, client) {
  var now = new Date();

  if(record_index == null) record_index = 0;

  if (packet.topic == "UserControl"){
    db.update({
      index: record_index.toString(),
      remote: packet.payload.toString()
    });
    db.push({
      time: now.toLocaleString(),
      remote: packet.payload.toString(),
      index: record_index
    });
    record_index++;
  }

  if (packet.topic == "IndoorSensor"){
    db.update({
      index: record_index.toString(),
      indoor: packet.payload.toString()
    });
    db.push({
      time: now.toLocaleString(),
      indoor: packet.payload.toString(),
      index: record_index
    });
    record_index++;
  }

  console.log('Received : ', packet.topic, ' || ', packet.payload.toString());
});
 
// fired when a client subscribes to a topic
server.on('subscribed', function(topic, client) {
  console.log('subscribed : ', topic);
});
 
// fired when a client unsubscribes to a topic
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