var mosca = require('mosca');
 
var settings = {
  port: 1883
};

var admin = require("firebase-admin");

var total_index = 0;
var remote_index = 0;
var indoor_index = 0;
var outdoor_index = 0;

var serviceAccount = require("./serviceAccountKey.json");

admin.initializeApp({
  credential: admin.credential.cert(serviceAccount),
  databaseURL: "https://smart-conditioner.firebaseio.com"
});

var db = admin.database().ref();
const currentRef = db.child("current");
const indoorRef = db.child("indoor");
const outdoorRef = db.child("outdoor");
const remoteRef = db.child("remote");
const totalRef = currentRef.child("total");
const remoteIdxRef = currentRef.child("remoteIdx");
const indoorIdxRef = currentRef.child("indoorIdx");
const outdoorIdxRef = currentRef.child("outdoorIdx");
const remoteNowRef = currentRef.child("remoteNow");

//here we start mosca
var server = new mosca.Server(settings);

server.on('ready', setup);
 
// fired when the mqtt server is ready
function setup() {
  console.log('Mosca server is up and running');
  indoorIdxRef.on("value",  function(snapshot){
    if(isNaN(snapshot.val())) indoor_index = 0;
    else indoor_index = Number(snapshot.val());
  });
  outdoorIdxRef.on("value",  function(snapshot){
    if(isNaN(snapshot.val())) outdoor_index = 0;
    else outdoor_index = Number(snapshot.val());
  });
  remoteIdxRef.on("value",  function(snapshot){
    if(isNaN(snapshot.val())) remote_index = 0;
    else remote_index = Number(snapshot.val());
  });
  totalRef.on("value",  function(snapshot){
    if(isNaN(snapshot.val())) total_index = 0;
    else total_index = Number(snapshot.val());
  });
}

remoteNowRef.on("value", function(snapshot) {
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

  if(total_index == null) total_index = 0;
  if(remote_index == null) remote_index = 0;

  if (packet.topic == "UserControl"){
    remote_index++;
    total_index++;
    currentRef.update({
      total: total_index.toString(),
      remoteIdx: remote_index.toString(),
      remoteNow: packet.payload.toString()
    });
    remoteRef.push({
      time: now.toLocaleString(),
      value: packet.payload.toString(),
      index: remote_index
    });
  }

  if (packet.topic == "IndoorSensor"){
    indoor_index++;
    total_index++;
    currentRef.update({
      total: total_index.toString(),
      indoorIdx: indoor_index.toString(),
      indoorNow: packet.payload.toString()
    });
    indoorRef.push({
      time: now.toLocaleString(),
      value: packet.payload.toString(),
      index: indoor_index
    });
  }

  if (packet.topic == "OutdoorSensor"){
    outdoor_index++;
    total_index++;
    currentRef.update({
      total: total_index.toString(),
      outdoorIdx: outdoor_index.toString(),
      outdoorNow: packet.payload.toString()
    });
    outdoorRef.push({
      time: now.toLocaleString(),
      value: packet.payload.toString(),
      index: outdoor_index
    });
  }

  console.log('Received : ', packet.topic, ' || ', packet.payload.toString());
});
 
// fired when a client subscribes to a topic
server.on('subscribed', function(topic, client) {
  console.log('subscribed : ', topic);

  remoteNowRef.once("value", function(snapshot) {
    var message = {
      topic: "ServerControl",
      payload: snapshot.val()
    };
    
    server.publish(message, function(){});
  });
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