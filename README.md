# Keyloog

A simple keylogger for X11.

## Usage

    Usage: keyloog [OPTION]... [FILE]
    
      -a, --append          do not truncate output file
      -s, --simple          log only key down events
      -t, --time            log timestamps
    
      -d, --daemonize       run in the background
      -p, --pid-file=FILE   write PID to FILE
    
          --spoof=NAME      change command line to NAME
    
      -h, --help            display this help and exit
          --version         output version information and exit

If `FILE` is not present, write keypresses to stdout.

## License

Copyright (c) 2011, Curtis McEnroe <programble@gmail.com>

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted, provided that the above
copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
