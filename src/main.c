#define NANOVG_GLES2_IMPLEMENTATION
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <GLES3/gl3.h>
#include <asset_manager.h>
#include <asset_manager_jni.h>
#include <android_native_app_glue.h>
#include <android/sensor.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#include <assert.h>
#include <android/log.h>


/* OpenSL audio */
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

/* for fullscreen/keeping screen on */
#include <android/native_window.h>
#include <android/window.h>

#include "nanovg/nanovg.h"
#include "nanovg/nanovg_gl.h"
#include "nanovg/nanovg_gl_utils.h"

#ifndef APPNAME
#define APPNAME "native-activity"
#endif

#include "orb.h"

/* orb_data orb; */

/* #define LOGI(...) \ */
/*     ((void)__android_log_print(ANDROID_LOG_INFO,\ */
/*                                APPNAME,\ */
/*                                __VA_ARGS__)) */
typedef struct {
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;
    NVGcontext *vg;
    double t, pt;
    int animating;
} gfx_display;

typedef struct {
    SLObjectItf play_obj;
    SLmilliHertz sr_mhz;
    int sr;
    SLPlayItf play;
    int bufsize;
    SLAndroidSimpleBufferQueueItf bq;

    short *nextbuf;
    short *buf[2];
    int whichbuf;
    unsigned nextsize;

    SLObjectItf engine_obj;
    SLEngineItf engine;
    SLObjectItf output_mix;

    void (*init)(void *, int);
    void (*free)(void *);
    void *ud;
} audio_player;


struct UserData {
    orb_data *orb;
};

struct Engine {
    gfx_display gfx;
    audio_player snd;
    void *ud;
    void (*pointer)(void *, int, int, int, int, int);
} Engine;

static void audio_callback(SLAndroidSimpleBufferQueueItf bq,
                           void *context);

static EGLint context_attr [] = {
    EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE
};

static void display_init(gfx_display *gfx, ANativeWindow *window)
{
    const EGLint attribs[] = {
            EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
            EGL_BLUE_SIZE, 8,
            EGL_GREEN_SIZE, 8,
            EGL_RED_SIZE, 8,
            EGL_NONE
    };
    EGLint w, h, format;
    EGLint numConfigs;
    EGLConfig config;
    EGLSurface surface;
    EGLContext context;
    EGLint major, minor;
    EGLConfig *supportedConfigs;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);

    eglInitialize(display, &major, &minor);
    /* LOGI("EGL major: %d minor: %d", major, minor); */
    eglChooseConfig(display, attribs, NULL,0, &numConfigs);
    supportedConfigs = malloc(sizeof(EGLConfig) * numConfigs);
    eglChooseConfig(display, attribs, supportedConfigs, numConfigs, &numConfigs);
    int i = 0;
    for (i = 0; i < numConfigs; i++) {
        EGLConfig *cfg = &supportedConfigs[i];
        EGLint r, g, b, d;
        if (eglGetConfigAttrib(display, cfg, EGL_RED_SIZE, &r)   &&
            eglGetConfigAttrib(display, cfg, EGL_GREEN_SIZE, &g) &&
            eglGetConfigAttrib(display, cfg, EGL_BLUE_SIZE, &b)  &&
            eglGetConfigAttrib(display, cfg, EGL_DEPTH_SIZE, &d) &&
            r == 8 && g == 8 && b == 8 && d == 0 ) {

            config = supportedConfigs[i];
            break;
        }
    }
    if (i == numConfigs) {
        config = supportedConfigs[0];
    }

    eglGetConfigAttrib(display, config, EGL_NATIVE_VISUAL_ID, &format);
    surface = eglCreateWindowSurface(display, config, window, NULL);
    context = eglCreateContext(display, config, EGL_NO_CONTEXT, context_attr);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        /* LOGW("Unable to eglMakeCurrent"); */
        return;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    gfx->display = display;
    gfx->context = context;
    gfx->surface = surface;
    gfx->width = w;
    gfx->height = h;

    glViewport(0, 0, w, h);

    gfx->vg = nvgCreateGLES2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);

    gfx->pt = gfx->t = 0;
}

static void display_destroy(gfx_display* gfx) {
    if (gfx->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(gfx->display,
                       EGL_NO_SURFACE,
                       EGL_NO_SURFACE,
                       EGL_NO_CONTEXT);
        if (gfx->context != EGL_NO_CONTEXT) {
            eglDestroyContext(gfx->display, gfx->context);
        }

        if (gfx->surface != EGL_NO_SURFACE) {
            eglDestroySurface(gfx->display, gfx->surface);
        }

        eglTerminate(gfx->display);
    }
    gfx->animating = 0;
    gfx->display = EGL_NO_DISPLAY;
    gfx->context = EGL_NO_CONTEXT;
    gfx->surface = EGL_NO_SURFACE;
    nvgDeleteGLES2(gfx->vg);
    gfx->vg = NULL;
}

