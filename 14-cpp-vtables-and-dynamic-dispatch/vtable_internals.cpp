#include <iostream>
#include <iomanip>
#include <cstdint>

/*
 * ============================================================================
 * MODULE 14: C++ Virtual Method Tables (vtables) & Dynamic Dispatch
 * ============================================================================
 * Concepts:
 *   1. Itanium C++ ABI object memory layout: hidden _vptr at offset 0.
 *   2. The vtable structure stored in .rodata.
 *   3. Dynamic dispatch in assembly: mov rax, [rdi] ; call [rax + offset].
 *   4. Control Flow Integrity (CFI) defenses for indirect calls.
 */

class BaseService {
public:
    int service_id;

    BaseService() : service_id(101) {}
    virtual ~BaseService() = default;

    virtual void execute_task() {
        std::cout << "[BaseService] Executing default task (ID: " << service_id << ")\n";
    }

    virtual void authenticate_client(const char *client_name) {
        std::cout << "[BaseService] Authenticating client: " << client_name << "\n";
    }
};

class AdminService : public BaseService {
public:
    int privilege_level;

    AdminService() : privilege_level(999) {
        service_id = 777;
    }

    void execute_task() override {
        std::cout << "[AdminService] Executing privileged task with level " 
                  << privilege_level << " (ID: " << service_id << ")\n";
    }

    void authenticate_client(const char *client_name) override {
        std::cout << "[AdminService] Admin authenticated: " << client_name << "\n";
    }
};

void inspect_object(const char *label, BaseService *obj) {
    std::cout << "\n[*] Memory Inspection for: " << label << "\n";
    std::cout << "    Object Address (&obj)    : " << static_cast<void*>(obj) << "\n";

    // In 64-bit Itanium ABI, the first 8 bytes of the object are the _vptr
    uintptr_t *vptr = *reinterpret_cast<uintptr_t**>(obj);
    std::cout << "    vptr value (vtable ptr)  : " << static_cast<void*>(vptr) << " (resides in .rodata)\n";

    // Dump first 3 entries in the vtable (destructor, execute_task, authenticate_client)
    std::cout << "    vtable[0] (Destructor/TypeInfo) : " << reinterpret_cast<void*>(vptr[0]) << "\n";
    std::cout << "    vtable[1] (execute_task slot)   : " << reinterpret_cast<void*>(vptr[1]) << "\n";
    std::cout << "    vtable[2] (authenticate slot)   : " << reinterpret_cast<void*>(vptr[2]) << "\n";

    // Call virtual method via dynamic dispatch
    std::cout << "[*] Invoking virtual method obj->execute_task()...\n";
    obj->execute_task();
}

int main() {
    std::cout << "=== 0xLabs: C++ vtable & Dynamic Dispatch Internals ===\n";

    BaseService base;
    AdminService admin;

    inspect_object("BaseService Instance", &base);
    inspect_object("AdminService Instance", &admin);

    std::cout << "\n[+] Dynamic dispatch demonstrated successfully.\n";
    return 0;
}
