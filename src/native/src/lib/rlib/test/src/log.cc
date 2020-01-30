#include <rlib/log.hpp>

using namespace rlib;
int main() {
    // logging to stdout.
    logger stdout_logger(std::cout);
    stdout_logger.info("test");
    stdout_logger.debug("running shit.");
    stdout_logger.info("{} is {} {}."_format("hust asm", 1, "shit"));
    // logging to file.
    logger file_logger("/tmp/rlib.test.log");
    file_logger.info("shit here.");
    file_logger.verbose("???");
    
    log_level_t my_level = file_logger.register_log_level("MyLogLev");
    file_logger.log(my_level, "my info.... whit's a fuck");
    return 0;
}
