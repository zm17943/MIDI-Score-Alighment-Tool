
#include "MidiRoll.h"
#include "humlib.h"

using namespace std;
using namespace smf;
using namespace hum;

const float WIN = 1.0;
const float yTreshold1 = 0.8;
const float yTreshold2 = 1.3;

class HumdrumNote;
class MidiNote;

class MidiNote {
	public:
		int pitch = -1;         // MIDI key number of MIDI note
		int link = -1;          // index of note in Humdrum list
		double time = 0;        // Onset time
		bool available = true;  // The note has not been linked
		float score = -1;       // y value
		MidiEvent* source = NULL;
};


class HumdrumNote {
	public:
		int pitch = -1;          // MIDI key number of Humdrum note
		int link = -1;           // index of note in MIDI list
		HumdrumToken* source = NULL;
		int subtoken = 0;        // index into HumdrumToken for note
		int measure = 0;         // measure number note is found in
		float qtime = 0;         // Onset time
		float score = -1;        // y value
		bool available = true;   // The note has not been linked
};


// function declarations
void   linkNotes          (vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, map<int, vector<float>>& diff);
void   printLinks         (vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, map<int, vector<float>>& diff);

float  computeValue       (vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int type, int index);
float  computeValueUp     (vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int type, int index);
float  computeValueDo     (vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int type, int index);

vector<int> setWinMidi    (vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int index);
int    findFirstNote      (vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, MidiNote target);

bool   pitchMatch         (vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int above, int below, int i, map<int, vector<float>>& diff);
bool   valueMatch         (vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int above, int below, int i, map<int, vector<float>>& diff);
bool   valueMatchUp       (vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int above, int below, int i, map<int, vector<float>>& diff);
bool   valueMatchDo       (vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int above, int below, int i, map<int, vector<float>>& diff);

void   printMidiNotes     (vector<MidiNote>& midinotes);
void   setTime            (vector<MidiNote>& midinotes);
void   printHumdrumNotes  (vector<HumdrumNote>& humnotes);
void   loadMidiNotes      (vector<MidiNote>& midinotes, MidiRoll& midifile);
void   loadHumdrumNotes   (vector<HumdrumNote>& humnotes, HumdrumFile& humfile);
double milliseconds       (double input);
void   printUsage         (const string& command);
void   printRatios        (map<int, vector<float>>& diff);


///////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv) {
	Options options;
	options.define("m|print-midi-notes=b", "Print list of input MIDI notes");
	options.define("h|print-humdrum-notes=b", "Print list of input Humdrum notes");
	options.define("t|print-error-rate=b", "Print error rate for each key");
	options.process(argc, argv);

	if (options.getArgCount() != 2) {
		printUsage(options.getCommand());
		exit(1);
	}

	MidiRoll midifile;
	midifile.read(options.getArg(1));
	vector<MidiNote> midinotes;
	loadMidiNotes(midinotes, midifile);
	midifile.linkNotePairs();
	midifile.doTimeAnalysis();

	HumdrumFile humfile;
	humfile.read(options.getArg(2));
	vector<HumdrumNote> humnotes;
	loadHumdrumNotes(humnotes, humfile);

        vector<int> mTh;
        vector<int> hTm;
	if (options.getBoolean("print-midi-notes")) {
		setTime(midinotes);
		printMidiNotes(midinotes);
	} else if (options.getBoolean("print-humdrum-notes")) {
		printHumdrumNotes(humnotes);
	} else if (options.getBoolean("print-error-rate")) {
		map<int, vector<float>> diff;
		setTime(midinotes);
	    linkNotes(midinotes, humnotes, diff);
		//printLinks(midinotes, humnotes, diff);
		printRatios(diff);
	}
	else {
		map<int, vector<float>> diff;
		setTime(midinotes);
	    linkNotes(midinotes, humnotes, diff);
		printLinks(midinotes, humnotes, diff);
	}
        return 0;
}


///////////////////////////////////////////////////////////////////////////

