const path = require("path");
const fs = require('fs');
const glob = require('glob');
const inkjet = require('inkjet');
var Module = require('./NftMarkerCreator.min.js');

let srcTest = path.join(__dirname, '/input/');

let imagePath = glob.sync(srcTest +'/**/*.{jpg,JPG,jpeg,JPEG}', {});

let fileNameWithExt = path.basename(imagePath[0]);
let fileName = path.parse(fileNameWithExt).name


if(imagePath.length > 1){
    console.log("\nERROR: Multiple images in INPUT directory!\n")
    process.exit(1);
}

let buf = fs.readFileSync(imagePath[0]);

let imageData = {
    sizeX: 0,
    sizeY: 0,
    nc: 0,
    dpi: 0,
    array: []
}

inkjet.decode(buf, function(err, decoded) {
    if(err){
        console.log("\n" + err + "\n");
        process.exit(1);
    }else{
        let newArr = [];
        
        for(let j = 0; j < decoded.data.length; j+=4){
            newArr.push(decoded.data[j+2])
            newArr.push(decoded.data[j+1])
            newArr.push(decoded.data[j])
            newArr.push(decoded.data[j+3])
        }

        // for(let j = 0; j < 4; j++){ 
        //     console.log("newArr: "+j + "-> " +  (newArr[j]));
        // }
        // for(let j = 0; j < 4; j++){ 
        //     console.log("original: "+j + "-> " +  (decoded.data[j]));
        // }
        
        imageData.array = newArr;
    }
});

inkjet.exif(buf, function(err, metadata) {
    if(err){
        console.log("\n" + err + "\n");
        process.exit(1);
    }else{
        let dpi = Math.min(parseInt(metadata.XResolution.value), parseInt(metadata.YResolution.value));

        if(dpi == null || dpi == undefined || dpi == NaN){
            console.log("\nWARNING: No DPI value found! Using 72 as default value!\n")
            dpi = 72;
        }
    
        imageData.sizeX = metadata.ImageWidth.value;
        imageData.sizeY = metadata.ImageLength.value;
        imageData.nc = metadata.SamplesPerPixel.value;
        imageData.dpi = dpi;
    }
});

var params = [
    0,
    fileNameWithExt
];

for (let j = 2; j < process.argv.length; j++) {
   params.push(process.argv[j]);
}

let heapSpace = Module._malloc(imageData.array.length * imageData.array.BYTES_PER_ELEMENT);
Module.HEAPU8.set( imageData.array, heapSpace);

Module._createImageSet(heapSpace ,imageData.dpi, imageData.sizeX, imageData.sizeY, imageData.nc, fileName, params.length, params)

Module._free(heapSpace);

let filenameIset = "asa.iset";
let filenameFset = "asa.fset";
let filenameFset3 = "asa.fset3";

let content = Module.FS.readFile(filenameIset);
let contentFset = Module.FS.readFile(filenameFset);
let contentFset3 = Module.FS.readFile( filenameFset3);

fs.writeFileSync( path.join(__dirname, '/output/') + filenameIset, content);
fs.writeFileSync( path.join(__dirname, '/output/') + filenameFset, contentFset);
fs.writeFileSync( path.join(__dirname, '/output/') + filenameFset3, contentFset3);
