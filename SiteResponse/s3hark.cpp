
#include <fstream>
#include <iostream>
#include "../UI/SiteResponse.h"

#ifdef WIN32  
#include <windows.h>  
#include <direct.h>  
#else  
#include <unistd.h>  
#include <sys/types.h>  
#include <sys/stat.h>  
#include <dirent.h>  
#endif  
/**
* @brief isFileOrDir
*
* Detailed description.
* @param[in] path
* @return int 0:doesn't exist，1:directory, -1:file
*/
inline int isFileOrDir(std::string path)
{
#ifdef WIN32
    DWORD dwAttr = GetFileAttributes(path.c_str());
    //Not exist
    if (dwAttr == 0xFFFFFFFF)
    {
        return 0;
    }
    //exist
    else if (dwAttr & FILE_ATTRIBUTE_DIRECTORY)
    {
        return 1;
    }
    //is File
    else
    {
        return -1;
    }
#else
    if (0 == access(path.c_str(), 0))
    {
        struct stat *buf;

        buf = (struct stat *)malloc(sizeof(struct stat));
        memset(buf, 0, sizeof(struct stat));

        stat(path.c_str(), buf);

        if (S_ISDIR(buf->st_mode))
        {
            free(buf);
            buf = NULL;
            return 1;
        }
        else
        {
            free(buf);
            buf = NULL;
            return -1;
        }
    }
    else
    {
        return 0;
    }
#endif
}

int main(int argc, char **argv)
{
    if (argc != 5)
    {
        std::cout << "Please specify 3 parameters like this: s3hark inputFile workDir outputDir log \n";
        std::cout << "inputFile : the input file (json) \n";
        std::cout << "workDir   : the directory where you have Rock-x.time and Rock-x.vel \n";
        std::cout << "outputDir : the directory where outputs will be saved \n";
        std::cout << "log       : path of log file \n";
        //std::getchar();
        return -1;
    }

    std::string configureFile = argv[1];
    std::string anaDir = argv[2];
    std::string outDir = argv[3];
    std::string log = argv[4];

    if (isFileOrDir(outDir) != 1)
    {
        std::cout << outDir.c_str() << " directory doesn't exist." << "\n";
        return 1;
    }

    if (isFileOrDir(anaDir) != 1)
    {
        std::cout << anaDir.c_str() << " directory doesn't exist." << "\n";
        return 1;
    }

    if (isFileOrDir(configureFile) != -1)
    {
        std::cout << configureFile.c_str() << " doesn't exist2." << "\n";
        return 1;
    }

    SiteResponse *srt = new SiteResponse(configureFile, anaDir, outDir, log);

    //srt->buildTcl();
    if (srt->run() == -1)
        std::cout << "Analysis was not successful. Read " << log << "\n";

    return 1;
}
