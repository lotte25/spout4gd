# Spout4GD
A mod that implements the <cf>Spout2</c> library into Geometry Dash.

## Features
 - Output FPS up to 360 FPS
 - Smooth Frame Pacing
 - Custom Output Resolution
 - Cursor Capture
 - Custom Cursor Scale
 - Custom Cursor Filter (Nearest, Linear)

## How to use?
 1. Install the mod
 2. Install the [OBS Spout2 Plugin](https://github.com/Off-World-Live/obs-spout2-plugin)
 3. Add Spout2 Capture to your scene
 4. Profit!



## FAQ
### What is Spout2?
<cf>Spout2</c> lets a game or app share frames or textures to another app (like OBS) using only the GPU memory, allowing full quality video streaming with full transparency support, and without performance or latency hits. <cd>VTube Studio</c> [is a great example](https://x.com/KatoGrey/status/1670503778510929922).

###  But what does it solve here?
Performance and latency hits caused by Game Capture and Desktop Duplication.
This mod copies the frame directly from the GPU memory and OBS reads it the exact moment it finishes rendering, without pausing the game's thread. This prevents the stutters and visual latency caused by other traditional methods.