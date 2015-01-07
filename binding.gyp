{
  "targets": [
    {
      "target_name": "imgToPixmap",
      "sources": [ "src/pixmap.cc" ],
      'cflags!': [ '-fno-exceptions', '-fpermissive' ],
      'cflags_cc!': [ '-fno-exceptions', '-fpermissive' ],
      "include_dirs" : [
        "<!(node -e \"require('nan')\")"
      ],
      "conditions": [
        ['OS=="mac"', {
          'xcode_settings': {
            'GCC_ENABLE_CPP_EXCEPTIONS': 'YES',
            'OTHER_CFLAGS': [
              '<!@(GraphicsMagick++-config --cppflags)'
            ]
          },
          "libraries": [
             '<!@(GraphicsMagick++-config --ldflags --libs)',
          ],
          'cflags': [
            '<!@(GraphicsMagick++-config --cppflags)'
          ],
        }], ['OS=="linux"', { # not windows not mac
          "libraries": [
            '<!@(GraphicsMagick++-config --ldflags --libs)',
          ],
          'cflags': [
            '<!@(GraphicsMagick++-config --cppflags)'
          ],
        }]
      ]
    }
  ]
}
