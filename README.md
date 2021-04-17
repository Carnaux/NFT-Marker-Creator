# NFT MARKER CREATOR

This editor creates NFT markers for ARTOOLKIT 5.x, and it is part of the efforts to bring NFT tracking to jsartoolkit5 (and eventually projects like AR.js).

A Node version (preferred) and Web version are provided.

Check out the wiki to learn how to generate good markers! 
https://github.com/Carnaux/NFT-Marker-Creator/wiki/Creating-good-markers

If any errors occours, please, open a issue. 

## ZFT File

The zft file compresses the iset, fset and fset3 files into one!

## Node version

### How to use it

1. Clone this repository.

2. Install all dependencies.

    ` npm install `


3. Put the image you want inside the app folder. You can just paste it or you can create a folder. e.g

     - markerCreatorAppFolder
         - app.js
         - NftMarkerCreator.min.js
         - IMAGE.PNG :arrow_left:
         - ...

     or

     - markerCreatorAppFolder
          - app.js
          - NftMarkerCreator.min.js
          - FOLDER/IMAGE.PNG :arrow_left:
          - ...

4. Run it

    ` node app.js -i PATH/TO/IMAGE`

     In the end of the process an "output" folder will be created(if it does not exist) with the marker files.

You can use additional flags with the run command.

e.g node app.js -i image.png -level=4 -min_thresh=8

    -zft
          Flag for creating only the zft file
    -noConf 
          Disable confirmation after the confidence level
    -Demo
          Creates the demo configuration
    -level=n
         (n is an integer in range 0 (few) to 4 (many). Default 2.'
    -sd_thresh=<sd_thresh>
    -max_thresh=<max_thresh>
    -min_thresh=<min_thresh>
    -leveli=n
         (n is an integer in range 0 (few) to 3 (many). Default 1.'
    -feature_density=<feature_density>
    -dpi=f: 
          Override embedded JPEG DPI value.
    -max_dpi=<max_dpi>
    -min_dpi=<min_dpi>
    -background
         Run in background, i.e. as daemon detached from controlling terminal. (macOS and Linux only.)
    --help -h -?  
          Display this help
   
5. The generated files will be on the "output" folder.

6. (OPTIONAL) You can test your marker using the demo folder!

     - Just run `npm run demo`.

     - It should open a server at: http://localhost:3000/ 

     If you want to create the demo configuration when you create a marker, add `-Demo` to the command parameters.

     e.g node app.js -i image.png -Demo

# Web version

https://carnaux.github.io/NFT-Marker-Creator/

This version is less efficient for images with width and/or height with 1000px or higher.

Advanced options coming soon.

## Instructions

1. Upload any JPG/PNG image.
2. If the Number of Channels is missing, it will ask for it.
3. Click the generate button.
4. Choose between ZFT or the iset, fset, fset3 files.
5. The files will be automatically downloaded when the generation finishes.

------
#### If you want to generate you own NftMarkerCreator.min.js use the dev branch.

