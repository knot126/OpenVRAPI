#include "Include/VrApi.h"

#include <GLES3/gl3.h>
#include <EGL/egl.h>
#include <android/log.h>

#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define GL_CHECK(S) if ((e = glGetError()) != GL_NO_ERROR) { __android_log_print(ANDROID_LOG_FATAL, "OpenVRAPI", "Error in %s: %s: 0x%x", __FUNCTION__, S, e); abort(); }
#define GL_QCHK() if ((e = glGetError()) != GL_NO_ERROR) { __android_log_print(ANDROID_LOG_FATAL, "OpenVRAPI", "Error in %s: line %d: 0x%x", __FUNCTION__, __LINE__, e); abort(); }
#define BLIT_WITH_SHADER 1

typedef struct ovrMobile {
	ovrModeParms params;
	EGLContext egl_context;
	EGLDisplay egl_display;
	EGLSurface egl_surface;
	
#ifdef BLIT_WITH_SHADER
	GLuint blit_program;
#endif
} ovrMobile;

typedef struct ovrTextureSwapChain {
	// Render textures
	GLuint texture_count;
	GLuint textures[16];
} ovrTextureSwapChain;

typedef int ovrSystemUIType;

// HACK idfk what im supposed to do
ovrTextureSwapChain *gSwapChain;
ovrMobile *gOvr;

ovrInitializeStatus vrapi_Initialize(const ovrInitParms * initParms) {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_Initialize(%p) -> %d", initParms, VRAPI_INITIALIZE_SUCCESS);
	return VRAPI_INITIALIZE_SUCCESS;
}

void vrapi_Shutdown() {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_Shutdown()");
}

#ifdef BLIT_WITH_SHADER
#include "basic_glsl.h"

static GLuint LoadShader(GLenum type, const char *text) {
	GLuint shader;
	GLint status;
	
	shader = glCreateShader(type);
	
	const char *sourceCode[] = {
		"#version 300 es\nprecision mediump float;\n",
		(type == GL_VERTEX_SHADER) ? "#define VERTEX\n" : "#define FRAGMENT\n",
		text,
	};
	
	glShaderSource(shader, 3, sourceCode, NULL);
	
	glCompileShader(shader);
	
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	
	if (!status) {
		char log[2048] = {};
		glGetShaderInfoLog(shader, 2048, NULL, log);
		__android_log_print(ANDROID_LOG_FATAL, "OpenVRAPI", "%s shader compile failed: %s", (type==GL_VERTEX_SHADER) ? "Vertex" : "Fragment", log);
		abort();
	}
	
	return shader;
}

static GLuint LoadProgram(const char *text) {
	GLenum e;
	
	GLuint vert = LoadShader(GL_VERTEX_SHADER, text);
	GLuint frag = LoadShader(GL_FRAGMENT_SHADER, text);
	
	GLuint prog = glCreateProgram();
	
	if (!prog) {
		__android_log_print(ANDROID_LOG_FATAL, "OpenVRAPI", "Program creation failed");
		abort();
	}
	
	glAttachShader(prog, vert); GL_QCHK();
	glAttachShader(prog, frag); GL_QCHK();
	
	// glBindAttribLocation(prog, 0, "inPos");
	// glBindAttribLocation(prog, 1, "inTexCoord");
	
	glLinkProgram(prog);
	
	GLint status;
	glGetProgramiv(prog, GL_LINK_STATUS, &status);
	
	if (!status) {
		__android_log_print(ANDROID_LOG_FATAL, "OpenVRAPI", "Program failed to link");
		abort();
	}
	
	return prog;
}

struct XVertex {
	float x, y;
	float u, v;
};

struct XVertex gdata[] = {
	(struct XVertex) {0.0, 0.0, 0.0, 0.0},
	(struct XVertex) {0.0, 1.0, 0.0, 0.0},
	(struct XVertex) {1.0, 0.0, 0.0, 0.0},
};

