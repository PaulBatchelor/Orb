# ![Orb](img/logo.png)
Orb is a musical 2d puzzle game, designed primarily for Android devices. 

## Rules
### 1. Propel
Propel the Orb (the circle) and move it around. This is done by poking the
Orb in the direction you want it to go. If you poke too quickly, Orb will
run out of oxygen and you will have to wait for to reinflate before propelling
again.

![Propel](img/rule_propel.png)
### 2. Collide
Collide into squares in the space. 

![Collide](img/rule_collide.png)
### 3. Turn off
Turn off any "on" squares (filled in squares) by colliding into them. This
will change them into the "off" state (stroked squares). 

![Turn off](img/rule_turnoff.png)
### 4. Toggle
Any time a square gets turned off, it can toggle the state of other 
squares, on to off, and off to on. The rules for these are determined for
each level.

![Toggle](img/rule_toggle.png)


When all the squares have been turned off, you will ascend to the next level.
And so on, and so forth...

## Sound and Music

The soundtrack to Orb is synthesized entirely in realtime.  There are no
audio files used. The synthesis is done using
[Soundpipe](https://pbat.ch/proj/soundpipe.html). The audio components conist of the following:

- 3 FM oscillators, used as background drone music
- Enveloped filtered noise generator used for propulsion sounds
- Modal synthesis used as a simple physical model for collisions
- Feedback echo delay
- Modified FDN Reverb, optimized for Android mobile devices

## Graphics

The graphics are done using [NanoVG](https://github.com/memononen/nanovg),
a small anti-aliased 2d vector graphics library built on top of OpenGL. 


## Installing
Right now, Orb must be compiled from
[source](http://git.sr.ht/~pbatch/orb).

## Screenshots

![Level 1](img/small/orblevel1.png)

![Level 2](img/small/orblevel2.png)

![Level 3](img/small/orblevel3.png)

![Level 4](img/small/orblevel4.png)

![Level 5](img/small/orblevel5.png)

## demo

Currently hosted on [youtube](https://www.youtube.com/embed/Q3QjTMByalk).

{{FOOTER}}
