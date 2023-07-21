# combadge [Work in Progress]

A Star Trek combadge built with the ESP32.

![Enclosure](/assets/enclosure.png)

> No copyright infringement is intended.
> This is simply a personal endeavor for fun and learning.

## Details

- Hardware and electronics: [hardware/pcb](hardware/pcb)
- Main ESP32 firmware: [software/combadge](software/combadge)
- 3D-printable enclosure: [hardware/enclosure](hardware/enclosure)
- Nuggets for testing: [software/test](software/test)
- Helpful scripts/tools: [software/tools](software/tools)

This project is still in its early stages. This is pretty much the entire directory tree. Happy exploring!

![PCB 3D View](/assets/pcb3d.gif)

## Goals

The first priority is to get the hardware to a fabrication-ready state.
This means that the hardware should be able to comprehensibly stream bidirectional audio on battery.

Once this works well, some kind of basic walkie-talkie functionality will be implemented (maybe with [WebRTC](webrtc.org) or [Matrix](matrix.org) support?).
Ahead of this, the roadmap is largely undecided. Capable hardware leaves great scope for software.

## Motivations

This is a hobby project I am pursuing in order to broaden my knowledge and experience with embedded electronics and programming.
And of course, to end up with a combadge better than the ones they used on TV (I believe those are just molded and painted resin).
Please bear in mind that I am a beginner, especially when it comes to the electronics (remember that before you plug the batteries in).

## Community

Suggestions, tips, and contributions are welcome!
I hope that with some (of your?) time and effort, this combadge can grow to become more useful than a block of resin, while still retaining the charm.

All the tools used for this, as mentioned in the READMEs, are free and open-source. Thank you to those that made this possible!

See also [DansDesigns/ComBadge](https://github.com/DansDesigns/ComBadge), a project with a similar theme and very intriguing prospects.
