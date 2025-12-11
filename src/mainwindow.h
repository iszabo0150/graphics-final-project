#pragma once

#include <QWidget>
#include <QCheckBox>
#include <QPushButton>
#include <QRadioButton>
#include <QButtonGroup>

#include "realtime/realtime.h"
#include "utils/aspectratiowidget/aspectratiowidget.hpp"

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    void initialize();
    void finish();

private:
    void connectUIElements();
    void connectUploadFile();
    void connectSaveImage();
    void connectExtraCredit();
    void connectParticleSeasons();

    void applyFixedParams();
    void applyPrettyStyle();

    Realtime *realtime = nullptr;
    AspectRatioWidget *aspectRatioWidget = nullptr;

    QPushButton *uploadFile = nullptr;
    QPushButton *saveImage  = nullptr;

    // Particles + seasons
    QCheckBox *ec1 = nullptr; // particles master
    QRadioButton *seasonWinter = nullptr;
    QRadioButton *seasonSpring = nullptr;
    QRadioButton *seasonSummer = nullptr;
    QRadioButton *seasonAutumn = nullptr;
    QButtonGroup *seasonGroup  = nullptr;

    // Other toggles
    QCheckBox *ec2 = nullptr;

private slots:
    void onUploadFile();
    void onSaveImage();

    void onExtraCredit1();
    void onExtraCredit2();

    void onSeasonChanged();
};
