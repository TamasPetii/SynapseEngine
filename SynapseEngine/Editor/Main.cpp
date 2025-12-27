#include <GLFW/glfw3.h>
#include "Engine/Logger/SynLog.h"

int main(void)
{
    Syn::Logger::Get().AddSink(std::make_shared<Syn::ConsoleSink>());
    Syn::Logger::Get().AddSink(std::make_shared<Syn::FileSink>());
    Syn::Logger::Get().AddSink(std::make_shared<Syn::MemorySink>(5000));

    if (!glfwInit())
        return -1;

    GLFWwindow* window = glfwCreateWindow(640, 480, "Teszt Ablak", NULL, NULL);

	Syn::Info("Window Created");

    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    while (!glfwWindowShouldClose(window))
    {
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}