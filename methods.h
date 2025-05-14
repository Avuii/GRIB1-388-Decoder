// methods.cpp
#include "classes.h"
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <cstring>
#include <cmath>

// --- Section::readBytes ---
std::vector<unsigned char> Section::readBytes(std::ifstream& file, int count) {
    std::vector<unsigned char> buffer(count);
    file.read(reinterpret_cast<char*>(buffer.data()), count);
    return buffer;
}

// --- Section0 ---
void Section0::read(std::ifstream& file) {
    char ch; std::string marker;
    do {
        marker.clear();
        for (int i = 0; i < 4; ++i) { file.read(&ch,1); marker.push_back(ch); }
        if (file.eof()) throw std::runtime_error("GRIB not found");
        file.seekg(-3, std::ios::cur);
    } while (marker != "GRIB");
    file.seekg(3, std::ios::cur);
    auto b = readBytes(file,3);
    totalLength = (b[0]<<16)|(b[1]<<8)|b[2];
    file.seekg(1, std::ios::cur);
}
void Section0::write(std::ofstream& out) const {
    out << "SECTION 0\n"
        << "\tmessage length:\t" << totalLength << "\n"
        << "\tsection length:\t" << sectionLength << "\n\n";
}

// --- Section1 ---
void Section1::read(std::ifstream& file) {
    auto lenB = readBytes(file,3);
    PDSlength = (lenB[0]<<16)|(lenB[1]<<8)|lenB[2];
    file.seekg(-3, std::ios::cur);
    auto d = readBytes(file, PDSlength);

    unsigned char flags = d[7];
    GDS = flags & 0x80;  BMS = flags & 0x40;
    indicator = d[9];
    pressure = (d[10]<<8)|d[11];

    year   = 2000 + d[12];
    month  = d[13];
    day    = d[14];
    hour   = d[15];
    minute = d[16];
    DecimalScale = (d[26]<<8)|d[27];
}
void Section1::write(std::ofstream& out) const {
    out << "SECTION 1\n"
        << "\tPDS length:\t" << PDSlength << "\n"
        << "\tGDS: " << (GDS?"YES":"NO") << " | BMS: " << (BMS?"YES":"NO") << "\n"
        << "\tdate: " << day<<"."<<month<<"."<<year<<" "<<hour<<":"<<minute<<"\n"
        << "\tdecimal scale:\t" << DecimalScale << "\n"
        << "\tindicator:\t" << int(indicator) << "\n"
        << "\tpressure (hPa):\t" << pressure << "\n\n";
}

// --- Section2 ---
static auto parseCoord = [](uint8_t b1, uint8_t b2, uint8_t b3) {
    uint32_t raw = (b1<<16)|(b2<<8)|b3;
    return Section2::Coordinate{ (raw&0x800000)!=0, int(raw&0x7FFFFF) };
};
void Section2::read(std::ifstream& file) {
    auto lenB = readBytes(file,3);
    GDSlength = (lenB[0]<<16)|(lenB[1]<<8)|lenB[2];
    file.seekg(-3,std::ios::cur);
    auto d = readBytes(file,GDSlength);

    nRows       = (d[9]<<8)|d[10];
    startLat    = parseCoord(d[11],d[12],d[13]);
    startLon    = parseCoord(d[14],d[15],d[16]);
    endLat      = parseCoord(d[17],d[18],d[19]);
    endLon      = parseCoord(d[20],d[21],d[22]);
    incrementNS = ((d[25]<<8)|d[26])/1000.0f;

    values.clear();
    for (int i = 33; i+1 < GDSlength; i+=2)
        values.push_back((d[i]<<8)|d[i+1]);
}
void Section2::write(std::ofstream& out) const {
    auto fmt = [&](const Coordinate& C){
        std::ostringstream s;
        s << (C.isNegative?"-":"") << C.value/1000.0 << "°";
        return s.str();
    };
    out << "SECTION 2\n"
        << "\tGDS length:\t"   << GDSlength   << "\n"
        << "\tstartLat/Lon:\t" << fmt(startLat)<<"/"<<fmt(startLon)<<"\n"
        << "\tendLat/Lon:\t"   << fmt(endLat)  <<"/"<<fmt(endLon)  << "\n"
        << "\tnRows:\t"        << nRows       << "\n"
        << "\tinc N-S:\t"      << incrementNS << "\n";
    for (int i=0; i<values.size(); ++i)
        out << "\t  row "<<i+1<<": "<<values[i]<<"\n";
    out<<"\n";
}