// Compute the y-value for a MIDI note(type == 0) or a humdrum note(type == 1)
float computeValue(vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int type, int index){
	if(index == 0 or index == midinotes.size() or index == humnotes.size()) return -1;        // ??
	int last, next;
	if(type == 0){
		int key = midinotes[index].pitch;
		int j = index - 1;
		bool find = false;
		while(j >= 0){
			if(midinotes[j].pitch == key){
				find = true;
				last = j;
				break;
			}
			j --;
		}
		if(!find) return -1;
		find = false;
		j = index + 1;
		while(j < midinotes.size()){
			if(midinotes[j].pitch == key){
				find = true;
				next = j;
				break;
			}
			j ++;
		}
		if(!find) return -1;
	}else if(type == 1){
		int key = humnotes[index].pitch;
		int j = index - 1;
		bool find = false;
		while(j >= 0){
			if(humnotes[j].pitch == key){
				find = true;
				last = j;
				break;
			}
			j --;
		}
		if(!find) return -1;
		find = false;
		j = index + 1;
		while(j < humnotes.size()){
			if(humnotes[j].pitch == key){
				find = true;
				next = j;
				break;
			}
			j ++;
		}
		if(!find) return -1;
	}else{
		return -1;
	}
	return type == 0 ? (midinotes[next].time-midinotes[index].time)/(midinotes[index].time-midinotes[last].time) : (humnotes[next].qtime-humnotes[index].qtime)/(humnotes[index].qtime-humnotes[last].qtime);
}


// Compute the y-value for a MIDI note(type == 0) or a humdrum note(type == 1)
float computeValueUp(vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int type, int index){
	if(index == 0) return -1;        
	int last = -1, lastlast = -1;
	bool find = false;
	if(type == 0){
		int key = midinotes[index].pitch;
		int j = index - 1;
		while(j >= 0){
			if(midinotes[j].pitch == key){
				if(last == -1) last = j;
				else lastlast = j;
				if(last > 0 && lastlast > 0){
					find = true;
				    break;
				}
			}
			j --;
		}
		if(!find) return -1;
	}else if(type == 1){
		int key = humnotes[index].pitch;
		int j = index - 1;
		bool find = false;
		while(j >= 0){
			if(humnotes[j].pitch == key){
				if(last == -1) last = j;
				else lastlast = j;
				if(last > 0 && lastlast > 0){
					find = true;
				    break;
				}
			}
			j --;
		}
		if(!find) return -1;
	}else{
		return -1;
	}
	return type == 0 ? (midinotes[lastlast].time-midinotes[last].time)/(midinotes[last].time-midinotes[index].time) : (humnotes[lastlast].qtime-humnotes[last].qtime)/(humnotes[last].qtime-humnotes[index].qtime);
}


// Compute the y-value for a MIDI note(type == 0) or a humdrum note(type == 1)
float computeValueDo(vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int type, int index){
	if(index == midinotes.size() or index == humnotes.size()) return -1;       
	int next = -1, nextnext = -1;
	bool find = false;
	if(type == 0){
		int key = midinotes[index].pitch;
		int j = index + 1;
		while(j <= midinotes.size()){
			if(midinotes[j].pitch == key){
				if(next == -1) next = j;
				else nextnext = j;
				if(next > 0 && nextnext > 0){
					find = true;
				    break;
				}
			}
			j ++;
		}
		if(!find) return -1;
	}else if(type == 1){
		int key = humnotes[index].pitch;
		int j = index + 1;
		bool find = false;
		while(j <= humnotes.size()){
			if(humnotes[j].pitch == key){
				if(next == -1) next = j;
				else nextnext = j;
				if(next > 0 && nextnext > 0){
					find = true;
				    break;
				}
			}
			j ++;
		}
		if(!find) return -1;
	}else{
		return -1;
	}
	return type == 0 ? (midinotes[index].time-midinotes[next].time)/(midinotes[next].time-midinotes[nextnext].time) : (humnotes[index].qtime-humnotes[next].qtime)/(humnotes[next].qtime-humnotes[nextnext].qtime);
}



