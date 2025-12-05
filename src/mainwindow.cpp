#include "mainwindow.h"
#include "settings.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QLabel>
#include <QGroupBox>
#include <QDir>
#include <QSignalBlocker>
#include <iostream>

namespace {

// Ensures exactly one season is selected (checkboxes used like radio buttons).
void selectSeason(QCheckBox *winter,
                  QCheckBox *spring,
                  QCheckBox *summer,
                  QCheckBox *autumn,
                  QCheckBox *chosen) {
    if (!winter || !spring || !summer || !autumn || !chosen) return;

    winter->blockSignals(true);
    spring->blockSignals(true);
    summer->blockSignals(true);
    autumn->blockSignals(true);

    winter->setChecked(chosen == winter);
    spring->setChecked(chosen == spring);
    summer->setChecked(chosen == summer);
    autumn->setChecked(chosen == autumn);

    winter->blockSignals(false);
    spring->blockSignals(false);
    summer->blockSignals(false);
    autumn->blockSignals(false);
}

void setSeasonSettingsWinter() {
    settings.particlesWinter = true;
    settings.particlesSpring = false;
    settings.particlesSummer = false;
    settings.particlesAutumn = false;
}
void setSeasonSettingsSpring() {
    settings.particlesWinter = false;
    settings.particlesSpring = true;
    settings.particlesSummer = false;
    settings.particlesAutumn = false;
}
void setSeasonSettingsSummer() {
    settings.particlesWinter = false;
    settings.particlesSpring = false;
    settings.particlesSummer = true;
    settings.particlesAutumn = false;
}
void setSeasonSettingsAutumn() {
    settings.particlesWinter = false;
    settings.particlesSpring = false;
    settings.particlesSummer = false;
    settings.particlesAutumn = true;
}

} // namespace

