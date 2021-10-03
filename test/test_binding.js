
const fs = require("fs");
const Photonapi = require("../dist/binding.js");
const assert = require("assert");

assert(Photonapi, "The expected function is undefined");

function testBasic()
{
    initRes = Photonapi.Init();
    assert(0 == initRes, "Could not initialize Photon API!");
    //var content = fs.readFileSync("p88.png");
    var content = fs.readFileSync("im92322_rotated_part2_shrunk.jpg");
    //var content = fs.readFileSync("99722.jpg");
    //console.log("content");
    //console.log(content.toString('hex').match(/../g).join(' '));
    //console.log("------------------------");

    const addr = "tcp://73.157.86.121:4400";
    //const addr = "tcp://192.168.0.124:8000";
    const requester =  Photonapi.Connect(addr);
 var jsonStr = Photonapi.Search(requester, "none", content, 0, 2, false, false);
    //const buf = new Buffer.from("",'ascii');  
    //var jsonStr = Photonapi.Search(requester, 'dejavuai.com/images/mirflickr/0/9/96370.jpg',buf , 0, 2, false, false);
    console.log("Before 1 console");
    console.log(jsonStr);
    console.log("After 1 console");

    // Build a collection of unique imageIDs
    const obj = JSON.parse(jsonStr);

    if ("results" in obj)
    {
        var uniqueIDs = new Set();

        for (result of obj.results)
            uniqueIDs.add(result.match_id);

        // Get thumbnails for each matchID and save them on disk
        for (matchID of uniqueIDs)
        {
            console.log("Requesting thumbnail for " + matchID);
            console.log("typeof matchID :");
            console.log(typeof matchID);
            var thumbData = Photonapi.GetThumbnail(requester, matchID, "JPEG");

            if (thumbData.properties.length != 0)
            {
                console.log(thumbData.properties);
                fs.writeFileSync(matchID + ".jpg", thumbData.data);
            }
        }
    }

    res = Photonapi.Disconnect(requester);
    Photonapi.Uninit();
}

assert.doesNotThrow(testBasic, undefined, "testBasic threw an expection");

console.log("Tests passed- everything looks OK!");