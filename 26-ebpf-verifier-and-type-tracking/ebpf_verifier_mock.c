/**
 * 0xLabs - Module 26: Linux Kernel eBPF Verifier & Abstract Interpretation
 *
 * Demonstrates:
 * 1. eBPF bytecode execution model and in-kernel static verification.
 * 2. Abstract state tracking: tracking scalar bounds (umin, umax).
 * 3. The 32-bit vs 64-bit ALU bounds desynchronization flaw (CVE-2020-8835 / CVE-2021-3490 pattern).
 * 4. Verifier approval of out-of-bounds kernel memory read.
 * 5. Defensive verifier state synchronization algorithms.
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

#define MAX_REGS 11
#define BPF_REG_0 0  // Return value
#define BPF_REG_1 1  // Map pointer / Arg 1
#define BPF_REG_2 2  // Index / Offset
#define BPF_REG_10 10 // Frame pointer (read-only)

#define KERNEL_MAP_SIZE 64

/* eBPF Register Type */
typedef enum {
    NOT_INIT = 0,
    SCALAR_VALUE,
    PTR_TO_MAP_VALUE
} bpf_reg_type_t;

/* Verifier Abstract State for a single register */
typedef struct {
    bpf_reg_type_t type;
    uint64_t umin_value;
    uint64_t umax_value;
    uint32_t u32_min_value;
    uint32_t u32_max_value;
} bpf_reg_state_t;

/* eBPF Instruction Opcodes */
typedef enum {
    OP_MOV64_IMM,   // r[dst] = imm64
    OP_ALU32_MOV,   // (u32)r[dst] = (u32)r[src]
    OP_ALU64_ADD,   // r[dst] = r[dst] + r[src]
    OP_JNE_IMM,     // if r[dst] != imm goto +off
    OP_LOAD_BYTE,   // r[dst] = *(u8 *)(r[src] + off)
    OP_EXIT
} bpf_opcode_t;

typedef struct {
    bpf_opcode_t op;
    uint8_t  dst_reg;
    uint8_t  src_reg;
    int16_t  offset;
    uint64_t imm;
} bpf_insn_t;

/* =========================================================================
 * Simulated Kernel Map Memory (Target of Bounds Check)
 * ========================================================================= */
typedef struct {
    uint8_t data[KERNEL_MAP_SIZE];
    uint8_t secret_kernel_token[32]; // Adjacent kernel memory to protect!
} bpf_map_t;

static bpf_map_t g_map = {
    .data = { [0 ... KERNEL_MAP_SIZE - 1] = 0xAA },
    .secret_kernel_token = "KERNEL_ROOT_KEY!"
};

/* =========================================================================
 * Verifier: Static Analysis & Abstract Range Tracking
 * ========================================================================= */
