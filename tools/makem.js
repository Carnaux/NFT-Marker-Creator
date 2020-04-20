/*
 * Simple script for running emcc on ARToolKit
 * @author zz85 github.com/zz85
 */

var
	exec = require('child_process').exec,
	path = require('path'),
	fs = require('fs'),
	child;

var HAVE_NFT = 1;

var EMSCRIPTEN_ROOT = process.env.EMSCRIPTEN;
var ARTOOLKIT5_ROOT = process.env.ARTOOLKIT5_ROOT || "../emscripten/artoolkit5";

if (!EMSCRIPTEN_ROOT) {
	console.log("\nWarning: EMSCRIPTEN environment variable not found.")
	console.log("If you get a \"command not found\" error,\ndo `source <path to emsdk>/emsdk_env.sh` and try again.");
}

var EMCC = EMSCRIPTEN_ROOT ? path.resolve(EMSCRIPTEN_ROOT, 'emcc') : 'emcc';
var EMPP = EMSCRIPTEN_ROOT ? path.resolve(EMSCRIPTEN_ROOT, 'em++') : 'em++';
var OPTIMIZE_FLAGS = ' -Oz '; // -Oz for smallest size
var MEM = (256 *1024 * 1024) ; // 64MB


var SOURCE_PATH = path.resolve(__dirname, '../emscripten/') + '/';
var OUTPUT_PATH = path.resolve(__dirname, '../build/') + '/';

var BUILD_DEBUG_FILE = 'NftMarkerCreator.debug.js';
var BUILD_WASM_FILE = 'NftMarkerCreator_wasm.js';
var BUILD_MIN_FILE = 'NftMarkerCreator.min.js';

var MAIN_SOURCES = [
	'assemble.c',
];

MAIN_SOURCES = MAIN_SOURCES.map(function(src) {
	return path.resolve(SOURCE_PATH, src);
}).join(' ');

let srcTest = path.resolve(__dirname, ARTOOLKIT5_ROOT + '/lib/SRC/');

var ar_sources = [
    'Video/video.c',
    'ARUtil/log.c',
    'ARUtil/file_utils.c',
].map(function(src) {
	return path.resolve(__dirname, ARTOOLKIT5_ROOT + '/lib/SRC/', src);
});

var ar2_sources = [
	'handle.c',
	'imageSet.c',
	'jpeg.c',
	'marker.c',
	'featureMap.c',
	'featureSet.c',
	'selectTemplate.c',
	'surface.c',
	'tracking.c',
	'tracking2d.c',
	'matching.c',
	'matching2.c',
	'template.c',
	'searchPoint.c',
	'coord.c',
	'util.c',
].map(function(src) {
	return path.resolve(__dirname, ARTOOLKIT5_ROOT + '/lib/SRC/AR2/', src);
});

var kpm_sources = [
	'KPM/kpmHandle.cpp',
	'KPM/kpmRefDataSet.cpp',
	'KPM/kpmMatching.cpp',
	'KPM/kpmResult.cpp',
	'KPM/kpmUtil.cpp',
	'KPM/kpmFopen.c',
	'KPM/FreakMatcher/detectors/DoG_scale_invariant_detector.cpp',
	'KPM/FreakMatcher/detectors/gaussian_scale_space_pyramid.cpp',
	'KPM/FreakMatcher/detectors/gradients.cpp',
	'KPM/FreakMatcher/detectors/harris.cpp',
	'KPM/FreakMatcher/detectors/orientation_assignment.cpp',
	'KPM/FreakMatcher/detectors/pyramid.cpp',
	'KPM/FreakMatcher/facade/visual_database_facade.cpp',
	'KPM/FreakMatcher/matchers/hough_similarity_voting.cpp',
	'KPM/FreakMatcher/matchers/freak.cpp',
	'KPM/FreakMatcher/framework/date_time.cpp',
	'KPM/FreakMatcher/framework/image.cpp',
	'KPM/FreakMatcher/framework/logger.cpp',
	'KPM/FreakMatcher/framework/timers.cpp',
].map(function(src) {
	return path.resolve(__dirname, ARTOOLKIT5_ROOT + '/lib/SRC/', src);
});

if (HAVE_NFT) {
	ar_sources = ar_sources
	.concat(ar2_sources)
	.concat(kpm_sources);
}

var DEFINES = ' ';
if (HAVE_NFT) DEFINES += ' -D HAVE_NFT ';

var FLAGS = '' + OPTIMIZE_FLAGS;
// var FLAGS = '';
FLAGS += ' -Wno-warn-absolute-paths ';
FLAGS += ' -s TOTAL_MEMORY=' + MEM + ' ';
FLAGS += ' -s ALLOW_MEMORY_GROWTH=1 ';
FLAGS += ' -s USE_ZLIB=1';
FLAGS += ' -s USE_LIBJPEG=1';
FLAGS += ' -s ASSERTIONS=1';
FLAGS += ' --memory-init-file 0 '; // for memless file
FLAGS += ' -s FORCE_FILESYSTEM=1';

// var WASM_FLAGS = ' -s BINARYEN_TRAP_MODE=clamp';
var WASM_FLAGS = "";
var PRE_FLAGS = ' --pre-js ' + path.resolve(__dirname, '../emscripten/wasm_loader.js') +' ';