void MainWindow::initialize() {
    realtime = new Realtime;
    aspectRatioWidget = new AspectRatioWidget(this);
    aspectRatioWidget->setAspectWidget(realtime, 3.f/4.f);

    QHBoxLayout *hLayout = new QHBoxLayout;
    QVBoxLayout *vLayout = new QVBoxLayout();
    vLayout->setAlignment(Qt::AlignTop);
    hLayout->addLayout(vLayout);
    hLayout->addWidget(aspectRatioWidget, 1);
    this->setLayout(hLayout);

    // Create labels in sidebox
    QFont font;
    font.setPointSize(12);
    font.setBold(true);

    QLabel *tesselation_label = new QLabel();
    tesselation_label->setText("Tesselation");
    tesselation_label->setFont(font);

    QLabel *camera_label = new QLabel();
    camera_label->setText("Camera");
    camera_label->setFont(font);

    QLabel *ec_label = new QLabel();
    ec_label->setText("Extra Credit");
    ec_label->setFont(font);

    QLabel *param1_label = new QLabel();
    param1_label->setText("Parameter 1:");

    QLabel *param2_label = new QLabel();
    param2_label->setText("Parameter 2:");

    QLabel *near_label = new QLabel();
    near_label->setText("Near Plane:");

    QLabel *far_label = new QLabel();
    far_label->setText("Far Plane:");

    // Create file uploader for scene file
    uploadFile = new QPushButton();
    uploadFile->setText(QStringLiteral("Upload Scene File"));

    saveImage = new QPushButton();
    saveImage->setText(QStringLiteral("Save Image"));

    // Creates the boxes containing the parameter sliders and number boxes
    QGroupBox *p1Layout = new QGroupBox();
    QHBoxLayout *l1 = new QHBoxLayout();
    QGroupBox *p2Layout = new QGroupBox();
    QHBoxLayout *l2 = new QHBoxLayout();

    // Create slider controls to control parameters
    p1Slider = new QSlider(Qt::Orientation::Horizontal);
    p1Slider->setTickInterval(1);
    p1Slider->setMinimum(1);
    p1Slider->setMaximum(25);
    p1Slider->setValue(1);

    p1Box = new QSpinBox();
    p1Box->setMinimum(1);
    p1Box->setMaximum(25);
    p1Box->setSingleStep(1);
    p1Box->setValue(1);

    p2Slider = new QSlider(Qt::Orientation::Horizontal);
    p2Slider->setTickInterval(1);
    p2Slider->setMinimum(1);
    p2Slider->setMaximum(25);
    p2Slider->setValue(1);

    p2Box = new QSpinBox();
    p2Box->setMinimum(1);
    p2Box->setMaximum(25);
    p2Box->setSingleStep(1);
    p2Box->setValue(1);

    // Adds the slider and number box to the parameter layouts
    l1->addWidget(p1Slider);
    l1->addWidget(p1Box);
    p1Layout->setLayout(l1);

    l2->addWidget(p2Slider);
    l2->addWidget(p2Box);
    p2Layout->setLayout(l2);

    // Creates the boxes containing the camera sliders and number boxes
    QGroupBox *nearLayout = new QGroupBox();
    QHBoxLayout *lnear = new QHBoxLayout();
    QGroupBox *farLayout = new QGroupBox();
    QHBoxLayout *lfar = new QHBoxLayout();

    // Create slider controls to control near/far planes
    nearSlider = new QSlider(Qt::Orientation::Horizontal);
    nearSlider->setTickInterval(1);
    nearSlider->setMinimum(1);
    nearSlider->setMaximum(1000);
    nearSlider->setValue(10);

    nearBox = new QDoubleSpinBox();
    nearBox->setMinimum(0.01f);
    nearBox->setMaximum(10.f);
    nearBox->setSingleStep(0.1f);
    nearBox->setValue(0.1f);

    farSlider = new QSlider(Qt::Orientation::Horizontal);
    farSlider->setTickInterval(1);
    farSlider->setMinimum(1000);
    farSlider->setMaximum(10000);
    farSlider->setValue(10000);

    farBox = new QDoubleSpinBox();
    farBox->setMinimum(10.f);
    farBox->setMaximum(100.f);
    farBox->setSingleStep(0.1f);
    farBox->setValue(100.f);

    // Adds the slider and number box to the camera layouts
    lnear->addWidget(nearSlider);
    lnear->addWidget(nearBox);
    nearLayout->setLayout(lnear);

    lfar->addWidget(farSlider);
    lfar->addWidget(farBox);
    farLayout->setLayout(lfar);

    // Extra Credit:
    ec1 = new QCheckBox();
    ec1->setText(QStringLiteral("Particle Systems"));
    ec1->setChecked(false);

    // New: particle season sub-toggles (sub-toggles of ec1)
    particlesWinter = new QCheckBox();
    particlesWinter->setText(QStringLiteral("Winter: Snow"));
    particlesWinter->setChecked(true);

    particlesSpring = new QCheckBox();
    particlesSpring->setText(QStringLiteral("Spring: Rain"));
    particlesSpring->setChecked(false);

    particlesSummer = new QCheckBox();
    particlesSummer->setText(QStringLiteral("Summer: Fireflies"));
    particlesSummer->setChecked(false);

    particlesAutumn = new QCheckBox();
    particlesAutumn->setText(QStringLiteral("Autumn: Leaves"));
    particlesAutumn->setChecked(false);

    // Default: season toggles disabled until master is enabled
    particlesWinter->setEnabled(false);
    particlesSpring->setEnabled(false);
    particlesSummer->setEnabled(false);
    particlesAutumn->setEnabled(false);

    // Keep settings in sync with defaults
    settings.extraCredit1 = ec1->isChecked();
    setSeasonSettingsWinter();

    ec2 = new QCheckBox();
    ec2->setText(QStringLiteral("Screen Space Bloom"));
    ec2->setChecked(false);

    // Bloom strength UI
    QLabel *bloomStrengthLabel = new QLabel();
    bloomStrengthLabel->setText("Bloom Strength:");

    QGroupBox *bloomStrengthLayout = new QGroupBox();
    QHBoxLayout *lbloom = new QHBoxLayout();

    bloomStrengthSlider = new QSlider(Qt::Orientation::Horizontal);
    bloomStrengthSlider->setTickInterval(10);
    bloomStrengthSlider->setMinimum(0);
    bloomStrengthSlider->setMaximum(300); // 0.00 .. 3.00
    bloomStrengthSlider->setValue(100);   // 1.00

    bloomStrengthBox = new QDoubleSpinBox();
    bloomStrengthBox->setMinimum(0.0);
    bloomStrengthBox->setMaximum(3.0);
    bloomStrengthBox->setSingleStep(0.05);
    bloomStrengthBox->setDecimals(2);
    bloomStrengthBox->setValue(1.0);

    lbloom->addWidget(bloomStrengthSlider);
    lbloom->addWidget(bloomStrengthBox);
    bloomStrengthLayout->setLayout(lbloom);

    // Strength UI disabled unless bloom is enabled
    bloomStrengthLabel->setEnabled(false);
    bloomStrengthSlider->setEnabled(false);
    bloomStrengthBox->setEnabled(false);

    // Keep settings in sync with the default bloom strength
    settings.bloomStrength = 1.0f;

    ec3 = new QCheckBox();
    ec3->setText(QStringLiteral("Extra Credit 3"));
    ec3->setChecked(false);

    ec4 = new QCheckBox();
    ec4->setText(QStringLiteral("Extra Credit 4"));
    ec4->setChecked(false);

    vLayout->addWidget(uploadFile);
    vLayout->addWidget(saveImage);

    vLayout->addWidget(tesselation_label);
    vLayout->addWidget(param1_label);
    vLayout->addWidget(p1Layout);
    vLayout->addWidget(param2_label);
    vLayout->addWidget(p2Layout);

    vLayout->addWidget(camera_label);
    vLayout->addWidget(near_label);
    vLayout->addWidget(nearLayout);
    vLayout->addWidget(far_label);
    vLayout->addWidget(farLayout);

    // Extra Credit:
    vLayout->addWidget(ec_label);
    vLayout->addWidget(ec1);
    vLayout->addWidget(particlesWinter);
    vLayout->addWidget(particlesSpring);
    vLayout->addWidget(particlesSummer);
    vLayout->addWidget(particlesAutumn);

    vLayout->addWidget(ec2);
    vLayout->addWidget(bloomStrengthLabel);
    vLayout->addWidget(bloomStrengthLayout);

    vLayout->addWidget(ec3);
    vLayout->addWidget(ec4);

    connectUIElements();

    // Set default values of 5 for tesselation parameters
    onValChangeP1(5);
    onValChangeP2(5);

    // Set default values for near and far planes
    onValChangeNearBox(0.1f);
    onValChangeFarBox(10.f);
}

