var imgToPixmap = require('..');
var charm = require('charm')();
var x256 = require('x256');

charm.pipe(process.stdout);

(function draw (img) {
  var width = img.width, height = img.height;
  var pixels = img.data;

  var cols = 120;
  var dx = width / cols;
  var dy = 2 * dx;

  for (var y = 0; y < height; y += dy) {
    for (var x = 0; x < width; x += dx) {
      var i = (Math.floor(y) * width + Math.floor(x)) * 4;

      var ix = x256([ pixels[i], pixels[i+1], pixels[i+2] ]);
      if (pixels[i+3] > 0) {
        charm.background(ix).write(' ');
      }
      else {
        charm.display('reset').write(' ');
      }
    }
    charm.display('reset').write('\r\n');
  }

})(imgToPixmap({ src: 'flashlight.jpg' }));

charm.display('reset').end();
