#ifndef TESTING_RAB_H
#define TESTING_RAB_H


#include <memory>
#include <unordered_map>

class RAB {
    struct RABFile {
        std::wstring filename;
        bool selected = false;
        unsigned int NameStartPos;
        unsigned int ContentStartPos;
        unsigned int Size;
    };
public:
    typedef std::shared_ptr<RAB> Ptr;
    static Ptr instance;
    static Ptr getRAB();

    void Read(const std::string& file);
    void SetArchiveName(std::string name) { m_archiveName = name; }
    void Insert(const std::string& file, int pos, const std::string& filename, int nametablepos);
    void Scan();
    int totalFilesInArchive;
    int totalFoldersInArchive;
    std::vector<std::wstring> folders;
    std::vector<RABFile> files;

    std::unordered_map<std::wstring, std::vector<char>> FILE;
    std::unordered_map<std::wstring, std::vector<RABFile>> FOLDER;

    std::wstring folderName_tmp;

    int numFiles;
    int fileTreeStructPos;
    int numFolders;
    int nameTablePos;

private:
    std::string m_archiveName;
    int m_dataStartOfs;
    std::vector<char> m_buf;
    std::string m_file; //the actual .rab you opened
    std::shared_ptr<std::ifstream> m_ifs;
    std::streamsize m_size;
};


#endif //TESTING_RAB_H
