# Adapted from https://docs.platformio.org/en/stable/scripting/examples/override_package_files.html


"""
Patches were generated with the following commands:
diff '.pio/libdeps/esp32-s3-devkitc-1/WebServer_ESP32_W6100/src/WebServer_ESP32_W6100.hpp' /tmp/WebServer_ESP32_W6100.hpp > patches/dont-include-webserver-h.patch
diff '.pio/libdeps/esp32-s3-devkitc-1/GFX Library for Arduino/src/databus/Arduino_ESP32LCD8.cpp' /tmp/Arduino_ESP32LCD8.cpp > patches/fix-esp32lcd8-write-pixels.patch
"""


from os.path import isfile
from pathlib import Path

Import("env")

LIBDEPS_DIR: Path = Path(env.subst("$PROJECT_DIR")) / ".pio" / "libdeps" /env.subst("$PIOENV")
LIBDEPS_PATCHFLAG_PATH: Path = Path(LIBDEPS_DIR / ".patching-done")
PATCHES_DIR: Path = Path(env.subst("$PROJECT_DIR")) / "patches"

# patch file only if we didn't do it before
if not isfile(LIBDEPS_PATCHFLAG_PATH):

    files_to_patch = [
        (
            LIBDEPS_DIR / "WebServer_ESP32_W6100" / "src" / "WebServer_ESP32_W6100.hpp",
            PATCHES_DIR / "dont-include-webserver-h.patch",
        ),
        (
            LIBDEPS_DIR / "GFX Library for Arduino" / "src" / "databus" / "Arduino_ESP32LCD8.cpp",
            PATCHES_DIR / "fix-esp32lcd8-write-pixels.patch",
        ),
    ]

    for original_file, patch_file in files_to_patch:
        print("Patching {} with {}".format(original_file, patch_file))
        if not original_file.is_file():
            raise Exception("Library file {} does not exist".format(original_file))
        if not patch_file.is_file():
            raise Exception("Patch file {} does not exist".format(patch_file))

        # -l means ignore whitespace
        env.Execute("patch -l '{}' '{}'".format(original_file, patch_file))

    Path(LIBDEPS_PATCHFLAG_PATH).touch()
