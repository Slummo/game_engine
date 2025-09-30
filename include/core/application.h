#pragma once

#include <memory>

class Window;
class Engine;

class Application {
public:
    Application();

    bool init();
    void run();

    ~Application();

private:
    std::unique_ptr<Window> window;
    std::unique_ptr<Engine> engine;
};