static void DrawWithProgram(GLuint prog, GLuint tex) {
	GLenum e;
	
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_DEPTH_TEST);
	
	glUseProgram(prog); GL_QCHK();
	
	// Set texture
	glActiveTexture(GL_TEXTURE0); GL_QCHK();
	glBindTexture(GL_TEXTURE_2D, tex); GL_QCHK();
	
	// Set sampler to use texture zero
	GLint loc = glGetUniformLocation(prog, "uTexture"); GL_QCHK();
	glUniform1i(loc, 0); GL_QCHK();
	
	// Setup inPos and inTexCoord
	GLint inPosLoc = glGetAttribLocation(prog, "inPos"); GL_QCHK();
	if (inPosLoc != -1) {
		glVertexAttribPointer(inPosLoc, 2, GL_FLOAT, GL_FALSE, sizeof(struct XVertex), &gdata[0].x); GL_QCHK();
		glEnableVertexAttribArray(inPosLoc); GL_QCHK();
	}
	
	GLint inTexCoordLoc = glGetAttribLocation(prog, "inTexCoord"); GL_QCHK();
	if (inTexCoordLoc != -1) {
		glVertexAttribPointer(inTexCoordLoc, 2, GL_FLOAT, GL_FALSE, sizeof(struct XVertex), &gdata[0].u); GL_QCHK();
		glEnableVertexAttribArray(inTexCoordLoc); GL_QCHK();
	}
	
	// Draw
	glDrawArrays(GL_TRIANGLES, 0, 3); GL_QCHK();
}
#endif

ovrMobile* vrapi_EnterVrMode(const ovrModeParms* parms) {
	ovrMobile *ovr = malloc(sizeof *ovr);
	memset(ovr, 0, sizeof *ovr);
	
	ovr->params = *parms;
	ovr->egl_context = (EGLContext) ovr->params.ShareContext;
	ovr->egl_display = (EGLDisplay) ovr->params.Display;
	
	// Get an EGLSurface from a ANativeDisplay
	if (ovr->params.Flags & VRAPI_MODE_FLAG_NATIVE_WINDOW) {
		EGLint err;
		EGLint config_id;
		EGLConfig config;
		
		// Get the id of the context's config
		eglQueryContext(ovr->egl_display, ovr->egl_context, EGL_CONFIG_ID, &config_id);
		
		if ((err = eglGetError()) != EGL_SUCCESS) {
			__android_log_print(ANDROID_LOG_FATAL, "OpenVRAPI", "eglQueryContext failed: %d", err);
			abort();
		}
		
		// Get the EGLConfig from the config id
		EGLint config_attribs[] = {EGL_CONFIG_ID, config_id, EGL_NONE};
		EGLint config_count = 0;
		
		eglChooseConfig(ovr->egl_display, config_attribs, &config, 1, &config_count);
		
		if ((err = eglGetError()) != EGL_SUCCESS) {
			__android_log_print(ANDROID_LOG_FATAL, "OpenVRAPI", "eglChooseConifg failed: %d", err);
			abort();
		}
		
		// Create EGL surface from native window
		ovr->egl_surface = eglCreateWindowSurface(ovr->egl_display, config, (void*)ovr->params.WindowSurface, NULL);
		
		if ((err = eglGetError()) != EGL_SUCCESS) {
			__android_log_print(ANDROID_LOG_FATAL, "OpenVRAPI", "eglCreateWindowSurface failed: %d", err);
			abort();
		}
		
		// Actually bind the context and surface
		eglMakeCurrent(ovr->egl_display, ovr->egl_surface, ovr->egl_surface, ovr->egl_context);
		
		if ((err = eglGetError()) != EGL_SUCCESS) {
			__android_log_print(ANDROID_LOG_FATAL, "OpenVRAPI", "eglMakeCurrent failed: %d", err);
			abort();
		}
	}
	
#ifdef BLIT_WITH_SHADER
	ovr->blit_program = LoadProgram(shaderSource);
#endif
	
	gOvr = ovr;
	
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_EnterVrMode(%p)", parms);
	return ovr;
}

void vrapi_LeaveVrMode(ovrMobile* ovr) {
	// todo
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_LeaveVrMode(%p)", ovr);
	free(ovr);
	gOvr = NULL;
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

double vrapi_GetTimeInSeconds() {
	struct timespec ts;
	clock_gettime(CLOCK_REALTIME, &ts);
	double result = (double)ts.tv_sec + 1e-9 * ts.tv_nsec;
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_GetTimeInSeconds() -> %f", result);
	return result;
}

void default_texture(GLuint texId) {
	GLenum e;
	char content[] = {255, 255, 255, 0, 0, 0, 255, 255, 255, 0, 0, 0};
	
	glActiveTexture(GL_TEXTURE0); GL_CHECK("glActiveTexture");
	glBindTexture(GL_TEXTURE_2D, texId); GL_CHECK("glBindTexture");
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, content); GL_CHECK("glTexImage2D");
}

