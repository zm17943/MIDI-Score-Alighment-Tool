# MIDI-Score-Alighment-Tool(C++)

This is my final project for Music 254 at Stanford, taught by Eleanor Selfridge-Field and Craig Sapp.

The alignment tool "midi2score.cpp" is under "tools" folder, and thanks Craig for providing other codes for loading MIDI and humdrum files. Follow the steps below to align your MIDI and score:

### Directories
| Name | Description | 
|============|=============|
| tools | Location for the source code for the program. |
| tests | Test MIDI and Humdrum files to link. |
| bin | Output directory for compiled program. |
| src | Source code for support libraries: midifile, and humlib |
| include | Include files for support libraries. |
| lib | Directory for compiled support libraries. |
| obj | Temporary directory for storing support library object files. |

### Preraration
1. Humdrum file for the score (See http://doc.verovio.humdrum.org for reference)
2. MIDI file for the performance.
The folder "tests" includes four song examples.

### Do the alighment
The example files "chopin-prelude28n20.mid" and "chopin-prelude28n20.krn" can be used to test the program:
1. `Make` under the root directory.
2. To print the MIDI notes list, execute <br />
  `bin/midi2score tests/chopin-prelude28n20.mid tests/chopin-prelude28n20.krn -m`
3. To print the Humdrum notes list, execute <br />
  `bin/midi2score tests/chopin-prelude28n20.mid tests/chopin-prelude28n20.krn -h`
4. To print the alignment result, execute <br />
  `bin/midi2score tests/chopin-prelude28n20.mid tests/chopin-prelude28n20.krn`

The algorithm is still being improved. The idea is to do the one-by-one MIDI to score notes link with reference to the music context, which is simple but effective proved on four song examples. Details can be seen in "writeup.pdf"

<br />
<br />
<br />
Contact: Miao Zhang (miaoz18@stanford.edu)

