#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <sstream>
#include <cstring>
#include "RAB.h"
#include "utility.h"
#include "CMPLHandler.h"

RAB::Ptr RAB::instance{nullptr};
RAB::Ptr RAB::getRAB()
{
    if(!instance)
        instance = std::make_shared<RAB>();
    return instance;
}

void RAB::Read(const std::string& file)
{
    m_file = file;
    m_ifs = std::make_shared<std::ifstream>(m_file, std::ios::binary | std::ios::ate);
    m_size = m_ifs->tellg();
    m_ifs->seekg(0, std::ios::beg);

    std::cout << "RAB Archive size: " << m_size << "\n";

    m_buf.resize(m_size);

    if(m_size == -1)
        std::cout << "EMPTY" << std::endl;

    Scan();
}

void RAB::Insert(const std::string &file, int pos, const std::string &filename, int nametablepos)
{
    m_ifs->seekg(0, std::ios::beg); //reset seek position

    ///Raw image file
    std::ifstream ifs(file, std::ios::binary | std::ios::ate);
    std::streamsize size = ifs.tellg();
    ifs.seekg(0, std::ios::beg);
    std::vector<char> imageBuf(size);
    if(ifs.read(imageBuf.data(), size))
    {
        CMPLHandler CMPL = CMPLHandler(imageBuf, true);
        std::vector<char> compressedFile = CMPL.Compress();

        std::cout << filename << std::endl;

        if(m_ifs->read(m_buf.data(), m_size))
        {
            const char* values = filename.c_str();
            const char* end = values + strlen(values);
            m_buf.insert(m_buf.begin()+nametablepos, values, end);
            int cs = nametablepos + filename.size();
            for(int i{nametablepos+1}; i < cs; i++)
            {
                m_buf[i] = '\u0000';
                i++;
            }
            m_buf[cs-2] = '\u0000';

            int contentSize = pos + compressedFile.size();

            for(int i{pos}; i < contentSize; ++i)
            {
                m_buf.insert(m_buf.begin()+i, imageBuf[i]);
                i++;
                //std::wcout << m_buf[i] << std::endl;
            }
        }
        m_buf[20] = 8; //amount of files
        uint16_t uv = 821;
        uint8_t msb = (uv & 0xFF00U) >> 8U;
        uint8_t lsb = (uv & 0x00FFU);
        ///m_buf[8] = msb + lsb; //content start*/
        auto cc = lsb << 8 | msb;
        std::stringstream ss;
        ss << std::hex << cc;
        std::string ins = ss.str();
        std::string newv;
        for(int i{0}; i < ins.size(); i=i+2)
        {
            int bufpos = 0;
            std::stringstream mystring;
            mystring << ins[i] << ins[i+1];
            std::cout << "part is > " << mystring.str() << "\n";
            std::string yeet = mystring.str();
            std::string test = HexToASCII(yeet);
            std::cout << "byte is > " <<  test << "\n";
            newv += test;
            m_buf[8+bufpos] = test[0];
            std::cout << "test> " << test[0] << "' '" << test[1] << "\n";
            bufpos++;
        }

        /*const char* values = newv.c_str();
        const char* end = values + strlen(values);
        m_buf.insert(m_buf.begin()+8, values, end);*/
        std::cout << "REAL:: " << std::hex << cc;
        using convert_type = std::codecvt_utf8<wchar_t>; std::wstring_convert<convert_type, wchar_t> converter;
        std::string converted_str = converter.to_bytes( L"/home/ruko/CLionProjects/EDF-Visual-RAB-Editor/cmake-build-release/new_rab.rab" );
        std::ofstream ofile = std::ofstream(converted_str, std::ios::binary | std::ios::out | std::ios::ate );
        for(char j : m_buf)
            ofile << j;
        ofile.close();

    }
}

