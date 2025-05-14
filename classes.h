
#ifndef CLASSES_H
#define CLASSES_H

#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <cmath>

// podstawowa sekcja
class Section {
protected:
    int length;
    std::vector<unsigned char> readBytes(std::ifstream& file, int count);

public:
    virtual void read(std::ifstream& file) = 0;
    virtual void write(std::ofstream& out) const = 0;
    virtual ~Section() = default;
};

// Sekcja 0
class Section0 : public Section {
    int totalLength;
    static constexpr int sectionLength = 8;

public:
    int getTotalLength() const { return totalLength; }
    void read(std::ifstream& file) override;
    void write(std::ofstream& out) const override;
};

// Sekcja 1
class Section1 : public Section {
    bool GDS, BMS;
    int PDSlength;
    unsigned char indicator;
    int pressure;
    int year, month, day, hour, minute;
    int DecimalScale;

public:
    int getLength() const { return PDSlength; }
    int getDecimalScale() const { return DecimalScale; }
    bool hasGDS() const { return GDS; }
    bool hasBMS() const { return BMS; }

    void read(std::ifstream& file) override;
    void write(std::ofstream& out) const override;
};

// Sekcja 2
class Section2 : public Section {
    int GDSlength;
    int nRows;
    float incrementNS;
    std::vector<uint16_t> values;
public:
    struct Coordinate {
        bool isNegative;
        int value;
    };
private:
    Coordinate startLat, startLon, endLat, endLon;

public:
    int getLength() const { return GDSlength; }
    void read(std::ifstream& file) override;
    void write(std::ofstream& out) const override;
};

// Sekcja 3
class Section3 : public Section {
    int length;

public:
    int getLength() const { return length; }
    void read(std::ifstream& file) override;
    void write(std::ofstream& out) const override;
};

// Sekcja 4
class Section4 : public Section {
    int BDSlength;
    int BinaryScale;
    float ReferenceValue;
    int BitsPerValue;
    int number1, number2, number3;

public:
    int getLength() const { return BDSlength; }
    float getReferenceValue() const { return ReferenceValue; }
    int getBinaryScale() const { return BinaryScale; }
    int getNumber1() const { return number1; }
    int getNumber2() const { return number2; }
    int getNumber3() const { return number3; }

    void read(std::ifstream& file) override;
    void write(std::ofstream& out) const override;
};

// Sekcja 5
class Section5 : public Section {
    static constexpr int lengthEOF = 4;

public:
    int getLength() const { return lengthEOF; }
    void read(std::ifstream& file) override;
    void write(std::ofstream& out) const override;
};

// Parser całego pliku
class GribParser {
    Section0 sec0;
    Section1 sec1;
    Section2 sec2;
    Section3 sec3;
    Section4 sec4;
    Section5 sec5;

public:
    void read(std::ifstream& file);
    void write(const std::string& outFile) const;
    bool checkTotalLength() const;

private:
    void writeTemperatures(std::ofstream& out) const;
};

#endif // CLASSES_H
