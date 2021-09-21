# Builder

This repository creates the NftMakerCreator.js file used in the marker creator(master branch). Use it only if you want to make changes to the NftMakerCreator.js.

## How to Build

### Important

If you want to build for the web version with frameworks like Vue.js you need to uncomment the flags below, found on line 121 of `tools/mekem.js`.

```
FLAGS += ' -s MODULARIZE';
FLAGS += ' -s EXPORT_ES6=1';
FLAGS += ' -s USE_ES6_IMPORT_META=0';
```

### Recommended: Build using Docker

1. Install Docker (if you havn't already): [get Docker](https://www.docker.com/)
2. Clone artoolkit5 repository on your machine: `git submodule update --init`
3. `npm install`
4. From inside NFT-Marker-Creator directory run `docker run -dit --name nftCreator -v $(pwd):/src trzeci/emscripten-slim:latest bash` to download and start the container, in preparation for the build
5. `docker exec nftCreator npm run build-local` to build JS version of artoolkit5
6. `docker stop nftCreator` to stop the container after the build, if needed
7. `docker rm nftCreator` to remove the container
8. `docker rmi trzeci/emscripten-slim:latest` to remove the Docker image, if you don't need it anymore

### ⚠️ Not recommended ⚠️ : Build local with manual emscripten setup

To prevent issues with Emscripten setup and to not have to maintain several build environments (macOS, Windows, Linux) we only maintain the **Build using Docker**. Following are the instructions of the last know build on Linux which we verified are working. **Use at own risk.**

1. Install build tools
  1. Install node.js (https://nodejs.org/en/)
  2. Install python2 (https://www.python.org/downloads/)
  3. Install emscripten (https://emscripten.org/docs/getting_started/downloads.html#download-and-install)
    - We used emscripten version 2.0.30
    - On macOS, you can install using [brew](https://brew.sh/): `brew install emscripten`
2. Clone [ARToolKit5 project](https://github.com/artoolkitx/artoolkit5) somewhere to get the latest source files. Then:
  - configure/build ARToolKit
    - on macOS, open `ARToolkit5.xcodeproj` and build
      - you may need to modify ARToolKit5 > Build Settings > Apple Clang - Custom Compiler Flags > Other C Flags and remove "-march=core2"
  - point NFT-Marker-Creator to your ARToolKit5
    - create a link in the `NFT-Marker-Creator/emscripten/` directory that points to ARToolKit5 (`NFT-Marker-Creator/emscripten/artoolkit5`)
    - or, set the `ARTOOLKIT5_ROOT` environment variable to point to your ARToolKit5 clone
    - or, change the `tools/makem.js` file to point to your artoolkit5 clone (line 20)
3. Building
  1. Make sure `EMSCRIPTEN` env variable is set (e.g. `EMSCRIPTEN=/usr/lib/emsdk_portable/emscripten/master/ node tools/makem.js`
    - This is not necessary if you installed emscripten using brew on macOS
  3. Run `npm install`
  4. Run `npm run build-local`

During development, you can run ```npm run watch```, it will rebuild the library everytime you change ```./js/``` directory.

## Structure

- main folder

    - build

        - Nft-Marker-Creator.min.js

    - emscripten

        - artoolkit

        - zlib

        - assemble.c

        - markerCompress.c

        - markerCompress.h

        - wasm_load.js

    - tools

        - gen_embind_code.js

        - makem.js

     - conf.json

     - package.json


 #### Build system created by Kalwalt - https://github.com/kalwalt
