#include "Include/VrApi.h"

#include <GLES3/gl3.h>
#include <android/log.h>

#include <time.h>
#include <stdlib.h>
#include <string.h>

typedef struct ovrMobile {
	int dummy;
} ovrMobile;

typedef struct ovrTextureSwapChain {
	GLuint texture_count;
	GLuint textures[16];
} ovrTextureSwapChain;

typedef int ovrSystemUIType;

ovrInitializeStatus vrapi_Initialize(const ovrInitParms * initParms) {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_Initialize(%p) -> %d", initParms, VRAPI_INITIALIZE_SUCCESS);
	return VRAPI_INITIALIZE_SUCCESS;
}

void vrapi_Shutdown() {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_Shutdown()");
}

bool vrapi_ShowSystemUI(const ovrJava *java, const ovrSystemUIType type) {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_ShowSystemUI(%p, %d) -> false", java, type);
	return false;
}

void vrapi_SetPropertyInt(const ovrJava* java, const ovrProperty propType, const int intVal) {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_SetPropertyInt(%p, %d, %d)", java, propType, intVal);
}

int vrapi_GetSystemPropertyInt(const ovrJava* java, const ovrSystemProperty propType) {
	// todo
	int result = 0;
	
	switch (propType) {
		case VRAPI_SYS_PROP_DEVICE_TYPE:
			result = 0; // todo
			break;
			
		case VRAPI_SYS_PROP_MAX_FULLSPEED_FRAMEBUFFER_SAMPLES:
			result = 2; // todo
			break;
			
		case VRAPI_SYS_PROP_DISPLAY_PIXELS_WIDE:
			result = 2880; // todo, pixel 2 xl for now
			break;
			
		case VRAPI_SYS_PROP_DISPLAY_PIXELS_HIGH:
			result = 1440;
			break;
			
		case VRAPI_SYS_PROP_DISPLAY_REFRESH_RATE:
			result = 60;
			break;
			
		case VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_WIDTH:
			result = 1024; // todo tho the docs say they always return this
			break;
			
		case VRAPI_SYS_PROP_SUGGESTED_EYE_TEXTURE_HEIGHT:
			result = 1024;
			break;
		
		case 128: // VRAPI_SYS_PROP_MULTIVIEW_AVAILABLE
			result = VRAPI_FALSE; // todo
			break;
		
		case VRAPI_SYS_PROP_FOVEATION_AVAILABLE:
			result = VRAPI_FALSE; // todo
			break;
		
		default:
			break;
	}
	
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_GetSystemPropertyInt(%p, %d) -> %d", java, propType, result);
	
	return result;
}

float vrapi_GetSystemPropertyFloat(const ovrJava* java, const ovrSystemProperty propType) {
	// todo
	float result = 0.0f;
	
	switch (propType) {
		case 13: // VRAPI_SYS_PROP_BACK_BUTTON_SHORTPRESS_TIME
			result = 0.15f;
			break;
		
		default:
			break;
	}
	
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_GetSystemPropertyFloat(%p, %d) -> %f", java, propType, result);
	
	return result;
}

ovrResult vrapi_SetPerfThread(ovrMobile* ovr, const ovrPerfThreadType type, const uint32_t threadId) {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_SetPerfThread(%p, %d, %u) -> %d", ovr, type, threadId, ovrSuccess);
	return ovrSuccess;
}

ovrResult vrapi_SetClockLevels(ovrMobile* ovr, const int32_t cpuLevel, const int32_t gpuLevel) {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_SetClockLevels(%p, %d, %d) -> %d", ovr, cpuLevel, gpuLevel, ovrSuccess);
	return ovrSuccess;
}

ovrMobile* vrapi_EnterVrMode(const ovrModeParms* parms) {
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_EnterVrMode(%p)", parms);
	return malloc(sizeof(ovrMobile));
}

void vrapi_LeaveVrMode(ovrMobile* ovr) {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_LeaveVrMode(%p)", ovr);
	free(ovr);
}

double vrapi_GetTimeInSeconds() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	double result = (double)ts.tv_sec + 1e-9 * ts.tv_nsec;
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_GetTimeInSeconds() -> %f", result);
	return result;
}

