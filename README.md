# MIDI-Score-Alighment-Tool(C++)

This is my final project for Music 254 at Stanford, taught by Eleanor Selfridge-Field and Craig Sapp.

The main alighment tool "midi2score.cpp" is under "tools" folder, and thanks Craig for providing other codes for loading MIDI and humdrum files. Follow the steps below to align your MIDI and score:

### Preraration
1. Humdrum file for the score (See http://doc.verovio.humdrum.org for reference)
2. MIDI file for the performance.
The folder "tests" includes 4 song examples.

### Do the alighment
The example files "chopin-prelude28n20.mid" and "chopin-prelude28n20.krn" can be used to test the program:
1. `Make` under the midi2score folder.
2. To print the MIDI notes list, execute
  `bin/midi2score tests/chopin-prelude28n20.mid tests/chopin-prelude28n20.krn -m`
3. To print the Humdrum notes list, execute
  `bin/midi2score tests/chopin-prelude28n20.mid tests/chopin-prelude28n20.krn -h`
4. To print the alignment result, execute
  `bin/midi2score tests/chopin-prelude28n20.mid tests/chopin-prelude28n20.krn`






