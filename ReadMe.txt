VUPlayer 4.9.1
Copyright (c) 2020 James Chapman
------------------------------------------------------------------------------
website: http://www.vuplayer.com
email:   james@vuplayer.com
------------------------------------------------------------------------------

Overview
--------
VUPlayer is an open-source multi-format audio player for Windows 7 SP1 or later.
Audioscrobbler functionality requires an internet connection and a Last.fm account.


Command-line
------------
Ordinarily, VUPlayer stores application settings and metadata in a SQLite database in the user documents folder.
To run in 'portable' mode, leaving no database footprint, the application can be launched using the following command-line argument(s):

	VUPlayer.exe -portable [settings file]

The optional [settings file] parameter specifies the location of an application settings file to use.
The 'Export Settings' function in the main application can be used to save the current settings in the correct format.
Please note that Gracenote and Audioscrobbler functionality is disabled when running in 'portable' mode.


Credits
-------
Main application is Copyright (c) 2020 James Chapman
http://www.vuplayer.com

BASS library is Copyright (c) 1999-2020 Un4seen Developments Ltd.
http://www.un4seen.com

Ogg Vorbis is Copyright (c) 1994-2020 Xiph.Org
http://xiph.org/vorbis/

Opus is Copyright (c) 2011-2020 Xiph.Org
https://opus-codec.org/

FLAC is Copyright (c) 2000-2009 Josh Coalson, 2011-2020 Xiph.Org
http://xiph.org/flac/

WavPack is Copyright (c) 1998-2020 David Bryant
http://www.wavpack.com

Audioscrobbler is Copyright (c) 2020 Last.fm Ltd.
http://www.last.fm


License
-------
Copyright (c) 2020 James Chapman

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