ovrTextureSwapChain* vrapi_CreateTextureSwapChain(ovrTextureType type, ovrTextureFormat format, int width, int height, int levels, bool buffered) {
	ovrTextureSwapChain *chain = malloc(sizeof *chain);
	memset(chain, 0, sizeof *chain);
	
	if (type != VRAPI_TEXTURE_TYPE_2D) {
		__android_log_print(ANDROID_LOG_WARN, "OpenVRAPI", "Texture arrays not supported!!");
	}
	
	if (format == VRAPI_TEXTURE_FORMAT_8888) {
		__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "Use VRAPI_TEXTURE_FORMAT_8888");
	}
	
	chain->texture_count = buffered ? 3 : 1;
	
	glGenTextures(chain->texture_count, chain->textures);
	
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_CreateTextureSwapChain(type=%d, format=%d, width=%d, height=%d, levels=%d, buffered=%s) -> %p", type, format, width, height, levels, buffered ? "true" : "false", chain);
	return chain;
}

void vrapi_DestroyTextureSwapChain(ovrTextureSwapChain* chain) {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_DestroyTextureSwapChain(%p)", chain);
	
	glDeleteTextures(chain->texture_count, chain->textures);
	free(chain);
}

int vrapi_GetTextureSwapChainLength(ovrTextureSwapChain* chain) {
	int count = chain->texture_count;
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_GetTextureSwapChainLength(%p) -> %d", chain, count);
	return count;
}

unsigned int vrapi_GetTextureSwapChainHandle(ovrTextureSwapChain* chain, int index) {
	int handle = index < chain->texture_count ? chain->textures[index] : 0;
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_GetTextureSwapChainHandle(%p, %d) -> %d", chain, index, handle);
	return handle;
}

ovrResult vrapi_SubmitFrame2(ovrMobile* ovr, const ovrSubmitFrameDescription2* frameDescription) {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_SubmitFrame2(%p, %p) -> %d", ovr, frameDescription, ovrSuccess);
	return ovrSuccess;
}

ovrTracking2 vrapi_GetPredictedTracking2(ovrMobile* ovr, double absTimeInSeconds) {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_GetPredictedTracking2(%p, %f) -> [struct]", ovr, absTimeInSeconds);
	
	ovrTracking2 tracking;
	tracking.Status = VRAPI_TRACKING_STATUS_ORIENTATION_TRACKED | VRAPI_TRACKING_STATUS_ORIENTATION_VALID;
	tracking.HeadPose.Pose.Orientation = (ovrQuatf) {0.0, 0.0, 0.0, 1.0};
	tracking.HeadPose.Pose.Position = (ovrVector3f) {0.0, 0.0, 0.0};
	tracking.HeadPose.AngularVelocity = (ovrVector3f) {0.0, 0.0, 0.0};
	tracking.HeadPose.LinearVelocity = (ovrVector3f) {0.0, 0.0, 0.0};
	tracking.HeadPose.AngularAcceleration = (ovrVector3f) {0.0, 0.0, 0.0};
	tracking.HeadPose.LinearAcceleration = (ovrVector3f) {0.0, 0.0, 0.0};
	tracking.HeadPose.TimeInSeconds = absTimeInSeconds;
	tracking.HeadPose.PredictionInSeconds = vrapi_GetTimeInSeconds() - absTimeInSeconds;
	
	for (size_t k = 0; k < 2; k++) {
		for (size_t j = 0; j < 4; j++) {
			for (size_t i = 0; i < 4; i++) {
				if (i == j) {
					tracking.Eye[k].ProjectionMatrix.M[i][j] = 1.0f;
					tracking.Eye[k].ViewMatrix.M[i][j] = 1.0f;
				}
				else {
					tracking.Eye[k].ProjectionMatrix.M[i][j] = 0.0f;
					tracking.Eye[k].ViewMatrix.M[i][j] = 0.0f;
				}
			}
		}
	}
	
	return tracking;
}

double vrapi_GetPredictedDisplayTime(ovrMobile* ovr, long long frameIndex) {
	// todo
	double res = vrapi_GetTimeInSeconds() + (1.0/60.0/2.0) * frameIndex;
	
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_GetPredictedDisplayTime(%p, %lld) -> %f", ovr, frameIndex, res);
	
	return res;
}
