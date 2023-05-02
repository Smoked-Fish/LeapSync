# LeapSync

LeapSync is a 3DS homebrew application that lets you use your 3DS as a controller for games on your PC. With LeapSync, you can transmit data from your 3DS's buttons, joysticks and gyroscope to your PC, allowing you to play games using your 3DS as a controller.

## Requirements

To use LeapSync, you need the following:

- A 3DS running a custom firmware, such as Luma3DS.
- The LeapSyncServer program installed on your PC (see [LeapSyncServer](https://github.com/Smoked-Fish/LeapSyncServer) for details).

## Installation

To install LeapSync, follow these steps:

1. Download the latest version of LeapSync from the [releases section](https://github.com/Smoked-Fish/LeapSync/releases) of this repository.
2. Copy the LeapSync.cia file or LeapSync.3dsx file to your 3DS's SD card.
3. Install the LeapSync.cia file using FBI or another CIA installer on your 3DS, or launch the LeapSync.3dsx file using a homebrew launcher.

Note that installing the .cia file will allow you to launch LeapSync as a system application, while using the .3dsx file will require you to launch LeapSync through a homebrew launcher. The .cia file may also have better compatibility with some custom firmware setups.

If you're not sure which file to use, try the .cia file first. If you encounter issues with it, you can switch to the .3dsx file instead.

## Usage

Once you have installed LeapSync on your 3DS and LeapSyncServer on your PC, you can use your 3DS as a controller for games on your PC that support DualShock4 input. To use LeapSync, follow these steps:

1. Launch LeapSyncServer on your PC.
2. Launch LeapSync on your 3DS.
4. Start a game on your PC that supports DualShock4 input.
5. Use your 3DS as a controller for the game.

## Tips

To improve your connection between your 3DS and PC, I recommend the following:

- Enable the hotspot feature on your PC to create a Wi-Fi network that your 3DS can connect to.
- Set the Wi-Fi network to use the 2.4GHz frequency band, as this is the frequency that the 3DS supports.
- If you need to change your 3DS's Wi-Fi connection, you can do so by pressing L + Down + Select to open the Rosalina menu, then navigating to System Configuration > Control Wireless Connection.

## Acknowledgements

The following tools were used in the creation of LeapSync:

- devkitPro: A cross-platform development environment for creating homebrew applications, used for the development of LeapSync.
- makerom: Used to create the .cia file for LeapSync.
- ba-GUI-nnertool: Used to create the banner and icon files for LeapSync.

A special thanks to the developers of these tools for their contribution to the 3DS homebrew community.

## License

LeapSync is licensed under the MIT License. See [LICENSE](LICENSE) for details.