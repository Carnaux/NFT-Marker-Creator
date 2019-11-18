# NFT MARKER CREATOR 

This editor creates NFT markers for ARTOOKIT 5.x

and it is part of the efforts to bring NFT tracking to AR.js

## How to Build

To build it is needed to setup the emscripten enviroment.

1. Clone the dev branch.

2. Download and copy https://github.com/kalwalt/libjpeg-for-jsartoolkit5 to a folder named "libjpeg" in the "emscripten" folder.

3. Download and copy https://github.com/artoolkit/artoolkit5 to a folder named "artoolkit5" in the "emscripten" folder.

4. Edit the .split(/\s+/).join(' PATH TO LIBJPEG') in the file makem.js.

5. In the emcc env, run "node tools/makem.js".


## Structure

- main folder

    - build
     
        - Nft-Marker-Creator.js

    - emscripten
    
        - artoolkit
        
        - libjpeg
        
        - assemble.c

    - tools
    
        - gen_embind_code.js
        
        - makem.js
     
     - conf.json
     
     - package.json
     
     - html/css/js files for testing the Editor
     
 #### Build system created by Kalwalt - https://github.com/kalwalt