ovrTextureSwapChain* vrapi_CreateTextureSwapChain(ovrTextureType type, ovrTextureFormat format, int width, int height, int levels, bool buffered) {
	ovrTextureSwapChain *chain = malloc(sizeof *chain);
	memset(chain, 0, sizeof *chain);
	
	if (!gSwapChain) {
		gSwapChain = chain;
	}
	
	if (type != VRAPI_TEXTURE_TYPE_2D) {
		__android_log_print(ANDROID_LOG_WARN, "OpenVRAPI", "Texture arrays not supported!!");
	}
	
	if (format == VRAPI_TEXTURE_FORMAT_8888) {
		__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "Use VRAPI_TEXTURE_FORMAT_8888");
	}
	
	chain->texture_count = buffered ? 3 : 1;
	
	glGenTextures(chain->texture_count, chain->textures);
	
	for (size_t i = 0; i < chain->texture_count; i++) {
		default_texture(chain->textures[i]);
	}
	
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

// void copy_framebuffer(GLuint texId) {
// 	GLenum e;
// 	
// 	if (glIsTexture(texId) != GL_TRUE) {
// 		__android_log_print(ANDROID_LOG_WARN, "OpenVRAPI", "texId=%d is not a valid texture, won't blit", texId);
// 		return;
// 	}
// 	
// 	default_texture(texId);
// 	
// 	// FB for texture
// 	GLuint fboId = 0;
// 	glGenFramebuffers(1, &fboId); GL_CHECK("glGenFramebuffers");
// 	glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId); GL_CHECK("glBindFramebuffer(GL_READ_FRAMEBUFFER, fboId)");
// 	glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texId, 0); GL_CHECK("glFramebufferTexture2D");
// 	
// 	// Bind default draw buffer and blit
// 	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0); GL_CHECK("glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0)");
// 	
// 	// Assert that framebuffers are complete
// 	GLenum readStatus = glCheckFramebufferStatus(GL_READ_FRAMEBUFFER), drawStatus = glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER);
// 	
// 	if (readStatus != GL_FRAMEBUFFER_COMPLETE || drawStatus != GL_FRAMEBUFFER_COMPLETE) {
// 		__android_log_print(ANDROID_LOG_WARN, "OpenVRAPI", "Incomplete framebuffer(s): read=0x%x draw=0x%x", readStatus, drawStatus);
// 		// abort();
// 	}
// 	else {
// 		glBlitFramebuffer(0, 0, 1024, 1024, 0, 0, 1024, 1024, GL_COLOR_BUFFER_BIT, GL_LINEAR); GL_CHECK("glBlitFramebuffer");
// 	}
// 	
// 	// Destroy FB
// 	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); GL_CHECK("glBindFramebuffer(GL_READ_FRAMEBUFFER, 0)");
// 	glDeleteFramebuffers(1, &fboId); GL_CHECK("glDeleteFramebuffers");
// }

ovrResult vrapi_SubmitFrame2(ovrMobile* ovr, const ovrSubmitFrameDescription2* frameDescription) {
	// todo
	EGLint err;
	
	__android_log_print(ANDROID_LOG_INFO, "OpenVRAPI", "vrapi_SubmitFrame2 Flags=0x%x SwapInterval=%d FrameIndex=%llu DisplayTime=%f", frameDescription->Flags, frameDescription->SwapInterval, frameDescription->FrameIndex, frameDescription->DisplayTime);
	
	// EGLSurface surface = eglGetCurrentSurface( EGL_DRAW );
	eglMakeCurrent(ovr->egl_display, ovr->egl_surface, ovr->egl_surface, ovr->egl_context);
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
	
	// glViewport(0,0,1024,1024);
	glClearColor(0.0, 1.0, frameDescription->FrameIndex/300.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	
	if (gSwapChain) {
		// copy_framebuffer(gSwapChain->textures[0]);
		// GLuint tex;
		// glGenTextures(1, &tex);
		// default_texture(tex);
		// glDeleteTextures(1, &tex);
#ifdef BLIT_WITH_SHADER
		DrawWithProgram(ovr->blit_program, gSwapChain->textures[0]);
#endif
	}
	else {
		__android_log_print(ANDROID_LOG_FATAL, "OpenVRAPI", "gSwapChain is null");
		abort();
	}
	
	eglSwapBuffers(ovr->egl_display, ovr->egl_surface);
	
	if ((err = eglGetError()) != EGL_SUCCESS) {
		__android_log_print(ANDROID_LOG_FATAL, "OpenVRAPI", "eglSwapBuffers failed: %d", err);
		abort();
	}
	
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
