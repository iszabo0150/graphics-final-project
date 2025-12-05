#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QSlider>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>

#include "realtime.h"
#include "utils/aspectratiowidget/aspectratiowidget.hpp"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void initialize();
    void finish();

private:
    void connectUIElements();
    void connectParam1();
    void connectParam2();
    void connectNear();
    void connectFar();

    void connectUploadFile();
    void connectSaveImage();

    void connectExtraCredit();          // ec1..ec4
    void connectParticleSeasons();      // winter/spring/summer/autumn
    void connectBloomStrength();        // slider (and optional box)

    Realtime *realtime = nullptr;
    AspectRatioWidget *aspectRatioWidget = nullptr;

    QPushButton *uploadFile = nullptr;
    QPushButton *saveImage = nullptr;

    QSlider *p1Slider = nullptr;
    QSlider *p2Slider = nullptr;
    QSpinBox *p1Box = nullptr;
    QSpinBox *p2Box = nullptr;

    QSlider *nearSlider = nullptr;
    QSlider *farSlider = nullptr;
    QDoubleSpinBox *nearBox = nullptr;
    QDoubleSpinBox *farBox = nullptr;

    // Extra Credit:
    QCheckBox *ec1 = nullptr; // particles master toggle
    QCheckBox *ec2 = nullptr; // bloom toggle
    QCheckBox *ec3 = nullptr;
    QCheckBox *ec4 = nullptr;

    // Particle season sub-toggles (only one should be true at a time)
    QCheckBox *particlesWinter = nullptr;
    QCheckBox *particlesSpring = nullptr;
    QCheckBox *particlesSummer = nullptr;
    QCheckBox *particlesAutumn = nullptr;

    // Bloom strength UI (drives settings.bloomStrength)
    QSlider *bloomStrengthSlider = nullptr;       // e.g. 0..300 maps to 0.0..3.0
    QDoubleSpinBox *bloomStrengthBox = nullptr;   // optional but very nice

private slots:
    void onUploadFile();
    void onSaveImage();

    void onValChangeP1(int newValue);
    void onValChangeP2(int newValue);
    void onValChangeNearSlider(int newValue);
    void onValChangeFarSlider(int newValue);
    void onValChangeNearBox(double newValue);
    void onValChangeFarBox(double newValue);

    // Extra Credit:
    void onExtraCredit1();
    void onExtraCredit2();
    void onExtraCredit3();
    void onExtraCredit4();

    // Particle season sub-toggles
    void onParticlesWinter();
    void onParticlesSpring();
    void onParticlesSummer();
    void onParticlesAutumn();

    // Bloom strength
    void onBloomStrengthSlider(int v);
    void onBloomStrengthBox(double v);
};