// Return a list of candidate humdrum notes which are within the window(1s) of the target midi note
vector<int> setWinMidi(vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int index){
	vector<int> v;
	vector<int> result;
	int j;
	// if(midinotes[index].time - midinotes[index-1].time > WIN){  // if the MIDI note is start of a new measure far from the previous note
	// 	vector<int> result0;
	// 	float win = WIN + midinotes[index].time - midinotes[index-1].time;
	// 	int j0 = index - 1;
	//     while(j0 >= 0){
	// 		if(!midinotes[j0].available) break;
	// 		j0 --;
	// 	}
	// 	j0 = midinotes[j0].link;
	// 	int earliest0 = j0;
	// 	while(j0 < humnotes.size() && humnotes[j0].qtime - humnotes[earliest0].qtime <= win){
	// 		j0 ++;
	// 	}
	// 	result0.push_back(earliest0);
	// 	result0.push_back(j0);
	// 	return result0;		
	// }
	float win = WIN;
	j = index - 1;
	while(j >= 0){
		if(midinotes[index].time - midinotes[j].time > WIN) break;
		if(!midinotes[j].available)
			v.push_back(j);
		j --;
	} 
	if(v.size() == 0){                  // if no previous linked midinote within the window, keep finding previous one
		while(j >= 0){
			if(!midinotes[j].available){
				v.push_back(j);
				break;
			}
			j --;
		}
		win = WIN + midinotes[index].time - midinotes[j].time;
		int temp = j;
		while(j >= 0 && midinotes[temp].time - midinotes[j].time <= WIN){
			if(!midinotes[j].available)
				v.push_back(j);
			j --;
		}
	}
	sort(v.begin(), v.end());              // v stores all linked MIDI notes within the window(1s) of the target midi note 
	int earliest = midinotes[v[0]].link;   
	int latest = midinotes[v[0]].link;
	for(int item : v){
		if(midinotes[item].link < earliest)
			earliest = midinotes[item].link;
		if(midinotes[item].link > latest)
			latest = midinotes[item].link;
	}
	int above, below;                      // the range of Humdrum index where a valid match might happen
	if(earliest == 0) above = 0;
	else{
		j = earliest - 1;
		while(j >= 0 && humnotes[earliest].qtime - humnotes[j].qtime <= win){
			j --;
		}
		above = j + 1;
	}
	if(latest == humnotes.size() - 1) below = humnotes.size() - 1;
	else{
		j = latest + 1;
		while(j < humnotes.size() && humnotes[j].qtime - humnotes[latest].qtime <= win){
			j ++;
		}
		below = j - 1;
	}
	result.push_back(above);
	result.push_back(below);
	return result;
}

// Return the first Humdrum note's index which has the same pitch with the target MIDI note
int findFirstNote(vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, MidiNote target){
	int j = 0;                                           // find the link for the first modinote
	while(j < humnotes.size() && humnotes[j].pitch != target.pitch){
		j ++;
	}
	return j;
}

bool pitchMatch(vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int above, int below, int i, map<int, vector<float>>& diff){
	bool find = false;
	for(int p = above; p <= below; p++){
		if(humnotes[p].available && humnotes[p].pitch == midinotes[i].pitch){
			find = true;
			midinotes[i].link = p;
			midinotes[i].available = false;
			humnotes[p].available = false;
			humnotes[p].link = i;
			float value = -1;                                  // The y value is definetely -1 if "pitchMatch" is called
			if(diff.find(midinotes[i].pitch) != diff.end())
				diff[midinotes[i].pitch].push_back(value);
			else{
				vector<float> v;
				v.push_back(value);
				diff[midinotes[i].pitch] = v;
			}
			break;
		}
	}
	return find;
}


