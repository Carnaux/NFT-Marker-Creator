var imageLoader = document.getElementById('imageLoader');
    imageLoader.addEventListener('change', handleImage, false);
var canvas = document.getElementById('imageCanvas');
var ctx = canvas.getContext('2d');




function handleImage(e){
  EXIF.getData(e.target.files[0], function() {
  
    var dpiX = EXIF.getTag(this, "XResolution");
    var dpiY = EXIF.getTag(this, "YResolution");

    var nc = EXIF.getTag(this, "ColorSpace");
    var dpi = Math.min(dpiX, dpiY);
    console.log("imageExif, dpiX: ", dpiX, " dpiY: ", dpiY, " nc: ", nc )
    var reader = new FileReader();
    reader.onload = function(event){
        var img = new Image();
        img.onload = function(){
            canvas.width = img.width;
            canvas.height = img.height;
            ctx.drawImage(img,0,0);
            var imgData = ctx.getImageData(0,0,img.width,img.height);
            console.log("arr", imgData)
            Module._createImageSet( imgData.data, dpi, img.width, img.height, nc)
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
  let imgCreate = Module.FS.readFile("tempFileRead.jpeg");

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

  var d = document.createElement('a');
  d.download = imgCreate;
  d.href = URL.createObjectURL(new Blob([contentFset3], {type: mime}));
  d.style.display = 'none';

  document.body.appendChild(a);
  a.click();

  document.body.appendChild(b);
  b.click();

  document.body.appendChild(c);
  c.click();

  document.body.appendChild(d);
  d.click();
}

