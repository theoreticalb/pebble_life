# pebble_life
An implementation of Conway's Life for the Pebble smart watch.  

# Features:

* Multiple playback speeds.
* Multiple sizes, all the way down to a grid of 144*152 cells.
* That's pretty much it.  It is just Life, after all.

Implemented accessors into a character array for the cell values so that every bit could be used.  Boolean arrays in C take up a char per element, so a little fudging is necessary to fit two screen-sized generations in memory at the same time.

Also including a number of the assets it was uploaded to the store with, which may be useful if you're trying to get the sizing of marketing assets right.  (For the sake of completion: assets included are Creative Commons Attribution-ShareAlike 4.0 International)

