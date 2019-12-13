# Builder 

This repository creates the NftMakerCreator.js file used in the marker creator(master branch). Use it only if you want to make changes to the NftMakerCreator.js.

## How to Build

To build it is needed to setup the emscripten enviroment.

1. Clone the dev branch.

2. Download and copy https://github.com/kalwalt/libjpeg-for-jsartoolkit5 to a folder named "libjpeg" in the "emscripten" folder.

3. Download and copy https://github.com/artoolkit/artoolkit5 to a folder named "artoolkit5" in the "emscripten" folder.

4. In the emcc env, run "node tools/makem.js".


## Structure

- main folder

    - build
     
        - Nft-Marker-Creator.min.js

    - emscripten
    
        - artoolkit
        
        - libjpeg
        
        - assemble.c

    - tools
    
        - gen_embind_code.js
        
        - makem.js
     
     - conf.json
     
     - package.json
     
     
 #### Build system created by Kalwalt - https://github.com/kalwalt
