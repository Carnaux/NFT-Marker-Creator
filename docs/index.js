var imageLoader = document.getElementById('imageLoader');
imageLoader.addEventListener('change', handleImage, false);
var canvas = document.getElementById('imageCanvas');
var hideCanvas = document.getElementById('hideCanvas');
hideCanvas.style.display = "none";
var ctx = canvas.getContext('2d');
ctx.fillStyle = "#949494";
ctx.fillRect(0, 0, canvas.width, canvas.height);
var ctxHide = hideCanvas.getContext('2d');

var reader = new FileReader();

var name;
var nameWithExt;

var globalObj = {
    dpi: 0,
    nc: 0,
    w: 0,
    h: 0,
    arr: []
}

function handleImage(e) {
    nameWithExt = e.target.files[0].name;
    console.log("Image uploaded: " + nameWithExt);

    name = nameWithExt.substr(0, nameWithExt.lastIndexOf('.'));

    let extJpg = nameWithExt.substr(nameWithExt.lastIndexOf('.'));

    let confidenceEl = document.getElementById("confidenceLevel");
    let childEls = confidenceEl.getElementsByClassName("confidenceEl");
    for(let i = 0; i < childEls.length; i++){
        childEls[i].src = "./icons/star2.svg";
    }

    if (extJpg == '.jpg' || extJpg == '.jpeg' || extJpg == '.JPG' || extJpg == '.JPEG') {
        useJpeg(e);
    } else if (extJpg == '.png' || extJpg == '.PNG') {
        globalObj.dpi = 72;
        readImage(e)
    } else {
        console.log("Invalid image format!");
    }

    document.getElementById("generateBt").disabled = false;
}

function generate() {
    var imageCanvas = document.querySelector('#imageCanvas');
    imageCanvas.style.opacity = 0.25;

    var okSign = document.querySelector('.checkmark-cover');
    okSign.style.display = 'none';

    var spinner = document.querySelector('.spinner-container');
    spinner.style.display = 'block';

    setTimeout(() => {
        let cmdArr = [0, name];

        let paramStr = cmdArr.join(' ');
        console.log(paramStr)
        let StrBuffer = Module._malloc(paramStr.length + 1);
        Module.writeStringToMemory(paramStr, StrBuffer);
        
        let heapSpace = Module._malloc(globalObj.arr.length * globalObj.arr.BYTES_PER_ELEMENT); // 1
        Module.HEAPU8.set(globalObj.arr, heapSpace); // 2

        Module._createImageSet(heapSpace, globalObj.dpi, globalObj.w, globalObj.h, globalObj.nc, StrBuffer);

        Module._free(heapSpace);
        Module._free(StrBuffer);
        
        downloadIset();
    }, 500);
}

function downloadIset() {
    let mime = "application/octet-stream";

    let filenameIset = "asa.iset";
    let filenameFset = "asa.fset";
    let filenameFset3 = "asa.fset3";

    let ext = ".iset";
    let ext2 = ".fset";
    let ext3 = ".fset3";

    let content = Module.FS.readFile(filenameIset);
    let contentFset = Module.FS.readFile(filenameFset);
    let contentFset3 = Module.FS.readFile(filenameFset3);

    var a = document.createElement('a');
    a.download = name + ext;
    a.href = URL.createObjectURL(new Blob([content], { type: mime }));
    a.style.display = 'none';

    var b = document.createElement('a');
    b.download = name + ext2;
    b.href = URL.createObjectURL(new Blob([contentFset], { type: mime }));
    b.style.display = 'none';

    var c = document.createElement('a');
    c.download = name + ext3;
    c.href = URL.createObjectURL(new Blob([contentFset3], { type: mime }));
    c.style.display = 'none';

    document.body.appendChild(a);
    a.click();

    document.body.appendChild(b);
    b.click();

    document.body.appendChild(c);
    c.click();

    var spinner = document.querySelector('.spinner-container');
    spinner.style.display = 'none';

    var okSign = document.querySelector('.checkmark-cover');
    okSign.style.display = 'block';
}

function getUint8(str) {
    let base64 = str.substr(23, str.length);
    var raw = atob(base64);
    var rawLength = raw.length;
    var array = new Uint8Array(new ArrayBuffer(rawLength));

    for (i = 0; i < rawLength; i++) {
        array[i] = raw.charCodeAt(i);
    }

    // console.log("arr", array)
    return array;
}

function openModal() {
    let modalWrapper = document.getElementById("modal");
    modalWrapper.style.display = "block";
}

