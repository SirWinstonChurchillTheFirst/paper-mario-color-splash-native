// Color Splash debugging (not part of a release build): who hands the title a "named object" (vtable 0x100880A4)
// where it expects an object with 54+ virtual functions (null call at LR 0x025ACBF4 after A on the title screen).
// CS_DEBUG=1 turns the prints on.

#include "cafe/export.h"
#include "cafe/ppc_ops.h"
#include "cafe/runtime.h"

#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <cstring>

PPC_FUNC(sub_025AC9AC_orig);
PPC_FUNC(sub_025CB63C_orig);

namespace {

uint32_t rd(const uint8_t* base, uint32_t a) {
    uint32_t v;
    std::memcpy(&v, base + a, 4);
    return __builtin_bswap32(v);
}

bool on() {
    static const bool enabled = std::getenv("CS_DEBUG") != nullptr;
    return enabled;
}

std::atomic<int> g_calls{0}, g_named{0};

const char* str(const uint8_t* base, uint32_t a) {
    if (a < 0x01000000u) return "(none)";
    return reinterpret_cast<const char*>(base + a);
}

} // namespace

// The title's per-frame function that resolves 25 handles (this+8.., 5 per group) and calls slot 53 on each object.
PPC_FUNC(sub_025AC9AC) {
    const uint32_t saved_this = ctx.r[3]; // call_guest below leaves its result in r3
    if (on()) {
        const uint32_t self = ctx.r[3];
        const int n = g_calls.fetch_add(1);
        bool bad = false;
        char line[2048];
        int used = std::snprintf(line, sizeof line, "csdbg: 025AC9AC this %08X lr %08X:", self, ctx.lr);
        for (int g = 0; g < 5; ++g) {
            const uint32_t group = self + 8 + 368 * g;
            for (int k = 0; k < 5; ++k) {
                const uint32_t h = group + 8 * k;
                const uint32_t h0 = rd(base, h), h1 = rd(base, h + 4);
                uint32_t obj = 0, vptr = 0;
                if (h0 != 0 || h1 != 0) {
                    obj = cafe::call_guest(ctx, 0x02414B1Cu, {h});
                    if (obj != 0) vptr = rd(base, obj + 12);
                }
                if (vptr == 0x100880A4u) bad = true;
                if (used < static_cast<int>(sizeof line) - 64 && (h0 | h1 | obj) != 0) {
                    used += std::snprintf(line + used, sizeof line - used, " [%d.%d %08X:%08X -> %08X vt %08X]", g, k, h0, h1, obj, vptr);
                }
            }
        }
        if (n < 3 || bad) {
            std::fprintf(stderr, "%s%s\n", line, bad ? "  <-- named object" : "");
            if (bad) {
                for (int g = 0; g < 5; ++g) {
                    for (int k = 0; k < 5; ++k) {
                        const uint32_t h = self + 8 + 368 * g + 8 * k;
                        const uint32_t h0 = rd(base, h), h1 = rd(base, h + 4);
                        if ((h0 | h1) == 0) continue;
                        const uint32_t table = 0x10130744u + ((h1 << 2) & 0x3FCu);
                        const uint32_t pool = rd(base, table);
                        std::fprintf(stderr, "csdbg:   handle %d.%d at %08X = %08X:%08X table %08X pool %08X pool-vt %08X get %08X\n", g, k, h, h0,
                                     h1, table, pool, pool ? rd(base, pool + 4) : 0, pool ? rd(base, rd(base, pool + 4) + 20) : 0);
                    }
                }
            }
        }
    }
    ctx.r[3] = saved_this;
    sub_025AC9AC_orig(ctx, base);
}

// Destructor of the 184-byte class (vtable 0x100887E4) the 'BGM' object belongs to.
PPC_FUNC(sub_025D4848_orig);
PPC_FUNC(sub_025D4848) {
    if (on() && ctx.r[3] != 0) {
        std::fprintf(stderr, "csdbg: dtor 100887E4-class %08X name '%.40s' flags %u lr %08X\n", ctx.r[3], str(base, rd(base, ctx.r[3] + 8)),
                     ctx.r[4], ctx.lr);
        cafe::dump_backtrace(ctx);
    }
    sub_025D4848_orig(ctx, base);
}

// The game's free(heaps, ptr): finds the heap that owns ptr among 5 roots (0x02415360), frees there.
PPC_FUNC(sub_024153E4_orig);
PPC_FUNC(sub_02415360_orig);
std::atomic<int> g_lost{0};
PPC_FUNC(sub_024153E4) {
    const uint32_t ptr = ctx.r[4], lr = ctx.lr;
    if (on() && ptr != 0) {
        const uint32_t saved3 = ctx.r[3], saved4 = ctx.r[4];
        const uint32_t heap = cafe::call_guest(ctx, 0x02415360u, {saved3, saved4});
        ctx.r[3] = saved3;
        ctx.r[4] = saved4;
        if (heap == 0 && g_lost.fetch_add(1) < 200) {
            std::fprintf(stderr, "csdbg: free of %08X: no heap owns it (lr %08X)\n", ptr, lr);
        } else if (ptr == 0x102DDD80u) {
            std::fprintf(stderr, "csdbg: free of %08X by heap %08X (heap vt %08X free %08X)\n", ptr, heap, rd(base, heap + 28),
                         rd(base, rd(base, heap + 28) + 28));
        }
    }
    sub_024153E4_orig(ctx, base);
}

// Named object constructor: X(this or 0, value, name) -> object (16 bytes: +4 value, +8 name copy, +12 vtable).
PPC_FUNC(sub_025CB63C) {
    const uint32_t value = ctx.r[4], name = ctx.r[5], lr = ctx.lr;
    sub_025CB63C_orig(ctx, base);
    if (on() && g_named.fetch_add(1) < 4000) {
        std::fprintf(stderr, "csdbg: named %08X value %08X name '%.60s' lr %08X\n", ctx.r[3], value, str(base, name), lr);
    }
}
