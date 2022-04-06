# Multi Channel Audio Capture

## A tool to capture multi-channel audio input and encode it into AAC file

### Installation

1) Copy `mcac.exe` to any folder you like
2) Make sure your audio device has [ASIO](https://en.wikipedia.org/wiki/Audio_Stream_Input/Output)™ drivers installed

#### Required libraries

AAC encoder

MCAC uses [CoreAudio](https://en.wikipedia.org/wiki/Core_Audio) encoder to write AAC format.
To install it you may have to follow instructions here:

https://obsproject.com/forum/resources/obs-studio-enable-coreaudio-aac-encoder-windows.220/

### Workflow

1) Run mcac.exe
2) Select your audio device from the dropdown menu
3) Select the channels you want to capture
4) Select the folder for your recording
5) Press Start button
6) Perform
7) Press Stop

### Features

MCAC tries to reconnect devices that experience power cycle or USB disconnection

### Settings

You do not have to manually configure settings.ini as most of the settings are
available from the main application screen.

By default MCAC records in the maximum quality available for the AAC encoder in VBR mode.
You may want to reduce the quality to save space within range 0-127 where
0 is smallest size and 127 is maximum quality.

Location: `%USERPROFILE%\AppData\Roaming\MultiChannelAudioCapture\settings.ini`

Example:

```
[main]
device=Yamaha Steinberg USB ASIO
encoder_quality=127
active_channels=0,1
output_folder_path=C:\Users\bob
```

### Output

By default output files are created in folder 
`%USERPROFILE%\AppData\Roaming\MultiChannelAudioCapture`

### Compatibility

The tool is tested on Windows 10 with Steinberg UR22C, Behringer UMC204HD, Voicemeeter Banana.

You may experience inability to share ASIO™ drivers between MCAC and you DAW. It's not
possible to resolve as the sharing is up to the device driver.

### Used libraries

1) JUCE framework. https://juce.com/
2) ASIO™ headers. ASIO is a trademark of Steinberg Media Technologies GmbH
