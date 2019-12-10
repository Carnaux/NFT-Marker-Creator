// meta-programming for generating embinds

var CONSTANTS = [
	/* for arDebug */
	"AR_DEBUG_DISABLE",
	"AR_DEBUG_ENABLE",
	"AR_DEFAULT_DEBUG_MODE",

	/* for arLabelingMode */
	"AR_LABELING_WHITE_REGION",
	"AR_LABELING_BLACK_REGION",
	"AR_DEFAULT_LABELING_MODE",

	/* for arlabelingThresh */
	"AR_DEFAULT_LABELING_THRESH",

	/* for arImageProcMode */
	"AR_IMAGE_PROC_FRAME_IMAGE",
	"AR_IMAGE_PROC_FIELD_IMAGE",
	"AR_DEFAULT_IMAGE_PROC_MODE",

	/* for arPatternDetectionMode */
	"AR_TEMPLATE_MATCHING_COLOR",
	"AR_TEMPLATE_MATCHING_MONO",
	"AR_MATRIX_CODE_DETECTION",
	"AR_TEMPLATE_MATCHING_COLOR_AND_MATRIX",
	"AR_TEMPLATE_MATCHING_MONO_AND_MATRIX",
	"AR_DEFAULT_PATTERN_DETECTION_MODE",

	/* for arMarkerExtractionMode */
	"AR_USE_TRACKING_HISTORY",
	"AR_NOUSE_TRACKING_HISTORY",
	"AR_USE_TRACKING_HISTORY_V2",
	"AR_DEFAULT_MARKER_EXTRACTION_MODE",

	/* for arGetTransMat */
	"AR_MAX_LOOP_COUNT",
	"AR_LOOP_BREAK_THRESH", // double!!
];

var enums = {
	AR_MATRIX_CODE_TYPE: [
		"AR_MATRIX_CODE_3x3",
		"AR_MATRIX_CODE_3x3_HAMMING63",
		"AR_MATRIX_CODE_3x3_PARITY65",
		"AR_MATRIX_CODE_4x4",
		"AR_MATRIX_CODE_4x4_BCH_13_9_3",
		"AR_MATRIX_CODE_4x4_BCH_13_5_5",
	],

	AR_LABELING_THRESH_MODE: [
		"AR_LABELING_THRESH_MODE_MANUAL",
		"AR_LABELING_THRESH_MODE_AUTO_MEDIAN",
		"AR_LABELING_THRESH_MODE_AUTO_OTSU",
		"AR_LABELING_THRESH_MODE_AUTO_ADAPTIVE",
	],

	AR_MARKER_INFO_CUTOFF_PHASE: [
		"AR_MARKER_INFO_CUTOFF_PHASE_NONE",
		"AR_MARKER_INFO_CUTOFF_PHASE_PATTERN_EXTRACTION",
		"AR_MARKER_INFO_CUTOFF_PHASE_MATCH_GENERIC",
		"AR_MARKER_INFO_CUTOFF_PHASE_MATCH_CONTRAST",
		"AR_MARKER_INFO_CUTOFF_PHASE_MATCH_BARCODE_NOT_FOUND",
		"AR_MARKER_INFO_CUTOFF_PHASE_MATCH_BARCODE_EDC_FAIL",
		"AR_MARKER_INFO_CUTOFF_PHASE_MATCH_CONFIDENCE",
		"AR_MARKER_INFO_CUTOFF_PHASE_POSE_ERROR",
		"AR_MARKER_INFO_CUTOFF_PHASE_POSE_ERROR_MULTI",
		"AR_MARKER_INFO_CUTOFF_PHASE_HEURISTIC_TROUBLESOME_MATRIX_CODES",
	],
}


decl_template = '\tenum_<{}>("{}")';
enum_template = '\t\t.value("{}", {})'
for (var key in enums) {
	console.log('\n')
	console.log(decl_template.replace(/\{\}/g, key));
	var list = enums[key];
	list.forEach(function(line) { console.log(enum_template.replace(/\{\}/g, line)) })
	console.log('\t;')
}


// // for ARBindEM.cpp
// var line_template = '\tconstant("{}", {});';
// console.log('EMSCRIPTEN_BINDINGS(constant_bindings) {')
// CONSTANTS.forEach(function(line) { console.log(line_template.replace(/\{\}/g, line)) });
// console.log('}')

// console.log('\n\n\n')

// // for test_embind.html
// var line_template = '\tconsole.log("{}", Module.{});';
// CONSTANTS.forEach(function(line) { console.log(line_template.replace(/\{\}/g, line)) });
