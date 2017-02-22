/*
* @author Matt Bryson http://www.github.com/mattbryson
* @see https://github.com/mattbryson/TouchSwipe-Jquery-Plugin
* @see http://labs.rampinteractive.co.uk/touchSwipe/
* @see http://plugins.jquery.com/project/touchSwipe
* @license
* Copyright (c) 2010-2015 Matt Bryson
* Dual licensed under the MIT or GPL Version 2 licenses.
*/

Copyright (c) 2010-2015 Matt Bryson

Dual licensed under the MIT or GPL Version 2 licenses.

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

# TouchSwipe 1.6
A jQuery plugin to be used on touch devices such as iPad, iPhone, Android etc.

Detects single and multiple finger swipes, pinches and falls back to mouse 'drags' on the desktop.

Time and distance thresholds can be set to distinguish between swipe gesture and slow drag.

Allows exclusion of child elements (interactive elements) as well allowing page scrolling or page zooming depending on configuration.

* Detects swipes in 4 directions, "up", "down", "left" and "right"
* Detects pinches "in" and "out"
* Supports single finger or double finger touch events
* Supports click events both on the touchSwipe object and its child objects
* Definable threshold / maxTimeThreshold to determin when a gesture is actually a swipe
* Events triggered for swipe "start","move","end" and "cancel"
* End event can be triggered either on touch release, or as soon as threshold is met
* Allows swiping and page scrolling
* Disables user input elements (Button, form, text etc) from triggering swipes

## Demos, examples and docs

[http://labs.rampinteractive.co.uk/touchSwipe](http://labs.rampinteractive.co.uk/touchSwipe)  
[http://labs.rampinteractive.co.uk/touchSwipe/docs](http://labs.rampinteractive.co.uk/touchSwipe/docs)


## Installation  
### NPM
````bash
npm install jquery-touchswipe --save
````
### Bower
````bash
bower install jquery-touchswipe --save
````
### Manual
Include the minified file in your project.
````html
<script type="text/javascript" src="js/jquery.touchSwipe.min.js"></script>
````

## Usage
````javascript
$(function() {
  $("#test").swipe( {
    //Generic swipe handler for all directions
    swipe:function(event, direction, distance, duration, fingerCount, fingerData) {
      $(this).text("You swiped " + direction );  
    }
  });

  //Set some options later
  $("#test").swipe( {fingers:2} );
});
````

For full demos, code examples and documentation, see below.


## Development
Install dependencies
````bash
npm install
````

To minify
````bash
npm run minify
````

To build docs
````bash
npm run docs
````

To do both
````bash
npm run build
 ````

### For port to XUI see:
https://github.com/cowgp/xui-touchSwipe