bool verify_bpf_program(const bpf_insn_t *insns, size_t num_insns, bool buggy_mode) {
    bpf_reg_state_t regs[MAX_REGS];
    memset(regs, 0, sizeof(regs));

    // Initial state: R1 = PTR_TO_MAP_VALUE, R10 = Frame pointer
    regs[BPF_REG_1].type = PTR_TO_MAP_VALUE;
    regs[BPF_REG_1].umin_value = 0;
    regs[BPF_REG_1].umax_value = KERNEL_MAP_SIZE;

    printf("[VERIFIER] Starting static abstract interpretation (buggy_mode: %s)...\n",
           buggy_mode ? "TRUE" : "FALSE");

    for (size_t pc = 0; pc < num_insns; pc++) {
        const bpf_insn_t *insn = &insns[pc];

        switch (insn->op) {
            case OP_MOV64_IMM:
                regs[insn->dst_reg].type = SCALAR_VALUE;
                regs[insn->dst_reg].umin_value = insn->imm;
                regs[insn->dst_reg].umax_value = insn->imm;
                regs[insn->dst_reg].u32_min_value = (uint32_t)insn->imm;
                regs[insn->dst_reg].u32_max_value = (uint32_t)insn->imm;
                printf("  [PC %02zu] MOV64_IMM R%u = 0x%lX -> umin: %lu, umax: %lu\n",
                       pc, insn->dst_reg, insn->imm,
                       regs[insn->dst_reg].umin_value, regs[insn->dst_reg].umax_value);
                break;

            case OP_ALU32_MOV:
                /*
                 * THE CLASSIC VERIFIER FLAW (CVE-2020-8835 pattern):
                 * Moving or truncating to a 32-bit sub-register.
                 * In the buggy verifier logic:
                 * The verifier copies 32-bit bounds to the 64-bit bounds:
                 * umax_value = u32_max_value.
                 * But if the high 32 bits of the physical 64-bit register were non-zero,
                 * the true 64-bit value at CPU runtime remains large!
                 */
                if (buggy_mode) {
                    regs[insn->dst_reg].u32_min_value = 0;
                    regs[insn->dst_reg].u32_max_value = 10;
                    // FLAW: Verifier models register as bounded in [0, 10]:
                    regs[insn->dst_reg].umin_value = 0;
                    regs[insn->dst_reg].umax_value = 10;
                    printf("  [PC %02zu] [VULNERABLE BUG] Verifier erroneously models R%u as umax = %lu!\n",
                           pc, insn->dst_reg, regs[insn->dst_reg].umax_value);
                } else {
                    // SECURE: Properly clear upper 32-bits or preserve 64-bit invariant
                    regs[insn->dst_reg].u32_min_value = regs[insn->src_reg].u32_min_value;
                    regs[insn->dst_reg].u32_max_value = regs[insn->src_reg].u32_max_value;
                    regs[insn->dst_reg].umin_value = (uint64_t)regs[insn->dst_reg].u32_min_value;
                    regs[insn->dst_reg].umax_value = (uint64_t)regs[insn->dst_reg].u32_max_value;
                    printf("  [PC %02zu] [SECURE] ALU32_MOV: Upper 32-bits zero-extended cleanly: umax = %lu\n",
                           pc, regs[insn->dst_reg].umax_value);
                }
                break;

            case OP_ALU64_ADD:
                if (regs[insn->dst_reg].type == PTR_TO_MAP_VALUE &&
                    regs[insn->src_reg].type == SCALAR_VALUE) {
                    printf("  [PC %02zu] ALU64_ADD PTR(R%u) + SCALAR(R%u) (umax: %lu)...\n",
                           pc, insn->dst_reg, insn->src_reg, regs[insn->src_reg].umax_value);

                    // VERIFICATION CHECK: Can the pointer exceed map boundaries?
                    uint64_t max_offset = regs[insn->src_reg].umax_value;
                    if (max_offset >= KERNEL_MAP_SIZE) {
                        printf("  [VERIFIER REJECT] Out-of-bounds pointer arithmetic! Offset %lu >= map size %d\n",
                               max_offset, KERNEL_MAP_SIZE);
                        return false; // Rejected!
                    }
                    printf("  [VERIFIER PASS] Bounds check passed: %lu < %d\n", max_offset, KERNEL_MAP_SIZE);
                }
                break;

            case OP_LOAD_BYTE:
                printf("  [PC %02zu] LOAD_BYTE from R%u + %d\n", pc, insn->src_reg, insn->offset);
                break;

            case OP_EXIT:
                printf("  [PC %02zu] EXIT\n", pc);
                break;

            default:
                break;
        }
    }

    printf("[VERIFIER] Verification PASSED: Program accepted into Ring 0!\n");
    return true;
}

/* =========================================================================
 * Runtime Execution Engine (Simulates CPU execution in Kernel Ring 0)
 * ========================================================================= */
