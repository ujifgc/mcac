#include "core.h"
#include "device.h"
#include "device_manager.h"
#include "MainComponent.h"
#include "writer.h"

HWND message_window = NULL;
DeviceManager* device_manager = nullptr;
String output_folder_path;
float last_buffer_magnitude[MAX_INPUT_CHANNELS] = {};
float buffer_magnitude[MAX_INPUT_CHANNELS] = {};
bool active_channels[MAX_INPUT_CHANNELS] = {};

LRESULT CALLBACK MainComponent::message_window_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
    if (message < WM_USER) return DefWindowProc(hwnd, message, wparam, lparam);
    int index = (int)wparam;

    String device_name = device_manager->device_names[index];

    switch (message) {
    case WM_USER_RESET_REQUEST:
        main_component->showStatus("MCAC: Device requested sowtware reset.", Colours::yellow, device_name);
        main_component->onRestartRequest(device_name);
        break;
    case WM_USER_RESTART:
        main_component->onRestartRequest(device_name);
        break;
    case WM_USER_INPUT_ERROR:
        main_component->showStatus(device_manager->device_errors[index], Colours::yellow, device_name);
        break;
    case WM_USER_INPUT_STATUS: {
        AsioDevice* instance = device_manager->get_instance(index);
        if (instance) {
            main_component->showStatus(instance->status, device_name);
            if (instance->status == DeviceStatus::Open) main_component->resized();
        }
        break;
    }
    case WM_USER_WRITER_ERROR:
        if (writer) main_component->showWriterStatus(writer->message, Colours::yellow);
        break;
    case WM_USER_WRITER_STATUS:
        if (writer) main_component->showWriterStatus(writer->message, Colours::lightblue);
        break;
    case WM_USER_UPDATE:
        break;
    case WM_USER_CLEANUP:
        device_manager->cleanup_except(index);
        break;
    case WM_USER_ASYNC_UPDATE:
        main_component->handleAsyncUpdate();
        break;
    }

    return 0;
}

void MainComponent::CreateMessageWindow() {
    const char *class_name = APP_NAME "_MESSAGE_WINDOW";
    WNDCLASSEX wx = {};
    wx.cbSize = sizeof(WNDCLASSEXW);
    wx.lpfnWndProc = message_window_proc;
    wx.hInstance = 0;
    wx.lpszClassName = class_name;
    if (RegisterClassEx(&wx))
        message_window = CreateWindowEx(0, class_name, class_name, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, NULL, NULL);
}

