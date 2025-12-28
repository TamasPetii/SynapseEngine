#include <GLFW/glfw3.h>
#include "Engine/Logger/SynLog.h"

#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include <random>
#include "Engine/Engine.h"
#include "Engine/TestComponent.h"
#include "Engine/Registry/Type/TypeInfo.h"
#include "Engine/Registry/Pool/Pool.h"
#include "Engine/Registry/Pool/Mapping/SynMapping.h"
#include "Engine/Registry/Pool/Storage/SynStorage.h"
#include "Engine/Registry/Registry.h"

int main()
{
    Syn::Engine engine;
	engine.Initialize();
    Syn::Info("Editor: Registered TestComponent with TypeID {}", Syn::TypeInfo<Syn::TestComponent>::ID);

    if (!glfwInit()) return -1;
    GLFWwindow* window = glfwCreateWindow(100, 100, "Test", NULL, NULL);
    if (!window) { glfwTerminate(); return -1; }
    glfwMakeContextCurrent(window);
    while (!glfwWindowShouldClose(window)) {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
    return 0;
}