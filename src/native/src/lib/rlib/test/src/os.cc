#include <rlib/sys/os.hpp>

#if RLIB_OS_ID != OS_LINUX
#error fuck
#endif

static_assert(rlib::os_info::os == rlib::os_info::os_t::LINUX);
static_assert(rlib::os_info::compiler == rlib::os_info::compiler_t::GCC);

int main(){

}
