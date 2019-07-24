var imageLoader = document.getElementById('imageLoader');
    imageLoader.addEventListener('change', handleImage, false);
var canvas = document.getElementById('imageCanvas');
var ctx = canvas.getContext('2d');
let imgData;

function handleImage(e){
    var reader = new FileReader();
    reader.onload = function(event){
        var img = new Image();
        img.onload = function(){
            canvas.width = img.width;
            canvas.height = img.height;
            ctx.drawImage(img,0,0);
            let imageData = ctx.getImageData(0,0,canvas.width,canvas.height);
            readImage(imageData);

        }
        img.src = event.target.result;
    }
    reader.readAsDataURL(e.target.files[0]); 
    
}

function readImage(imageData){
    let ret = Module._createImageSet(imageData.data, 220, imageData.width, imageData.height);
    console.log("Returned: ", ret)
}