function closeModal() {
    let modalWrapper = document.getElementById("modal");
    modalWrapper.style.display = "none";
}

function setValueFromModal() {
    let input = document.getElementById("modalInput").value;
    globalObj.nc = parseInt(input);
    closeModal();
}

function detectColorSpace(arr) {
    let target = parseInt(arr.length / 4);

    let counter = 0;

    for (let j = 0; j < arr.length; j += 4) {
        let r = arr[j];
        let g = arr[j + 1];
        let b = arr[j + 2];

        if (r == g && r == b) {
            counter++;
        }
    }

    if (target == counter) {
        return 1;
    } else {
        return 3;
    }
}

function useJpeg(e) {
    EXIF.getData(e.target.files[0], function () {
        var dpi1 = parseFloat(EXIF.getTag(this, "XResolution"));

        if (isNaN(dpi1) || dpi1 == null) {
            globalObj.dpi = 72
        } else {
            globalObj.dpi = dpi1;
        }

        var nc1 = EXIF.getTag(this, "ComponentsConfiguration")

        if (isNaN(nc1) || nc1 == null) {
            var nc2 = parseFloat(EXIF.getTag(this, "SamplesPerPixel"));
            if (isNaN(nc2) || nc2 == null) {
                // openModal();
            } else {
                globalObj.nc = nc2;
            }
        } else {
            globalObj.nc = nc1;
        }

        readImage(e);
    });

}

function readImage(e) {
    reader.onload = function (event) {

        var img = new Image();
        img.onload = function () {
            var canvasEl = document.querySelector('#imageCanvas');
            canvas.width = canvasEl.clientWidth;
            canvas.height = canvasEl.clientHeight;

            hideCanvas.width = img.width;
            hideCanvas.height = img.height;

            globalObj.w = img.width;
            globalObj.h = img.height;

            ctxHide.drawImage(img, 0, 0);

            ctx.drawImage(img, 0, 0, img.width, img.height,     // source rectangle
                0, 0, canvas.width, canvas.height); // destination rectangle

            var imgData = ctxHide.getImageData(0, 0, hideCanvas.width, hideCanvas.height);

            let newArr = [];

            let verifyColorSpace = detectColorSpace(imgData.data);

            if (verifyColorSpace == 1) {
                for (let j = 0; j < imgData.data.length; j += 4) {
                    newArr.push(imgData.data[j]);
                }
            } else if (verifyColorSpace == 3) {
                for (let j = 0; j < imgData.data.length; j += 4) {
                    newArr.push(imgData.data[j]);
                    newArr.push(imgData.data[j + 1]);
                    newArr.push(imgData.data[j + 2]);
                }
            }

            globalObj.nc = verifyColorSpace;

            let uint = new Uint8Array(newArr);

            globalObj.arr = uint;

            let confidence = calculateQuality();
            let confidenceEl = document.getElementById("confidenceLevel");
            let childEls = confidenceEl.getElementsByClassName("confidenceEl");
            for(let i = 0; i < parseInt(confidence.l); i++){
                childEls[i].src = "./icons/star.svg";
            }
            confidenceEl.scrollIntoView();
        }
        img.src = event.target.result;
    }
    reader.readAsDataURL(e.target.files[0]);
}

function calculateQuality(){
    let gray = toGrayscale(globalObj.arr);
    let hist = getHistogram(gray);
    let ent = 0;
    let totSize = globalObj.w * globalObj.h;
    for(let i = 0; i < 255; i++){ 
        if(hist[i] > 0){
            let temp = (hist[i]/totSize)*(Math.log(hist[i]/totSize));
            ent += temp;
        }
    }
    
    let entropy = (-1 * ent).toFixed(2);
    let oldRange = (5.17 - 4.6);  
    let newRange = (5 - 0);  
    let level = (((entropy - 4.6) * newRange) / oldRange);
    
    if(level > 5){
        level = 5;
    }else if(level < 0){
        level = 0;
    }
    return {l:level.toFixed(2), e: entropy};
}

function toGrayscale(arr){
    let gray = [];
    for(let i = 0; i < arr.length; i+=3){
        let avg = (arr[i] + arr[i+1] + arr[i+2])/3;
        gray.push(parseInt(avg));
    }
    return gray;
}

function getHistogram(arr){
    let hist = [256];
    for(let i = 0; i < arr.length; i++){
        hist[i] = 0;
    }
    for(let i = 0; i < arr.length; i++){
        hist[arr[i]]++;
    }
    return hist;
}
