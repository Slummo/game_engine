#include "core/application.h"

int main(void) {
    Application app;
    if (!app.init()) {
        return 1;
    }

    app.run();
    app.shutdown();

    return 0;
}
