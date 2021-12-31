/**
 * Longpress is a jQuery plugin that makes it easy to support long press
 * events on mobile devices and desktop borwsers.
 *
 * @name longpress
 * @version 0.1.2
 * @requires jQuery v1.2.3+
 * @author Vaidik Kapoor
 * @license MIT License - http://www.opensource.org/licenses/mit-license.php
 *
 * For usage and examples, check out the README at:
 * http://github.com/vaidik/jquery-longpress/
 *
 * Copyright (c) 2008-2013, Vaidik Kapoor (kapoor [*dot*] vaidik -[at]- gmail [*dot*] com)
 * Copyright (c) 2021-2022 Teus Benschop
 */

(function($) {
  $.fn.longpress = function(longCallback, shortCallback, duration) {
    if (typeof duration === "undefined") {
      duration = 500;
    }
    
    return this.each(function() {
      var $this = $(this);
      
      // to keep track of how long something was pressed
      var mouse_down_time;
      var timeout;
      
      // mousedown or touchstart callback
      function mousedown_callback(e) {
        mouse_down_time = new Date().getTime();
        var context = $(this);
        
        // set a timeout to call the longpress callback when time elapses
        timeout = setTimeout(function() {
          if (typeof longCallback === "function") {
            longCallback.call(context, e);
          } else {
            $.error('Callback required for long press. You provided: ' + typeof longCallback);
          }
        }, duration);
      }
      
      // Mouseup or touchend callback.
      function mouseup_callback(e) {
        var press_time = new Date().getTime() - mouse_down_time;
        if (press_time < duration) {
          // cancel the timeout
          clearTimeout(timeout);
          
          // call the shortCallback if provided
          if (typeof shortCallback === "function") {
            shortCallback.call($(this), e);
          } else if (typeof shortCallback === "undefined") {
            ;
          } else {
            $.error('Optional callback for short press should be a function.');
          }
        }
      }
      
      // Cancel long press event if the finger or mouse was moved.
      // Later it was discovered that cancelling the long press event upon move was not useful.
      // With browsers and mouse movement, there was often a tiny move detected.
      // This tiny move would cancel the event.
      // The result is that the long press event would behave erratically.
      // The same was found to be the case for touch events.
      // While doing the touch, small movements were always detected.
      // This prevented the long press to work on touch devices.
      // The solution for just now is to always prevent the mouse / touch move events.
      function move_callback(e) {
        e.preventDefault();
        // clearTimeout(timeout);
      }
      
      // Browser support.
      $this.on('mousedown', mousedown_callback);
      $this.on('mouseup', mouseup_callback);
      $this.on('mousemove', move_callback);
      
      // Mobile support.
      $this.on('touchstart', mousedown_callback);
      $this.on('touchend', mouseup_callback);
      $this.on('touchmove', move_callback);
    });
  };
}(jQuery));