void MainWindow::finish() {
    realtime->finish();
    delete(realtime);
}

void MainWindow::connectUIElements() {
    connectUploadFile();
    connectSaveImage();
    connectParam1();
    connectParam2();
    connectNear();
    connectFar();
    connectExtraCredit();
    connectParticleSeasons();
    connectBloomStrength();
}

void MainWindow::connectUploadFile() {
    connect(uploadFile, &QPushButton::clicked, this, &MainWindow::onUploadFile);
}

void MainWindow::connectSaveImage() {
    connect(saveImage, &QPushButton::clicked, this, &MainWindow::onSaveImage);
}

void MainWindow::connectParam1() {
    connect(p1Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP1);
    connect(p1Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP1);
}

void MainWindow::connectParam2() {
    connect(p2Slider, &QSlider::valueChanged, this, &MainWindow::onValChangeP2);
    connect(p2Box, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged),
            this, &MainWindow::onValChangeP2);
}

void MainWindow::connectNear() {
    connect(nearSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeNearSlider);
    connect(nearBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onValChangeNearBox);
}

void MainWindow::connectFar() {
    connect(farSlider, &QSlider::valueChanged, this, &MainWindow::onValChangeFarSlider);
    connect(farBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
            this, &MainWindow::onValChangeFarBox);
}

void MainWindow::connectExtraCredit() {
    connect(ec1, &QCheckBox::clicked, this, &MainWindow::onExtraCredit1);
    connect(ec2, &QCheckBox::clicked, this, &MainWindow::onExtraCredit2);
    connect(ec3, &QCheckBox::clicked, this, &MainWindow::onExtraCredit3);
    connect(ec4, &QCheckBox::clicked, this, &MainWindow::onExtraCredit4);
}

void MainWindow::connectParticleSeasons() {
    connect(particlesWinter, &QCheckBox::clicked, this, &MainWindow::onParticlesWinter);
    connect(particlesSpring, &QCheckBox::clicked, this, &MainWindow::onParticlesSpring);
    connect(particlesSummer, &QCheckBox::clicked, this, &MainWindow::onParticlesSummer);
    connect(particlesAutumn, &QCheckBox::clicked, this, &MainWindow::onParticlesAutumn);
}

void MainWindow::connectBloomStrength() {
    if (bloomStrengthSlider) {
        connect(bloomStrengthSlider, &QSlider::valueChanged, this, &MainWindow::onBloomStrengthSlider);
    }
    if (bloomStrengthBox) {
        connect(bloomStrengthBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged),
                this, &MainWindow::onBloomStrengthBox);
    }
}

void MainWindow::onUploadFile() {
    QString configFilePath = QFileDialog::getOpenFileName(
        this,
        tr("Upload File"),
        QDir::currentPath()
            .append(QDir::separator())
            .append("scenefiles")
            .append(QDir::separator())
            .append("realtime")
            .append(QDir::separator())
            .append("required"),
        tr("Scene Files (*.json)")
        );

    if (configFilePath.isNull()) {
        std::cout << "Failed to load null scenefile." << std::endl;
        return;
    }

    settings.sceneFilePath = configFilePath.toStdString();
    std::cout << "Loaded scenefile: \"" << configFilePath.toStdString() << "\"." << std::endl;
    realtime->sceneChanged();
}

