#pragma once

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

#include "juce/JuceHeader.h"

#define APP_NAME "MultiChannelAudioCapture"
#define ACTIVE_DEVICE_INI_KEY "device"
#define ACTIVE_CHANNELS_INI_KEY "active_channels"
#define OUTPUT_FOLDER_INI_KEY "output_folder_path"
#define ENCODER_QUALITY_INI_KEY "encoder_quality"

extern String settings_file;
extern String appdata_folder;
extern String instance_name;
extern String output_folder_path;
extern HWND message_window;
extern class MainComponent* main_component;
extern class Writer* writer;
extern CRITICAL_SECTION writer_section;
extern bool active_channels[MAX_INPUT_CHANNELS];

void WriteSettings(String key, String value);
String ReadSettingsString(String key, String default_value = "");
String getMainTitle();

class Volumeter : public Component {
    void paint(Graphics& g) override {
        g.fillAll(Colours::plum);
    }
};

#define OFFSET_DB 6.0f
#define RANGE_DB 96.0f

static inline float magnitude_to_db(float magnitude) {
    float db = 20.0f * log10f(magnitude);
    return isfinite(db) ? db : -RANGE_DB;
}

class MainComponent  : public juce::Component, public juce::Button::Listener, public juce::AsyncUpdater {
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button* button) override;

private:
    //==============================================================================
    Label ui_devices_select_label{ "", "Audio device" };
    Label ui_device_channels_list_label{ "", "Channels to capture" };
    Label ui_device_sample_rate, ui_device_sample_type, ui_device_in_channels, ui_device_out_channels, ui_device_status_message, ui_writer_status_message;
    ToggleButton ui_device_channels_list[MAX_INPUT_CHANNELS];
    ComboBox ui_devices_select;
    Label ui_writer_output_folder_label{ "", "Output folder" };
    Label ui_writer_output_folder_path;
    Label ui_writer_free_space;
    TextButton ui_writer_select_output_folder_button{ "Browse...", "Select output folder" };
    TextButton ui_writer_start_button{ "Start", "Start recording" };
    TextButton ui_writer_stop_button{ "Stop", "Stop recording" };

    Volumeter volumeter[MAX_INPUT_CHANNELS];

    class Device* device = nullptr;
    const int ID_BASE = 1000;
    int left_column_width = 0;
    void showStatus(String text, Colour color, String device_name);
    void showStatus(DeviceStatus status, String device_name);
    void showWriterStatus(String text, Colour color);
    int drawChannels(int, int, int, int);

    static LRESULT CALLBACK message_window_proc(HWND, UINT, WPARAM, LPARAM);
    
    void CreateMessageWindow();
    void onDeviceChange();
    void onRestartRequest(String device_name);
    void handleAsyncUpdate();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
