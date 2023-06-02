#ifndef _H264_H
#define _H264_H

/*
enum NALU_Type
{
    UNUSED = 0,
    SLICE = 1,      // Slice Layer Without Partitioning
    SLICE_DPA = 2,  // Slice Data Partition A Layer
    SLICE_DPB = 3,  // Slice Data Partition B Layer
    SLICE_DPC = 4,  // Slice Data Partition C Layer
    SLICE_IDR = 5,  // Slice Layer Without Partitioning
    SEI = 6,        // Supplemental Enhancement Information
    SPS = 7,        // Sequence Parameter Set
    PPS = 8,        // Picture Parameter Set
    AUD = 9,        // Access Unit Delimiter
    EO_SEQ = 10,    // End Of Sequence
    EO_STREAM = 11, // End Of Stream
    FILLER = 12,    // Filler Data
};

enum NALU_Priority
{
    DISPOSABLE = 0,
    LOW = 1,
    MIDDLE = 2,
    HIGH = 3,
};
*/

// bit order is "Little Endian".
struct NALU_Header
{
    unsigned char type : 5;
    unsigned char NRI : 2; // NAL Reference IDC
    unsigned char F : 1;   // Forbidden Zero Bit
};

int parse_h264(const char *);

#endif