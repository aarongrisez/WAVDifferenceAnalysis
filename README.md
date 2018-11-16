# WAVDifferenceAnalyzer

A short program for detecting differences between two WAV files.


Getting Started
---------------

To use, compile both `DifferenceAnalysis.cpp` and `AudioFile.cpp`. Be sure that the files to be analyzed are in the same directory as the generated binary. Absolute paths to WAV files will likely work, but I haven't tested them extensively.

The files included in this repository have been successfuly compiled under the following conditions:

1. Linux (Ubuntu 18.04.1) using `g++` with the `std=c++11` flag
2. Windows 10 in Visual Studio 2017

Usage
-----
As of 11/15/18, features implemented are:

- Detect either the beginning or end of a range where two WAV files differ samplewise

Features in Development:

- Calculate number of total different samples
- Detect the beginning and end of all ranges where two WAV files differ samplewise (under adequate assumptions)
- Write difference ranges (difference beginning and difference end) to a file
- Segment and reconstruct files such that the total number of different samples is minimized

Assumptions:
------------
1. The two WAV files are mostly similar - very different WAV files will still succeed, but the intent of this program is to process WAVs that are closely related. One example could be a recording which later had a segment punched in.
2. The differences occur in blocks consisting of more than one contiguous sample. This is a reasonable assumption for most use-cases--any significant editing of an audio file will yield changes in more than one sample.
3. The two WAV files can be of different lengths, but should be lined up at the left. i.e.: the leftmost sample (first sample) in the first file corresponds to the leftmost sample in the second file. The exception to this would be if the difference began at the start of the file. Accounting for that case is in development.

Dependencies
------------

AudioFile is written and maintained by Adam Stark.

[http://www.adamstark.co.uk](http://www.adamstark.co.uk)


Audio File License
------------------

Copyright (c) 2017 Adam Stark

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
