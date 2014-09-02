
## About

EmbedDIBits is a small Python 3 program that can convert images to C
byte arrays. The output is compatible with the Win32 API functions:
[SetDIBits][], [StretchDIBits][], [AlphaBlend][] etc...

This is useful in 4kb or 64kb demos, where it isn't possible link against
libc or to use resource files with [Crinkler][] or as a general method
to embed images without dealing with image formats complexity.

## Installation and usage

To install, just make sure you are using Python 3.3+ and have the [Pillow][]
module installed. EmbedDIBits is a single Python script that you can put
in your PATH.

Let's say we have an image named: 'sprite.bmp', living in the current
folder. To generate the 'sprite.h' header:

```bash
$ EmbedDIBits.py sprite.bmp
sprite.bmp -> sprite.h
```

And that's it. Now, from C:

```c
// include the generated header:
#include "sprite.h"

// declare a BITMAPINFO with the same width/height as the sprite
// using 32bit depth:
static const BITMAPINFO sprbmi = {
    { sizeof(BITMAPINFOHEADER), WIDTH, -HEIGHT, 1, 32, BI_RGB, 0, 0, 0, 0, 0 },
    { 0, 0, 0, 0 }
};

// draw it (the variable is the image file basename without extension):
StretchDIBits(..., sprite, &sprbmi, DIB_RGB_COLORS, SRCCOPY);
```

And here is the result:

![Screenshot](https://raw.github.com/Beluki/EmbedDIBits/master/Screenshot/Screenshot.png)

The 32 bit format is used because it's a lot easier to deal with sprites
in the same format, it's the most general format and it's compatible with
[AlphaBlend][], allowing for per-pixel alpha. Crinkler will compress
the images anyway, so it doesn't matter. Formats supported are BMP, JPG,
PNG, TGA, TIFF... anything supported by Pillow itself.

There are two complete examples in the [Examples][] folder, one with and one
without alpha-blending.

## Command-line options

EmbedDIBits has some options that can be used to change the behavior:

* `--newline [dos, mac, unix, system]` changes the newline format.
  I tend to use Unix newlines everywhere, even on Windows. The default is
  `system`, which uses the current platform newline format.

* `--no-buffer` makes it write directly to the target files. This is about
  50% slower, but it uses less memory. Useful on huge images and on devices
  that don't have much memory.

* `--quiet` supresses the `sprite.bmp -> sprite.h` messages during conversion.
  Errors will still be printed to stderr. Useful for scripts.

* `--stdout` writes the generated code to standard output. It can be used
  to generate a single header with all the sprites:

  ```bash
  $ EmbedDIBits.py --stdout *.png > sprites.h
  (sprites.h is generated, errors go to stderr)
  ```

## Limitations

Since EmbedDIBits chooses variable names based on the input image filenames,
it's possible to produce incorrect C identifiers in some corner cases, for
example when using reserved C keywords as filenames.

I believe the convenience of just using 'sprite' in the common case, outweights
the complexity of doing any kind of name-mangling for the corner cases.

## Portability

Information and error messages are written to stdout and stderr
respectively, using the current platform newline format and encoding.

Generated output (the C code) is written as UTF-8, using the newline format
specified by the option `--newline ...`. By default, platform newlines are used
because byte by byte equality of generated files is not important.

The exit status is 0 on success and 1 on errors. After an error,
EmbedDIBits skips the current file and proceeds with the next one
instead of aborting. It can be interrupted with Control + C.

EmbedDIBits is tested on Windows 7 and 8 and on Debian (both x86 and x86-64)
using Python 3.3+ and Pillow 2.1.0+. Older versions are not supported.

## Alternatives

There are other programs (notably [GIMP][]) that can export to C. Unfortunately,
I'm not aware of any program whose output is directly usable by StretchDIBits
and the other Win32 API functions.

## Status

This program is finished!

EmbedDIBits is feature-complete and has no known bugs. Unless issues are reported
I plan no further development on it other than maintenance.

## License

Like all my hobby projects, this is Free Software. See the [Documentation][]
folder for more information. No warranty though.

[AlphaBlend]: http://msdn.microsoft.com/en-us/library/windows/desktop/dd183351%28v=vs.85%29.aspx
[SetDIBits]: http://msdn.microsoft.com/en-us/library/windows/desktop/dd162973%28v=vs.85%29.aspx
[StretchDIBits]: http://msdn.microsoft.com/en-us/library/windows/desktop/dd145121%28v=vs.85%29.aspx

[Crinkler]: http://www.crinkler.net
[GIMP]: http://www.gimp.org
[Pillow]: https://pypi.python.org/pypi/Pillow

[Documentation]: https://github.com/Beluki/EmbedDIBits/tree/master/Documentation
[Examples]: https://github.com/Beluki/EmbedDIBits/tree/master/Examples

