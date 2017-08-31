# this file explains how to compile modules into D(ynamic)L(inked)L(ibrary)
# the main program will load the modules as library to fulfill its function.

############################################################################
#                                  USAGE                                   #
# 1. follow the examples, write your own modules.                          #
# 2. add rules to compile that. you can follow examples given below.       #
# 3. note your code must be compiled as dynamic linked library. Any other  #
#    format will be on your own to make that works.                        #
# 4. some useful functions are provided in <module_util/utils.h>. Select   #
#    what you want, and add dependencies for that.                         #
# 5. if you found any bugs in functions we provided or interfaces, please  #
#    don't hesitate to contact us at github.com.                           #
#                                                                          #
#                                                          Your sincerely, #
#                                                                   ****** #
############################################################################

include config.mk

# your modules example
# there should be some scripts to auto_detect the dependencies and
# generates the correct makefile.
# but (eh..I promise) it will be here in a (perhaps much more) later version.
# thanks for co-operation.

main : $(YOUR_MODULE)
	$(TERM_ECHO)$(ECHO_PROG) "     --   destination main built."

my_module : simple_proxy.o
	$(TERM_ECHO)$(ECHO_PROG) "     LD   my_module.so"
	$(TERM_ECHO)$(CC) $(DLLFLAGS) -o my_module.so simple_proxy.o

se_proxy : se_proxy.o aes.o kokke_aes.o
	$(TERM_ECHO)$(ECHO_PROG) "     LD   se_proxy.so"
	$(TERM_ECHO)$(CC) $(DLLFLAGS) -o se_proxy.so se_proxy.o kokke_aes.o aes.o

simple_proxy.o : simple_proxy/simple_proxy.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   simple_proxy.o"
	$(TERM_ECHO)$(CC) -c $(CFLAGS) -o simple_proxy.o simple_proxy/simple_proxy.c

se_proxy.o : simple_encrypted_proxy/se_proxy.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   se_proxy.o"
	$(TERM_ECHO)$(CC) -c $(CFLAGS) -o se_proxy.o simple_encrypted_proxy/se_proxy.c

### optional dependencies
### these are built-in useful tools we provided for you.
### please, don't modify stuff below
### except extreme conditions

octet_order.o : module_util/utils.h module_util/octet_order.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   octet_order.o"
	$(TERM_ECHO)$(CC) -c $(CFLAGS) -o octet_order.o module_util/octet_order.c

kokke_aes.o : module_util/utils.h module_util/kokke_aes.c module_util/kokke_aes.h
	$(TERM_ECHO)$(ECHO_PROG) "     CC   kokke_aes.o"
	$(TERM_ECHO)$(CC) -c $(CFLAGS) -o kokke_aes.o module_util/kokke_aes.c

### notice: using aes must compile with kokke_aes
aes.o : module_util/utils.h module_util/aes.c module_util/aes.h kokke_aes.o
	$(TERM_ECHO)$(ECHO_PROG) "     CC   aes.o"
	$(TERM_ECHO)$(CC) -c $(CFLAGS) -o aes.o module_util/aes.c

fmt.o : module_util/utils.h module_util/fmt.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   fmt.o"
	$(TERM_ECHO)$(CC) -c $(CFLAGS) -o fmt.o module_util/fmt.c

### testers:
### use this to test your module using the test program we provided.
### notice this test can only thest the correctiveness of your module.
### things about memory management -- you must take care yourself.
### read our simple_proxy module for example on the agreement that
### module communicate with the main program(s/c).

test: $(YOUR_MODULE_TEST)
	$(TERM_ECHO)$(ECHO_PROG) "     --   destination test built."

my_module_test : simple_proxy.o test.o
	$(TERM_ECHO)$(ECHO_PROG) "     LD   test"
	$(TERM_ECHO)$(CC) -o test test.o simple_proxy.o

se_proxy_test : se_proxy.o test.o kokke_aes.o aes.o
	$(TERM_ECHO)$(ECHO_PROG) "     LD   test"
	$(TERM_ECHO)$(CC) -o test test.o se_proxy.o kokke_aes.o aes.o

test.o : test.c
	$(TERM_ECHO)$(ECHO_PROG) "     CC   test.o"
	$(TERM_ECHO)$(CC) -c -o test.o test.c


### cleaners
clean : clean_intermediate clean_object clean_test
	$(TERM_ECHO)$(ECHO_PROG) "  CLEAN   done"

clean_intermediate :
	$(TERM_ECHO)$(ECHO_PROG) "  CLEAN   intermediate files"
	$(TERM_IGNR)$(TERM_ECHO)$(REMOVE_FILE) *.o $(CLEAN_OUTPUT) $(CANCEL_ERRNO)

clean_object :
	$(TERM_ECHO)$(ECHO_PROG) "  CLEAN   generated files"
	$(TERM_IGNR)$(TERM_ECHO)$(REMOVE_FILE) *.so $(CLEAN_OUTPUT) $(CANCEL_ERRNO)

clean_test :
	$(TERM_ECHO)$(ECHO_PROG) "  CLEAN   test file"
	$(TERM_IGNR)$(TERM_ECHO)$(REMOVE_FILE) test $(CLEAN_OUTPUT) $(CANCEL_ERRNO)