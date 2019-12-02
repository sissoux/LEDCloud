# LED CLOUD

This firmware is intended to be used on and artificial LED Cloud. It recreates thunder effects using WS2812B LED Strips, a teensy 3.2 and an audio shiel.


## Usage

Few JSON commands are supported now : 

{method:fadeToHSV,H:160,S:255,V:255,Delay:2000}
Fade from current color to input HSV within Delay.

{method:setToRGB,R:255,G:100,B:0}
Set all LEDs to this RGB setting.

{method:setToHSV,H:255,S:100,V:0}
Set all LEDs to this HSV setting.

{method:flash}
Trigger a random flash (number of LEDs, time and position are random).
Note: This will blackout all other LEDs even in c ase they are not black.

{method:groupFlash,Group:0,Direction:1}
Trigger group flash (time and position are random).
Note: This will blackout all other LEDs even in c ase they are not black.

{method:rdmGroupFlash}
Trigger a random flash of a group (group ID, time and position are random).
Note: This will blackout all other LEDs even in c ase they are not black.

{method:rain}
Starts and stop the raining sound.

{method:thunder}
Triggers a thunder sound and associated FX type (Still to be developped)
