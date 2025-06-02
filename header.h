#ifndef HEADER_H
#define HEADER_H

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cmath>
#include <sstream>


using namespace std;

class Section {
protected:

    int length;

    vector<unsigned char> readBytes(ifstream& file, int count)
    {
        vector<unsigned char> buffer(count);
        file.read((char*)buffer.data(), count);
        return buffer;
    }

public:

    virtual void read(ifstream& file) = 0;
    virtual void write(ofstream& out) const = 0;
    virtual ~Section() = default;
};

class Section0 : public Section
{
    int totalLength;
    int sectionLength=8;

public:
    int getTotalLength() const {return totalLength;}

    void read(ifstream& file) override {
        char ch;
        string marker;
        do
        {
            marker.clear();
            for (int i = 0; i < 4; ++i)
            {
                file.read(&ch, 1);
                marker.push_back(ch);
            }
            if (file.eof()) throw runtime_error("GRIB not found");
            file.seekg(-3, ios::cur);
        } while (marker != "GRIB");

        file.seekg(3, ios::cur);

        auto bytes = readBytes(file, 3);
        totalLength = (bytes[0] << 16) | (bytes[1] << 8) | bytes[2];
        file.seekg(1, ios::cur);
    }


    void write(ofstream& out) const override
    {
        out << "SECTION 0\n";
        out << "\tmessage length:\t\t" << totalLength <<endl;
        out << "\tsection length:\t\t"<<sectionLength<<endl;
        out<<endl;
    }
};

class Section1 : public Section
{
    bool GDS, BMS;
    int PDSlength;
    unsigned char indicator;
    int pressure;
    int year, month, day, hour, minute;
    int DecimalScale;

public:

    int getLength() const {return PDSlength;}
    int getDecimalScale() const { return DecimalScale; }

    bool hasGDS() const { return GDS; }
    bool hasBMS() const { return BMS; }

    void read(ifstream& file) override
    {
        auto lenBytes = readBytes(file, 3);
        PDSlength = (lenBytes[0] << 16) | (lenBytes[1] << 8) | lenBytes[2];

        file.seekg(-3, ios::cur);
        auto data = readBytes(file, PDSlength);

        // B 10 Indicator of type of level or layer
        indicator = data[9]; // 100-isobaric level

        unsigned char highByte = data[10];
        unsigned char lowByte = data[11];
        pressure = (highByte << 8) | lowByte;


        unsigned char flags = data[7];
        GDS = flags & 0b10000000;
        BMS = flags & 0b01000000;

        year = 2000+abs((int)data[12]);
        month = abs((int)data[13]);
        day = abs((int)data[14]);

        hour = abs((int)data[15]);
        minute = abs((int)data[16]);

        DecimalScale = (data[26] << 8) | data[27];
    }


    void write(ofstream& out) const override
    {
        out << "SECTION 1"<<endl;
        out << "\tPDS length:\t\t\t" << PDSlength << endl;
        out << "\tGDS:\t" << (GDS ? "YES" : "NO") << "  |  BMS:\t" << (BMS ? "YES" : "NO") << endl;
        out << "\tdate:\t\t\t\t" << day << "." << month << "." << year <<"\t"<<hour<<":"<<minute<< endl;
        out << "\tdecimal scale:\t\t" << DecimalScale <<endl;
        out << "\tindicator:\t\t\t" <<  static_cast<int>(indicator) <<endl;
        out<< "\tpressure (hPa):\t\t"<<pressure <<endl;
        out<<endl;
    }
};

class Section2 : public Section
{
    int GDSlength;
    int tab[5];
    int nRows;
    float incrementNS;
    vector<uint16_t> values;
    int Number1,Number2,Number3;

    public:
    Section2() {};
    ~Section2() = default;

    struct Coordinate {
        bool isNegative; // 1 = S/W, 0 = N/E
        int value;       // wartość w tysięcznych stopniach (czyli np  52485 = 52.485°)
    };

    Coordinate startLat, startLon, endLat, endLon;

    int getLength() const
    {
        return GDSlength;
    }

    Coordinate parseCoordinate(uint8_t byte1, uint8_t byte2, uint8_t byte3)
    {
        uint32_t raw = (byte1 << 16) | (byte2 << 8) | byte3;
        bool isNegative = (raw & 0x800000) != 0;     // najstarszy bit
        int value = raw & 0x7FFFFF;                  // tylko 23 młodsze bity
        return { isNegative, value };
    };

    float ByToFloat(string bytes)
    {
        uint32_t val=0;
        for (int i =0; i<4;++i)
        {
            val=(val<<8)|static_cast<unsigned char>(bytes[i]);
        }
        float result;
        memcpy(&result,&val,sizeof(float));
        return result;
    };

