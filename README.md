# Poodle
Poodle is a sketch for M5Paper, that for playing Wordle-like game on M5Paper.

## How to run
1. This sketch requires M5Paper and microSD card (<= 16GB)
2. Copy words.txt to microSD card. You can use custome words.txt instead
3. Put font.ttf into microSD card if you want to use custome font. Open Sans recommended
4. Build and transfer this project as PlatformIO project

## How to play
1. Push "NEW" button to start new game. The answer will be chosen randomly from words.txt
2. Guess answer word in 5 letters and type it with onscreen keyboard. Push "<" key to delete last type
3. Push "=" key to check answer. If words.txt doesn't contain input word, it will be cleared
4. Circle mark means the letter and its position are correct
5. Triangle mark means the answer contains the letter but differenct position
6. Otherwise, the answer doesn't contain the letter
7. Guess the answer word in 6 times

## Other operation
- Push "OFF" button to turn off power. The screen remains because of e-ink display
- Push power on button of M5Paper during game, it will export screenshot to microSD card in PMG file

## Dependencies
This PlatformIO project depends on following libraries:
- M5EPD https://github.com/m5stack/M5EPD

## Licenses Notation

### "words.txt" in this project is used under following license:
WordNet Release 3.0 This software and database is being provided to you, the LICENSEE, by Princeton University under the following license. By obtaining, using and/or copying this software and database, you agree that you have read, understood, and will comply with these terms and conditions.: Permission to use, copy, modify and distribute this software and database and its documentation for any purpose and without fee or royalty is hereby granted, provided that you agree to comply with the following copyright notice and statements, including the disclaimer, and that the same appear on ALL copies of the software, database and documentation, including modifications that you make for internal use or for distribution. WordNet 3.0 Copyright 2006 by Princeton University. All rights reserved. THIS SOFTWARE AND DATABASE IS PROVIDED "AS IS" AND PRINCETON UNIVERSITY MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED. BY WAY OF EXAMPLE, BUT NOT LIMITATION, PRINCETON UNIVERSITY MAKES NO REPRESENTATIONS OR WARRANTIES OF MERCHANT- ABILITY OR FITNESS FOR ANY PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE, DATABASE OR DOCUMENTATION WILL NOT INFRINGE ANY THIRD PARTY PATENTS, COPYRIGHTS, TRADEMARKS OR OTHER RIGHTS. The name of Princeton University or Princeton may not be used in advertising or publicity pertaining to distribution of the software and/or database. Title to copyright in this software, database and any associated documentation shall at all times remain with Princeton University and LICENSEE agrees to preserve same.


### "default_16MB.csv" in this project is used under following license:
MIT License

Copyright (c) 2020 m5stack

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
