#include "core.h"
#include "MainComponent.h"

String settings_file;
String appdata_folder;
String instance_name = "main";
FileLogger *logger = nullptr;
LogLevel log_level = LogLevel::Debug;
MainComponent* main_component = nullptr;

class MainWindow : public juce::DocumentWindow {
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)

public:
    MainWindow(juce::String name) : DocumentWindow(name, juce::Desktop::getInstance().getDefaultLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId), DocumentWindow::allButtons) {
        setUsingNativeTitleBar(true);
        main_component = new MainComponent();
        setContentOwned(main_component, true);

        setResizable(true, true);
        const int fixed_width = 10 + 315 + 20 + 285 + 10;
        setResizeLimits(fixed_width, 300, 8000, 8000);
        centreWithSize(getWidth(), getHeight());
        DocumentWindow::setName(getMainTitle());
        Component::setVisible(true);
    }

    void closeButtonPressed() override {
        JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class MCACApplication : public juce::JUCEApplication {
    std::unique_ptr<MainWindow> mainWindow;

public:
    MCACApplication() {}

    const juce::String getApplicationName() override       { return ProjectInfo::projectName; }
    const juce::String getApplicationVersion() override    { return ProjectInfo::versionString; }
    bool moreThanOneInstanceAllowed() override             { return true; }

    void initialise(const juce::String& commandLine) override {
        if (commandLine != "") instance_name = commandLine;

        appdata_folder = File::getSpecialLocation(File::userApplicationDataDirectory).getFullPathName() + "\\" + APP_NAME;

        File folder(appdata_folder);
        if (!folder.exists()) folder.createDirectory();

        logger = new FileLogger(appdata_folder + "\\" + instance_name + "_events.log", APP_NAME " @ build " __DATE__ " " __TIME__);

        settings_file = appdata_folder + "\\settings.ini";

        mainWindow.reset(new MainWindow (getApplicationName()));
    }

    void shutdown() override {
        mainWindow = nullptr;
    }

    ~MCACApplication() {
        delete logger;
        logger = nullptr;
    }

    void systemRequestedQuit() override {
        quit();
    }

    void anotherInstanceStarted(const juce::String& /*commandLine*/) override {
        // When another instance of the app is launched while this one is running,
        // this method is invoked, and the commandLine parameter tells you what
        // the other instance's command-line arguments were.
    }
};

START_JUCE_APPLICATION(MCACApplication)
