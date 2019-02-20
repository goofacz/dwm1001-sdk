#!/usr/bin/python

#
# Copyright (C) 2018 Tomasz Jankowski <t.jankowski AT pwr.edu.pl>
#                                     <tomasz.jankowski.mail AT gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see http://www.gnu.org/licenses/.
#

import enum
import re

dwt_time_units = 1.0 / 499.2e6 / 128.0
frequency_offset_multiplier = 998.4e6 / 2.0 / 1024.0 / 131072.0
hertz_to_ppm_multiplier_channel_5 = -1.0e6 / 6489.6e6
speed_of_light = 299702547.0


class TokenType(enum.Enum):
    result = 1,
    prompt = 2,
    error = 3,
    info = 4,
    debug = 5,


def process_input(uart, verbose):
    patterns = (
        (TokenType.result, r'Result:(?P<content>[\s\S]+)\r\n#'),
        (TokenType.prompt, r'#> '),
        (TokenType.error, r'ERROR: (?P<content>.+)\r\n'),
        (TokenType.info, r'INFO: (?P<content>.+)\r\n'),
        (TokenType.debug, r'DEBUG: (?P<content>.+)\r\n'),
    )

    buffer = ''
    while True:
        element = uart.read(1).decode('ascii')
        buffer += element
        if verbose and element != '\r':
            print(element, end='')

        for pattern in patterns:
            match = re.search(pattern[1], buffer)
            if match:
                return pattern[0], match


def write_command(uart, command, verbose):
    uart.write(f'{command}\r'.encode('ascii'))
    uart.readline()
    if verbose:
        print(command)


def compute_clock_offset_ratio(carrier_integrator):
    ratio = carrier_integrator
    ratio *= (frequency_offset_multiplier * hertz_to_ppm_multiplier_channel_5 / 1.0e6)
    return ratio