static int32_t engine_handle_input(struct android_app* app,
                                   AInputEvent* event)
{
    /* aavwrap_engine* engine = (aavwrap_engine*)app->userData; */
    int action;
    struct Engine *e;

    e = (struct Engine *)app->userData;
    if (AKeyEvent_getKeyCode(event) == AKEYCODE_BACK) {
        return 1; /* back button will crash the app, so disable it */
    }
    action = AInputEvent_getType(event);
    if (action == AINPUT_EVENT_TYPE_MOTION) {
        int x, y, s;
        int idx;
        int pid;

        e->gfx.animating = 1;
        x = y = s = -1;
        action = AMotionEvent_getAction(event);
        idx = (action &
               AMOTION_EVENT_ACTION_POINTER_INDEX_MASK) >>
            AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT;

        x = AMotionEvent_getX(event, idx);
        y = AMotionEvent_getY(event, idx);
        pid = AMotionEvent_getPointerId(event, idx);

        action &= AMOTION_EVENT_ACTION_MASK;
        switch (action) {
            case AMOTION_EVENT_ACTION_DOWN:
            case AMOTION_EVENT_ACTION_POINTER_DOWN:
                if (e->pointer != NULL) {
                    LOGI("DOWN: %d %d", idx, pid);
                    e->pointer(e->ud, idx, pid, x, y, 1);
                }
                break;

            case AMOTION_EVENT_ACTION_UP:
            case AMOTION_EVENT_ACTION_POINTER_UP:
                if (e->pointer != NULL) {
                    e->pointer(e->ud, idx, pid, x, y, 0);
                }
                break;
        }


        if (e->pointer != NULL) {
            int count, i;
            count = AMotionEvent_getPointerCount(event);

            for (i = 0; i < count; i++) {
                x = AMotionEvent_getX(event, i);
                y = AMotionEvent_getY(event, i);
                pid = AMotionEvent_getPointerId(event, i);
                e->pointer(e->ud, -1, pid, x, y, -1);
            }
        }
        return 1;
    }
    return 0;
}

/* groaning intensifies */
static void create_bbq(audio_player *ap, int sr, int bufsize)
{
    SLresult result;
    SLObjectItf bqPlayerObject;
    SLPlayItf bqPlayerPlay;
    SLAndroidSimpleBufferQueueItf bqPlayerBufferQueue;

    ap->sr_mhz = sr * 1000;
    ap->bufsize = bufsize;

    SLDataLocator_AndroidSimpleBufferQueue loc_bufq =
        {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 2};

    SLDataFormat_PCM format_pcm = {SL_DATAFORMAT_PCM, 1, SL_SAMPLINGRATE_8,
        SL_PCMSAMPLEFORMAT_FIXED_16, SL_PCMSAMPLEFORMAT_FIXED_16,
        SL_SPEAKER_FRONT_CENTER, SL_BYTEORDER_LITTLEENDIAN};

    format_pcm.samplesPerSec = ap->sr_mhz;
    SLDataSource audioSrc = {&loc_bufq, &format_pcm};

    SLDataLocator_OutputMix loc_outmix =
        {SL_DATALOCATOR_OUTPUTMIX, ap->output_mix};
    SLDataSink audioSnk = {&loc_outmix, NULL};

    const SLInterfaceID ids[3] = {
        SL_IID_BUFFERQUEUE,
         SL_IID_VOLUME, SL_IID_EFFECTSEND,
         /*SL_IID_MUTESOLO,*/
    };

    const SLboolean req[3] = {
        SL_BOOLEAN_TRUE,
        SL_BOOLEAN_TRUE,
        SL_BOOLEAN_TRUE,
        /*SL_BOOLEAN_TRUE,*/
    };

    result = (*ap->engine)->CreateAudioPlayer(
        ap->engine,
        &bqPlayerObject,
        &audioSrc, &audioSnk,
        2, ids, req);

    assert(result == SL_RESULT_SUCCESS);
    (void)result;

    result = (*bqPlayerObject)->Realize(bqPlayerObject, SL_BOOLEAN_FALSE);
    assert(result == SL_RESULT_SUCCESS);
    (void)result;

    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_PLAY, &bqPlayerPlay);
    assert(result == SL_RESULT_SUCCESS);
    (void)result;

    result = (*bqPlayerObject)->GetInterface(bqPlayerObject, SL_IID_BUFFERQUEUE,
            &bqPlayerBufferQueue);
    assert(result == SL_RESULT_SUCCESS);
    (void)result;

    result = (*bqPlayerBufferQueue)->RegisterCallback(bqPlayerBufferQueue, &audio_callback, ap);
    assert(result == SL_RESULT_SUCCESS);
    (void)result;

    ap->play_obj = bqPlayerObject;
    ap->play = bqPlayerPlay;
    ap->bq = bqPlayerBufferQueue;
}

