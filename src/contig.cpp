#include "runner.h"
#include "mole.h"
#include "map.h"
#include "contig_builder.h"

#include <vector>
#include <map>
#include <string>
#include <iostream>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/assign.hpp>

#include <log4cxx/logger.h>

static log4cxx::LoggerPtr logger(log4cxx::Logger::getLogger("nanoARCS.Contig"));

class Contigging : public Runner {
public:
    virtual int run(const Properties options, const Arguments& arg);
private:
    Contigging() : Runner("a:f:o:t:p:", boost::assign::map_list_of('a', "algorithm")('f', "correctedfile")('o', "prefix")('t', "threads")('p', "parameter")) {
        RunnerManager::instance()->install("contig", this);
    }
    int checkOptions(const Properties options, const Arguments& args) const {
        if(options.find("h") != options.not_found() || args.size() == 0) {
            printHelps();
            return 1;
        }
        return 0;
    }
    int printHelps() const {
        std::cout << boost::format(
                "nanoARCS contig [OPTION] ... READSFILE\n"
                "Compute pairwise overlap between all the molecule in READS\n"
                "\n"
                "      -h, --help                       display this help and exit\n"
                "\n"
                "      -a, --algorithm=A                use A correct function (default: vote)\n"
                "      -f, --overlapfile=file           input file\n"
                "      -t, --threads=NUM                use NUM threads to construct the index (default: 1)\n"
                "      -o, --prefix=PREFIX              write index to file using PREFIX instead of prefix of READSFILE\n"
                "\n"
                ) << std::endl;
        return 256;
    }
    static Contigging _runner;
};
Contigging Contigging::_runner;
int Contigging::run(const Properties options, const Arguments& args) {
    int r = 0;
    if((r = checkOptions(options, args)) == 1) {
        return r;
    }
    std::string input = args[0];
    LOG4CXX_INFO(logger, boost::format("input file is: %s") % input);
    std::string output = boost::filesystem::path(input).stem().string() + ".out";
    if(options.find("prefix") != options.not_found()) {
        output = options.get< std::string > ("prefix");
    }
    LOG4CXX_INFO(logger, boost::format("output file is: %s") % output);
    std::string parameter_file = "parameters.ini";
    if(options.find("parameters.ini") != options.not_found()) {
        parameter_file = options.get< std::string > ("parameter");
    }
    Map maptool(parameter_file);
    ContigBuilder builder(maptool, output);
    if(!builder.build(input, output, options.get< double > ("minscore", 25.0), options.get< size_t> ("threads", 1))) {
        LOG4CXX_ERROR(logger, boost::format("Fail to build contig from %s corrected centers") % input);
        r = -1;
    }
    return r;
}