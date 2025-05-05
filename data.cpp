/*
Sekcja 0:
1. długość message'a, Bajt: 5-7 (od znaku G do ostatniej 7-ki).                                                     DONE
2. porównać wynik z sumą długości wszystkich sekcji w message'u.                                                    DONE
Uwaga, w pliku który Państwo macie są dwa message'e.                                                        WRITE LOOP !

Sekcja 1:
1. B 1-3 długość sekcji PDS;                                                                                        DONE
2. B 8 obecność sekcji GDS i BMS - odkodować maskę bitów.                                                           DONE
3. B 27-28 wykładnik skali dziesiętnej; bit nr 1 w Bajcie 27 - bit znaku.                                           DONE
4. B od 13 do 17 - data wykonania komunikatu.                                                                       DONE
5. B od 11-12 Height, pressure, etc. of the level or layer.                                                         DONE
6. B 10 Indicator of type of level or layer                                                                         DONE

Sekcja 2:
1. B 1-3 długość sekcji GDS;                                                                                        DONE
2. B 9-10 Nrows                                                                                                     DONE
3. B 11-13 szerokość geograficzna początku oktantu danych;                                                          DONE
4. B 14-16 długość geograficzna początku oktantu danych;                                                            DONE
5. B 18-20 szerokość geograficzna końca oktantu danych;                                                             DONE
6. B 21-23 długość geograficzna końca oktantu danych;                                                               DONE
7. B 26-27 increment N-S                                                                                            DONE
8. 29-32 Reserved (set to zero) - 3 liczby po 10 bitow                                                              DONE
9. B 33- 178 2bajtowe liczby                                                                                        DONE

Sekcja 4:
1. B 1-3 długość sekcji BDS;                                                                                        DONE
2. B 5-6 współczynnik skali binarnej E;                                                                             DONE
3. B 7-10 wartość referencyjna (minimalna) R;                                                                       DONE
4. B 11 - liczba bitów dla kodowanej liczby n- bitów;                                                               DONE
5. zaczynając od Bajtu 12-go proszę odczytać kilka kolejnych wartości (10bitowych).                                 DONE

wzor na temperature: y= (RefVal + (x *2^E))/10^D
*/

#include "header.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cmath>
#include <sstream>




int main() {
string fileIN="20150310.00.W.dwa_griby.grib";
ifstream input(fileIN, ios::binary);

int messageCount = 0;

    try {
        while (input.peek() != EOF) {
            GribParser parser;
            streampos startPos = input.tellg();
            try{ parser.read(input);}
            catch (const std::exception& ex) {
            cerr << "Error GRIB #" << messageCount + 1 << ": " << ex.what() << endl;
            break;
        }
            ostringstream outName;
            outName << "GRIB_data_" << messageCount + 1 << ".txt";
            cout<< "\nData saved in "<< outName.str() << endl;
            parser.write(outName.str());

            if (parser.checkTotalLength()) {

                cout << "\nMessage #" << messageCount + 1 << ": length VALID" << endl;
            } else {

                cout << "Message #" << messageCount + 1 << ": length INVALID" << endl;
            }

            ++messageCount;

            // Pozycja w pliku powinna już być ustawiona na następny GRIB przez parser.read()
        }
        cout<<endl;
        cout<<"-------------------------------------------------------------------------"<<endl;
        cout << "Processed " << messageCount << " GRIB messages." << endl;


    } catch (const std::exception& ex) {
        cerr << "Error: " << ex.what() << endl;
        return 1;
    }

    return 0;
}
