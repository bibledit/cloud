The MIT License (MIT)

Copyright (c) 2013 naoxink

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

notifIt!
=

Simple notifications with JQuery.

Now you can send notifications of everything you want and when you want, simply and quickly.
Easy to learn and use. Customize with your favorite colors, define the size you want, set the opacity, make a sticky one and much more!

#### Give it a try! [Demo](http://naoxink.hol.es/notifIt)

#### Structure
```
 notifIt
 ├── css
 │   └── notifIt.css
 ├── demo.html
 ├── dev
 │   └── notifIt.js
 └── js
     ├── notifIt.js
     └── notifIt.min.js
```

#### Plug
```html
<head>
	<script type='text/javascript' src='js/notifIt.js'></script>
	<link rel='stylesheet' type='text/css' href='css/notifIt.css'>
</head>
```

#### & Play
```javascript
notif({
	msg: "<b>Oops!</b> A wild error appeared!",
	type: "error",
	position: "center"
});
```

## `notif()`

#### Configuration

Variable name|Type|Posible values|Default
---|---|---|---
type|`String`|success, error, warning, info|default
msg|`String`|Message|
position|`String`|left, center, right, bottom|right
width|`Integer`-`String`|Number > 0, 'all'|400
height|`Integer`|Number between 0 and 100|60
autohide|`Boolean`|true, false|true
opacity|`Float`|From 0 to 1|1
multiline|`Boolean`|true, false|false
fade|`Boolean`|true, false|false
bgcolor|`String`|HEX color|#444
color|`String`|HEX color|#EEE
timeout|`Integer`|Miliseconds|5000
zindex|`Integer`|The z-index of the notification|null (ignored)
offset|`Integer`|The offset in pixels from the edge of the screen|0
callback|`Function`|Function|null (ignored),
clickable|`Boolean`|true, false|false
append (dev)|`Boolean`|true, false|false


## `notif_confirm()`
### Description
Now you can ask 'yes' or 'no' easy as --
```javascript
var myCallback = function(choice){
    if(choice){
        notif({
            'type': 'success',
            'msg': 'Yeah!',
            'position': 'center'
        })
    }else{
        notif({
            'type': 'error',
            'msg': ':(',
            'position': 'center'
        })
    }
}

notif_confirm({
	'textaccept': 'Let\'s go!',
	'textcancel': 'I\'ll think about it',
	'message': 'Shall we?',
	'callback': myCallback
})
```

#### Configuration

Variable name|Type|Default|Optional
---|---|---|---
textaccept|`String`|Accept|Yes
textcancel|`String`|Cancel|Yes
message|`String`|Is that what you want to do?|Yes
callback|`Function`|null|Yes
fulllscreen|`Boolean`|false|Yes

#### Response
Function returns `true` or `false`
If callback is passed, it recieves a param `true` or `false`


# More things :)
- [bgfader](https://github.com/naoxink/bgfader)
- [Sublime text color scheme](https://github.com/naoxink/nxk-sublime-color-scheme)
- [asdText](https://github.com/naoxink/asdText)
- [View more](https://github.com/naoxink?tab=repositories)
