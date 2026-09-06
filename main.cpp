#include <cstdlib>
#include <string>

#include <travel_cpp/error_code.h>
#include <travel_cpp/get_input.h>
#include <travel_cpp/log.h>


int main(int argc, char* argv[])
{
    mini_log::Logger logger("app");
    logger.info("Start, number of arg = " + std::to_string(argc) + ".");
    logger.info("End.");
    return EXIT_SUCCESS;
}
