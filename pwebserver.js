const express = require('express');
const bodyParser = require('body-parser');
const { response } = require('express');
const cors = require('cors');
const assert = require("assert");
const Photonapi = require("./dist/binding.js");
const fs = require("fs");

const app = express();

app.use(cors())

app.use(bodyParser.urlencoded({
    extended: true
  }));
app.use(bodyParser.json({ limit: '3000kb' }));



app.post('/search/', (req, res) => {
  console.log("search");
  const requester = connectSS();

  console.log("jsoreq.body.url");
  console.log(req.body.url);

  if (req.body.url==="none"){
    const buf = new Buffer.from(req.body.buffer,'ascii');
    var jsonStr = Photonapi.Search(requester, "none" , buf , 0, 2, false, false);
  } else {   
    var jsonStr = Photonapi.Search(requester, "https://dejavuai.com/images/mirflickr/0/8/88962.jpg" , '' , 0, 2, false, false);
  }
  disconnectSS(requester);
  res.send(jsonStr);
})

app.get('/getthumbnail/:id', (req, res) => {
  console.log("app.get(/getthumbnail/:id");
  const requester = connectSS();
  thumbData = Photonapi.GetThumbnail(requester, Number(req.params.id), "JPEG");
  disconnectSS(requester);
  res.send("data:image/png;base64," + Buffer.from(thumbData.data).toString('base64'));
})

app.listen(3001, () => {})

// ------------- functions

function connectSS() {
  initRes = Photonapi.Init();
  assert(0 == initRes, "Could not initialize Photon API!");
  const addr = "tcp://73.157.86.121:4400";
  //const addr = "tcp://192.168.0.124:8000";
  const requester =  Photonapi.Connect(addr);
  return requester;
}

function disconnectSS(requester) {
  Photonapi.Disconnect(requester);
  Photonapi.Uninit();
}


// Request search server
function requestSearchServer(event, imageId, res){
    const ws = new WebSocket("ws://localhost:8082");
    // ws://f1.sergei.info:8082
    ws.addEventListener("open", () =>{
        data = JSON.stringify({
          event: event,
          data: imageId
        })
        ws.send(data);
    });

    ws.addEventListener("message", ({data}) =>{
      const object = JSON.parse(data);
      res.send(JSON.stringify(object));
    });

  }