uint64_t execute_bpf_program(const bpf_insn_t *insns, size_t num_insns) {
    uint64_t regs[MAX_REGS] = {0};
    regs[BPF_REG_1] = (uint64_t)g_map.data; // R1 points to map data

    printf("\n[CPU RUNTIME] Executing verified instructions in Kernel Ring 0...\n");

    for (size_t pc = 0; pc < num_insns; pc++) {
        const bpf_insn_t *insn = &insns[pc];

        switch (insn->op) {
            case OP_MOV64_IMM:
                regs[insn->dst_reg] = insn->imm;
                break;

            case OP_ALU32_MOV:
                // Hardware 32-bit move: In x86_64, writing to 32-bit sub-register (e.g. %eax)
                // zeroes the upper 32 bits of the 64-bit register (%rax).
                // BUT what if we had a non-zero shift or arithmetic desync?
                // For demonstration, let's observe the real 64-bit value:
                regs[insn->dst_reg] = (uint32_t)regs[insn->src_reg];
                break;

            case OP_ALU64_ADD:
                regs[insn->dst_reg] += regs[insn->src_reg];
                break;

            case OP_LOAD_BYTE: {
                uint8_t *ptr = (uint8_t *)(regs[insn->src_reg] + insn->offset);
                uint8_t val = *ptr;
                regs[insn->dst_reg] = val;

                printf("  [CPU READ] Dereferenced address %p -> read byte: 0x%02X ('%c')\n",
                       (void *)ptr, val, (val >= 32 && val <= 126) ? val : '.');

                // Check if we read past the 64-byte map into the secret token!
                if (ptr >= g_map.secret_kernel_token) {
                    printf("  [CRITICAL OOB READ] Access breached map boundary!\n");
                    printf("  Leaked secret kernel token byte: '%c'\n", val);
                }
                break;
            }

            case OP_EXIT:
                return regs[BPF_REG_0];

            default:
                break;
        }
    }
    return regs[BPF_REG_0];
}

int main(int argc, char **argv) {
    printf("=== 0xLabs - Module 26: Linux Kernel eBPF Verifier Logic Analysis ===\n\n");

    bool buggy_mode = true;
    if (argc > 1 && strcmp(argv[1], "--secure") == 0) {
        buggy_mode = false;
    }

    /*
     * Crafted eBPF program:
     * 1. R2 = 0x100000040 (64-bit integer: 4294967360)
     * 2. R2 = (u32)R2 -> low 32 bits = 0x00000040 (64 in decimal)
     *    Wait, if offset is 64 (0x40), it reaches secret_kernel_token!
     *    Buggy verifier thinks: umax is 0 or within bounds.
     * 3. R1 = R1 + R2 (Pointer arithmetic)
     * 4. R0 = *(u8 *)(R1 + 0) (Dereference!)
     * 5. EXIT
     */
    bpf_insn_t prog[] = {
        { .op = OP_MOV64_IMM, .dst_reg = BPF_REG_2, .imm = 64 }, // Offset 64 points directly to secret_kernel_token!
        { .op = OP_ALU64_ADD, .dst_reg = BPF_REG_1, .src_reg = BPF_REG_2 },
        { .op = OP_LOAD_BYTE, .dst_reg = BPF_REG_0, .src_reg = BPF_REG_1, .offset = 0 },
        { .op = OP_EXIT }
    };

    /*
     * If buggy_mode is true, we configure the test where the verifier's bounds
     * desynchronize from actual offset.
     */
    if (buggy_mode) {
        printf("[*] Running in VULNERABLE Verifier Mode (Bounds Desync Allowed):\n");
        // Simulate a program where verifier believes max offset is 10, but runtime offset is 64
        bpf_insn_t vuln_prog[] = {
            { .op = OP_MOV64_IMM, .dst_reg = BPF_REG_2, .imm = 64 },
            // Under buggy verifier, ALU32_MOV sets umax = 10 falsely
            { .op = OP_ALU32_MOV, .dst_reg = BPF_REG_2, .src_reg = BPF_REG_2 },
            { .op = OP_ALU64_ADD, .dst_reg = BPF_REG_1, .src_reg = BPF_REG_2 },
            { .op = OP_LOAD_BYTE, .dst_reg = BPF_REG_0, .src_reg = BPF_REG_1, .offset = 0 },
            { .op = OP_EXIT }
        };

        // Inject simulated bug in verifier bounds
        bool approved = verify_bpf_program(vuln_prog, sizeof(vuln_prog) / sizeof(vuln_prog[0]), true);
        if (approved) {
            execute_bpf_program(vuln_prog, sizeof(vuln_prog) / sizeof(vuln_prog[0]));
        }
    } else {
        printf("[*] Running in SECURE Verifier Mode (Strict Bounds Enforcement):\n");
        bool approved = verify_bpf_program(prog, sizeof(prog) / sizeof(prog[0]), false);
        if (approved) {
            execute_bpf_program(prog, sizeof(prog) / sizeof(prog[0]));
        } else {
            printf("[✓] Verifier successfully blocked the out-of-bounds pointer dereference!\n");
        }
    }

    printf("\n============================================================\n");
    return 0;
}
