#include <string>
#include <vector>

#ifndef TESTING_UTILITY_H
#define TESTING_UTILITY_H

std::string WcharToString(std::wstring wch);

char* IntToBytes( int i, bool flip = true );

void Read4Bytes( unsigned char *byte, std::vector<char> buf, int pos );

void Read4BytesReversed( unsigned char *chunk, std::vector<char> buf, int pos );

int GetIntFromChunk( unsigned char *byte );

std::string HexToASCII(std::string);

std::wstring ToString( int i );
std::wstring ToString( float f );

std::wstring ReadUnicode( std::vector<char> chunk, int pos, bool swapEndian = false );

#endif //TESTING_UTILITY_H
