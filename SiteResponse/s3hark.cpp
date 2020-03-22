
#include <fstream>
#include <iostream>
#include "../UI/SiteResponse.h"

int main(int argc, char** argv)
{
	if (argc != 4)
	{
        std::cout << "Please specify 3 parameters like this: s3hark input.json workDir outputDir \n";
        std::cout << "input.json: this is the input file \n";
        std::cout << "workDir   : this is the directory where you have Rock-x.time and Rock-x.vel \n";
        std::cout << "outputDir : this is the directory where outputs will be saved \n";
		//std::getchar();
		return -1;
	}

    std::string configureFile = argv[1];
	std::string anaDir = argv[2];
	std::string outDir = argv[3];
    SiteResponse *srt = new SiteResponse(configureFile,anaDir,outDir);

    //srt->buildTcl();
    srt->run();
                

    return 1;

}
