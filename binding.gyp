{
  'conditions': [
    ['OS=="linux"', {
      'targets': [
        {
          'target_name': 'mouse',
          'sources': ['mouse_linux.cc'],
        },
      ],
    }],
    ['OS=="mac"', {
      'targets': [
        {
          'target_name': 'mouse',
          'sources': ['mouse_mac.cc'],
        },
      ],
    }],
    ['OS=="win"', {
      'targets': [
        {
          'target_name': 'mouse',
          'sources': ['mouse_win.cc'],
        },
      ],
    }],
  ],
}
