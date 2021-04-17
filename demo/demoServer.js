const express = require('express');
const path = require("path");
const app = new express();

process.title = "demoNFT";

app.use("/static",express.static('public'));

app.get('/', function(request, response){
    response.sendFile(path.join(__dirname + '/nft.html'));
});

app.listen(3000)

console.log("Demo running at http://localhost:3000/");