void RAB::Scan()
{
    m_ifs->seekg(0, std::ios::beg); //reset seek position
    folders.clear();
    files.clear();
    FILE.clear();
    FOLDER.clear();
    if(m_ifs->read( m_buf.data(), m_size)) {
        unsigned char seg[4];
        int position;

        Read4Bytes( seg, m_buf, 0x8 );

        m_dataStartOfs = GetIntFromChunk( seg );
        std::wcout << L"Data starting at " + ToString( m_dataStartOfs ) + L"\n";

        Read4Bytes( seg, m_buf, 0x14 );
        numFiles = GetIntFromChunk( seg );
        std::wcout << L"Number of archived files: " + ToString( numFiles ) + L"\n";

        Read4Bytes( seg, m_buf, 0x1c );
        fileTreeStructPos = GetIntFromChunk( seg );
        std::wcout << L"File Tree Struct position: " + ToString( fileTreeStructPos ) + L"\n";

        Read4Bytes( seg, m_buf, 0x20 );
        numFolders = GetIntFromChunk( seg );
        std::wcout << L"Number of archived Folders: " + ToString( numFolders ) + L"\n";

        Read4Bytes( seg, m_buf, 0x24 );
        nameTablePos = GetIntFromChunk( seg );
        std::wcout << L"Name Table position: " + ToString( nameTablePos ) + L"\n";

        //Read folders:
        position = nameTablePos;

        int byteStartPos = nameTablePos +9;
        std::cout << byteStartPos << "\n";

        std::wstring tmp;
        int folderReq = 0;
        int filesReq = 0;
        int lastFolderPos;

        //Find folders
        /*for(int j{byteStartPos}; j < j+1; ++j)
        {
            if(folderReq != numFolders)
            {
                if((m_buf[j] != 0))
                {
                    tmp += m_buf[j];
                    if(m_buf[j+2] == 0)
                    {
                        ///folders.emplace_back(tmp);
                        std::wcout << L"folders added: " << tmp << std::endl;
                        ++folderReq;
                        tmp = L"";
                        lastFolderPos = j+1;
                    }
                    ++j;
                }
            }
            else
                break;
        }*/

        ///Find files
        /*for(int j{lastFolderPos}; j < j+1; ++j)
        {
            if(filesReq != numFiles)
            {
                if((m_buf[j] != 0))
                {
                    tmp += m_buf[j];
                    int b = j;
                    if(m_buf[j+2] == 0)
                    {
                        ///files.emplace_back(tmp);
                        std::cout << b << "||" << b+j;
                        std::wcout << L"files added: " << tmp << std::endl;
                        ++filesReq;
                        tmp = L"";
                    }
                    ++j;
                }
            }
            else
                break;
        }*/

        std::vector<std::wstring> trySort;
        std::vector<std::wstring> dds, mdb, lod;
        for(auto& str : files)
        {
            if(str.filename.find(L".lod.dds") != std::wstring::npos)
            {
                lod.emplace_back(str.filename);
            }
            if(str.filename.find(L".mdb") != std::wstring::npos)
            {
                mdb.emplace_back(str.filename);
            }
        }
        for(auto& str : files)
        {
            if(str.filename.find(L".lod") != std::wstring::npos) {}
            else if(str.filename.find(L".dds") != std::wstring::npos)
                dds.emplace_back(str.filename);
        }


        for(auto& s : lod)
            trySort.emplace_back(s);

        for(auto& s : dds)
            trySort.emplace_back(s);

        for(auto& s : mdb)
            trySort.emplace_back(s);


        for(auto& s : trySort)
            std::wcout << L"SORTED: " << s << std::endl;

        //for(int k{0}; k < trySort.size(); ++k)
        //files[k] = trySort[k];
        //std::reverse(files.begin(), files.end());

        //Read files:

        for( int i = 0; i < numFolders; ++i ) {
            Read4Bytes(seg, m_buf, position);
            folders.push_back(ReadUnicode(m_buf, position + GetIntFromChunk(seg)));
            position += 4;
        }

        position = 0x28;
        //std::map<int, std::vector<char>> FILE;
        for( int i = 0; i < numFiles; ++i )
        {
            RABFile rabstore;
            std::wcout << L"\n";
            std::wcout << L"FILE: ";
            Read4Bytes( seg, m_buf, position );
            std::wstring fileName = ReadUnicode( m_buf,  position + GetIntFromChunk( seg ));
            rabstore.filename = fileName;
            std::wcout << ReadUnicode( m_buf,  position + GetIntFromChunk( seg )) << position;
            std::wcout << L"_" << std::dec << position + GetIntFromChunk( seg ) << L"_";
            std::stringstream stream;
            stream << position + GetIntFromChunk( seg );
            stream >> std::dec >> rabstore.NameStartPos;
            std::wcout << fileName + L"\n";

            Read4Bytes( seg, m_buf, position );
            int fileNS =  GetIntFromChunk( seg );
            //rabstore.NameStartPos = fileNS; ///this was uncommented
            std::wcout << L"--FILENAME START POS: " + ToString( fileNS ) + L"\n";

            position += 0x4;

            Read4Bytes( seg, m_buf, position );
            int fileSize = GetIntFromChunk( seg );
            rabstore.Size = fileSize;
            std::wcout << L"--FILE SIZE: " + ToString( fileSize ) + L"\n";

            position += 0x4;

            Read4Bytes( seg, m_buf, position );

            std::wstring folderName = folders.at( GetIntFromChunk( seg ) );

            std::wcout << L"--PARENT NAME: " + folderName + L"\n";

            position += 0x4;

            position += 0x4;

            unsigned char data[8];

            Read4BytesReversed( seg, m_buf, position );
            data[0] = seg[0];
            data[1] = seg[1];
            data[2] = seg[2];
            data[3] = seg[3];

            position += 0x4;
            Read4BytesReversed( seg, m_buf, position );
            data[4] = seg[0];
            data[5] = seg[1];
            data[6] = seg[2];
            data[7] = seg[3];

            position += 0x4;

            Read4Bytes( seg, m_buf, position );
            int fileStart = GetIntFromChunk( seg );
            rabstore.ContentStartPos = fileStart;
            std::wcout << L"--CONTENT START POS: " + ToString( fileStart ) + L"\n";

            position += 0x4;

            //Unknown block:
            std::wcout << L"--UNKNOWN BLOCK: ";
            Read4BytesReversed( seg, m_buf, position );
            for(unsigned char j : seg)
            {
                std::wcout << L"0x";
                std::wcout << std::hex << j;
                std::wcout << L" ";
            }
            std::wcout << L"\n";

            position += 0x4;

            ///Emplace RABFile details for content block
            files.emplace_back(rabstore);

            //std::wstring correctedPath = path + L"\\" + folderName + L"\\" + fileName;

            ///Extract a file to a local file vector:

            std::vector< char > tempFile;
            for( int j = 0; j < fileSize; ++j )
            {
                tempFile.push_back(m_buf[fileStart + j]);
            }
            FILE[fileName] = tempFile;

            if(folderName != folderName_tmp)
            {
                files.clear();
                rabstore.filename = fileName;
                files.emplace_back(rabstore);
                folderName_tmp = folderName;
            }
            FOLDER[folderName] = files;
        }
        std::string cx;
        cx += "mkdir 'tmp/" + m_archiveName + "'";
        system(cx.c_str());
        cx.clear();
        for(auto f : FOLDER)
        {
            cx += "mkdir 'tmp/" + m_archiveName + "/" + WcharToString(f.first) + "'";
            system(cx.c_str());
            cx.clear();
            std::string out = "tmp/" + m_archiveName + "/" + WcharToString(f.first) + "/";
            std::cout << out;
            for(int x{0}; x < f.second.size(); ++x)
            {
                CMPLHandler CMPL = CMPLHandler(FILE[f.second[x].filename], false);
                std::vector<char> decompressedFile = CMPL.Decompress();

                using convert_type = std::codecvt_utf8<wchar_t>; std::wstring_convert<convert_type, wchar_t> converter;
                std::string converted_str = converter.to_bytes( f.second[x].filename );
                std::ofstream ofile = std::ofstream(out + converted_str, std::ios::binary | std::ios::out | std::ios::ate );
                for(char j : decompressedFile)
                    ofile << j;
                ofile.close();
            }
            //after the files are exported do a system call to ffmpeg and convert all the files into .png
            std::string mc = "cd " + out + ";for file in *dds; do convert $file $(basename $file .dds).png; done";
            system(mc.c_str());
        }
    }
}
