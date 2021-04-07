# fungi soundscape

## an audio(visual) performance

* Inspired by the unfamiliar shapes found in all kinds of fungi growing in nature, the autonomous algorithm transforms images of different fungus into music, resulting in an eery and uncanny composition.


Website link: [link](https://antoniohof.com/Fungi-soundscape).


-----

* The general idea is to create an algorithm that receives recent mushroom observations and turns it into music by analyzing mushroom patterns using simple computer vision.
* Try to derive an algoritmin from the mushroom shape or growth pattern. Image to sound

### Details
* developed using openframeworks and the amazing ofxPDSP addon
* final version is app named 'new_test'

### TODO
* pull images from mushroom database - ok
* use openCv to proces mushroom image - ok
* create a bit matrix to play with a low sampled image - ok
* do the image fetching and processing in another thread - ok
* keep adding new mushrooms as the previous image is consumed by the algo - ok
* use more instruments - ok
* create Mushroom.h Class and display more information about the schroom in the screen (location, full name) - ok
* use some kind of beat/kick instrument - (playing with attack/decay/sustain makes it more ritmic) - ok
* play with modules, what parameter use to control modules - ok
* how can I make the piece evolve in a more 'musical' way? - ok

* add a black and white map with latitude/longitude of observation

### Potential for improving
* how to skip an image when it's not well defined/processed? Example: when hands are present, rulers, etc
* make use of Machine Learning to get mushroom shapes instead of opencv/computer vision
* Missing beats, a lot of mushrooms sounds a bit the same. Tweak parameters, add instruments.

### screenshots
![Screenshot](/mushroom_1.png?raw=true "screen shot 1")
![Screenshot](/mushroom_2.png?raw=true "screen shot 2")
![Screenshot](/mushroom_3.png?raw=true "screen shot 3")
![Screenshot](/mushroom_4.png?raw=true "screen shot 4")
