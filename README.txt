********************************************************************************
* The cave v0.0.1
********************************************************************************

This is a tech demo where I'm trying different "advanced things".
Don't expect to find any gameplay there... yet...
The source code is a ridiculous pile of crap. Don't dive in it, you've been warned...

Credits :
* The intro music is "Harp test" by Mingos
* The girl/forest picture is done with random stuff grabbed from the net. Their
  licence probably forbid me to use them. I'll replace them with proper
  artwork when there is an actual game...

Notes :
* You can change dynamically the font with PageUp/PageDown.
* You can drag the user interface windows around. They'll remember their
  position the next time you start the game.
* The chapter screen uses non square fonts even if the console font is
  square. Square fonts are great for roguelike renderings, but suck with
  walls of text. This involves putting several fonts in the font bitmap and
  doing dirty ascii mapping tricks.
* Clouds are generated in real time (this is not a looping bitmap), but the
  weather and day time does not change because the chapter one is supposed
  to last a single afternoon.
* Water zone have real time ripples. Try to swim or throw stones in them.
* Water zone have fish shoals you can play with. No fishing yet but you
  can catch a fish with bare hand and even cook it if you manage to find
  how to start a campfire.
* The chapter screen use a text generator inspired by Elite planet description
  generator. It's not very rich right now, but you can easily increase the number
  of text variations exponentially by tweaking data/cfg/chapter1.txg
* In case you didn't notice (it's quite subtle), in the title screen, the wall
  of the cave is shaded with dot3 normal mapping.
* The sidekick is totally dumb and only follows you. Same for the stags in
  the woods. No interaction with them yet.
* I'm not happy with the drag'n drop item crafting system. To be replaced
  with a proper crafting screen later...
* Currently, the source code contains 19k lines of messy C++. Totally indecent.
  Most of it is disabled. Amongst other things, it contains the complete
  Pyromancer! source code. Trust me, you don't want to stick your finger or
  any other part of your body in there.
  The most decent modules can be found here : http://doryen.eptalys.net/demos