    void read(ifstream& file) override
    {
        auto lenBytes = readBytes(file, 3);
        GDSlength = (lenBytes[0] << 16) | (lenBytes[1] << 8) | lenBytes[2];

        file.seekg(-3, ios::cur);
        auto data = readBytes(file, GDSlength);

        nRows = (data[9]) | data[10];

        startLat = parseCoordinate(data[10], data[11], data[12]);
        startLon = parseCoordinate(data[13], data[14], data[15]);
        endLat   = parseCoordinate(data[17], data[18], data[19]);
        endLon   = parseCoordinate(data[20], data[21], data[22]);

        incrementNS = (static_cast<float>((static_cast<unsigned char>(data[25]) << 8) | static_cast<unsigned char>(data[26])))/1000.0f;

        // Odczyt 3 liczb 10-bitowych z bajtów 29-33
        Number1 = (data[29] << 2) | (data[30] >> 6);  // Pierwsza liczba z bajtów 29-30
        Number2 = ((data[30] & 0x3F) << 4) | (data[31] >> 4); // Druga liczba z bajtów 30-31
        Number3 = ((data[31] & 0x0F) << 6) | (data[32] >> 2); // Trzecia liczba z bajtów 31-32

        // Odczyt wartości 2-bajtowych od bajtu 33 do 178
        values.clear();
        for (int i = 32; i < 178; i += 2)
        {
            uint16_t value = (static_cast<unsigned char>(data[i]) << 8) | static_cast<unsigned char>(data[i + 1]);
            values.push_back(value);
        }
    }


    void write(ofstream& out) const override
    {
        out << "SECTION 2"<<endl;
        out << "\tGDS length:\t" << GDSlength << endl;

        out << "\tstartLatitude:\t\t" << (startLat.isNegative ? "SOUTH " : "NORTH ")<< startLat.value / 1000.0 << "°" << endl;
        out << "\tstartLongitude:\t\t" << (startLon.isNegative ? "WEST " : "EAST ")<< startLon.value / 1000.0 << "°" << endl;
        out << "\tendLatitude:\t\t" << (endLat.isNegative ? "SOUTH " : "NORTH ")<< endLat.value / 1000.0 << "°" << endl;
        out << "\tendLongitude:\t\t" << (endLon.isNegative ? "WEST " : "EAST ")<< endLon.value / 1000.0 << "°" << endl;

        out<< "\tnumber of rows:\t\t"<<nRows<<endl;
        out << "\tincrement N-S:\t\t" << incrementNS << endl;
        out << "\tval 29-32:\t\t\t"  << Number1 << " "<< Number2 << " "<< Number3 << endl;

        out<< "\tval for "<<nRows<<" points: "<<endl;
        for (size_t i = 0; i < values.size(); ++i)
        {
            out << "\t\trow " << i + 1 << ":\t\t\t" << values[i] << endl;
        }
        out << endl;
    };
};

class Section3 : public Section
{
private:
int length;

public:
    Section3(){};
    ~Section3() = default;

    int getLength() const
    {
        return length;
    }


    void read(ifstream& file) override
    {
        auto lenBytes = readBytes(file, 3);
        length = (lenBytes[0] << 16) | (lenBytes[1] << 8) | lenBytes[2];

        file.seekg(-3, ios::cur);
        auto data = readBytes(file, length);
    };

    void write(ofstream& out) const override
    {
        out << "SECTION 3"<<endl;
        out << "\tlength:\t" << length << endl;
        out<<endl;
    }
};



class Section4 : public Section
{
private:
    int BDSlength;
    int BinaryScale;
    int ReferenceValue;
    int BitsPerValue;
   int number1,number2,number3;

public:
    int getLength() const {return BDSlength;}
    int getReferenceValue() const { return ReferenceValue; }
    int getBinaryScale() const { return BinaryScale; }
    int getNumber1() const { return number1; }
    int getNumber2() const { return number2; }
    int getNumber3() const { return number3; }

    float ByToFloat(string bytes)
    {
        uint32_t val=0;
        for (int i =0; i<4;++i)
            {
            val=(val<<8)|static_cast<unsigned char>(bytes[i]);
        }
        float result;\
        memcpy(&result,&val,sizeof(float));
        return result;
    }

