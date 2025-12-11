#include "mainwindow.h"
#include "settings.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QGroupBox>
#include <QDir>
#include <iostream>

void MainWindow::applyFixedParams() {
    // Hard-lock these globally
    settings.shapeParameter1 = 5;
    settings.shapeParameter2 = 5;
    settings.nearPlane = 0.10f;
    settings.farPlane  = 1000.f;
}

void MainWindow::applyPrettyStyle() {
    this->setStyleSheet(R"(
    QWidget {
      background: #0b0a12;
      color: #f5f1ff;
      font-size: 13px;
    }

    /* KEY FIX: text widgets should not paint a background */
    QLabel, QCheckBox, QRadioButton, QGroupBox::title {
      background: transparent;
    }

    /* If you see “boxed” text anywhere, this forces transparency */
    * {
      background-clip: padding;
    }

    QGroupBox {
      background: rgba(24, 18, 34, 210);
      border: 1px solid rgba(241, 179, 166, 120);
      border-radius: 14px;
      margin-top: 14px;
      padding: 12px;
    }
    QGroupBox::title {
      subcontrol-origin: margin;
      subcontrol-position: top left;
      padding: 0 8px;
      color: rgba(245, 241, 255, 235);
      font-weight: 700;
      letter-spacing: 0.5px;
    }

    QPushButton {
      background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                                  stop:0 rgba(241, 179, 166, 220),
                                  stop:1 rgba(201, 178, 215, 220));
      color: #14101f;
      border: 1px solid rgba(255,255,255,60);
      border-radius: 12px;
      padding: 10px 12px;
      font-weight: 700;
    }
    QPushButton:hover { border: 1px solid rgba(255,255,255,120); }

    /* add gap between indicator and text */
    QCheckBox {
      spacing: 12px;
      padding: 4px 2px;
    }
    QCheckBox::indicator {
      width: 18px; height: 18px;
      border-radius: 6px;
      border: 1px solid rgba(255,255,255,90);
      background: rgba(10, 8, 18, 140);
    }
    QCheckBox::indicator:checked {
      background: rgba(241, 179, 166, 210);
      border: 1px solid rgba(255,255,255,140);
    }

    /* add gap between indicator and text */
    QRadioButton {
      spacing: 12px;
      padding: 4px 2px;
    }
    QRadioButton::indicator {
      width: 18px; height: 18px;
      border-radius: 9px;
      border: 1px solid rgba(255,255,255,90);
      background: rgba(10, 8, 18, 140);
    }
    QRadioButton::indicator:checked {
      background: rgba(201, 178, 215, 220);
      border: 1px solid rgba(255,255,255,140);
    }

    QLabel#SideHint {
      color: rgba(245, 241, 255, 170);
      font-size: 12px;
      background: transparent;
    }
  )");
}

void MainWindow::initialize() {
    realtime = new Realtime;
    aspectRatioWidget = new AspectRatioWidget(this);
    aspectRatioWidget->setAspectWidget(realtime, 3.f/4.f);

    auto *hLayout = new QHBoxLayout;
    auto *side = new QVBoxLayout;
    side->setAlignment(Qt::AlignTop);
    side->setContentsMargins(16, 16, 16, 16);
    side->setSpacing(12);

    hLayout->addLayout(side);
    hLayout->addWidget(aspectRatioWidget, 1);
    this->setLayout(hLayout);

    applyPrettyStyle();
    applyFixedParams();

    // Scene group
    QGroupBox *sceneBox = new QGroupBox("Scene");
    QVBoxLayout *sceneLayout = new QVBoxLayout;

    uploadFile = new QPushButton("Upload Scene File");

    sceneLayout->addWidget(uploadFile);
    sceneBox->setLayout(sceneLayout);

    // Effects group
    QGroupBox *effectsBox = new QGroupBox("Seasonal Effects");
    QVBoxLayout *effectsLayout = new QVBoxLayout;

    ec1 = new QCheckBox("Weather");
    ec2 = new QCheckBox("Bloom");

    ec1->setChecked(false);
    ec2->setChecked(false);

    effectsLayout->addWidget(ec1);
    effectsLayout->addWidget(ec2);
    effectsBox->setLayout(effectsLayout);

    // Seasons group (disabled until particles enabled)
    QGroupBox *seasonBox = new QGroupBox("Change the Season!");
    QVBoxLayout *seasonLayout = new QVBoxLayout;

    seasonWinter = new QRadioButton("Winter");
    seasonSpring = new QRadioButton("Spring");
    seasonSummer = new QRadioButton("Summer");
    seasonAutumn = new QRadioButton("Autumn");

    seasonGroup = new QButtonGroup(this);
    seasonGroup->addButton(seasonWinter, 0);
    seasonGroup->addButton(seasonSpring, 1);
    seasonGroup->addButton(seasonSummer, 2);
    seasonGroup->addButton(seasonAutumn, 3);

    seasonWinter->setChecked(true);

    seasonWinter->setEnabled(false);
    seasonSpring->setEnabled(false);
    seasonSummer->setEnabled(false);
    seasonAutumn->setEnabled(false);

    seasonLayout->addWidget(seasonWinter);
    seasonLayout->addWidget(seasonSpring);
    seasonLayout->addWidget(seasonSummer);
    seasonLayout->addWidget(seasonAutumn);
    seasonBox->setLayout(seasonLayout);

    side->addWidget(sceneBox);
    side->addWidget(effectsBox);
    side->addWidget(seasonBox);

    connectUIElements();

    // Default settings
    settings.extraCredit1 = ec1->isChecked();
    settings.extraCredit2 = ec2->isChecked();

    settings.particlesWinter = true;
    settings.particlesSpring = false;
    settings.particlesSummer = false;
    settings.particlesAutumn = false;

    applyFixedParams();
}

