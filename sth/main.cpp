#include "classes.h"
#include "methods.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cmath>
#include <sstream>

using namespace std;

int main() {
    string fileIN = "example.grib";
    ifstream input(fileIN, ios::binary);

    int messageCount = 0;

    try {
        while (input.peek() != EOF) {
            GribParser parser;
            streampos startPos = input.tellg();
            try {
                parser.read(input);
            }
            catch (const exception& ex) {
                cerr << "Error GRIB #" << messageCount + 1 << ": " << ex.what() << endl;
                break;
            }

            ostringstream outName;
            outName << "GRIB_data_" << messageCount + 1 << ".txt";
            cout << "\nData saved in " << outName.str() << endl;
            parser.write(outName.str());

            if (parser.checkTotalLength()) {
                cout << "\nMessage #" << messageCount + 1 << ": length VALID" << endl;
            } else {
                cout << "Message #" << messageCount + 1 << ": length INVALID" << endl;
            }

            ++messageCount;

        }
        cout << endl;
        cout << "-------------------------------------------------------------------------" << endl;
        cout << "Processed " << messageCount << " GRIB messages." << endl;
    }
    catch (const exception& ex) {
        cerr << "Error: " << ex.what() << endl;
        return 1;
    }

    return 0;
}