void MainWindow::onSaveImage() {
    if (settings.sceneFilePath.empty()) {
        std::cout << "No scene file loaded." << std::endl;
        return;
    }

    std::string sceneName = settings.sceneFilePath.substr(0, settings.sceneFilePath.find_last_of("."));
    sceneName = sceneName.substr(sceneName.find_last_of("/") + 1);

    QString filePath = QFileDialog::getSaveFileName(
        this,
        tr("Save Image"),
        QDir::currentPath()
            .append(QDir::separator())
            .append("student_outputs")
            .append(QDir::separator())
            .append("realtime")
            .append(QDir::separator())
            .append("required")
            .append(QDir::separator())
            .append(sceneName),
        tr("Image Files (*.png)")
        );

    std::cout << "Saving image to: \"" << filePath.toStdString() << "\"." << std::endl;
    realtime->saveViewportImage(filePath.toStdString());
}

void MainWindow::onValChangeP1(int newValue) {
    p1Slider->setValue(newValue);
    p1Box->setValue(newValue);
    settings.shapeParameter1 = p1Slider->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeP2(int newValue) {
    p2Slider->setValue(newValue);
    p2Box->setValue(newValue);
    settings.shapeParameter2 = p2Slider->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeNearSlider(int newValue) {
    nearBox->setValue(newValue / 100.f);
    settings.nearPlane = nearBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeFarSlider(int newValue) {
    farBox->setValue(newValue / 100.f);
    settings.farPlane = farBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeNearBox(double newValue) {
    nearSlider->setValue(int(newValue * 100.f));
    settings.nearPlane = nearBox->value();
    realtime->settingsChanged();
}

void MainWindow::onValChangeFarBox(double newValue) {
    farSlider->setValue(int(newValue * 100.f));
    settings.farPlane = farBox->value();
    realtime->settingsChanged();
}

// Extra Credit:

void MainWindow::onExtraCredit1() {
    settings.extraCredit1 = ec1->isChecked();

    // Enable/disable season toggles based on master
    bool enabled = settings.extraCredit1;
    particlesWinter->setEnabled(enabled);
    particlesSpring->setEnabled(enabled);
    particlesSummer->setEnabled(enabled);
    particlesAutumn->setEnabled(enabled);

    realtime->settingsChanged();
}

void MainWindow::onExtraCredit2() {
    settings.extraCredit2 = ec2->isChecked();

    // Enable/disable bloom strength controls when bloom toggled
    if (bloomStrengthSlider) bloomStrengthSlider->setEnabled(settings.extraCredit2);
    if (bloomStrengthBox)    bloomStrengthBox->setEnabled(settings.extraCredit2);

    // If you want the label to gray out too, this finds it by walking parent widgets.
    // Keeping it simple: layout disables slider + box which is enough.

    realtime->settingsChanged();
}

void MainWindow::onExtraCredit3() {
    settings.extraCredit3 = ec3->isChecked();
    realtime->settingsChanged();
}

void MainWindow::onExtraCredit4() {
    settings.extraCredit4 = ec4->isChecked();
    realtime->settingsChanged();
}

// Particle season sub-toggles:

void MainWindow::onParticlesWinter() {
    selectSeason(particlesWinter, particlesSpring, particlesSummer, particlesAutumn, particlesWinter);
    setSeasonSettingsWinter();
    realtime->settingsChanged();
}

void MainWindow::onParticlesSpring() {
    selectSeason(particlesWinter, particlesSpring, particlesSummer, particlesAutumn, particlesSpring);
    setSeasonSettingsSpring();
    realtime->settingsChanged();
}

void MainWindow::onParticlesSummer() {
    selectSeason(particlesWinter, particlesSpring, particlesSummer, particlesAutumn, particlesSummer);
    setSeasonSettingsSummer();
    realtime->settingsChanged();
}

void MainWindow::onParticlesAutumn() {
    selectSeason(particlesWinter, particlesSpring, particlesSummer, particlesAutumn, particlesAutumn);
    setSeasonSettingsAutumn();
    realtime->settingsChanged();
}

// Bloom strength:

void MainWindow::onBloomStrengthSlider(int v) {
    // Map 0..300 -> 0.00..3.00
    float strength = float(v) / 100.f;

    if (bloomStrengthBox) {
        QSignalBlocker blocker(*bloomStrengthBox);
        bloomStrengthBox->setValue(double(strength));
    }

    settings.bloomStrength = strength;
    realtime->settingsChanged();
}

void MainWindow::onBloomStrengthBox(double v) {
    float strength = float(v);
    int sliderV = int(std::round(strength * 100.f));

    if (bloomStrengthSlider) {
        QSignalBlocker blocker(*bloomStrengthSlider);
        bloomStrengthSlider->setValue(sliderV);
    }

    settings.bloomStrength = strength;
    realtime->settingsChanged();
}
