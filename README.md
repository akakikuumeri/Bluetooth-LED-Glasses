# Bluetooth-LED-Glasses
Animating LED sunglasses with Bluetooth connectivity with ATMega328

![in action](https://i.imgur.com/tzNodzB.png)

Demo https://www.youtube.com/watch?v=CdjDhWBZKLs

This is an interactive monocrhrome LED screen built into a pair of plastic sunglasses from a 100 yen shop. Small white SMT LEDs are glue onto the glasses, and soldered into a 9 rows of up to 32 LEDs per row. The number varies per row, and this is adjusted for in software. I used enamel coated magnet wire, because it doesn't conduct to itself even when crossing and touching wires.

An HC-05 or similar bluetooth module is used to update the displayed message. In a demo I had a twitter hashtag update the text.

 Apart from a scrolling text display with two different fonts to choose from, there is also an animating eyes mode, a wavy repeating animation, and a faux "sound levels" bar animation, as well as a battery display mode. These can be switched with a push button.

![closeup](https://i.imgur.com/u9Fpa7M.png)

 The columns are run by a couple of shift register ICs, and the rows are run by 9 GPIO pins on an Arduino Mini, through small transistors.

![parts](https://i.imgur.com/x9wY1g5.png)

![wiring](https://i.imgur.com/97CHK6B.png)
Note how the rows and columns each have common wires running through each LEDs anode and cathode. These will be multiplexed to address each individual pixel. Note in green how the enameled wires can be bunched together. They won't short together unless the coating is removed with heat.

![soldering](https://i.imgur.com/Jbqgy0N.png)
This was mainly an exercise in small scale soldering. In the future, I would rather do this with flexible PCBs.

##Related project
Here is a version of the same project with only row-based control: https://github.com/akakikuumeri/LEDglasses