bool valueMatch(vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int above, int below, int i, map<int, vector<float>>& diff){
	float candidateValue = -1.0;
	int candidateIndex;
	float y;
	float value1 = computeValue(midinotes, humnotes, 0, i);
	if(value1 == -1) return false;
	for(int p = above; p <= below; p++){
		if(!humnotes[p].available || humnotes[p].pitch != midinotes[i].pitch) continue;
		float value2 = computeValue(midinotes, humnotes, 1, p);
		y = value1 / value2;
		if(y <= yTreshold2 && y >= yTreshold1){
			if(abs(y - 1) < abs(candidateValue - 1)){
				candidateValue = y;
			    candidateIndex = p;
			}
		}
	}
	if(candidateValue == -1.0) return false;
	midinotes[i].link = candidateIndex;
	midinotes[i].available = false;
	humnotes[candidateIndex].available = false;
	humnotes[candidateIndex].link = i;
	midinotes[i].score = candidateValue;
	if(diff.find(midinotes[i].pitch) != diff.end())
		diff[midinotes[i].pitch].push_back(y);
	else{
		vector<float> v;
		v.push_back(y);
		diff[midinotes[i].pitch] = v;
	}
	return true;
}


bool valueMatchUp(vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int above, int below, int i, map<int, vector<float>>& diff){
	float candidateValue = -1.0;
	int candidateIndex;
	float y;
	float value1 = computeValueUp(midinotes, humnotes, 0, i);
	if(value1 == -1) return false;
	for(int p = above; p <= below; p++){
		if(!humnotes[p].available || humnotes[p].pitch != midinotes[i].pitch) continue;
		float value2 = computeValueUp(midinotes, humnotes, 1, p);
		y = value1 / value2;
		if(y <= yTreshold2 && y >= yTreshold1){
			if(abs(y - 1) < abs(candidateValue - 1)){
				candidateValue = y;
			    candidateIndex = p;
			}
		}
	}
	if(candidateValue == -1.0) return false;
	midinotes[i].link = candidateIndex;
	midinotes[i].available = false;
	humnotes[candidateIndex].available = false;
	humnotes[candidateIndex].link = i;
	midinotes[i].score = candidateValue;
	if(diff.find(midinotes[i].pitch) != diff.end())
		diff[midinotes[i].pitch].push_back(y);
	else{
		vector<float> v;
		v.push_back(y);
		diff[midinotes[i].pitch] = v;
	}
	return true;
}


bool valueMatchDo(vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, int above, int below, int i, map<int, vector<float>>& diff){
	float candidateValue = -1.0;
	int candidateIndex;
	float y;
	float value1 = computeValueDo(midinotes, humnotes, 0, i);
	if(value1 == -1) return false;
	for(int p = above; p <= below; p++){
		if(!humnotes[p].available || humnotes[p].pitch != midinotes[i].pitch) continue;
		float value2 = computeValueDo(midinotes, humnotes, 1, p);
		y = value1 / value2;
		if(y <= yTreshold2 && y >= yTreshold1){
			if(abs(y - 1) < abs(candidateValue - 1)){
				candidateValue = y;
			    candidateIndex = p;
			}
		}
	}
	if(candidateValue == -1.0) return false;
	midinotes[i].link = candidateIndex;
	midinotes[i].available = false;
	humnotes[candidateIndex].available = false;
	humnotes[candidateIndex].link = i;
	midinotes[i].score = candidateValue;
	if(diff.find(midinotes[i].pitch) != diff.end())
		diff[midinotes[i].pitch].push_back(y);
	else{
		vector<float> v;
		v.push_back(y);
		diff[midinotes[i].pitch] = v;
	}
	return true;
}


//////////////////////////////
//
// linkNotes -- Match notes in the midinotes and humnotes array.
//

