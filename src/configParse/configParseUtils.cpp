#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

using namespace std;

void configThrow(string what) {
#ifdef CONFIG_DEBUG
	cerr << what << endl;
#endif
	(void) what;
	throw exception();
}

void configErrorLog(string expected, string& actual) {
	cerr << "Error: expected \"" << expected
			<< "\" found \"" << actual << "\" \n";
	configThrow(expected);
}

//skips empty lines, collapses WS, returns "" on eof
string parseNextLine(ifstream& file) {
	string line;
	while (getline(file, line)) {
		line = line.substr(0, line.find('#'));
		istringstream iss(line);
		string res;
		if (!(iss >> res))
			continue;

		string token;
		while (iss >> token) {
			res += ' ';
			res += token;
		}
		return res;
	}
	return "";
}

