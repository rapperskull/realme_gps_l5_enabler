# realme GPS L5 Enabler Magisk Module

Magisk module to enable L5/E5a/B2a GPS bands on realme/OPPO/OnePlus devices with Qualcomm Snapdragon processors. It has been tested only on realme GT 2 Pro (RMX3301), but it should work on other devices too if there's hardware support.

## Changelog

### v0.0.2

- Attempt to fix triple-reboot issue

### v0.0.1

- First release

## How it works

At each boot, it extracts the `oplusstanvbk` partition, patches it by changing the `/nv/item_files/gps/cgps/me/gnss_multiband_configuration` property, and saves it in the module's directory.

Then, a loop device pointing to the patched file is created and the symlinks under `/dev/block/by-name` and `/dev/block/bootdevice/by-name` are changed to point to the patched block device instead of the original one.

The change is applied only to the current slot, so that you can install OTA updates without problems.

The source code of the patcher is available under [src/gnss_patcher.c](src/gnss_patcher.c) and is based on [libnvbk](https://github.com/rapperskull/libnvbk).

If you want, you can compile it under Windows or Linux, patch the partition offline and flash it with fastboot.

## Compiling

Every dependency is included as a git submodule, so make sure to run `git submodule update --init --recursive` after cloning.

To compile you need a working CMake environment.

When cross-compiling for Android, the Magisk zip file is created automatically, but make sure `BUILD_SHARED_LIBS` is `OFF`.

## License

Unless otherwise specified, all the files in this repository are licensed under the [Apache License 2.0](https://www.apache.org/licenses/LICENSE-2.0).

The full text of the license can be found in [LICENSE](LICENSE).

## Dependencies

This project depends on [libnvbk](https://github.com/rapperskull/libnvbk), also written by me, and distributed under Apache License 2.0. The library itself has other dependencies.

Under Windows, a [getopt](https://github.com/skeeto/getopt) implementation is used, distributed under The Unlicense, and effectively Public Domain.
