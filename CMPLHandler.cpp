#include "CMPLHandler.h"
#include <iostream>
#include "utility.h"

//Takes a vector<char> that can be used for either decompression or compression
//of files in the CMPL format. Which individually can be inserted directly into .rab
//files, however require that you maintain positioning of the file in the archive
//so that we can properly handle where data starts etc (at least I think)
CMPLHandler::CMPLHandler( std::vector< char > inFile, bool useFakeCompression )
{
    data = std::move(inFile);
    m_useFakeCompression = useFakeCompression;
}

std::vector<char> CMPLHandler::Decompress()
{
    //Check header:
    if( data[0] != 'C' && data[1] != 'M' && data[2] != 'P' && data[3] != 'L' )
    {
        std::wcout << L"FILE IS NOT CMPL COMPRESSED!\n";
        return data;
    }
    else
        std::wcout << L"BEGINNING DECOMPRESSION\n";

    //Variables
    uint8_t bitbuf;
    uint8_t bitbufcnt;

    std::vector< char > out;

    char mainbuf[4096];

    int bufPos;

    unsigned char seg[4];
    Read4BytesReversed( seg, data, 4 );
    int desiredSize = GetIntFromChunk( seg );

    int streamPos = 8;

    //Init buffer:
    for( int i = 0; i < 4078; ++i )
        mainbuf[i] = 0;
    bitbuf = 0;
    bitbufcnt = 0;
    bufPos = 4078;

    //Loop
    while( streamPos < data.size( ) )
    {
        //If bitbuf empty
        if( bitbufcnt == 0 )
        {
            //Read and store in buffer
            bitbuf = data[streamPos];
            bitbufcnt = 8;

            ++streamPos;
        }

        //if first bit is one, copy byte from input
        if( bitbuf & 0x1 > 0 )
        {
            out.push_back( data[streamPos] );
            mainbuf[bufPos] = data[streamPos];
            ++bufPos;
            if( bufPos >= 4096 )
                bufPos = 0;
            ++streamPos;
        }
        else //Copy bytes from buffer
        {
            uint8_t chunk[2];
            chunk[1] = data[streamPos + 1];
            chunk[0] = data[streamPos];

            streamPos += 2;

            uint16_t val = ( chunk[0] << 8 ) | chunk[1];
            int copyLen = ( val & 0xf ) + 3;
            int copyPos = val >> 4;

            for( int i = 0; i < copyLen; ++i )
            {
                unsigned char byte = mainbuf[copyPos];
                out.push_back( byte );
                mainbuf[bufPos] = byte;

                ++bufPos;
                if( bufPos >= 4096 )
                    bufPos = 0;

                ++copyPos;
                if( copyPos >= 4096 )
                    copyPos = 0;
            }
        }

        --bitbufcnt;
        bitbuf >>= 1;
    }

    if( out.size( ) == desiredSize )
    {
        std::cout << "FILE SIZE MATCH! " + std::to_string( (uint64_t)desiredSize ) + " bytes expected, got " + std::to_string( (int)out.size( ) ) +  " DECOMPRESSION SUCCESSFUL!\n";
    }
    else
        std::cout << "FILE SIZE MISMATCH! " + std::to_string( (uint64_t)desiredSize ) + " bytes expected, got " + std::to_string( (int)out.size( ) ) + " DECOMPRESSION FAILED!\n";

    return out;
}

std::vector<char> CMPLHandler::Compress( ) {
    //Declare output
    std::vector<char> out;

    //Fill header:
    out.push_back('C');
    out.push_back('M');
    out.push_back('P');
    out.push_back('L');

    //File size
    char *sizeBytes = IntToBytes(data.size(), false);

    out.push_back(sizeBytes[0]);
    out.push_back(sizeBytes[1]);
    out.push_back(sizeBytes[2]);
    out.push_back(sizeBytes[3]);

    free(sizeBytes);

    //Begin compression:

    //Use fake compression, faster compile times but extremly ineffecient
    if (m_useFakeCompression) {
        std::wcout << L"File using SIMPLE/FAKE Compression.\n";

        //Prepare to format the data into something that the game's CMPL decompressor will read, this data will be trash, not really compressed and is horrible, but just do it anyway.
        int count = 0;
        while (count < data.size()) {
            //Push "Copy 8 bits directly" command to reader
            out.push_back(0b11111111);

            for (int i = 0; i < 8; ++i) {
                if (count >= data.size())
                    break;
                out.push_back(data[count]);
                ++count;
            }
        }
    } else {
        //CMPL Compression Algorithm by BlueAmulet
        std::vector<uint8_t> out;
        std::vector<uint8_t> temp;
        int16_t mainbuf[4096]; //maybe mainbuf = 4096
        uint16_t bufPos = 4078;
        size_t streamPos = 0;
        uint8_t bits = 0;

        for (size_t i = 0; i < 4096; i++) {
            if (i < bufPos) {
                mainbuf[i] = 0;
            } else {
                // Mark end of buffer as uninitialized
                mainbuf[i] = -1;
            }
        }

        size_t dataSize = data.size();

        while (streamPos < dataSize) {
            bits = 0;
            for (size_t i = 0; i < 8; i++) {
                if (streamPos >= dataSize) {
                    break;
                }
                size_t bestPos = 0;
                size_t bestLen = 0;
                // Try to find match in buffer
                // TODO: Properly support repeating data
                for (size_t jo = 0; jo < sizeof(mainbuf); jo++) {
                    uint16_t j = (bufPos - jo) & 0xFFF;
                    if (mainbuf[j] == (int16_t) (uint16_t) data[streamPos]) {
                        size_t matchLen = 0;
                        for (size_t k = 0; k < 18; k++) {
                            if ((streamPos + k) < dataSize && ((j + k) & 0xFFF) != bufPos &&
                                mainbuf[(j + k) & 0xFFF] == (int16_t) (uint16_t) data[streamPos + k]) {
                                matchLen = k + 1;
                            } else {
                                break;
                            }
                        }
                        if (matchLen > bestLen) {
                            bestLen = matchLen;
                            bestPos = j;
                        }
                    }
                }
                // Repeating byte check
                if (mainbuf[(bufPos - 1) & 0xFFF] == (int16_t) (uint16_t) data[streamPos]) {
                    size_t matchLen = 0;
                    for (size_t k = 0; k < 18; k++) {
                        if ((streamPos + k) < dataSize &&
                            mainbuf[(bufPos - 1) & 0xFFF] == (int16_t) (uint16_t) data[streamPos + k]) {
                            matchLen = k + 1;
                        } else {
                            break;
                        }
                    }
                    if (matchLen > bestLen) {
                        bestLen = matchLen;
                        bestPos = (bufPos - 1) & 0xFFF;
                    }
                }
                // Is copy viable?
                if (bestLen >= 3) {
                    // Write copy data
                    uint16_t copyVal = bestLen - 3;
                    copyVal |= bestPos << 4;
                    temp.push_back(copyVal >> 8);
                    temp.push_back(copyVal & 0xFF);
                    for (size_t j = 0; j < bestLen; j++) {
                        mainbuf[bufPos] = data[streamPos];
                        bufPos = (bufPos + 1) & 0xFFF;
                        streamPos++;
                    }
                } else {
                    // Copy from input
                    temp.push_back(data[streamPos]);
                    mainbuf[bufPos] = data[streamPos];
                    bufPos = (bufPos + 1) & 0xFFF;
                    streamPos++;
                    bits |= 1 << i;
                }
            }
            out.push_back(bits);
            out.insert(out.end(), temp.begin(), temp.end());
            temp.clear();
        }
    }

    return out;
}