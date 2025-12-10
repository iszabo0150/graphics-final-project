#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

struct Settings {
    std::string sceneFilePath;
    int shapeParameter1 = 1;
    int shapeParameter2 = 1;
    float nearPlane = 0.1f;
    float farPlane = 400.f;
    bool perPixelFilter = false;
    bool kernelBasedFilter = false;
    bool extraCredit1 = false;
    bool extraCredit2 = false;
    bool extraCredit3 = false;
    bool extraCredit4 = false;

    // Particle season selection (treat like radio buttons)
    bool particlesWinter = true;
    bool particlesSpring = false;
    bool particlesSummer = false;
    bool particlesAutumn = false;
};


// The global Settings object, will be initialized by MainWindow
extern Settings settings;

#endif // SETTINGS_H
