# Bouncing OS Logo
The infamous bouncing DVD logo, except it's actually the logo of your OS.
(Currently) for unix-like systems only (Linux, MacOS, Android etc).

# Recordings

https://github.com/user-attachments/assets/b4edad95-618d-450e-af42-3f0726e00aa4

https://github.com/user-attachments/assets/510e22d8-d583-462e-a9e9-f5fb6cc0aeb1

https://github.com/user-attachments/assets/4ef05e0d-0ed6-41d6-b3f4-8888bfa6e39e

## Dependencies
[`fastfetch`](https://github.com/fastfetch-cli/fastfetch) is used to fetch the ASCII art of the machine's logo.

## Try It Out
```
git clone https://github.com/Shaj2311/Bouncing-OS-Logo
cd Bouncing-OS-Logo
gcc main.c -o main
./main
```
## Features
* Logo is different for every operating system. If fastfetch recognizes your OS, so will this program. 
* Screen bounds update in real time. Zoom in and out as much as you like, and the logo will adapt.
