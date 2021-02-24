//
// Copyright (c) 2013 Mikko Mononen memon@inside.org
//
// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.
//

#define NANOVG_GL2_IMPLEMENTATION
#include <stdio.h>
#include <GL/gl.h>

#ifdef NANOVG_GLEW
#  include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>
#include <GL/glext.h>

#include "nanovg.h"
#include "nanovg_gl.h"

#include "orb.h"
#include "jack.h"

#ifndef MAX
#define MAX(A, B) (A > B ? A : B)
#endif

#ifndef MIN
#define MIN(A, B) (A < B ? A : B)
#endif

#ifndef CLAMP
#define CLAMP(N, A, B) (MIN(MAX(N, A), B))
#endif

void errorcb(int error, const char* desc)
{
	printf("GLFW error %d: %s\n", error, desc);
}

int premult = 0;

static orb_data *g_orb = NULL;

static void key(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	NVG_NOTUSED(scancode);
	NVG_NOTUSED(mods);
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);
}

static void mouse_button_callback(GLFWwindow *window, 
    int button,
    int action,
    int mods)
{
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        orb_poke(g_orb);
    }
}

int main()
{
	GLFWwindow* window;
	NVGcontext* vg = NULL;
    orb_data orb;

	if (!glfwInit()) {
		printf("Failed to init GLFW.");
		return -1;
	}
    
    g_orb = &orb;

	glfwSetErrorCallback(errorcb);

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
#ifdef DEMO_MSAA
	glfwWindowHint(GLFW_SAMPLES, 4);
#endif

	window = glfwCreateWindow(960, 600, "NanoVG", NULL, NULL);
//	window = glfwCreateWindow(1000, 600, "NanoVG", glfwGetPrimaryMonitor(), NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}

	glfwSetKeyCallback(window, key);

	glfwMakeContextCurrent(window);
#ifdef NANOVG_GLEW
    if(glewInit() != GLEW_OK) {
		printf("Could not init glew.\n");
		return -1;
	}
#endif

#ifdef DEMO_MSAA
	vg = nvgCreateGL2(NVG_STENCIL_STROKES | NVG_DEBUG);
#else
	vg = nvgCreateGL2(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
#endif
	if (vg == NULL) {
		printf("Could not init nanovg.\n");
		return -1;
	}

	glfwSwapInterval(0);

	glfwSetTime(0);

    glfwSetMouseButtonCallback(window, mouse_button_callback);

    int winWidth, winHeight;
    glfwGetWindowSize(window, &winWidth, &winHeight);
    orb.width = winWidth;
    orb.height = winHeight;
    orb_grid_calculate(&orb);
    orb_init(&orb, 44100);
    orb_start_jack(&orb, 44100);

	while (!glfwWindowShouldClose(window))
	{
		double mx, my;
		int fbWidth, fbHeight;
		float pxRatio;

		glfwGetCursorPos(window, &mx, &my);
		glfwGetWindowSize(window, &winWidth, &winHeight);
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

		pxRatio = (float)fbWidth / (float)winWidth;

		glViewport(0, 0, fbWidth, fbHeight);

        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

		nvgBeginFrame(vg, winWidth, winHeight, pxRatio);

        mx = CLAMP(mx / winWidth, 0, 1);
        my = CLAMP(my / winHeight, 0, 1);

        orb.mouse.x_pos = mx;
        orb.mouse.y_pos = my;

        orb_set_vals(&orb);
        orb.width = winWidth;
        orb.height = winHeight;
        orb_grid_calculate(&orb);

        orb_step(vg, &orb);

		nvgEndFrame(vg);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	nvgDeleteGL2(vg);

	glfwTerminate();
    orb_stop_jack(&orb);
    orb_destroy(&orb);
	return 0;
}
