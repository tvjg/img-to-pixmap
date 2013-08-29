var test        = require('tap').test
  , imgToPixmap = require('..');

var colorLine  = '4px-color-line.jpg'
  , monoSquare = '4px-mono-square.png';

test('imgToPixmap does not have argument', function (t) {
  var singleArgEx = {
    name: 'Error',
    message: 'imgToPixmap() requires a single options hash as an argument'
  };

  t.throws(function () { imgToPixmap() }, singleArgEx, 'throw an error');
  t.end();
});

test('imgToPixmap passed non-object', function (t) {
  var shouldBeObjEx = {
    name: 'Error',
    message: 'imgToPixmap()\'s argument should be an object'
  };

  t.throws(function () { imgToPixmap(9) }, shouldBeObjEx, 'throw an error');
  t.end();
});

test('imgToPixmap options argument', function (t) {
  var shouldHaveSrcEx = {
    name: 'Error',
    message: "imgToPixmap()'s argument should have \"src\" containing either a String filepath or a Buffer instance"
  };

  t.throws(function () { imgToPixmap({}) }, shouldHaveSrcEx, 'throw an error if missing src property');
  t.throws(function () { imgToPixmap({src:9}) }, shouldHaveSrcEx, 'throw an error if src property is not string or Buffer');
  t.end();
});

test('test imgToPixmap on 4px color line sample', function (t) {
  var img = imgToPixmap({src: colorLine});

  t.equal(img.height, 1, 'height is 1');
  t.equal(img.width, 4, 'width is 4');

  var expectedData = new Buffer('fe0000ff00ff01ff0000feffffffffff', 'hex');
  t.equivalent(img.data, expectedData, 'pixel data as expected');
  t.end();
});

test('test imgToPixmap on 4px monochromatic square sample', function (t) {
  var img = imgToPixmap({src: monoSquare});

  t.equal(img.height, 2, 'height is 2');
  t.equal(img.width, 2, 'width is 2');

  var expectedData = new Buffer('ffffffff000000ff000000ffffffffff', 'hex');
  t.equivalent(img.data, expectedData, 'pixel data as expected');
  t.end();
});
