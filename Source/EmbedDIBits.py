#!/usr/bin/env python
# -*- coding: utf-8 -*-

"""
EmbedDIBits.
Convert images to Win32 DIB byte arrays.
"""


import io
import os
import sys

from argparse import ArgumentParser, RawDescriptionHelpFormatter


# Information and error messages:

def outln(line):
    """ Write 'line' to stdout, using the platform encoding and newline format. """
    print(line, flush = True)


def errln(line):
    """ Write 'line' to stderr, using the platform encoding and newline format. """
    print('EmbedDIBits.py: error:', line, file = sys.stderr, flush = True)


# IO utils:

def noext(filepath):
    """ Return 'filepath' without the file extension. """
    return os.path.splitext(filepath)[0]


def noext_basename(filepath):
    """ Return 'filepath' basename without the file extension. """
    return noext(os.path.basename(filepath))


def utf8_bytes(string):
    """ Convert 'string' to bytes using UTF-8. """
    return bytes(string, 'UTF-8')


# Non-builtin imports:

try:
    from PIL import Image

except ImportError:
    errln('EmbedDIBits requires the following modules:')
    errln('Pillow 2.0+ - <https://pypi.python.org/pypi/Pillow>')
    sys.exit(1)


# All the C output is done in bytes to avoid automatic newline conversion
# and default platform encodings.


# Since there are only 256 possible pixel R, G, B values
# we cache all the conversions to hexadecimal values, as bytes.
# e.g. BYTES_RGB_HEX[15] = b'0F'

BYTES_RGB_HEX = [utf8_bytes('{:0>2X}'.format(number)) for number in range(256)]


# Possible newline options, as bytes:

BYTES_NEWLINES = {
    'dos'    : b'\r\n',
    'mac'    : b'\r',
    'unix'   : b'\n',
    'system' : utf8_bytes(os.linesep),
}


def dibits2h(image, buffer, variable, newline):
    """
    Convert a Pillow 'image' to a C byte array, using 'variable' (as UTF-8)
    for declaration and 'newline' format (as bytes).

    The 'buffer' can be anything that implements a write() method,
    such as a io.BytesIO() instance or a file descriptor,
    as long as it accepts bytes.
    """
    # ensure that there is an alpha channel:
    if image.mode != 'RGBA':
        image = image.convert('RGBA')

    width, height = image.size
    total = width * height
    pixels = image.load()

    # emit declaration:
    buffer.write(newline)
    buffer.write(utf8_bytes('DWORD {}[{}] = {{'.format(variable, total)))
    buffer.write(newline)

    # indent the first line:
    buffer.write(b'    ')

    # emit pixeldata:
    linepixels = 0
    for y in range(height):
        for x in range(width):

            # newline/indentation after six pixels (70 characters):
            if linepixels == 6:
                buffer.write(newline)
                buffer.write(b'    ')
                linepixels = 0

            linepixels += 1

            # output 0xARGB order with premultiplied alpha:
            R, G, B, A = pixels[x, y]
            buffer.write(b'0x' + BYTES_RGB_HEX[A]
                               + BYTES_RGB_HEX[int((A * R) / 0xFF)]
                               + BYTES_RGB_HEX[int((A * G) / 0xFF)]
                               + BYTES_RGB_HEX[int((A * B) / 0xFF)]
                               + b',')

    # finish the declaration:
    buffer.write(newline)
    buffer.write(b'};')
    buffer.write(newline)


def compile_image(image, descriptor, variable, newline, buffering):
    """
    A wrapper over dibits2h() that uses an internal buffer to hold the result
    when 'buffering' is True. Otherwise it writes directly to the given descriptor.
    """
    if buffering:
        buffer = io.BytesIO()
        dibits2h(image, buffer, variable, newline)
        descriptor.write(buffer.getvalue())

    else:
        dibits2h(image, descriptor, variable, newline)


# Parser:

def make_parser():
    parser = ArgumentParser(
        description = __doc__,
        formatter_class = RawDescriptionHelpFormatter,
        epilog = 'example: EmbedDIBits.py box.png wall.png --stdout > sprites.h',
        usage  = 'EmbedDIBits.py filepath [filepath ...] [option [options ...]]')

    # positional:
    parser.add_argument('filepaths',
        help = 'image to convert to a C header file',
        metavar = 'filepath', nargs = '+')

    # optional:
    parser.add_argument('--newline',
        help = 'use a specific newline mode (default: system)',
        choices = ['dos', 'mac', 'unix', 'system'], default = 'system')

    parser.add_argument('--no-buffer',
        help = 'write directly to files (slower, uses less memory)',
        action = 'store_true')

    parser.add_argument('--quiet',
        help = 'do not print file -> target information',
        action = 'store_true')

    parser.add_argument('--stdout',
        help = 'print code to standard output instead of files',
        action = 'store_true')

    return parser


# Entry point:

def main():
    parser = make_parser()
    options = parser.parse_args()
    status = 0

    buffering = not(options.no_buffer)
    newline = BYTES_NEWLINES[options.newline]

    for filepath in options.filepaths:
        try:
            image = Image.open(filepath)
            variable = noext_basename(filepath)

            # compile to stdout:
            if options.stdout:
                compile_image(image, sys.stdout.buffer, variable, newline, buffering)

            # compile to a C header file
            # (stdout used only for information):
            else:
                target = noext(filepath) + '.h'

                if not options.quiet:
                    outln('{} -> {}'.format(filepath, target))

                with open(target, 'wb') as descriptor:
                    compile_image(image, descriptor, variable, newline, buffering)

        # note that this does not handle KeyboardInterrupt
        # since it is derived from BaseException:
        except Exception as err:
            errln('{} - {}'.format(filepath, err))
            status = 1

    sys.exit(status)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        pass

