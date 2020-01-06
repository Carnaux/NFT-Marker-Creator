# Builder

This repository creates the NftMakerCreator.js file used in the marker creator(master branch). Use it only if you want to make changes to the NftMakerCreator.js.

## How to Build

To build it is needed to setup the emscripten enviroment.

1. Clone the dev branch.

2. Download and copy https://github.com/artoolkit/artoolkit5 to a folder named "artoolkit5" in the "emscripten" folder or you can save it in another location and configure the environment variable **ARTOOLKIT5_ROOT**.

3. In the emcc env, run:

```
npm run build
```


## Structure

- main folder

    - build

        - Nft-Marker-Creator.min.js

    - emscripten

        - artoolkit

        - assemble.c

    - tools

        - gen_embind_code.js

        - makem.js

     - conf.json

     - package.json


 #### Build system created by Kalwalt - https://github.com/kalwalt
