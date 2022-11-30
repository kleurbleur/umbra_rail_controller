The controller software for the UMBRA a/v performance done by Zalán Szakacs, Nikos ten Hoedt and Mark Ridder. 

USAGE:
1. Install PlatformIO -> https://platformio.org/install/ide?install=vscode
2. Download the above files by clicking on the green "CODE" button and on the bottom of the list click "Download ZIP" 
3. Unzip the folder in a location you can easily access
4. If you open Visual Studio Code, PlatformIO should open too. In PlatformIO there's a button on the middle right "Open Project" and select the unzipped folder
5. Select from the folder on the left "src" and then "main.cpp"
6. Change in this file the controller name (line 22) and the network settings (from line 24) and save
7. Connect the Olimex controller via the mini usb port 
8. With the project open hit "control+option+u"
9. A tab on the bottom should open with all kinds of text flowing till it says SUCCES! at the end. It takes in my laptop 14s. Disconnect the Olimex.
10. Repeat 6 till 9 for every controller or network settings change