// --- Section3 ---
void Section3::read(std::ifstream& file) {
    auto lenB = readBytes(file,3);
    length = (lenB[0]<<16)|(lenB[1]<<8)|lenB[2];
    file.seekg(-3,std::ios::cur);
    file.seekg(length,std::ios::cur);
}
void Section3::write(std::ofstream& out) const {
    out << "SECTION 3\n\tlength: " << length << "\n\n";
}

// --- Section4 ---
static float bytesToFloat(const unsigned char* p){
    uint32_t v = (p[0]<<24)|(p[1]<<16)|(p[2]<<8)|p[3];
    float f; std::memcpy(&f,&v,sizeof f); return f;
}
void Section4::read(std::ifstream& file) {
    auto lenB = readBytes(file,3);
    BDSlength = (lenB[0]<<16)|(lenB[1]<<8)|lenB[2];
    file.seekg(-3,std::ios::cur);
    auto d = readBytes(file,BDSlength);

    BinaryScale    = (d[4]<<8)|d[5];
    ReferenceValue = bytesToFloat(&d[6]);
    BitsPerValue   = d[10];
    uint32_t comb = (d[11]<<24)|(d[12]<<16)|(d[13]<<8)|d[14];
    number1 = (comb>>22)&0x3FF;
    number2 = (comb>>12)&0x3FF;
    number3 = (comb>>2)&0x3FF;
}
void Section4::write(std::ofstream& out) const {
    out << "SECTION 4\n"
        << "\tBDS length:\t"      << BDSlength      << "\n"
        << "\tbinary scale:\t"    << BinaryScale    << "\n"
        << "\treference value:\t" << ReferenceValue << "\n"
        << "\tbits per value:\t"  << BitsPerValue   << "\n"
        << "\tvals1-3:\t"         << number1<<" "<<number2<<" "<<number3<<"\n\n";
}

// --- Section5 ---
void Section5::read(std::ifstream& file) {
    char ch; std::string m;
    do {
        m.clear();
        for (int i=0;i<4;++i){ file.read(&ch,1); m.push_back(ch); }
        if (file.eof()) throw std::runtime_error("EOF not found");
        file.seekg(-3,std::ios::cur);
    } while (m!="7777");
    file.seekg(3,std::ios::cur);
}
void Section5::write(std::ofstream& out) const {
    out << "SECTION 5\n\tlength: " << lengthEOF << "\n\n";
}

// --- GribParser ---
void GribParser::read(std::ifstream& f) {
    sec0.read(f);
    sec1.read(f);
    if (sec1.hasGDS()) sec2.read(f);
    if (sec1.hasBMS()) sec3.read(f);
    sec4.read(f);
    sec5.read(f);
}
void GribParser::writeTemperatures(std::ofstream& o) const {
    auto R = sec4.getReferenceValue();
    auto E = sec4.getBinaryScale();
    auto D = sec1.getDecimalScale();
    auto calc = [&](int x){ return (R + x * std::pow(2, E)) / std::pow(10, D); };
    o << "TEMPS\n"
      << "\tT1:\t" << calc(sec4.getNumber1()) << "°C\n"
      << "\tT2:\t" << calc(sec4.getNumber2()) << "°C\n"
      << "\tT3:\t" << calc(sec4.getNumber3()) << "°C\n\n";
}
void GribParser::write(const std::string& fn) const {
    std::ofstream o(fn);
    sec0.write(o);
    sec1.write(o);
    if (sec1.hasBMS()) sec3.write(o);
    if (sec1.hasGDS()) sec2.write(o);
    sec4.write(o);
    sec5.write(o);
    writeTemperatures(o);
}
bool GribParser::checkTotalLength() const {
    int sum = 8 + sec1.getLength()
            + (sec1.hasGDS()?sec2.getLength():0)
            + (sec1.hasBMS()?sec3.getLength():0)
            + sec4.getLength()
            + sec5.getLength();
    return sum == sec0.getTotalLength();
}
