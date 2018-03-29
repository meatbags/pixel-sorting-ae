#include "PixelSort.hpp"
#include "PixelSortSorter.hpp"

static PF_Err PixelSort8(
	void *refcon,
	A_long xL,
	A_long yL,
	PF_Pixel8 *inP,
	PF_Pixel8 *outP
) {
	PF_Err err = PF_Err_NONE;
	register PixelSortInfo *info = (PixelSortInfo*)refcon;
	AEGP_SuiteHandler suites(info->in_data->pica_basicP);

	// allocate memory
	AEGP_MemHandle mem_handle;
	int mem_size = info->length * sizeof(PixelKey8);
	PixelKey8 *pixels;
	ERR(suites.MemorySuite1()->AEGP_NewMemHandle(NULL, "PixelIndex8[] memory allocation error.", mem_size, AEGP_MemFlag_NONE, &mem_handle));
	ERR(suites.MemorySuite1()->AEGP_LockMemHandle(mem_handle, (void**)&pixels));
	
	// sort
	Sorter8 sorter = Sorter8(xL, yL, info->vec);
	ERR(sorter.sort(info, outP, pixels));

	// free memory
	suites.MemorySuite1()->AEGP_UnlockMemHandle(mem_handle);
	suites.MemorySuite1()->AEGP_FreeMemHandle(mem_handle);

	return err;
}

static PF_Err PixelSort16(
	void *refcon,
	A_long xL,
	A_long yL,
	PF_Pixel16 *inP,
	PF_Pixel16 *outP
) {
	PF_Err err = PF_Err_NONE;

	outP->alpha = inP->alpha;
	outP->red = inP->red;
	outP->green = inP->green;
	outP->blue = inP->blue;
	
	return err;
}

static PF_Err
Render(
	PF_InData *in_data,
	PF_OutData *out_data,
	PF_ParamDef	*params[],
	PF_LayerDef	*output
) {
	PF_Err err = PF_Err_NONE;
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	A_long linesL = output->extent_hint.bottom - output->extent_hint.top;
	PixelSortInfo info;
	PF_EffectWorld *inputP = &params[INPUT_LAYER]->u.ld;
	double qscale = ((double)inputP->width / (double)in_data->width);

	// get user options
	AEFX_CLR_STRUCT(info);
	info.mode = (int)params[PARAM_MODE]->u.pd.value;
	info.key = (int)params[PARAM_KEY]->u.pd.value;
	info.order = (int)params[PARAM_ORDER]->u.pd.value;
	info.angle = FIX2D(params[PARAM_ANGLE]->u.ad.value) * PF_RAD_PER_DEGREE;
	info.vec.set(cos(info.angle), sin(info.angle));
	info.length = (int)params[PARAM_LENGTH]->u.sd.value;
	info.threshold = params[PARAM_THRESHOLD]->u.fs_d.value / 100.0;
	info.centre.set(FIX2D(params[PARAM_CENTRE]->u.td.x_value), FIX2D(params[PARAM_CENTRE]->u.td.y_value));
	info.ref = inputP;
	info.in_data = in_data;
	info.mask_active = params[PARAM_MASK_ACTIVE]->u.bd.value == 1;
	if (info.mask_active) {
		info.mask = &params[PARAM_MASK_LAYER]->u.ld;
		info.mask_scale = FIX2D(params[PARAM_MASK_SCALE]->u.ad.value) / 100.0;
	}

	if (PF_WORLD_IS_DEEP(output)) {
		ERR(suites.Iterate16Suite1()->iterate(in_data, 0, linesL, inputP, NULL, (void*)&info, PixelSort16, output));
	} else {
		ERR(suites.Iterate8Suite1()->iterate(in_data, 0, linesL, inputP, NULL, (void*)&info, PixelSort8, output));
	}

	return err;
}

static PF_Err About(
	PF_InData *in_data,
	PF_OutData *out_data,
	PF_ParamDef *params[],
	PF_LayerDef *output
) {
	AEGP_SuiteHandler suites(in_data->pica_basicP);
	suites.ANSICallbacksSuite1()->sprintf(out_data->return_msg, "%s v%d.%d\r%s", "PixelSort", MAJOR_VERSION, MINOR_VERSION, "Pixel-sorting operations by @meatbags");

	return PF_Err_NONE;
}

static PF_Err GlobalSetup(
	PF_InData *in_data,
	PF_OutData *out_data,
	PF_ParamDef	*params[],
	PF_LayerDef	*output
) {
	out_data->out_flags = PF_OutFlag_DEEP_COLOR_AWARE;
	out_data->my_version = PF_VERSION(MAJOR_VERSION, MINOR_VERSION, BUG_VERSION, STAGE_VERSION, BUILD_VERSION);

	return PF_Err_NONE;
}

static PF_Err ParamsSetup(
	PF_InData *in_data,
	PF_OutData *out_data,
	PF_ParamDef	*params[],
	PF_LayerDef	*output
) {
	PF_ParamDef	def;

	AEFX_CLR_STRUCT(def);
	PF_ADD_POPUP("Mode", 4, 1, "Vector|Vertical|Horizontal|Radial", PARAM_MODE);
	AEFX_CLR_STRUCT(def);
	PF_ADD_POPUP("Compare", 2, 1, "Lightness|Darkness", PARAM_KEY);
	AEFX_CLR_STRUCT(def);
	PF_ADD_POPUP("Order", 4, 1, "Ascending|Descending|Dip|Rise", PARAM_ORDER);
	AEFX_CLR_STRUCT(def);
	PF_ADD_ANGLE("Direction", 45, PARAM_ANGLE);
	AEFX_CLR_STRUCT(def);
	PF_ADD_SLIDER("Length", 1, 3000, 1, 100, 8, PARAM_LENGTH);
	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDER("Threshold", 0, 100, 0, 100, 0, 50, 0, 0, 0, PARAM_THRESHOLD);
	AEFX_CLR_STRUCT(def);
	PF_ADD_POINT("Centre", 0, 0, 0, PARAM_CENTRE);
	AEFX_CLR_STRUCT(def);
	PF_ADD_CHECKBOXX("Use Mask", 0, 0, PARAM_MASK_ACTIVE);
	AEFX_CLR_STRUCT(def);
	PF_ADD_LAYER("Mask Layer", 0, PARAM_MASK_LAYER);
	AEFX_CLR_STRUCT(def);
	PF_ADD_FLOAT_SLIDER("Mask Amount", 0, 100, 0, 100, 0, 100, 0, 0, 0, PARAM_MASK_SCALE);
	out_data->num_params = PARAM_COUNT;

	return PF_Err_NONE;
}

DllExport
PF_Err
EntryPointFunc(
	PF_Cmd cmd,
	PF_InData *in_data,
	PF_OutData *out_data,
	PF_ParamDef *params[],
	PF_LayerDef *output,
	void *extra
) {
	PF_Err err = PF_Err_NONE;
	try {
		switch (cmd) {
		case PF_Cmd_ABOUT:
			err = About(in_data, out_data, params, output);
			break;
		case PF_Cmd_GLOBAL_SETUP:
			err = GlobalSetup(in_data, out_data, params, output);
			break;
		case PF_Cmd_PARAMS_SETUP:
			err = ParamsSetup(in_data, out_data, params, output);
			break;
		case PF_Cmd_RENDER:
			err = Render(in_data, out_data, params, output);
			break;
		}
	}
	catch (PF_Err &thrown_err) {
		err = thrown_err;
	}

	return err;
}
