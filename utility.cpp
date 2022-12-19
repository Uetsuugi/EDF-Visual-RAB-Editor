#include "utility.h"
#include <codecvt>
#include <locale>

std::string WcharToString(std::wstring wch)
{
    using convert_type = std::codecvt_utf8<wchar_t>; std::wstring_convert<convert_type, wchar_t> converter;
    return converter.to_bytes(wch);
}

char* IntToBytes( int i, bool flip )
{
    char *bytes = (char*)malloc( sizeof( char ) * 4 );
    unsigned long n = i;

    if( !flip )
    {
        bytes[0] = ( n >> 24 ) & 0xFF;
        bytes[1] = ( n >> 16 ) & 0xFF;
        bytes[2] = ( n >> 8 ) & 0xFF;
        bytes[3] = n & 0xFF;
    }
    else
    {
        bytes[3] = ( n >> 24 ) & 0xFF;
        bytes[2] = ( n >> 16 ) & 0xFF;
        bytes[1] = ( n >> 8 ) & 0xFF;
        bytes[0] = n & 0xFF;
    }

    return bytes;
}

std::string HexToASCII(std::string hex)
{
    std::string ascii = "";
    for (size_t i = 0; i < hex.length(); i += 2)
    {
        std::string part = hex.substr(i, 2);

        char ch = stoul(part, nullptr, 16);

        ascii += ch;
    }
    return ascii;
}

void Read4Bytes( unsigned char *byte, std::vector<char> buf, int pos )
{
    byte[3] = buf[ pos ];
    byte[2] = buf[ pos + 1 ];
    byte[1] = buf[ pos + 2 ];
    byte[0] = buf[ pos + 3 ];
}

void Read4BytesReversed( unsigned char *chunk, std::vector<char> buf, int pos )
{
    chunk[0] = buf[ pos ];
    chunk[1] = buf[ pos + 1 ];
    chunk[2] = buf[ pos + 2 ];
    chunk[3] = buf[ pos + 3 ];
}

int GetIntFromChunk( unsigned char *byte )
{
    int num = 0;
    for(int i = 0; i < 4; i++)
    {
        num <<= 8;
        num |= byte[i];
    }

    return num;
}

std::wstring ToString( int i )
{
    return std::to_wstring( (uint64_t) i );
}
std::wstring ToString( float f )
{
    return std::to_wstring( (long double) f );
}

std::wstring ReadUnicode( std::vector<char> chunk, int pos, bool swapEndian )
{
    if( pos > chunk.size( ) )
        return L"";

    unsigned int bufPos = pos;

    std::vector< unsigned char > bytes;

    int zeroCount = 0;

    //Repeat until EOF, or otherwise broken
    while( bufPos < chunk.size() )
    {
        if( swapEndian )
        {
            if( bufPos % 2 )
            {
                bytes.push_back( chunk[bufPos] );
                bytes.push_back( chunk[bufPos-1] );
            }
        }
        else
            bytes.push_back( chunk[bufPos] );

        if( chunk[bufPos] == 0x00 )
        {
            zeroCount++;
            if( zeroCount == 2 )
                break;
        }
        else
            zeroCount = 0;
        bufPos++;
    }

    std::wstring wstr;

    for(int j{}; j < j+1; ++j)
    {
        if((bytes[j] != 0))
        {
            wstr += bytes[j];
            if(bytes[j+2] == 0)
            {
                //std::wcout << "<[_END_OF_FILENAME_]>" << L"\n";
            }
            ++j;
        }
        else
            break;
    }


    return wstr;
}