static void audio_create(audio_player *ap, int sr, int bufsize)
{
    SLresult result;

    ap->sr = sr;
    ap->bufsize = bufsize;

    result = slCreateEngine(&ap->engine_obj,
                            0, NULL, 0, NULL, NULL);
    assert(result == SL_RESULT_SUCCESS);
    (void)result;

    result = (*ap->engine_obj)->Realize(ap->engine_obj, SL_BOOLEAN_FALSE);
    assert(result == SL_RESULT_SUCCESS);
    (void)result;

    result = (*ap->engine_obj)->GetInterface(ap->engine_obj,
                                        SL_IID_ENGINE,
                                        &ap->engine);
    /* TODO ordering? */
    assert(result == SL_RESULT_SUCCESS);
    (void)result;

    const SLInterfaceID ids[1] = {SL_IID_ENVIRONMENTALREVERB};
    const SLboolean req[1] = {SL_BOOLEAN_FALSE};
    result = (*ap->engine)->CreateOutputMix(ap->engine,
                                            &ap->output_mix,
                                            1, ids, req);

    assert(result == SL_RESULT_SUCCESS);
    (void)result;

    result = (*ap->output_mix)->Realize(ap->output_mix,
                                        SL_BOOLEAN_FALSE);
    assert(result == SL_RESULT_SUCCESS);
    (void)result;

    /* now create the buffer queue audio player */

    create_bbq(ap, sr, bufsize);
}

static void audio_setup(audio_player *ap, int sr, int bufsize)
{
    ap->nextsize = bufsize * sizeof(short);
    ap->buf[0] = malloc(sizeof(short) * bufsize);
    memset(ap->buf[0], 0, sizeof(short) * bufsize);
    ap->buf[1] = malloc(sizeof(short) * bufsize);
    memset(ap->buf[1], 0, sizeof(short) * bufsize);
    ap->nextbuf = ap->buf[0];
    ap->whichbuf = 0;
}

static void audio_shutdown(audio_player *ap)
{
    (*ap->play)->SetPlayState(ap->play, SL_PLAYSTATE_PAUSED);
    if (ap->free != NULL) ap->free(ap->ud);
    if (ap->buf[0] != NULL) {
        free(ap->buf[0]);
        ap->buf[0] = NULL;
    }
    if (ap->buf[1] != NULL) {
        free(ap->buf[1]);
        ap->buf[1] = NULL;
    }
}

static pthread_mutex_t audioEngineLock = PTHREAD_MUTEX_INITIALIZER;

void render(void *ctx, short *buf, int bufsize)
{
    struct UserData *ud;

    ud = ctx;
    orb_audio_computei(ud->orb, buf, bufsize);
}

static void render_a_buffer(audio_player *ap)
{
    int bufsize;
    short *buf;

    bufsize = ap->bufsize;
    buf = ap->buf[ap->whichbuf];

    render(ap->ud, buf, bufsize);
}


static void audio_callback(SLAndroidSimpleBufferQueueItf bq,
                           void *context)
{
    audio_player *ap;

    ap = (audio_player *)context;

    (*ap->bq)->Enqueue(
        ap->bq,
        ap->buf[ap->whichbuf],
        ap->nextsize);

    if (ap->whichbuf == 0) ap->whichbuf = 1;
    else ap->whichbuf = 0;
    render_a_buffer(ap);
}

static void audio_start(audio_player *ap)
{
    SLresult result;

    result = (*ap->play)->SetPlayState(ap->play, SL_PLAYSTATE_PLAYING);
    assert(SL_RESULT_SUCCESS == result);
    (void)result;

    if (pthread_mutex_trylock(&audioEngineLock)) return;

    render_a_buffer(ap);
    result = (*ap->bq)->Enqueue(
        ap->bq,
        ap->buf[ap->whichbuf],
        ap->nextsize);

    if (SL_RESULT_SUCCESS != result) {
        pthread_mutex_unlock(&audioEngineLock);
    }

    if (ap->whichbuf) ap->whichbuf = 0;
    else ap->whichbuf = 1;
}

