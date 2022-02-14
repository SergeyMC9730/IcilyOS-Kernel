#include <stl/cstdlib/cstdint.h>
#include <stl/cstdlib/stdio.h>

#include <x86_64/applications/application_manager.hpp>
#include <x86_64/drivers/serial_legacy.hpp>

#include <x86_64/applications/test/main.hpp>
#include <x86_64/applications/help/main.hpp>
#include <x86_64/applications/settings/main.hpp>
#include <x86_64/applications/registers/main.hpp>
#include <x86_64/applications/shell/main.hpp>
#include <x86_64/applications/ls/main.hpp>
#include <x86_64/applications/cat/main.hpp>
#include <x86_64/applications/writefile/main.hpp>

#include <x86_64/settings.hpp>

#include <x86_64/checksum.hpp>

namespace firefly::applications {
    struct apps {
        int     *address;
        int      checksum;
        uint16_t access;
    } apps_s[256];
    int appp = 0;

    bool executing_from_app = false;

    void register_application(int *address, int checksum, const char *command_name){
        apps_s[appp].address = (int *)address;
        apps_s[appp].checksum = checksum;
        firefly::kernel::io::legacy::writeTextSerial("Registered %s Command on address 0x%X with checksum %d\n\n", command_name, address, checksum);
        appp++;

        return;
    }
    void registerApplications(){
        register_application((int *)applications::test::test_main, applications::test::getc(), "Test");
        register_application((int *)applications::help::help_main, applications::help::getc(), "Help");
        register_application((int *)applications::settings::settings_main, applications::settings::getc(), "Settings");
        register_application((int *)applications::registers::registers_main, applications::registers::getc(), "Registers");
        register_application((int *)applications::shell::shell_main, applications::shell::getc(), "Shell");
        register_application((int *)applications::ls::ls_main, applications::ls::getc(), "List");
        register_application((int *)applications::cat::cat_main, applications::cat::getc(), "Cat");
        register_application((int *)applications::writefile::writefile_main, applications::writefile::getc(), "Writefile");

        return;
    }

    int run(const char *application, [[maybe_unused]] uint16_t access_rights, char **argv){
        printf("\n");

        int checksum = firefly::kernel::checksum::checksum(application);

        if(firefly::kernel::settings::kernel_settings[0] == 0x9a){
            printf("Checksum: %d\n", checksum);
        }
        
        uint8_t temp_pointer = 0;
        while(temp_pointer < 255){
            if(apps_s[temp_pointer].checksum == checksum) {
                executing_from_app = true;
                int result = ((int (*)(int, char **))apps_s[temp_pointer].address)(sizeof(argv), argv);
                executing_from_app = false;
                if (result == -1) printf("\n[ERROR] An error has occurred in the application!\nExit code: %d\n", result);
                
                return result;

            }
            temp_pointer++;
        }
        return INT32_MAX;
    }
    
    namespace external {
        struct app {
            int size;
            int checksum;
            int *address;
        } app_data;

        int run([[maybe_unused]] char **argv){
            printf("\n");
            return 0;
        }
    }
}