# M5StickC-PLUS-TallyLight
A camera tally light project using the M5StickC-Plus and Node-RED. This project is largely inspired by the [Tally Arbiter](http://tallyarbiter.com/) project written by Joseph Adams.

I am currently using [Companion](https://bitfocus.io/companion) to integrate with our ATEM 1 M/E Constellation HD and send websocket messages to Node-RED. Node-RED will provide a websockets server and handle most of the logic.

This project is currently in early development and not yet functional.

First Milestones (July 2023):
- [ x ] WiFi & NTP
- [ x ] Initial websockets interface with Node-RED
- [ x ] Node-RED dashboard to show tally status, messages, etc
- [ x ] Basic tally light functionality
- [ x ] M5StickC-Plus basic power management

Secondary Goals:
- [ x ] Expand on config/parameters with WiFiManager and Node-RED
- [   ] Messaging from producer to tally clients

Stretch Goals:
- [wip] Advanced power management
- [   ] Migrate to M5Stack Core or Core2
- [   ] Integrate with Planning Center Online
- [   ] Integrate with ProPresenter