void linkNotes(vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, map<int, vector<float>>& diff) {
	int firstMIDI = 0;                 // the first MIDI note to be matched (it is likely to be extra note)
	int firstHum = findFirstNote(midinotes, humnotes, midinotes[firstMIDI]);
	while (humnotes[firstHum].qtime - humnotes[0].qtime > WIN){  // it is an extra note
		firstMIDI ++;
		midinotes[firstMIDI].available = false;
		firstHum = findFirstNote(midinotes, humnotes, midinotes[firstMIDI]);
	}
	midinotes[firstMIDI].link = firstHum;
	midinotes[firstMIDI].available = false;
	midinotes[firstMIDI].score = -1;
	humnotes[firstHum].available = false;
	humnotes[firstHum].link = firstMIDI;

    for(int i = 0; i < midinotes.size(); i++){   // traverse each MIDI note after the "valid" first one 
    	if(i == firstMIDI) continue;
        vector<int> win = setWinMidi(midinotes, humnotes, i);
		int above = win[0];
		int below = win[1];
		bool find = false;
		find = valueMatch(midinotes, humnotes, above, below, i, diff);
		if(find) continue;
		find = valueMatchUp(midinotes, humnotes, above, below, i, diff);
		if(find) continue;
		find = valueMatchDo(midinotes, humnotes, above, below, i, diff);
		if(find) continue;
		find = pitchMatch(midinotes, humnotes, above, below, i, diff);
    }
}


//////////////////////////////
//
// printLinks --
//

void printLinks(vector<MidiNote>& midinotes, vector<HumdrumNote>& humnotes, map<int, vector<float>>& diff) {
	cout << "MIDI_INDEX\tHUM_NOTE\tHUM_INDEX\tMIDI_NOTE\n";
	int len = midinotes.size() < humnotes.size()? midinotes.size() : humnotes.size();
	for (int i=0; i<len; i++) {
		if (midinotes[i].source == NULL) {
			cerr << "ERROR in MIDI note entry " << i << endl;
			continue;
		}
		if (humnotes[i].source == NULL) {
			cerr << "ERROR in Humdrum note entry " << i << endl;
			continue;
		}
		cout << i << "\t" << "\t";
		cout << midinotes[i].link << "\t" << "\t";
		cout << i << "\t" << "\t";
		cout << humnotes[i].link;
		cout << endl;
	}
	if (midinotes.size() > humnotes.size()){
		for (int i=humnotes.size(); i<midinotes.size(); i++){
			if (midinotes[i].source == NULL) {
			    cerr << "ERROR in MIDI note entry " << i << endl;
			    continue;
		    }
			cout << i << "\t" << "\t";
			cout << midinotes[i].link;
			cout << endl;
		}
	}
	if (midinotes.size() < humnotes.size()){
		for (int i=midinotes.size(); i<humnotes.size(); i++){
			if (humnotes[i].source == NULL) {
			    cerr << "ERROR in Humdrum note entry " << i << endl;
			    continue;
		    }
		    cout << "\t" << "\t";
		    cout << "\t" << "\t";
			cout << i << "\t" << "\t";
			cout << humnotes[i].link;
			cout << endl;
		}
	}
}


void printRatios(map<int, vector<float>>& diff){
	int count = 0;
	for(auto it = diff.begin(); it != diff.end(); it++){
		cout << it->first << ":" << endl;
		vector<float> v = it->second;
		for(float i : v){
			cout << i << " ";
			if(i > 2)
				count ++;
		}
		cout << endl;
	}
	cout << "***" << count << endl;
}



//////////////////////////////
//
// printMidiNotes -- Print a list of the input MIDI notes along with their
//    timings both in tick units and in seconds, as well as their durations.
//

void printMidiNotes(vector<MidiNote>& midinotes) {
	cout << "INDEX\tKEY\tTICK\tTDUR\tSEC\tSDUR\n";
	for (int i=0; i<midinotes.size(); i++) {
		if (midinotes[i].source == NULL) {
			cerr << "ERROR in MIDI note entry " << i << endl;
			continue;
		}
		cout << i << "\t";
		cout << midinotes[i].pitch << "\t";
		cout << midinotes[i].source->tick << "\t";
		cout << midinotes[i].source->getTickDuration() << "\t";
	    //cout << milliseconds(midinotes[i].source->seconds) << "\t";
		cout << midinotes[i].time << "\t";
		cout << milliseconds(midinotes[i].source->getDurationInSeconds());
		cout << endl;
	}
}



//////////////////////////////
//
// printHumdrumNotes --
//

