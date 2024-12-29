How to:
1. Create folder `build-libs-windows` in repository root
2. Download and extract `ffmpeg-release-full-shared.7z` into `build-libs-windows` ( from https://www.gyan.dev/ffmpeg/builds/ )
   Maybe it will be good idea to fix version and put this version in download script
3. Rename extracted folder to `ffmpeg`. Chech that `ffmpeg` folder contains `bin`, `include`, etc. folders.
4. Trying to use prebuild angle from https://github.com/mmozeiko/build-angle
   Download latest release and extract to `build-libs-windows`.
   Maybe it better to clone/adjust this build pipeline

5. After build you need to copy `bin` folders content to folder with `.exe`