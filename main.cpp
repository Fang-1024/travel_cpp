#include <cstdlib>
#include "log.h"
#include "error_code.h"

int main(const int argc, char *argv[]) {
    LOG_INFO("Start, number of arg = %d.", argc);

    LOG_INFO("End.");
    return EXIT_SUCCESS;
}
