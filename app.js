const path = require("path");
const fs = require('fs');
const glob = require('glob');
const inkjet = require('inkjet');
const readlineSync = require('readline-sync');
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
            
            
        }

        let uint = new Uint8Array(newArr);
   
        imageData.array = uint;
    }
});

inkjet.exif(buf, function(err, metadata) {
    if(err){
        console.log("\n" + err + "\n");
        process.exit(1);
    }else{
        // console.log(metadata)
        let dpi = Math.min(parseInt(metadata.XResolution.value), parseInt(metadata.YResolution.value));

        if(dpi == null || dpi == undefined || dpi == NaN){
            console.log("\nWARNING: No DPI value found! Using 72 as default value!\n")
            dpi = 72;
        }
        
        if(metadata.ImageWidth == null || metadata.ImageWidth == undefined){
            if(metadata. PixelXDimension == null || metadata. PixelXDimension == undefined){
                var answer = readlineSync.question('The image does not contain any width or height info, do you want to inform them?[y/n]\n');
                
                if(answer == "y"){
                    var answer2 = readlineSync.question('Inform the width and height: e.g W=200 H=400\n');

                    let vals = getValues(answer2, "wh");
                    imageData.sizeX = vals.w;
                    imageData.sizeY = vals.h;
                }else{
                    console.log("It's not possible to proceed without width or height info!")
                    process.exit(1);
                }
            }else{
                imageData.sizeX = metadata. PixelXDimension.value;
                imageData.sizeY = metadata. PixelYDimension.value;
            }
        }else{
            imageData.sizeX = metadata.ImageWidth.value;
            imageData.sizeY = metadata.ImageLength.value;
        }

        if(metadata.SamplesPerPixel == null || metadata.ImageWidth == undefined){
           
            var answer = readlineSync.question('The image does not contain the number of channels(nc), do you want to inform it?[y/n]\n');
            
            if(answer == "y"){
                var answer2 = readlineSync.question('Inform the number of channels(nc):(black and white images have NC=1, colored images have NC=3) e.g NC=3 \n');

                let vals = getValues(answer2, "nc");
                imageData.nc = vals;
            }else{
                console.log("It's not possible to proceed without the number of channels!")
                process.exit(1);
            }
           
        }else{
            imageData.nc = metadata.SamplesPerPixel.value;
        }

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

let ext = ".iset";
let ext2 = ".fset";
let ext3 = ".fset3";

let content = Module.FS.readFile(filenameIset);
let contentFset = Module.FS.readFile(filenameFset);
let contentFset3 = Module.FS.readFile( filenameFset3);

fs.writeFileSync( path.join(__dirname, '/output/') + fileName + ext, content);
fs.writeFileSync( path.join(__dirname, '/output/') + fileName + ext2, contentFset);
fs.writeFileSync( path.join(__dirname, '/output/') + fileName + ext3, contentFset3);


function getValues(str, type){
    let values;
    if(type == "wh"){
        let Wstr = "W=";
        let Hstr = "H=";
        var doesContainW = str.indexOf(Wstr);
        var doesContainH = str.indexOf(Hstr);
    
        let valW = parseInt(str.slice(doesContainW+2, doesContainH));
        let valH = parseInt(str.slice(doesContainH+2));
    
        values = {
            w: valW,
            h: valH
        }
    }else if(type == "nc"){
        let nc = "NC=";
        var doesContainNC = str.indexOf(nc);
        values = parseInt(str.slice(doesContainNC+3));
    }
    
    
    return values;
}