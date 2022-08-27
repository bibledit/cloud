Quill editing library from https://quilljs.com/

The version now in use is version 1.3.6.

The Quill operational code currently consist of the following files:
- quill.bubble.css
- quill.core.css
- quill.core.js
- quill.js
- quill.min.js
- quill.min.js.map
- quill.notes.css
- quill.snow.css

The Quill operational code is in minified form and in webpack format.

The Quill operational code was downloaded from a public release of Quill.
https://github.com/quilljs/quill/releases

The current directory has a subfolder called "source".
This folder contains the Typescript source code of the Quill editor.
This was downloaded from https://github.com/quilljs/quill

Bibledit has included several minified Javascript objects.
Bibledit does this intentionally.
There is several minifiers, like YUI, Uglify.JS, and others.
Each of them employs their own algorithms to makes the source smaller.
The Quill operational code is minified too.
The Quill operational code can be created from the Quill source code.
The folder "source/_develop/scripts" contains scripts for that.
Bibledit includes the minified versions provided by the developers.
This way the developers are more sure to have well-tested and reliable minified objects.