void printHumdrumNotes(vector<HumdrumNote>& humnotes) {
	cout << "INDEX\tKEY\tQTIME\tMEASURE\n";
	for (int i=0; i<humnotes.size(); i++) {
		if (humnotes[i].source == NULL) {
			cerr << "ERROR in Humdrum note entry " << i << endl;
			continue;
		}
		cout << i << "\t";
		cout << humnotes[i].pitch << "\t";
		//HTp token = humnotes[i].source;
		// cout << token->getDurationFromStart().getFloat() << "\t";
		cout << humnotes[i].qtime << "\t";
		cout << humnotes[i].measure;
		cout << endl;
	}
}


void setTime(vector<MidiNote>& midinotes){
	for(int i=0; i<midinotes.size(); i++){
		midinotes[i].time = milliseconds(midinotes[i].source->seconds);
	}
}


//////////////////////////////
//
// loadMidiNotes -- Create a time-sorted list of the MIDI notes from the
//     MIDI file.
//

void loadMidiNotes(vector<MidiNote>& midinotes, MidiRoll& midifile) {
	midinotes.clear();
	midifile.joinTracks();
	int notecount = 0;
	for (int i=0; i<midifile[0].getEventCount(); i++) {
		if (midifile[0][i].isNoteOn()) {
			notecount++;
		}
	}
	midinotes.resize(notecount);
	int counter = 0;
	for (int i=0; i<midifile[0].getEventCount(); i++) {
		if (!midifile[0][i].isNoteOn()) {
			continue;
		}
		midinotes.at(counter).pitch = midifile[0][i].getKeyNumber();
		midinotes.at(counter).source = &midifile[0][i];
		counter++;
	}
	// for (int i=0; i<counter; i++) {
	// 	midinotes[i].time = midinotes[i].source->seconds;
	// }
}



/////////////////////////////
//
// milliseconds --
//

double milliseconds(double input) {
	return (int(input*1000.0 + 0.5))/1000.0;
}



//////////////////////////////
//
// loadHumdrumNotes --
//

void loadHumdrumNotes(vector<HumdrumNote>& humnotes, HumdrumFile& humfile) {
	HumdrumNote hnote;
	humnotes.clear();
	humnotes.reserve(humfile.getLineCount()*10);
	int measure = 0;
	for (int i=0; i<humfile.getLineCount(); i++) {
		if (humfile[i].isBarline()) {
			HumRegex hre;
			if (hre.search(humfile[i].token(0), "=(\\d+)")) {
				measure = hre.getMatchInt(1);
			}
		}
		if (!humfile[i].isData()) {
			continue;
		}
		for (int j=0; j<humfile[i].getFieldCount(); j++) {
			HTp token = humfile.token(i, j);
			if (!token->isKern()) {
				continue;
			}
			if (token->isNull()) {
				continue;
			}
			if (token->isRest()) {
				continue;
			}
			int tcount = token->getSubtokenCount();
			for (int k=0; k<tcount; k++) {
				string subtok = token->getSubtoken(k);
				if (subtok.find("_") != string::npos) {
					continue;
				}
				if (subtok.find("]") != string::npos) {
					continue;
				}
				int midi = Convert::kernToMidiNoteNumber(subtok);
				hnote.pitch = midi;
				hnote.subtoken = k;
				hnote.source = token;
				hnote.qtime = token->getDurationFromStart().getFloat();
				hnote.measure = measure;
				humnotes.push_back(hnote);
			}
		}
	}

	if (humnotes.empty()) {
		return;
	}
	if (humnotes[0].measure > 0) {
		return;
	}

	int nonzero = 0;
	int value = -1;
	for (int i=1; i<humnotes.size(); i++) {
		if (humnotes[i].measure != 0) {
			nonzero = i;
			value = humnotes[i].measure;
			break;
		}
	}
	if (!nonzero) {
		return;
	}
	if (value <= 1) {
		return;
	}
	value--;
	for (int i=0; i<nonzero; i++) {
		humnotes[i].measure = value;
	}
}


//////////////////////////////
//
// printUsage --
//

void printUsage(const string& command) {
	cerr << "Usage: " << command << " file.mid file.krn" << endl;
}



