#include "Core/Application.h"

int main() {
    auto app = Syn::CreateApplication();
    app->Run();
    delete app;
    return 0;
}