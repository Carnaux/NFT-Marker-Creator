var imageLoader = document.getElementById('imageLoader');
    imageLoader.addEventListener('change', handleImage, false);
var canvas = document.getElementById('imageCanvas');
var ctx = canvas.getContext('2d');

var reader = new FileReader();


function handleImage(e){  
  EXIF.getData(e.target.files[0], function() {
    var allMetaData = EXIF.getAllTags(this);
    //console.log(JSON.stringify(allMetaData, null, "\t"))
  
    var dpiX = parseFloat(EXIF.getTag(this, "XResolution"));
    var dpiY = parseFloat(EXIF.getTag(this, "YResolution"));

    var nc = 1;

    var dpi = Math.min(dpiX, dpiY);

    var cpn = EXIF.getTag(this, "ComponentsConfiguration")

    if(isNaN(dpi) || dpi == null){
      dpi = 72;
    }
    if( cpn == "YCbCr"){
      nc = 1;
    }
    // if(isNaN(nc) || nc == null){
    //   nc = 1;
    // }
    console.log("imageExif, dpiX: ", dpiX, " dpiY: ", dpiY, " nc: ", nc )
    
    reader.onload = function(event){
      let array = getUint8(reader.result);
      console.log(e.target.files[0])
      var img = new Image();
      img.onload = function(){
          // base64toHEX(reader.result)

          canvas.width = img.width;
          canvas.height = img.height;
          ctx.drawImage(img,0,0);
          var imgData = ctx.getImageData(0,0,img.width,img.height);
          console.log(imgData)

          console.log("arr", array)
          let cmdArr = [e.target.files[0].name]
          Module._createImageSet( array, dpi, img.width, img.height, nc, 2, cmdArr)
      }
      img.src = event.target.result;
    }
    reader.readAsDataURL(e.target.files[0]);

   
  });

}

function readImage(imageData){
  let size = 0;
  let negativeSize = 0;
  for(let i = 0; i < imageData.data.length; i+=4 ){
    size += 3;
    negativeSize++;
  }
  // let tempData = [];
  // let data = new Uint8ClampedArray(size);
  // for(let i = 0; i < imageData.data.length; i+=4 ){
  //   tempData.push(imageData.data[i]);
  //   tempData.push(imageData.data[i+1]);
  //   tempData.push(imageData.data[i+2]);
  // }
  let data = new Uint8ClampedArray(size);
  for(let i = 0; i < imageData.data.length; i+=4 ){
    data[i] = imageData.data[i];
    data[i+1] = imageData.data[i+1];
    data[i+2] = imageData.data[i+2];
  }

  // var c = document.getElementById("myCanvas");
  // c.width = imageData.width;
  // c.height = imageData.height;
  // var ctx = c.getContext("2d");
  // var imgData = ctx.createImageData(640,480);
  // var i;
  // for (i = 0; i < imgData.data.length; i += 4) {
  //   imgData.data[i+0] = imageData.data[i];
  //   imgData.data[i+1] = imageData.data[i+1];
  //   imgData.data[i+2] = imageData.data[i+2];
  //   imgData.data[i+3] = 255;
  // }
  // ctx.putImageData(imgData, 0,0);
  
  // case 3:
  //     console.log("ERROR:")
  //     break;

  console.log(size)
  console.log(negativeSize)
  if(window.Module){
    console.log("x size: " + imageData.width)
    let ret = Module._createImageSet(data, 72, imageData.width, imageData.height);
    switch(ret){
      case 1:
        console.log("ERROR: Error reading image from file");
        break;
      case 2:
        console.log("ERROR: ImageSet generation error");
        break;
      case 3:
        console.log("ERROR: Saving iset error");
        break;
      case 4:
        console.log("ERROR: Error creating feature map")
        break;
      case 5:
        console.log("ERROR: Saving fset error")
        break;
      case 6:
        console.log("ERROR: Saving fset3 error")
        break;
      case 7:
        console.log("ERROR: Saving array to img")
        break;
    }
  }
}

function downloadIset(){
  let filenameIset = "asa.iset";
  let filenameFset = "asa.fset";
  let filenameFset3 = "asa.fset3";
  let mime = "application/octet-stream";

  let content = Module.FS.readFile(filenameIset);
  let contentFset = Module.FS.readFile(filenameFset);
  let contentFset3 = Module.FS.readFile(filenameFset3);

  var a = document.createElement('a');
  a.download = filenameIset;
  a.href = URL.createObjectURL(new Blob([content], {type: mime}));
  a.style.display = 'none';

  var b = document.createElement('a');
  b.download = filenameFset;
  b.href = URL.createObjectURL(new Blob([contentFset], {type: mime}));
  b.style.display = 'none';

  var c = document.createElement('a');
  c.download = filenameFset3;
  c.href = URL.createObjectURL(new Blob([contentFset3], {type: mime}));
  c.style.display = 'none';

  document.body.appendChild(a);
  a.click();

  document.body.appendChild(b);
  b.click();

  document.body.appendChild(c);
  c.click();
}

function getUint8(str){
  let base64 = str.substr(23, str.length);
  var raw = atob(base64);
  var rawLength = raw.length;
  var array = new Uint8Array(new ArrayBuffer(rawLength));

  for(i = 0; i < rawLength; i++) {
    array[i] = raw.charCodeAt(i);
  }
  return array;
}


function base64toHEX(str) {
  let base64 = str.substr(23, str.length);
  
  var raw = atob(base64);

  var HEX = '';

  for (let i = 0; i < raw.length; i++ ) {

    var _hex = raw.charCodeAt(i).toString(16)

    HEX += (_hex.length==2?_hex:'0'+_hex);

  }
  console.log(HEX.toUpperCase())
  // return HEX.toUpperCase();


 
}
