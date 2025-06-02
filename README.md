# GRIB1 Decoder in C++

A C++ decoder for **GRIB Edition 1** messages based on the **WMO FM 92 GRIB** specification, using template 388. This decoder parses and extracts meteorological and oceanographic data from GRIB1 binary files, which are widely used in numerical weather prediction and climate modeling.

---

## Features

- Parses all relevant sections of a GRIB1 message (0, 1, 2, 4, 5)
- Supports verification of message length integrity
- Extracts and decodes meteorological parameters such as temperature
- Handles GDS and BMS presence flags, decimal and binary scaling
- Outputs detailed, human-readable data reports per message
- Designed for educational and research purposes

---

## Output Description

The decoder generates a detailed, structured textual report for each processed GRIB message. The output is divided into sections corresponding to the GRIB1 message structure:

- **SECTION 0**  
  Displays the total length of the message and the length of section 0.

- **SECTION 1 (Product Definition Section - PDS)**  
  Provides metadata including:  
  - Length of the PDS section  
  - Presence flags indicating if the Grid Description Section (GDS) and Bit-map Section (BMS) are included  
  - Message timestamp (date and time)  
  - Decimal scale factor used in data decoding  
  - Type indicator of the vertical level or layer  
  - Pressure level in hectopascals (hPa), if applicable

- **SECTION 2 (Grid Description Section - GDS)**  
  Details about the spatial grid, including:  
  - Length of the GDS section  
  - Geographic coordinates marking the start and end points of the data octant (latitude and longitude)  
  - Number of rows in the data grid  
  - Increment step in the North-South direction  
  - Reserved fields  
  - Number of data points per grid row

- **SECTION 4 (Binary Data Section - BDS)**  
  Contains parameters for binary data decoding:  
  - Length of the BDS section  
  - Binary scale factor  
  - Reference (minimum) data value  
  - Number of bits used per encoded data value  
  - Sample decoded raw values from the data stream

- **SECTION 5**  
  Usually a short section indicating length (commonly 4 bytes).

- **Decoded Meteorological Data**  
  Provides human-readable physical values extracted from the binary data, for example temperature values in degrees Celsius.

Each message is saved as a separate text file (e.g., `Data_1.txt`), facilitating further analysis and verification.

---

### Prerequisites

- A C++17 (or later) compatible compiler
- Standard C++ libraries

### Building

Compile the decoder using your preferred compiler. For example, using `g++`:

## Author

Created for educational purposes by Avui.