static void audio_init(audio_player *ap)
{
    /* allocate buffers */
    audio_setup(ap, ap->sr, ap->bufsize);
    if (ap->init != NULL) ap->init(ap->ud, ap->sr);
    /* start up the thing */
    audio_start(ap);
}



static void draw(NVGcontext *vg, void *ctx, double dt)
{
    struct UserData *ud;

    ud = ctx;
    if (ud->orb != NULL) orb_step(vg, ud->orb);
    /* struct UserData *ud; */
    /* int w, h; */
    /* int i; */
    /* float space; */
    /* NVGcolor clr; */

    /* ud = (struct UserData *)context; */

    /* /\* TODO: don't use Engine *\/ */
    /* w = Engine.gfx.width; */
    /* h = Engine.gfx.height; */

    /* clr = nvgRGB(0x69, 0xB0, 0xF0); */

    /* space = w / 8.0; */

    /* nvgStrokeColor(vg, clr); */
    /* nvgStrokeWidth(vg, 8); */

    /* for (i = 0; i < 7; i++) { */
    /*     float xpos; */
    /*     nvgBeginPath(vg); */
    /*     xpos = (i + 1) * space; */
    /*     nvgMoveTo(vg, xpos, 0); */
    /*     nvgLineTo(vg, xpos, h); */
    /*     nvgStroke(vg); */
    /* } */

    /* if (ud->synth != NULL) { */
    /*     for (i = 0; i < NVOICES; i++) { */
    /*         struct Voice *v; */
    /*         v = &ud->synth->v[i]; */
    /*         if (v->x >= 0 && v->y >= 0 && v->a > 0.001) { */
    /*             clr = nvgRGBA(0x69, 0xB0, 0xF0, 0xFF*v->a); */
    /*             nvgBeginPath(vg); */
    /*             /\* TODO: make circle width use DPI *\/ */
    /*             nvgCircle(vg, v->ix, v->iy, (w > h ? w : h) * */
    /*                     (0.1 + 0.01*(1 - v->a))); */
    /*             nvgFillColor(vg, clr); */
    /*             nvgFill(vg); */
    /*         } */

    /*         if (v->s == 1 && v->a < 1) { */
    /*             v->a += dt * 2; */
    /*             if (v->a > 1) v->a = 1; */
    /*         } else if (v->s == 0 && v->a > 0.001) { */
    /*             v->a -= dt * 2; */
    /*             if (v->a < 0) v->a = 0; */
    /*         } */
    /*     } */
    /* } */
}

static double now_sec(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + tv.tv_usec*1e-6;
}

static void draw_frame(gfx_display *gfx, void *ud) {
    NVGcontext *vg;
    int w, h;
    float cs;
    struct Engine *e;

    /* TODO: pass as argument */
    e = &Engine;

    if (gfx->display == NULL) {
        return;
    }

    cs = 1.0 / 255.0;
    vg = gfx->vg;
    w = gfx->width;
    h = gfx->height;
    gfx->pt = gfx->t;
    gfx->t = now_sec();
    /* glClearColor(0x52*cs, 0x0F*cs, 0x53*cs, 1); */
    /* glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT); */
    /* nvgBeginFrame(vg, w, h, 1); */
    draw(vg, ud, gfx->t - gfx->pt);
    /* nvgEndFrame(vg); */
    eglSwapBuffers(gfx->display, gfx->surface);
}

static int amidead = 1;
static void engine_handle_cmd(struct android_app* app, int32_t cmd)
{
    struct Engine *e = (struct Engine*)app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            break;
        case APP_CMD_INIT_WINDOW:
            if (app->window != NULL) {
                display_init(&e->gfx, app->window);
                draw_frame(&e->gfx, e->ud);
            }

            if (amidead) {
                audio_init(&e->snd);
                amidead = 0;
            }
            break;
        case APP_CMD_TERM_WINDOW:
            display_destroy(&e->gfx);
            break;
        case APP_CMD_GAINED_FOCUS:
            e->gfx.animating = 1;
            draw_frame(&e->gfx, e->ud);
            if (amidead) {
                audio_init(&e->snd);
                amidead = 0;
            }
            break;
        case APP_CMD_LOST_FOCUS:
            e->gfx.animating = 1;
            draw_frame(&e->gfx, e->ud);
            if (!amidead) {
                audio_shutdown(&e->snd);
                amidead = 1;
            }
            break;
        case APP_CMD_DESTROY:
            break;
        case APP_CMD_STOP:
            break;
        case APP_CMD_PAUSE:
            if (!amidead) {
                audio_shutdown(&e->snd);
                amidead = 1;
            }
            display_destroy(&e->gfx);
            break;
    }
}

