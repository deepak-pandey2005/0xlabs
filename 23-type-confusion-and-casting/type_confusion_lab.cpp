/**
 * 0xLabs - Module 23: C++ Type Confusion & Unsafe Casting
 *
 * Demonstrates:
 * 1. Object memory layouts under single and multiple inheritance.
 * 2. Unchecked downcasting with static_cast vs dynamic_cast with RTTI.
 * 3. Type confusion resulting in memory corruption and field reinterpretation.
 * 4. Detection via Clang's -fsanitize=vptr and safe design patterns (std::variant).
 */

#include <iostream>
#include <string>
#include <vector>
#include <cstring>
#include <memory>
#include <cstdint>

enum class ObjectType {
    TEXT_DOCUMENT = 1,
    ADMIN_SESSION = 2
};

// Base class with virtual destructor (enabling RTTI)
class BaseEntity {
public:
    ObjectType type;
    virtual ~BaseEntity() = default;
    virtual void inspect() const = 0;
};

// Benign object holding a text document
class TextDocument : public BaseEntity {
public:
    char title[32];
    int char_count;

    TextDocument(const char *t, int count) {
        type = ObjectType::TEXT_DOCUMENT;
        strncpy(title, t, sizeof(title) - 1);
        title[sizeof(title) - 1] = '\0';
        char_count = count;
    }

    void inspect() const override {
        std::cout << "  [TextDoc] Title: '" << title << "', Words: " << char_count << "\n";
    }
};

// High-privilege session object with security callback
class AdminSession : public BaseEntity {
public:
    char admin_name[32];
    uint64_t privilege_mask;
    void (*audit_callback)(const char *);

    AdminSession(const char *name, uint64_t privs, void (*cb)(const char *)) {
        type = ObjectType::ADMIN_SESSION;
        strncpy(admin_name, name, sizeof(admin_name) - 1);
        admin_name[sizeof(admin_name) - 1] = '\0';
        privilege_mask = privs;
        audit_callback = cb;
    }

    void inspect() const override {
        std::cout << "  [AdminSession] Admin: '" << admin_name
                  << "', PrivMask: 0x" << std::hex << privilege_mask << std::dec << "\n";
    }

    void execute_audit() {
        if (audit_callback) {
            audit_callback(admin_name);
        }
    }
};

static void sample_audit_logger(const char *admin) {
    std::cout << "  [AUDIT SUCCESS] Authorized action logged for: " << admin << "\n";
}

/* =========================================================================
 * VULNERABLE FUNCTION: Unchecked static_cast
 *
 * Flaw: Assumes the passed entity is always an AdminSession based on
 * external state, user input, or faulty logic, and uses static_cast without
 * checking the real runtime type.
 * ========================================================================= */
void process_admin_action_vulnerable(BaseEntity *entity) {
    std::cout << "[!] Calling vulnerable handler with entity ptr: " << entity << "\n";

    // VULNERABILITY: static_cast does compile-time type conversion.
    // It assumes the object pointed to by 'entity' is truly an AdminSession.
    // No RTTI check occurs at runtime!
    AdminSession *session = static_cast<AdminSession *>(entity);

    std::cout << "    Interpreting entity as AdminSession:\n";
    std::cout << "    Name: " << session->admin_name << "\n";
    std::cout << "    Privilege Mask (Raw Hex): 0x" << std::hex << session->privilege_mask << std::dec << "\n";

    // If 'entity' is actually a TextDocument:
    // session->privilege_mask maps to the memory offset of TextDocument::char_count!
    // session->audit_callback maps past the end of TextDocument or onto adjacent memory!
    if (session->privilege_mask >= 0x00000000000000FFULL) {
        std::cout << "    [CONFUSION OCCURRED] High-privilege gate unlocked due to field reinterpretation!\n";
    }

    std::cout << "    Attempting to invoke audit execution on confused object...\n";
    session->execute_audit();
}

/* =========================================================================
 * SECURE FUNCTION: RTTI dynamic_cast and Tagged Verification
 * ========================================================================= */
void process_admin_action_secure(BaseEntity *entity) {
    std::cout << "[+] Calling SECURE handler with entity ptr: " << entity << "\n";

    // Defense 1: Explicit type tag check
    if (entity->type != ObjectType::ADMIN_SESSION) {
        std::cout << "    [REJECTED] Explicit tag check failed: Object is NOT AdminSession!\n";
        return;
    }

    // Defense 2: dynamic_cast utilizes RTTI stored in the vtable
    AdminSession *session = dynamic_cast<AdminSession *>(entity);
    if (!session) {
        std::cout << "    [REJECTED] dynamic_cast returned nullptr: RTTI mismatch!\n";
        return;
    }

    std::cout << "    [AUTHENTICATED] Admin session verified safely: " << session->admin_name << "\n";
    session->execute_audit();
}

int main(int argc, char **argv) {
    std::cout << "=== 0xLabs - Module 23: C++ Type Confusion & Unsafe Casting ===\n\n";

    bool run_vulnerable = false;
    bool run_secure = false;

    if (argc > 1 && strcmp(argv[1], "--vuln") == 0) {
        run_vulnerable = true;
    } else if (argc > 1 && strcmp(argv[1], "--secure") == 0) {
        run_secure = true;
    } else {
        std::cout << "Usage: " << argv[0] << " [--vuln | --secure]\n";
        return 1;
    }

    // Allocate a TextDocument
    TextDocument doc("Confidential Memo", 99999);
    std::cout << "[*] Created legitimate TextDocument object:\n";
    doc.inspect();

    // Allocate an AdminSession
    AdminSession admin("root_operator", 0x7FFFFFFF, sample_audit_logger);
    std::cout << "\n[*] Created legitimate AdminSession object:\n";
    admin.inspect();

    std::cout << "\n------------------------------------------------------------\n";

    if (run_vulnerable) {
        std::cout << "[TEST] Passing TextDocument to admin handler (Triggering Type Confusion)...\n";
        // Type confusion: BaseEntity pointer to TextDocument passed to admin handler
        process_admin_action_vulnerable(&doc);
    }

    if (run_secure) {
        std::cout << "[TEST] Passing TextDocument to secure admin handler...\n";
        process_admin_action_secure(&doc);

        std::cout << "\n[TEST] Passing real AdminSession to secure admin handler...\n";
        process_admin_action_secure(&admin);
    }

    std::cout << "\n============================================================\n";
    return 0;
}
