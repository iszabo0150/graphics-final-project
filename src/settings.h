#ifndef SETTINGS_H
#define SETTINGS_H

#include <string>

struct Settings {
    std::string sceneFilePath;
    int shapeParameter1 = 1;
    int shapeParameter2 = 1;
    float nearPlane = 1;
    float farPlane = 1;

    bool perPixelFilter = false;
    bool kernelBasedFilter = false;

    // extra credit toggles
    bool extraCredit1 = false; // particles master toggle
    bool extraCredit2 = false; // bloom toggle
    bool extraCredit3 = false;
    bool extraCredit4 = false;

    // new! particle season sub-toggles (only one should be true at a time)
    bool particlesWinter = true;
    bool particlesSpring = false;
    bool particlesSummer = false;
    bool particlesAutumn = false;

    // new! bloom strength (recommended range [0, 3])
    float bloomStrength = 1.0f;
};

extern Settings settings;

#endif // SETTINGS_H