void pointer(void *context, int i, int id, int x, int y, int s)
{
    if (id == 0 && s == 1) {
        struct UserData *ud;
        orb_data *orb;
        ud = context;
        orb = ud->orb;

        if (orb != NULL) {
            float w, h;
            w = Engine.gfx.width;
            h = Engine.gfx.height;

            orb->mouse.y_pos = y / h;
            orb->mouse.x_pos = x / w;
            orb_poke(orb);
        }
    }
}

void synth_init(void *ctx, int sr)
{
    float width, height;
    struct UserData *ud;
    orb_data *orb;

    ud = ctx;
    width = Engine.gfx.width;
    height = Engine.gfx.height;
    orb = malloc(sizeof(orb_data));

    orb_init(orb, sr);
    orb->width = width;
    orb->height = height;
    orb_grid_calculate(orb);
    orb_level_load(orb);

    ud->orb = orb;
}

void synth_free(void *ctx)
{
    struct UserData *ud;
    orb_data *orb;

    ud = ctx;
    orb = ud->orb;
    orb_audio_destroy(orb);
    free(orb);
    ud->orb = NULL;
}

static void hide_navbar(struct android_app* state)
{
	JNIEnv* env;
    JavaVM *vm;

    vm = state->activity->vm;
	(*vm)->AttachCurrentThread(vm, &env, NULL);

	jclass activityClass =
        (*env)->FindClass(env,
                          "android/app/NativeActivity");
	jmethodID getWindow =
        (*env)->GetMethodID(env,
                            activityClass,
                            "getWindow",
                            "()Landroid/view/Window;");

	jclass windowClass =
        (*env)->FindClass(env, "android/view/Window");
	jmethodID getDecorView =
        (*env)->GetMethodID(env,
                            windowClass,
                            "getDecorView",
                            "()Landroid/view/View;");

	jclass viewClass =
        (*env)->FindClass(env, "android/view/View");
	jmethodID setSystemUiVisibility =
        (*env)->GetMethodID(env, viewClass,
                            "setSystemUiVisibility",
                            "(I)V");

	jobject window =
        (*env)->CallObjectMethod(env,
                                 state->activity->clazz,
                                 getWindow);

	jobject decorView =
        (*env)->CallObjectMethod(env, window, getDecorView);

	jfieldID flagImmersiveStickyID =
        (*env)->GetStaticFieldID(env,
                                 viewClass,
                                 "SYSTEM_UI_FLAG_IMMERSIVE_STICKY",
                                 "I");
	jfieldID flagHideNavigationID =
        (*env)->GetStaticFieldID(env,
                                 viewClass,
                                 "SYSTEM_UI_FLAG_HIDE_NAVIGATION",
                                 "I");

	int flagImmersiveSticky =
        (*env)->GetStaticIntField(env,
                                  viewClass,
                                  flagImmersiveStickyID);
	int flagHideNavigation =
        (*env)->GetStaticIntField(env,
                                  viewClass,
                                  flagHideNavigationID);

	int flag = flagImmersiveSticky | flagHideNavigation;

	(*env)->CallVoidMethod(env,
                           decorView,
                           setSystemUiVisibility,
                           flag);

	(*vm)->DetachCurrentThread(vm);
}

void android_main(struct android_app *state)
{
    struct UserData ud;

    state->userData = &Engine;
    state->onAppCmd = engine_handle_cmd;
    state->onInputEvent = engine_handle_input;

    Engine.ud = &ud;
    ud.orb = NULL;
    Engine.pointer = pointer;
    Engine.snd.ud = Engine.ud;
    Engine.snd.init = synth_init;
    Engine.snd.free = synth_free;

    Engine.gfx.animating = 1;

    ANativeActivity_setWindowFlags(state->activity,
                                   AWINDOW_FLAG_FULLSCREEN,
                                   AWINDOW_FLAG_FULLSCREEN | AWINDOW_FLAG_KEEP_SCREEN_ON);


#ifdef LOFI_MODE
    audio_create(&Engine.snd, 22050, 512);
#else
    audio_create(&Engine.snd, 44100, 256);
#endif

    hide_navbar(state);

    while (1) {
        int ident;
        int events;
        struct android_poll_source* source;

        while ((ident=ALooper_pollAll(
                    Engine.gfx.animating ? 0 : -1,
                    NULL,
                    &events,
                    (void**)&source)) >= 0) {

            if (source != NULL) {
                source->process(state, source);
            }

            if (state->destroyRequested != 0) {
                display_destroy(&Engine.gfx);
                return;
            }
        }

        if (Engine.gfx.animating) {
            draw_frame(&Engine.gfx, Engine.ud);
        }
    }
}
