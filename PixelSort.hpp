#pragma once
#ifndef PixelSort_H
#define PixelSort_H

#define PF_TABLE_BITS 12
#define PF_TABLE_SZ_16 4096
#define PF_DEEP_COLOR_AWARE 1
#include "AEConfig.h"

#ifdef AE_OS_WIN
typedef unsigned short PixelType;
#include <Windows.h>
#endif

#include <math.h>
#include "entry.h"
#include "AE_Effect.h"
#include "AE_EffectCB.h"
#include "AE_Macros.h"
#include "Param_Utils.h"
#include "AE_EffectCBSuites.h"
#include "String_Utils.h"
#include "AE_GeneralPlug.h"
#include "AEFX_ChannelDepthTpl.h"
#include "AEGP_SuiteHandler.h"
#include "PixelSortVector.hpp"

#define	MAJOR_VERSION 1
#define	MINOR_VERSION 0
#define	BUG_VERSION	0
#define	STAGE_VERSION PF_Stage_DEVELOP
#define	BUILD_VERSION 1

enum {
	INPUT_LAYER = 0,
	PARAM_MODE,
	PARAM_KEY,
	PARAM_ORDER,
	PARAM_ANGLE,
	PARAM_LENGTH,
	PARAM_CENTRE,
	PARAM_MASK_ACTIVE,
	PARAM_MASK_LAYER,
	PARAM_MASK_SCALE,
	PARAM_COUNT
};

typedef struct {
	int mode;
	int key;
	int order;
	double angle;
	double length;
	Vector vec = Vector(0, 0);
	Vector centre = Vector(0, 0);
	PF_EffectWorld *ref;
	PF_EffectWorld *mask;
	bool mask_active;
	double mask_scale;
} PixelSortInfo;

#ifdef __cplusplus
extern "C" {
#endif

	DllExport PF_Err
	EntryPointFunc(
		PF_Cmd cmd,
		PF_InData *in_data,
		PF_OutData *out_data,
		PF_ParamDef *params[],
		PF_LayerDef	*output,
		void *extra
	);

#ifdef __cplusplus
}
#endif
#endif // PixelSort_H