//==============================================================================
MainComponent::MainComponent() {
    CreateMessageWindow();

    LookAndFeel::getDefaultLookAndFeel().setDefaultSansSerifTypefaceName("Lucida Console");

    addAndMakeVisible(ui_devices_select_label);
    addAndMakeVisible(ui_device_channels_list_label);
    addAndMakeVisible(ui_devices_select);

    addAndMakeVisible(ui_device_sample_rate); ui_device_sample_rate.setColour(Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(ui_device_sample_type); ui_device_sample_type.setColour(Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(ui_device_in_channels); ui_device_in_channels.setColour(Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(ui_device_out_channels); ui_device_out_channels.setColour(Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(ui_device_status_message); ui_device_status_message.setJustificationType(juce::Justification::right);
    addAndMakeVisible(ui_writer_status_message); ui_writer_status_message.setJustificationType(juce::Justification::right);

    addAndMakeVisible(ui_writer_output_folder_label);
    addAndMakeVisible(ui_writer_output_folder_path); ui_writer_output_folder_path.setColour(Label::textColourId, juce::Colours::lightblue);
    addAndMakeVisible(ui_writer_free_space); ui_writer_free_space.setColour(Label::textColourId, juce::Colours::lightgreen);
    addAndMakeVisible(ui_writer_select_output_folder_button); ui_writer_select_output_folder_button.setColour(Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(ui_writer_start_button); ui_writer_start_button.setColour(Label::textColourId, juce::Colours::lightgrey);
    addAndMakeVisible(ui_writer_stop_button); ui_writer_stop_button.setColour(Label::textColourId, juce::Colours::lightgrey);
    ui_writer_stop_button.setEnabled(false);

    ui_writer_select_output_folder_button.addListener(this);
    ui_writer_start_button.addListener(this);
    ui_writer_stop_button.addListener(this);

    for (int i = 0; i < MAX_INPUT_CHANNELS; i += 1) {
        addAndMakeVisible(ui_device_channels_list[i], 10);
        addAndMakeVisible(volumeter[i], 5);
        ui_device_channels_list[i].addListener(this);
    }

    String active_channels_string = ReadSettingsString(ACTIVE_CHANNELS_INI_KEY, "");
    StringArray tokens, active_channel_indexes;
    tokens.addTokens(active_channels_string, ",", "\"");
    memset(active_channels, 0, sizeof(active_channels));
    for (int i = 0; i < tokens.size(); i++) {
        active_channel_indexes.add(tokens[i]);
        active_channels[tokens[i].getIntValue()] = true;
    }
    for (String channel : active_channel_indexes) {
        int channel_index = channel.getIntValue();
        if (channel_index >= MAX_INPUT_CHANNELS) continue;
        ui_device_channels_list[channel_index].setToggleState(true, dontSendNotification);
    }

	setSize(500, 500);

    InitializeCriticalSection(&writer_section);
    device_manager = new DeviceManager();
    const String* device_names_array = device_manager->device_names;
    StringArray device_names = StringArray(device_names_array, device_manager->devices_count);
    ui_devices_select.addItemList(device_names, ID_BASE);

    String active_device_name = ReadSettingsString(ACTIVE_DEVICE_INI_KEY);
    if (active_device_name != L"") {
        int active_device_name_id = device_names.indexOf(active_device_name);
        if (active_device_name_id == -1) {
            ui_devices_select.addItem("(inactive: " + active_device_name + ")", ID_BASE - 1);
            ui_devices_select.setSelectedId(ID_BASE - 1);
        }
        else {
            ui_devices_select.setSelectedId(ID_BASE + active_device_name_id);
        }
    }

    ui_devices_select.onChange = [this] { onDeviceChange(); };
}

MainComponent::~MainComponent()
{
    DeviceManager *copy = device_manager;
    device_manager = nullptr;
    delete copy;
    DeleteCriticalSection(&writer_section);
    DestroyWindow(message_window);
}

//==============================================================================
void MainComponent::paint (Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    g.setColour(Colours::darkgrey);
    g.drawLine(left_column_width + 18.f, 5.f, left_column_width + 18.f, getHeight() - 5.f, 2);
}

void MainComponent::resized() {
    if (device != nullptr && device->instance != nullptr && device->get_device_status() >= DeviceStatus::Open) {
        ui_device_sample_rate.setText(String(device->instance->sample_rate, 0), dontSendNotification);
        ui_device_sample_type.setText(device->instance->sample_type == ASIOSTInt32LSB ? "Int32" : "Float32", dontSendNotification);
        ui_device_in_channels.setText("In: " + String(device->instance->input_channels_number), dontSendNotification);
        ui_device_out_channels.setText("Out: " + String(device->instance->output_channels_number), dontSendNotification);
    }
    else {
        ui_device_sample_rate.setText("-", dontSendNotification);
        ui_device_sample_type.setText("-", dontSendNotification);
        ui_device_in_channels.setText("In: -", dontSendNotification);
        ui_device_out_channels.setText("Out: -", dontSendNotification);
    }

	ui_device_sample_rate.setBounds(10, 65, 80, 20);
    ui_device_sample_type.setBounds(10 + 80, 65, 75, 20);
	ui_device_in_channels.setBounds(10 + 155, 65, 75, 20);
	ui_device_out_channels.setBounds(10 + 235, 65, 70, 20);
    left_column_width = 315;

    ui_devices_select_label.setBounds(10, 10, left_column_width, 20);
    ui_devices_select.setBounds(8, 30, left_column_width + 2, 30);
    ui_device_channels_list_label.setBounds(10, 90, left_column_width, 20);

    ui_device_status_message.setBounds(10, getHeight() - 25, left_column_width, 20);

    drawChannels(10, 120, left_column_width, 20);

    int right_column_x = left_column_width + 20, right_column_width = getWidth() - right_column_x - 8;
    ui_writer_output_folder_label.setBounds(right_column_x, 10, right_column_width, 20);
    ui_writer_output_folder_path.setBounds(right_column_x, 35, right_column_width, 20);
    ui_writer_free_space.setBounds(right_column_x, 65, 160, 20);
    ui_writer_select_output_folder_button.setBounds(getWidth() - 10 - 100, 58, 102, 30);
    ui_writer_start_button.setBounds(right_column_x + 5, 95, 102, 30);
    ui_writer_stop_button.setBounds(getWidth() - 10 - 100, 95, 102, 30);

    output_folder_path = ReadSettingsString(OUTPUT_FOLDER_INI_KEY, appdata_folder);
    File output_folder(output_folder_path);
    String short_path = (output_folder_path.length() < right_column_width / 9.3) ? output_folder_path : (output_folder_path.substring(0, 2) + "...\\" + output_folder.getFileName());
    ui_writer_output_folder_path.setText(short_path, dontSendNotification);

    int64 free_gibibytes = output_folder.getBytesFreeOnVolume() / 1024 / 1024 / 1024;
    String free_space = "Free: " + String(free_gibibytes) + " GiB";
    ui_writer_free_space.setText(free_space, dontSendNotification);

    ui_writer_status_message.setBounds(right_column_x, getHeight() - 25, right_column_width, 20);
}

int MainComponent::drawChannels(int x, int y, int w, int h) {
    int height = 0;
    if (device != nullptr && device->instance && device->get_device_status() == DeviceStatus::Open) {
        StringArray channel_names = device->instance->input_channel_names;
        int channels_count = channel_names.size();
        if (channels_count > MAX_INPUT_CHANNELS) channels_count = MAX_INPUT_CHANNELS;
        for (int i = 0; i < channels_count; i += 1) {
            ui_device_channels_list[i].setBounds(x, y + i * (h + 10), w, h);
            ui_device_channels_list[i].setButtonText(channel_names[i]);
            ui_device_channels_list[i].setVisible(true);
            height += (h + 10);
            volumeter[i].setVisible(true);
        }
        for (int i = channels_count; i < MAX_INPUT_CHANNELS; i += 1) {
            ui_device_channels_list[i].setVisible(false);
            volumeter[i].setVisible(false);
        }
    }
    else {
        for (int i = 0; i < MAX_INPUT_CHANNELS; i += 1) {
            ui_device_channels_list[i].setVisible(false);
            volumeter[i].setVisible(false);
        }
    }
    return height;
}

void MainComponent::onDeviceChange() {
    const String device_name = device_manager->device_names[ui_devices_select.getSelectedId() - ID_BASE];
    WriteSettings(ACTIVE_DEVICE_INI_KEY, device_name);

    for (int i = 0; i < MAX_INPUT_CHANNELS; i += 1) {
        volumeter[i].setBounds(325, 120 + 30 * i, 0, 20);
    }

    ui_device_status_message.setText("", dontSendNotification);
    device = device_manager->create_device(device_name);
    resized();
    device->open();
}

void MainComponent::onRestartRequest(String device_name) {
    const String selected_device_name = device_manager->device_names[ui_devices_select.getSelectedId() - ID_BASE];
    if (device_name == selected_device_name) {
        device_manager->cleanup(device_name);
        device = device_manager->create_device(device_name);
        resized();
        device->open();
    }
}

void MainComponent::showWriterStatus(String text, Colour color) {
    ui_writer_status_message.setColour(Label::textColourId, color);
    ui_writer_status_message.setText(text, dontSendNotification);
    mlog("MCAC Writer", text, color == Colours::yellow ? LogLevel::Error : LogLevel::Info);
}

static int retries = 0;

void MainComponent::showStatus(String text, Colour color, String device_name) {
    if (color == Colours::lightblue) {
        retries = 0;
    }
    else {
        retries += 1;
        text += " Offline: " + String(retries) + "s";
    }
    const String selected_device_name = device_manager->device_names[ui_devices_select.getSelectedId() - ID_BASE];
    if (device_name == selected_device_name) {
        ui_device_status_message.setColour(Label::textColourId, color);
        ui_device_status_message.setText(text, dontSendNotification);
    }
    mlog(device_name, text, color == Colours::yellow ? LogLevel::Error : LogLevel::Info);
}

void MainComponent::showStatus(DeviceStatus _status, String device_name) {
    String text;
    Colour color;

    switch (_status) {
    case DeviceStatus::None:
        text = "Error", color = Colours::yellow;
        break;
    case DeviceStatus::Initialized:
        return;
    case DeviceStatus::Open:
        text = "Ready", color = Colours::lightblue;
        break;
    default:
        break;
    }

    showStatus(text, color, device_name);
}

void MainComponent::buttonClicked(juce::Button* button) {
    if (button == &ui_writer_select_output_folder_button) {
        FileChooser browse_folder("Please select output folder", output_folder_path);
        if (browse_folder.browseForDirectory()) {
            output_folder_path = browse_folder.getResult().getFullPathName();
            WriteSettings(OUTPUT_FOLDER_INI_KEY, output_folder_path);
            resized();
        }
        return;
    }
    
    if (button == &ui_writer_start_button) {
        if (writer) {
            if (writer->open()) {
                ui_writer_start_button.setEnabled(false);
                ui_writer_stop_button.setEnabled(true);
            }
            else {
                bool ok = AlertWindow::showOkCancelBox(AlertWindow::AlertIconType::WarningIcon, "Failed to start recording", ui_writer_status_message.getText(), "Oh my Glob!", "Help", this);
                if (!ok) URL("https://github.com/ujifgc/mcac#troubleshooting").launchInDefaultBrowser();
            }
        }
        return;
    }
    
    if (button == &ui_writer_stop_button) {
        if (writer) writer->close();
        ui_writer_start_button.setEnabled(true);
        ui_writer_stop_button.setEnabled(false);
        return;
    }

    int channel_index = int(button - (Button*)ui_device_channels_list);
    if (channel_index >= 0 && channel_index < MAX_INPUT_CHANNELS) {
        String active_channels_string = ReadSettingsString(ACTIVE_CHANNELS_INI_KEY, "");
        StringArray tokens, active_channel_indexes;
        tokens.addTokens(active_channels_string, ",", "\"");
        memset(active_channels, 0, sizeof(active_channels));
        for (int i = 0; i < tokens.size(); i++) {
            active_channel_indexes.add(tokens[i]);
            active_channels[tokens[i].getIntValue()] = true;
        }
        if (ui_device_channels_list[channel_index].getToggleState() == true) {
            active_channel_indexes.addIfNotAlreadyThere(String(channel_index));
            active_channels[channel_index] = true;
        }
        else {
            active_channel_indexes.removeString(String(channel_index));
            active_channels[channel_index] = false;
        }

        byte active_channels_number = 0;
        if (device) active_channels_number = device->update_active_channels();
        active_channel_indexes.sortNatural();
        active_channels_string = active_channel_indexes.joinIntoString(",");
        WriteSettings(ACTIVE_CHANNELS_INI_KEY, active_channels_string);

        writer->uninit();

        ui_writer_start_button.setEnabled(true);
        ui_writer_stop_button.setEnabled(false);

        writer->init(0, active_channels_number);
        
        for (int i = 0; i < MAX_INPUT_CHANNELS; i += 1) {
            volumeter[i].setBounds(325, 120 + 30 * i, 0, 20);
        }
    }
}

void MainComponent::handleAsyncUpdate() {
    extern float buffer_magnitude[MAX_INPUT_CHANNELS];

    for (int i = 0; i < MAX_INPUT_CHANNELS; i += 1) {
        //if (shifted_channel_indexes[i] == -1) continue;
        int width = (int)round(RANGE_DB + 1.0 + magnitude_to_db(buffer_magnitude[i]));
        volumeter[i].setBounds(325 - width, 120 + 30 * i, width, 20);
    }
}