    void read(ifstream& file) override
    {
        auto lenBytes = readBytes(file, 3);
        BDSlength = (lenBytes[0] << 16) | (lenBytes[1] << 8) | lenBytes[2];

        file.seekg(-3, ios::cur);
        auto data = readBytes(file, BDSlength);
        BinaryScale = (data[4] << 8) | data[5];
        //ReferenceValue = (data[6] << 24) | (data[7] << 16) | (data[8] << 8) | data[9];  // Bajty 7-10 <- nie dziala
        string refBytes;
        refBytes += static_cast<char>(data[6]);
        refBytes += static_cast<char>(data[7]);
        refBytes += static_cast<char>(data[8]);
        refBytes += static_cast<char>(data[9]);
        ReferenceValue = ByToFloat(refBytes);

        BitsPerValue = data[10];

        // Odczytujemy cztery bajty od 12 jako 32-bitową liczbę (long)
        uint32_t combinedValue = (data[11] << 24) | (data[12] << 16) | (data[13] << 8) | data[14];

        // Wycinamy trzy liczby po 10 bitów
        number1 = (combinedValue >> 22) & 0x3FF;  // Pierwsze 10 bitów (bity 31-22)
        number2 = (combinedValue >> 12) & 0x3FF;  // Drugie 10 bitów (bity 21-12)
        number3 = (combinedValue >> 2) & 0x3FF;   // Trzecie 10 bitów (bity 11-2)
    }


    void write(ofstream& out) const override
    {
        out << "SECTION 4"<<endl;
        out << "\tBDS length:\t\t\t" << BDSlength << endl;
        out << "\tbinary scale:\t\t" << BinaryScale<< endl;
        out << "\treference value:\t" << ReferenceValue<< endl;
        out << "\tbits per value:\t\t" << BitsPerValue <<endl;

        //zaczynając od Bajtu 12-go proszę odczytać kilka kolejnych wartości po 10 bitów każda.
        out<<"\tVal from 12-15 byte:\t "<<endl;
        out<<"\t\t\t\t\t\t"<<number1<<"\t\t"<<number2<<"\t\t"<<number3<<"\t\t"<<endl;
        out << endl;
    }
};

class Section5 : public Section
{
public:
    int lengthEOF=4;
    int getLength() const{return lengthEOF;}

    void read(ifstream& file) override
    {
        char ch;
        string marker;
        do
        {
            marker.clear();
            for (int i = 0; i < 4; ++i)
            {
                file.read(&ch, 1);
                marker.push_back(ch);
            }
            if (file.eof()) throw runtime_error("EOF not found (7777)");
            file.seekg(-3, ios::cur);
        } while (marker != "7777");
        file.seekg(3, ios::cur); // doskocz do końca
    }

    void write(ofstream& out) const override
    {
        out << "SECTION 5\n";
        out << "\tlength:\t\t\t\t"<<lengthEOF<<endl;
        out<<endl;
    }
};

class GribParser
{
    Section0 sec0;
    Section1 sec1;
    Section2 sec2;
    Section3 sec3;
    Section4 sec4;
    Section5 sec5;

public:
    GribParser()
    {
        Section0 sec0;
        Section1 sec1;
        Section2 sec2;
        Section3 sec3;
        Section4 sec4;
        Section5 sec5;
    }
    ~GribParser() =default;

    void writeTemperatures(ofstream& out) const
    {
        float R = sec4.getReferenceValue();
        int E = sec4.getBinaryScale();
        int D = sec1.getDecimalScale();

        auto calculate = [=](int x) -> float
        {
            return (R + (x * pow(2, E))) / pow(10, D);
        };

        out << "TEMPERATURES" << endl;
        out << "\tTemperature 1:\t" << calculate(sec4.getNumber1()) << " °C" << endl;
        out << "\tTemperature 2:\t" << calculate(sec4.getNumber2()) << " °C" << endl;
        out << "\tTemperature 3:\t" << calculate(sec4.getNumber3()) << " °C" << endl;
        out << endl;
    }

    void read(ifstream& file)
    {
        sec0.read(file);
        sec1.read(file);
        if (sec1.hasGDS()) sec2.read(file);
        if (sec1.hasBMS()) sec3.read(file);
        sec4.read(file);
        sec5.read(file);
    }


    void write(const string& outFile) const
    {
        ofstream out(outFile);
        sec0.write(out);
        sec1.write(out);
        if ( sec1.hasBMS()) sec3.write(out);
        if (sec1.hasGDS()) sec2.write(out);
        sec4.write(out);
        sec5.write(out);
        writeTemperatures(out);
    }

    bool checkTotalLength() const
    {
        int sum = 8; // Section0 zawsze ma 8 bajtów
        sum += sec1.getLength();
        if (sec1.hasGDS()) sum += sec2.getLength();
        if (sec1.hasBMS()) sum += sec3.getLength();
        sum += sec4.getLength();
        sum += sec5.getLength(); // to będzie 4

        return sum == sec0.getTotalLength();
    }
};

#endif