var EXPORTED_FUNCTIONS = ' -s EXPORTED_FUNCTIONS=["_createImageSet"] -s EXTRA_EXPORTED_RUNTIME_METHODS=["FS, writeStringToMemory"] ';

/* DEBUG FLAGS */
var DEBUG_FLAGS = ' -g ';
// DEBUG_FLAGS += ' -s ASSERTIONS=2 '
DEBUG_FLAGS += ' -s ASSERTIONS=1 '
DEBUG_FLAGS += ' --profiling '
// DEBUG_FLAGS += ' -s EMTERPRETIFY_ADVISE=1 '
DEBUG_FLAGS += ' -s ALLOW_MEMORY_GROWTH=1';
DEBUG_FLAGS += '  -s DEMANGLE_SUPPORT=1 ';

var INCLUDES = [
	path.resolve(__dirname, ARTOOLKIT5_ROOT + '/include'),
	OUTPUT_PATH,
	SOURCE_PATH,
	path.resolve(__dirname, ARTOOLKIT5_ROOT + '/lib/SRC/KPM/FreakMatcher'),
].map(function(s) { return '-I' + s }).join(' ');

function format(str) {
	for (var f = 1; f < arguments.length; f++) {
		str = str.replace(/{\w*}/, arguments[f]);
	}
	return str;
}

function clean_builds() {
	try {
		var stats = fs.statSync(OUTPUT_PATH);
	} catch (e) {
		fs.mkdirSync(OUTPUT_PATH);
	}

	try {
		var files = fs.readdirSync(OUTPUT_PATH);
		if (files.length > 0)
		for (var i = 0; i < files.length; i++) {
			var filePath = OUTPUT_PATH + '/' + files[i];
			if (fs.statSync(filePath).isFile())
				fs.unlinkSync(filePath);
		}
	}
	catch(e) { return console.log(e); }
}

var compile_arlib = format(EMCC + ' ' + INCLUDES + ' '
	+ ar_sources.join(' ')
	+ FLAGS + ' ' + DEFINES + ' -o {OUTPUT_PATH}libar.bc ',
		OUTPUT_PATH);

 var compile_kpm = format(EMCC + ' ' + INCLUDES + ' '
 	+ kpm_sources.join(' ')
 	+ FLAGS + ' ' + DEFINES + ' -o {OUTPUT_PATH}libkpm.bc ',
 		OUTPUT_PATH);

var compile_combine = format(EMCC + ' ' + INCLUDES + ' '
	+ ' {OUTPUT_PATH}libar.bc ' + MAIN_SOURCES
	+ FLAGS + ' -s WASM=0' + ' '  + DEBUG_FLAGS + DEFINES + ' -o {OUTPUT_PATH}{BUILD_FILE} ',
	 OUTPUT_PATH, OUTPUT_PATH, BUILD_DEBUG_FILE);

var compile_combine_min = format(EMCC + ' '  + INCLUDES + ' '
	+ ' {OUTPUT_PATH}libar.bc ' + MAIN_SOURCES + EXPORTED_FUNCTIONS
	+ FLAGS + ' -s WASM=0' + ' ' + DEFINES  + ' -o {OUTPUT_PATH}{BUILD_FILE} ',
 	OUTPUT_PATH, OUTPUT_PATH, BUILD_MIN_FILE);

var compile_wasm = format(EMCC + ' ' + INCLUDES + ' '
	+ ' {OUTPUT_PATH}libar.bc ' + MAIN_SOURCES + EXPORTED_FUNCTIONS
	+ FLAGS + WASM_FLAGS + DEFINES + PRE_FLAGS + ' -o {OUTPUT_PATH}{BUILD_FILE} ',
	 OUTPUT_PATH, OUTPUT_PATH, BUILD_WASM_FILE);

/*
var compile_all = format(EMCC + ' ' + INCLUDES + ' '
	+ ar_sources.join(' ')
	+ FLAGS + ' ' + DEFINES + ' -o {OUTPUT_PATH}{BUILD_FILE} ',
		OUTPUT_PATH, BUILD_FILE);
*/
var compile_all = format(EMCC + ' ' + INCLUDES + ' '
	+ ar_sources.join(' ')
	+ FLAGS + ' ' + DEFINES + ' -o {OUTPUT_PATH}{BUILD_FILE} ',
		OUTPUT_PATH, BUILD_DEBUG_FILE);

/*
 * Run commands
 */

function onExec(error, stdout, stderr) {
	if (stdout) console.log('stdout: ' + stdout);
	if (stderr) console.log('stderr: ' + stderr);
	if (error !== null) {
		console.log('exec error: ' + error.code);
		process.exit(error.code);
	} else {
		runJob();
	}
}

function runJob() {
	if (!jobs.length) {
		console.log('Jobs completed');
		return;
	}
	var cmd = jobs.shift();

	if (typeof cmd === 'function') {
		cmd();
		runJob();
		return;
	}

	console.log('\nRunning command: ' + cmd + '\n');
	exec(cmd, onExec);
}

var jobs = [];

function addJob(job) {
	jobs.push(job);
}

addJob(clean_builds);
addJob(compile_arlib);
//addJob(compile_kpm);
//addJob(compile_combine);
addJob(compile_wasm);
addJob(compile_combine_min);
// addJob(compile_all);

runJob();
