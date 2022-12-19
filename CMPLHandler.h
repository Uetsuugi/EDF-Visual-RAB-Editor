#ifndef TESTING_CMPLHANDLER_H
#define TESTING_CMPLHANDLER_H

#include <vector>

class CMPLHandler {
public:
    CMPLHandler(std::vector<char> inFile, bool useFakeCompression);
    std::vector<char> Decompress();
    std::vector<char> Compress();
    std::vector<char> data;
private:
    bool m_useFakeCompression;
};


#endif //TESTING_CMPLHANDLER_H