void MainWindow::finish() {
    realtime->finish();
    delete realtime;
    realtime = nullptr;
}

void MainWindow::connectUIElements() {
    connectUploadFile();
    connectExtraCredit();
    connectParticleSeasons();
}

void MainWindow::connectUploadFile() {
    connect(uploadFile, &QPushButton::clicked, this, &MainWindow::onUploadFile);
}

void MainWindow::connectExtraCredit() {
    connect(ec1, &QCheckBox::clicked, this, &MainWindow::onExtraCredit1);
    connect(ec2, &QCheckBox::clicked, this, &MainWindow::onExtraCredit2);
}

void MainWindow::connectParticleSeasons() {
    connect(seasonGroup, &QButtonGroup::idClicked, this, &MainWindow::onSeasonChanged);
}

void MainWindow::onUploadFile() {
    QString configFilePath = QFileDialog::getOpenFileName(
        this, tr("Upload File"),
        QDir::currentPath()
            .append(QDir::separator()).append("scenefiles")
            .append(QDir::separator()).append("realtime")
            .append(QDir::separator()).append("required"),
        tr("Scene Files (*.json)")
        );

    if (configFilePath.isNull()) {
        std::cout << "Failed to load null scenefile." << std::endl;
        return;
    }

    settings.sceneFilePath = configFilePath.toStdString();
    std::cout << "Loaded scenefile: \"" << settings.sceneFilePath << "\"." << std::endl;

    applyFixedParams();
    realtime->sceneChanged();
}

void MainWindow::onSaveImage() {
    if (settings.sceneFilePath.empty()) {
        std::cout << "No scene file loaded." << std::endl;
        return;
    }

    std::string sceneName = settings.sceneFilePath.substr(0, settings.sceneFilePath.find_last_of("."));
    sceneName = sceneName.substr(sceneName.find_last_of("/")+1);

    QString filePath = QFileDialog::getSaveFileName(
        this, tr("Save Image"),
        QDir::currentPath()
            .append(QDir::separator()).append("student_outputs")
            .append(QDir::separator()).append("realtime")
            .append(QDir::separator()).append("required")
            .append(QDir::separator()).append(sceneName),
        tr("Image Files (*.png)")
        );

    std::cout << "Saving image to: \"" << filePath.toStdString() << "\"." << std::endl;

    applyFixedParams();
    realtime->saveViewportImage(filePath.toStdString());
}

void MainWindow::onExtraCredit1() {
    settings.extraCredit1 = ec1->isChecked();

    bool enabled = settings.extraCredit1;
    seasonWinter->setEnabled(enabled);
    seasonSpring->setEnabled(enabled);
    seasonSummer->setEnabled(enabled);
    seasonAutumn->setEnabled(enabled);

    applyFixedParams();
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit2() {
    settings.extraCredit2 = ec2->isChecked();
    applyFixedParams();
    realtime->settingsChanged();
}

void MainWindow::onSeasonChanged() {
    // Respect current selected radio
    int id = seasonGroup->checkedId();

    settings.particlesWinter = (id == 0);
    settings.particlesSpring = (id == 1);
    settings.particlesSummer = (id == 2);
    settings.particlesAutumn = (id == 3);

    applyFixedParams();
    realtime->settingsChanged();
}
