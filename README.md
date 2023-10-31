# Saintcon 2023 Badge

# Updating your badge firmware
There are multiple ways to get new firmware on your badge. If you have access to your WiFi equipment, the easiest method may be to set up an SSID that matches the conference badge network. Then your badge should automatically connect to it and grab the newest firmware from the badge server. The SSID is **BadgeNet-OutOfScope** and the password is **compukidmikeismycopilot**

You can also manually flash your badge over USB. This will require python and esptool, both of which are free to download/use.  
If you're using Linux then I hope you can figure this out on your own just like you did for the rest of your OS.*  
If you're using a Mac, then it should "just work", right?*  
If you're using Windows, python is available from the windows store or you can download it from https://www.python.org/downloads/

Once you have python installed, open a terminal/command prompt and type "pip install esptool"

Now grab the firmware.bin file from this repo and use the following command to flash the firmware to your badge, substituting the firmware.bin file location.  
`esptool write_flash 0x10000 firmware.bin`  
NOTE: depending on where you installed python and esptool from, you may have to replace it with esptool.py or esptool.py.exe. You may also have to add the python/pip script directory to your PATH

If you have completely wiped the storage on your badge, you may have to rewrite the bootloader and partitions. In that case, you'll need to grab the other bin files and use this command:  
`esptool write_flash 0x0000 bootloader.bin 0x8000 partitions.bin 0xe000 boot_app0.bin 0x10000 firmware.bin`

**NOTE: If you find these instructions don't work or need some tweaking or I left out some important step, please let me know or submit a pull request.**

***I don't actually care which OS you use, I just don't have the time to gather/write instructions for all of them. Google is your friend.**
