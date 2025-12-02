#include "mainwindow.h"
#include "settings.h"

#include <QApplication>
#include <QScreen>
#include <QCommandLineParser>
#include <QTimer>
#include <iostream>
#include <QSettings>

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    QCoreApplication::setApplicationName("Bread Final");
    QCoreApplication::setOrganizationName("CS 1230");
    QCoreApplication::setApplicationVersion("1.0");

    // command-line argument parsing for automated testing
    QCommandLineParser parser;
    parser.setApplicationDescription("Bread Final - Visual Effects Testing");
    parser.addHelpOption();

    // scene and output files
    parser.addPositionalArgument("scene", "Scene file to load");
    QCommandLineOption outputOption(QStringList() << "o" << "output", "Output image path", "file");
    parser.addOption(outputOption);

    // feature toggles
    QCommandLineOption enableFogOption("enable-fog", "Enable fog");
    QCommandLineOption disableFogOption("disable-fog", "Disable fog");
    QCommandLineOption enableNormalMapOption("enable-normal-mapping", "Enable normal mapping");
    QCommandLineOption disableNormalMapOption("disable-normal-mapping", "Disable normal mapping");
    QCommandLineOption enableScrollingOption("enable-scrolling", "Enable scrolling textures");
    QCommandLineOption disableScrollingOption("disable-scrolling", "Disable scrolling textures");
    QCommandLineOption enableInstancingOption("enable-instancing", "Enable instanced rendering");
    QCommandLineOption disableInstancingOption("disable-instancing", "Disable instanced rendering");
    parser.addOption(enableFogOption);
    parser.addOption(disableFogOption);
    parser.addOption(enableNormalMapOption);
    parser.addOption(disableNormalMapOption);
    parser.addOption(enableScrollingOption);
    parser.addOption(disableScrollingOption);
    parser.addOption(enableInstancingOption);
    parser.addOption(disableInstancingOption);

    // fog parameters
    QCommandLineOption fogStartOption("fog-start", "Fog start distance", "value");
    QCommandLineOption fogEndOption("fog-end", "Fog end distance", "value");
    QCommandLineOption fogColorOption("fog-color", "Fog color (r,g,b)", "rgb");
    parser.addOption(fogStartOption);
    parser.addOption(fogEndOption);
    parser.addOption(fogColorOption);

    // scrolling parameters
    QCommandLineOption scrollSpeedOption("scroll-speed", "Scroll speed", "value");
    QCommandLineOption scrollDirOption("scroll-direction", "Scroll direction (x,y)", "xy");
    parser.addOption(scrollSpeedOption);
    parser.addOption(scrollDirOption);

    // headless mode for automated testing
    QCommandLineOption headlessOption("headless", "Run in headless mode (auto-save and exit)");
    parser.addOption(headlessOption);

    parser.process(a);

    // override settings from command-line arguments
    if (parser.isSet(enableFogOption)) settings.enableFog = true;
    if (parser.isSet(disableFogOption)) settings.enableFog = false;
    if (parser.isSet(enableNormalMapOption)) settings.enableNormalMapping = true;
    if (parser.isSet(disableNormalMapOption)) settings.enableNormalMapping = false;
    if (parser.isSet(enableScrollingOption)) settings.enableScrolling = true;
    if (parser.isSet(disableScrollingOption)) settings.enableScrolling = false;
    if (parser.isSet(enableInstancingOption)) settings.enableInstancing = true;
    if (parser.isSet(disableInstancingOption)) settings.enableInstancing = false;

    if (parser.isSet(fogStartOption)) {
        settings.fogStart = parser.value(fogStartOption).toFloat();
    }
    if (parser.isSet(fogEndOption)) {
        settings.fogEnd = parser.value(fogEndOption).toFloat();
    }
    if (parser.isSet(fogColorOption)) {
        QStringList rgb = parser.value(fogColorOption).split(',');
        if (rgb.size() == 3) {
            settings.fogColor = glm::vec3(rgb[0].toFloat(), rgb[1].toFloat(), rgb[2].toFloat());
        }
    }

    if (parser.isSet(scrollSpeedOption)) {
        settings.scrollSpeed = parser.value(scrollSpeedOption).toFloat();
    }
    if (parser.isSet(scrollDirOption)) {
        QStringList xy = parser.value(scrollDirOption).split(',');
        if (xy.size() == 2) {
            settings.scrollDirection = glm::vec2(xy[0].toFloat(), xy[1].toFloat());
        }
    }

    // load scene file if provided
    QStringList positionalArgs = parser.positionalArguments();
    if (!positionalArgs.isEmpty()) {
        settings.sceneFilePath = positionalArgs.first().toStdString();
    }

    QSurfaceFormat fmt;
    fmt.setVersion(4, 1);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);

    MainWindow w;
    w.initialize();
    w.resize(1280, 960);

    // headless mode: show, render, save, quit
    bool headless = parser.isSet(headlessOption);
    if (headless && parser.isSet(outputOption)) {
        w.show();
        // give it a moment to render
        QTimer::singleShot(100, &w, [&w, &parser, &a]() {
            w.saveScreenshot(parser.value("output").toStdString());
            QTimer::singleShot(100, &a, &QApplication::quit);
        });
    } else {
        w.show();
    }

    int return_val = a.exec();
    w.finish();
    return return_val;
}
