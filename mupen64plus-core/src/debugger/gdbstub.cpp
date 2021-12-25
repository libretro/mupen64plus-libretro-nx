// Copyright 2013 Dolphin Emulator Project
// Licensed under GPLv2+
// Refer to the license.txt file included.

// Originally written by Sven Peter <sven@fail0verflow.com> for anergistic.

#include <algorithm>
#include <atomic>
#include <climits>
#include <csignal>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <array>
#include <cstdint>
#include <map>
#include <numeric>
#include <fcntl.h>
#include <fmt/format.h>

#ifdef _WIN32
#include <winsock2.h>
// winsock2.h needs to be included first to prevent winsock.h being included by other includes
#include <io.h>
#include <iphlpapi.h>
#include <ws2tcpip.h>
#define SHUT_RDWR 2
#else
#include <netinet/in.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#endif

extern "C" {
#include "m64p_types.h"
#include "../main/main.h"
#include "m64p_common.h"
#include "m64p_config.h"
#include "m64p_debugger.h"
#include "m64p_frontend.h"
#include "m64p_types.h"
//#include "core_interface.h"
extern void n64DebugCallback(void* aContext, int aLevel, const char* aMessage);
#define DebugCallback n64DebugCallback

static void debugger_init(void);
static void debugger_vi(void);

void debugger_update(unsigned int pc);
}

#include <mupen64plus-next_common.h>
#include <vector>
//#define LOG_ERROR(...)
#define LOG_ERROR(...) GDBStub::FormatMsg(M64MSG_ERROR, __VA_ARGS__)
//#define LOG_DEBUG(...)
#define LOG_DEBUG(...) GDBStub::FormatMsg(M64MSG_VERBOSE, __VA_ARGS__)
//#define LOG_INFO(...)
#define LOG_INFO(...) GDBStub::FormatMsg(M64MSG_INFO, __VA_ARGS__)

namespace GDBStub {
namespace {
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;

using s8 = std::int8_t;
using s16 = std::int16_t;
using s32 = std::int32_t;
using s64 = std::int64_t;

using f32 = float;
using f64 = double;

using VAddr = u64;

using u128 = std::array<std::uint64_t, 2>;
static_assert(sizeof(u128) == 16, "u128 must be 128 bits wide");

constexpr int GDB_BUFFER_SIZE = 4095;

constexpr char GDB_STUB_START = '$';
constexpr char GDB_STUB_END = '#';
constexpr char GDB_STUB_ACK = '+';
constexpr char GDB_STUB_NACK = '-';

#ifndef SIGTRAP
constexpr u32 SIGTRAP = 5;
#endif

#ifndef SIGTERM
constexpr u32 SIGTERM = 15;
#endif

#ifndef MSG_WAITALL
constexpr u32 MSG_WAITALL = 8;
#endif

constexpr u32 SP_REGISTER = 29;
constexpr u32 STATUS_REGISTER = 32;
constexpr u32 LO_REGISTER = 33;
constexpr u32 HI_REGISTER = 34;
constexpr u32 BADVADDR_REGISTER = 35;
constexpr u32 CAUSE_REGISTER = 36;
constexpr u32 PC_REGISTER = 37;
constexpr u32 FCSR_REGISTER = 70;
constexpr u32 FIR_REGISTER = 71;

// For sample XML files see the GDB source /gdb/features
// GDB also wants the l character at the start
// This XML defines what the registers are for this specific ARM device
constexpr char target_xml[] =
    R"(l<?xml version="1.0"?>
<!-- edited with XMLSpy v2016 rel. 2 (x64) (http://www.altova.com) by Gilles (ALSTOM TIS) -->
<!DOCTYPE target SYSTEM "gdb-target.dtd">
<target version="1.0">
	<architecture>mips:4300</architecture>
    <feature name="org.gnu.gdb.mips.cpu" idatitle="General registers">
	  <reg name="zero" bitsize="64" type="data_ptr" regnum="0"/>
	  <reg name="at" bitsize="64" type="data_ptr"/>
	  <reg name="v0" bitsize="64" type="data_ptr"/>
	  <reg name="v1" bitsize="64" type="data_ptr"/>
	  <reg name="a0" bitsize="64" type="data_ptr"/>
	  <reg name="a1" bitsize="64" type="data_ptr"/>
	  <reg name="a2" bitsize="64" type="data_ptr"/>
	  <reg name="a3" bitsize="64" type="data_ptr"/>
	  <reg name="t0" bitsize="64" type="data_ptr"/>
	  <reg name="t1" bitsize="64" type="data_ptr"/>
	  <reg name="t2" bitsize="64" type="data_ptr"/>
	  <reg name="t3" bitsize="64" type="data_ptr"/>
	  <reg name="t4" bitsize="64" type="data_ptr"/>
	  <reg name="t5" bitsize="64" type="data_ptr"/>
	  <reg name="t6" bitsize="64" type="data_ptr"/>
	  <reg name="t7" bitsize="64" type="data_ptr"/>
	  <reg name="s0" bitsize="64" type="data_ptr"/>
	  <reg name="s1" bitsize="64" type="data_ptr"/>
	  <reg name="s2" bitsize="64" type="data_ptr"/>
	  <reg name="s3" bitsize="64" type="data_ptr"/>
	  <reg name="s4" bitsize="64" type="data_ptr"/>
	  <reg name="s5" bitsize="64" type="data_ptr"/>
	  <reg name="s6" bitsize="64" type="data_ptr"/>
	  <reg name="s7" bitsize="64" type="data_ptr"/>
	  <reg name="t8" bitsize="64" type="data_ptr"/>
	  <reg name="t9" bitsize="64" type="data_ptr"/>
	  <reg name="k0" bitsize="64" type="data_ptr"/>
	  <reg name="k1" bitsize="64" type="data_ptr"/>
	  <reg name="gp" bitsize="64" type="data_ptr"/>
	  <reg name="sp" bitsize="64" type="stack_ptr"/>
	  <reg name="fp" bitsize="64" type="data_ptr"/>
	  <reg name="ra" bitsize="64" type="data_ptr"/>
	  <reg name="lo" bitsize="64" regnum="33"/>
	  <reg name="hi" bitsize="64" regnum="34"/>
	  <reg name="pc" bitsize="64" regnum="37" type="code_ptr"/>
	</feature>
	<feature name="org.gnu.gdb.mips.fpu">
		<reg name="f0" bitsize="64" type="ieee_double" regnum="38"/>
		<reg name="f1" bitsize="64" type="ieee_double"/>
		<reg name="f2" bitsize="64" type="ieee_double"/>
		<reg name="f3" bitsize="64" type="ieee_double"/>
		<reg name="f4" bitsize="64" type="ieee_double"/>
		<reg name="f5" bitsize="64" type="ieee_double"/>
		<reg name="f6" bitsize="64" type="ieee_double"/>
		<reg name="f7" bitsize="64" type="ieee_double"/>
		<reg name="f8" bitsize="64" type="ieee_double"/>
		<reg name="f9" bitsize="64" type="ieee_double"/>
		<reg name="f10" bitsize="64" type="ieee_double"/>
		<reg name="f11" bitsize="64" type="ieee_double"/>
		<reg name="f12" bitsize="64" type="ieee_double"/>
		<reg name="f13" bitsize="64" type="ieee_double"/>
		<reg name="f14" bitsize="64" type="ieee_double"/>
		<reg name="f15" bitsize="64" type="ieee_double"/>
		<reg name="f16" bitsize="64" type="ieee_double"/>
		<reg name="f17" bitsize="64" type="ieee_double"/>
		<reg name="f18" bitsize="64" type="ieee_double"/>
		<reg name="f19" bitsize="64" type="ieee_double"/>
		<reg name="f20" bitsize="64" type="ieee_double"/>
		<reg name="f21" bitsize="64" type="ieee_double"/>
		<reg name="f22" bitsize="64" type="ieee_double"/>
		<reg name="f23" bitsize="64" type="ieee_double"/>
		<reg name="f24" bitsize="64" type="ieee_double"/>
		<reg name="f25" bitsize="64" type="ieee_double"/>
		<reg name="f26" bitsize="64" type="ieee_double"/>
		<reg name="f27" bitsize="64" type="ieee_double"/>
		<reg name="f28" bitsize="64" type="ieee_double"/>
		<reg name="f29" bitsize="64" type="ieee_double"/>
		<reg name="f30" bitsize="64" type="ieee_double"/>
		<reg name="f31" bitsize="64" type="ieee_double"/>
		<reg name="fcsr" bitsize="64" group="float"/>
		<reg name="fir" bitsize="64" group="float"/>
	</feature>
	<feature name="org.gnu.gdb.mips.cp0">
		<reg name="status" bitsize="64" regnum="32"/>
		<reg name="badvaddr" bitsize="64" regnum="35"/>
		<reg name="cause" bitsize="64" regnum="36"/>
	</feature>
</target>
)";

int gdbserver_socket = -1;

u8 command_buffer[GDB_BUFFER_SIZE];
u32 command_length;

u32 latest_signal = 0;
bool first_exception = true;

#ifdef _WIN32
WSADATA InitData;
#endif

/// Breakpoint Method
enum class BreakpointType {
    None,    ///< None
    Execute, ///< Execution Breakpoint
    Read,    ///< Read Breakpoint
    Write,   ///< Write Breakpoint
    Access   ///< Access (R/W) Breakpoint
};

struct BreakpointAddress {
    VAddr address;
    BreakpointType type;
};

struct Breakpoint {
    int index;
    bool active;
    VAddr addr;
    u64 len;
    std::array<u8, 4> inst;
};

using BreakpointMap = std::map<VAddr, Breakpoint>;
BreakpointMap breakpoints_execute;
BreakpointMap breakpoints_read;
BreakpointMap breakpoints_write;
BreakpointMap breakpoints_access;
} // Anonymous namespace

template<typename... Args>
void FormatMsg(int level, const char* fmt, Args... args)
{
    std::string msg = fmt::format(fmt, args...);
    DebugCallback((void*)"GDBStub", level, msg.c_str());
}

void Init(int port) {

    m64p_error rval = (*DebugSetCallbacks)(debugger_init,
        debugger_update,
        debugger_vi);

    // Setup initial gdbstub status
    breakpoints_execute.clear();
    breakpoints_read.clear();
    breakpoints_write.clear();
    breakpoints_access.clear();

    // Start gdb server
    LOG_INFO("Starting GDB server on port {}...", port);

    sockaddr_in saddr_server = {};
    saddr_server.sin_family = AF_INET;
    saddr_server.sin_port = htons(port);
    saddr_server.sin_addr.s_addr = INADDR_ANY;

#ifdef _WIN32
    WSAStartup(MAKEWORD(2, 2), &InitData);
#endif

    int tmpsock = static_cast<int>(socket(PF_INET, SOCK_STREAM, 0));
    if (tmpsock == -1) {
        LOG_ERROR("Failed to create gdb socket");
    }

    // Set socket to SO_REUSEADDR so it can always bind on the same port
    int reuse_enabled = 1;
    if (setsockopt(tmpsock, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse_enabled,
        sizeof(reuse_enabled)) < 0) {
        LOG_ERROR("Failed to set gdb socket option");
    }

    const sockaddr* server_addr = reinterpret_cast<const sockaddr*>(&saddr_server);
    socklen_t server_addrlen = sizeof(saddr_server);
    if (bind(tmpsock, server_addr, server_addrlen) < 0) {
        LOG_ERROR("Failed to bind gdb socket");
    }

    if (listen(tmpsock, 1) < 0) {
        LOG_ERROR("Failed to listen to gdb socket");
    }

    // Wait for gdb to connect
    LOG_INFO("Waiting for gdb to connect...");
    sockaddr_in saddr_client;
    sockaddr* client_addr = reinterpret_cast<sockaddr*>(&saddr_client);
    socklen_t client_addrlen = sizeof(saddr_client);
    gdbserver_socket = static_cast<int>(accept(tmpsock, client_addr, &client_addrlen));
    if (gdbserver_socket < 0) {
        // In the case that we couldn't start the server for whatever reason, just start CPU
        // execution like normal.
        //TODO

        LOG_ERROR("Failed to accept gdb client");
    }
    else {
        LOG_INFO("Client connected.");
        saddr_client.sin_addr.s_addr = ntohl(saddr_client.sin_addr.s_addr);
    }

    // Clean up temporary socket if it's still alive at this point.
    if (tmpsock != -1) {
        shutdown(tmpsock, SHUT_RDWR);
    }
}

void Shutdown() {
    LOG_INFO("Stopping GDB ...");

    (*DebugSetRunState)(M64P_DBG_RUNSTATE_RUNNING);

    if (gdbserver_socket != -1) {
        shutdown(gdbserver_socket, SHUT_RDWR);
        gdbserver_socket = -1;
    }

#ifdef _WIN32
    WSACleanup();
#endif

    LOG_INFO("GDB stopped.");
}

static u64 RegRead(std::size_t id) {
    if (id < 32) {
        u64* regs = (u64*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_REG);
        return regs[id];
    } else if (id == STATUS_REGISTER) {
        u32* cp0 = (u32*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_COP0);
        return (u64)cp0[12];
    } else if (id == LO_REGISTER) {
        return *(u64*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_LO);
    } else if (id == HI_REGISTER) {
        return *(u64*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_HI);
    } else if (id == BADVADDR_REGISTER) {
        u32* cp0 = (u32*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_COP0);
        return (u64)cp0[8];
    } else if (id == CAUSE_REGISTER) {
        u32* cp0 = (u32*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_COP0);
        return (u64)cp0[13];
    } else if (id == PC_REGISTER) {
        return (s64)*(s32*)(*DebugGetCPUDataPtr)(M64P_CPU_PC);
    } else if (id >= 38 && id < FCSR_REGISTER) {
        u64* cp1 = (u64*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_COP1_FGR_64);
        return cp1[id - 38];
    } else if (id == FCSR_REGISTER) {
        return 0; //TODO
    } else if (id == FIR_REGISTER) {
        return 0; //TODO
    } else {
        return 0;
    }
}

static void RegWrite(std::size_t id, u64 val) {
    if (id < 32) {
        u64* regs = (u64*) (*DebugGetCPUDataPtr)(M64P_CPU_REG_REG);
        regs[id] = val;
    } else if (id == STATUS_REGISTER) {
        u32* cp0 = (u32*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_COP0);
        cp0[12] = static_cast<u32>(val);
    } else if (id == LO_REGISTER) {
        *(u64*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_LO) = val;
    } else if (id == HI_REGISTER) {
        *(u64*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_HI) = val;
    } else if (id == BADVADDR_REGISTER) {
        u32* cp0 = (u32*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_COP0);
        cp0[8] = static_cast<u32>(val);
    } else if (id == CAUSE_REGISTER) {
        u32* cp0 = (u32*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_COP0);
        cp0[13] = static_cast<u32>(val);
    } else if (id == PC_REGISTER) {
        *(u32*)(*DebugGetCPUDataPtr)(M64P_CPU_PC) = static_cast<u32>(val);
    }else if (id >= 38 && id < FCSR_REGISTER) {
        u64* cp1 = (u64*)(*DebugGetCPUDataPtr)(M64P_CPU_REG_COP1_FGR_64);
        cp1[id - 38] = val;
    } else if (id == FCSR_REGISTER) {
        //TODO
    } else if (id == FIR_REGISTER) {
        //TODO
    }
}

/**
 * Turns hex string character into the equivalent byte.
 *
 * @param hex Input hex character to be turned into byte.
 */
static u8 HexCharToValue(u8 hex) {
    if (hex >= '0' && hex <= '9') {
        return hex - '0';
    } else if (hex >= 'a' && hex <= 'f') {
        return hex - 'a' + 0xA;
    } else if (hex >= 'A' && hex <= 'F') {
        return hex - 'A' + 0xA;
    }

    LOG_ERROR("Invalid nibble: {} ({:02X})", hex, hex);
    return 0;
}

/**
 * Turn nibble of byte into hex string character.
 *
 * @param n Nibble to be turned into hex character.
 */
static u8 NibbleToHex(u8 n) {
    n &= 0xF;
    if (n < 0xA) {
        return '0' + n;
    } else {
        return 'a' + n - 0xA;
    }
}

/**
 * Converts input hex string characters into an array of equivalent of u8 bytes.
 *
 * @param src Pointer to array of output hex string characters.
 * @param len Length of src array.
 */
static u32 HexToInt(const u8* src, std::size_t len) {
    u32 output = 0;
    while (len-- > 0) {
        output = (output << 4) | HexCharToValue(src[0]);
        src++;
    }
    return output;
}

/**
 * Converts input hex string characters into an array of equivalent of u8 bytes.
 *
 * @param src Pointer to array of output hex string characters.
 * @param len Length of src array.
 */
static u64 HexToLong(const u8* src, std::size_t len) {
    u64 output = 0;
    while (len-- > 0) {
        output = (output << 4) | HexCharToValue(src[0]);
        src++;
    }
    return output;
}

/**
 * Converts input array of u8 bytes into their equivalent hex string characters.
 *
 * @param dest Pointer to buffer to store output hex string characters.
 * @param src Pointer to array of u8 bytes.
 * @param len Length of src array.
 */
static void MemToGdbHex(u8* dest, const u8* src, std::size_t len) {
    while (len-- > 0) {
        u8 tmp = *src++;
        *dest++ = NibbleToHex(tmp >> 4);
        *dest++ = NibbleToHex(tmp);
    }
}

/**
 * Converts input gdb-formatted hex string characters into an array of equivalent of u8 bytes.
 *
 * @param dest Pointer to buffer to store u8 bytes.
 * @param src Pointer to array of output hex string characters.
 * @param len Length of src array.
 */
static void GdbHexToMem(u8* dest, const u8* src, std::size_t len) {
    while (len-- > 0) {
        *dest++ = (HexCharToValue(src[0]) << 4) | HexCharToValue(src[1]);
        src += 2;
    }
}

/**
 * Convert a u32 into a gdb-formatted hex string.
 *
 * @param dest Pointer to buffer to store output hex string characters.
 * @param v    Value to convert.
 */
static void IntToGdbHex(u8* dest, u32 v) {
    for (int i = 0; i < 8; i += 2) {
        dest[i] = NibbleToHex(static_cast<u8>(v >> (28-i*4)));
        dest[i+1] = NibbleToHex(static_cast<u8>(v >> (28-(i+1)*4)));
    }
}

/**
 * Convert a u64 into a gdb-formatted hex string.
 *
 * @param dest Pointer to buffer to store output hex string characters.
 * @param v    Value to convert.
 */
static void LongToGdbHex(u8* dest, u64 v) {
    for (int i = 0; i < 16; i += 2) {
        dest[i] = NibbleToHex(static_cast<u8>(v >> (60-i*4)));
        dest[i+1] = NibbleToHex(static_cast<u8>(v >> (60-(i+1)*4)));
    }
}

/**
 * Convert a gdb-formatted hex string into a u32.
 *
 * @param src Pointer to hex string.
 */
static u32 GdbHexToInt(const u8* src) {
    u32 output = 0;

    for (int i = 0; i < 8; i += 2) {
        output = (output << 4) | HexCharToValue(src[i]);
        output = (output << 4) | HexCharToValue(src[i+1]);
    }

    return output;
}

/**
 * Convert a gdb-formatted hex string into a u64.
 *
 * @param src Pointer to hex string.
 */
static u64 GdbHexToLong(const u8* src) {
    u64 output = 0;

    for (int i = 0; i < 16; i += 2) {
        output = (output << 4) | HexCharToValue(src[i]);
        output = (output << 4) | HexCharToValue(src[i+1]);
    }

    return output;
}

/// Read a byte from the gdb client.
static u8 ReadByte() {
    u8 c;
    std::size_t received_size = recv(gdbserver_socket, reinterpret_cast<char*>(&c), 1, MSG_WAITALL);
    if (received_size != 1) {
        LOG_ERROR("recv failed: {}", received_size);
        Shutdown();
    }

    return c;
}

/// Calculate the checksum of the current command buffer.
static u8 CalculateChecksum(const u8* buffer, std::size_t length) {
    return static_cast<u8>(std::accumulate(buffer, buffer + length, 0, std::plus<u8>()));
}

/**
 * Get the map of breakpoints for a given breakpoint type.
 *
 * @param type Type of breakpoint map.
 */
static BreakpointMap& GetBreakpointMap(BreakpointType type) {
    switch (type) {
    case BreakpointType::Execute:
        return breakpoints_execute;
    case BreakpointType::Read:
        return breakpoints_read;
    case BreakpointType::Write:
        return breakpoints_write;
    case BreakpointType::Access:
        return breakpoints_access;
    default:
        return breakpoints_read;
    }
}

/**
 * Remove the breakpoint from the given address of the specified type.
 *
 * @param type Type of breakpoint.
 * @param addr Address of breakpoint.
 */
static void RemoveBreakpoint(BreakpointType type, VAddr addr) {
    BreakpointMap& p = GetBreakpointMap(type);

    const auto bp = p.find(addr);
    if (bp == p.end()) {
        return;
    }

    LOG_DEBUG("gdb: removed a breakpoint: {:016X} bytes at {:016X} of type {}",
              bp->second.len, bp->second.addr, static_cast<int>(type));


    (*DebugBreakpointCommand)(M64P_BKP_CMD_REMOVE_IDX, bp->second.index, NULL);
    p.erase(addr);
}

/**
 * Send packet to gdb client.
 *
 * @param packet Packet to be sent to client.
 */
static void SendPacket(const char packet) {
    std::size_t sent_size = send(gdbserver_socket, &packet, 1, 0);
    if (sent_size != 1) {
        LOG_ERROR("send failed");
    }
}

/**
 * Send reply to gdb client.
 *
 * @param reply Reply to be sent to client.
 */
static void SendReply(const char* reply) {
    LOG_DEBUG("Reply: {}", reply);

    memset(command_buffer, 0, sizeof(command_buffer));

    command_length = static_cast<u32>(strlen(reply));
    if (command_length + 4 > sizeof(command_buffer)) {
        LOG_ERROR("command_buffer overflow in SendReply");
        return;
    }

    memcpy(command_buffer + 1, reply, command_length);

    u8 checksum = CalculateChecksum(command_buffer, command_length + 1);
    command_buffer[0] = GDB_STUB_START;
    command_buffer[command_length + 1] = GDB_STUB_END;
    command_buffer[command_length + 2] = NibbleToHex(checksum >> 4);
    command_buffer[command_length + 3] = NibbleToHex(checksum);

    u8* ptr = command_buffer;
    u32 left = command_length + 4;
    while (left > 0) {
        int sent_size = send(gdbserver_socket, reinterpret_cast<char*>(ptr), left, 0);
        if (sent_size < 0) {
            LOG_ERROR("gdb: send failed");
            return Shutdown();
        }

        left -= sent_size;
        ptr += sent_size;
    }
}

/// Handle query command from gdb client.
static void HandleQuery() {
    LOG_DEBUG("gdb: query '{}'", command_buffer + 1);

    const char* query = reinterpret_cast<const char*>(command_buffer + 1);

    if (strcmp(query, "TStatus") == 0) {
        SendReply("T0");
    } else if (strcmp(query, "Attached") == 0) {
        SendReply("1");
    } else if (strncmp(query, "Supported", strlen("Supported")) == 0) {
        // PacketSize needs to be large enough for target xml
        std::string buffer = "PacketSize=10000;qXfer:features:read+";
        SendReply(buffer.c_str());
    } else if (strncmp(query, "Xfer:features:read:target.xml:",
                       strlen("Xfer:features:read:target.xml:")) == 0) {
        SendReply(target_xml);
    } else if (strncmp(query, "Offsets", strlen("Offsets")) == 0) {
        const VAddr base_address = 0;
        std::string buffer = fmt::format("TextSeg={:0x}", base_address);
        SendReply(buffer.c_str());
    } else if (strncmp(query, "fThreadInfo", strlen("fThreadInfo")) == 0) {
        std::string val = "m";
        val.pop_back();
        SendReply(val.c_str());
    } else if (strncmp(query, "sThreadInfo", strlen("sThreadInfo")) == 0) {
        SendReply("l");
    } else {
        SendReply("");
    }
}

/// Handle set thread command from gdb client.
static void HandleSetThread() {
    SendReply("E01");
}

/// Handle thread alive command from gdb client.
static void HandleThreadAlive() {
    SendReply("E01");
}

/**
 * Send signal packet to client.
 *
 * @param signal Signal to be sent to client.
 */
static void SendSignal(u32 signal, bool full = true) {
    latest_signal = signal;

    std::string buffer;
    if (full) {
        buffer = fmt::format("T{:02x}{:02x}:{:016x};{:02x}:{:016x};", latest_signal,
                             PC_REGISTER, RegRead(PC_REGISTER),
                             SP_REGISTER, RegRead(SP_REGISTER));
    } else {
        buffer = fmt::format("T{:02x}", latest_signal);
    }

    SendReply(buffer.c_str());
}

/// Read command from gdb client.
static void ReadCommand() {
    command_length = 0;
    memset(command_buffer, 0, sizeof(command_buffer));

    u8 c = ReadByte();
    if (c == '+') {
        // ignore ack
        return;
    } else if (c == 0x03) {
        LOG_INFO("gdb: found break command");
        SendSignal(SIGTRAP);
        return;
    } else if (c != GDB_STUB_START) {
        LOG_DEBUG("gdb: read invalid byte {:02X}", c);
        return;
    }

    while ((c = ReadByte()) != GDB_STUB_END) {
        if (command_length >= sizeof(command_buffer)) {
            LOG_ERROR("gdb: command_buffer overflow");
            SendPacket(GDB_STUB_NACK);
            return;
        }
        command_buffer[command_length++] = c;
    }

    u8 checksum_received = HexCharToValue(ReadByte()) << 4;
    checksum_received |= HexCharToValue(ReadByte());

    u8 checksum_calculated = CalculateChecksum(command_buffer, command_length);

    if (checksum_received != checksum_calculated) {
        LOG_ERROR("gdb: invalid checksum: calculated {:02X} and read {:02X} for ${}# (length: {})",
                  checksum_calculated, checksum_received, command_buffer, command_length);

        command_length = 0;

        SendPacket(GDB_STUB_NACK);
        return;
    }

    SendPacket(GDB_STUB_ACK);
}

/// Check if there is data to be read from the gdb client.
static bool IsDataAvailable() {
    fd_set fd_socket;

    FD_ZERO(&fd_socket);
    FD_SET(static_cast<u32>(gdbserver_socket), &fd_socket);

    struct timeval t;
    t.tv_sec = 0;
    t.tv_usec = 0;

    if (select(gdbserver_socket + 1, &fd_socket, nullptr, nullptr, &t) < 0) {
        LOG_ERROR("select failed");
        return false;
    }

    return FD_ISSET(gdbserver_socket, &fd_socket) != 0;
}

/// Send requested register to gdb client.
static void ReadRegister() {
    static u8 reply[64];
    memset(reply, 0, sizeof(reply));

    u32 id = HexCharToValue(command_buffer[1]);
    if (command_buffer[2] != '\0') {
        id <<= 4;
        id |= HexCharToValue(command_buffer[2]);
    }

    LongToGdbHex(reply, RegRead(id));

    SendReply(reinterpret_cast<char*>(reply));
}

/// Send all registers to the gdb client.
static void ReadRegisters() {
    static u8 buffer[GDB_BUFFER_SIZE - 4];
    memset(buffer, 0, sizeof(buffer));

    u8* bufptr = buffer;

    for (u32 reg = 0; reg <= FIR_REGISTER; reg++) {
        LongToGdbHex(bufptr + reg * 16, RegRead(reg));
    }

    SendReply(reinterpret_cast<char*>(buffer));
}

/// Modify data of register specified by gdb client.
static void WriteRegister() {
    const u8* buffer_ptr = command_buffer + 3;

    u32 id = HexCharToValue(command_buffer[1]);
    if (command_buffer[2] != '=') {
        ++buffer_ptr;
        id <<= 4;
        id |= HexCharToValue(command_buffer[2]);
    }

    RegWrite(id, GdbHexToLong(buffer_ptr));

    SendReply("OK");
}

/// Modify all registers with data received from the client.
static void WriteRegisters() {
    const u8* buffer_ptr = command_buffer + 1;

    if (command_buffer[0] != 'G')
        return SendReply("E01");

    for (u32 i = 0, reg = 0; reg <= FIR_REGISTER; i++, reg++) {
        RegWrite(reg, GdbHexToLong(buffer_ptr + i * 16));
    }

    SendReply("OK");
}

/// Read location in memory specified by gdb client.
static void ReadMemory() {
    static u8 reply[GDB_BUFFER_SIZE - 4];

    auto start_offset = command_buffer + 1;
    const auto addr_pos = std::find(start_offset, command_buffer + command_length, ',');
    VAddr addr = HexToLong(start_offset, static_cast<u64>(addr_pos - start_offset));

    start_offset = addr_pos + 1;
    const u64 len =
        HexToLong(start_offset, static_cast<u64>((command_buffer + command_length) - start_offset));

    if (len * 2 > sizeof(reply)) {
        SendReply("E01");
    }

    //TODO
    /*if (!IsValidVirtualAddress(addr)) {
        return SendReply("E00");
    }*/

    std::vector<u8> data(len);

    LOG_DEBUG("gdb: addr: {:016X} len: {:016X}", addr, len);

    IgnoreTLBExceptions = 2;
    for (int i = 0; i < len; i++)
        data[i] = (*DebugMemRead8)((u32)(addr + i));
    IgnoreTLBExceptions = 0;

    MemToGdbHex(reply, data.data(), len);
    reply[len * 2] = '\0';
    SendReply(reinterpret_cast<char*>(reply));
}

/// Modify location in memory with data received from the gdb client.
static void WriteMemory() {
    auto start_offset = command_buffer + 1;
    auto addr_pos = std::find(start_offset, command_buffer + command_length, ',');
    VAddr addr = HexToLong(start_offset, static_cast<u64>(addr_pos - start_offset));

    start_offset = addr_pos + 1;
    auto len_pos = std::find(start_offset, command_buffer + command_length, ':');
    u64 len = HexToLong(start_offset, static_cast<u64>(len_pos - start_offset));

    //TODO
    /*if (!IsValidVirtualAddress(addr)) {
        return SendReply("E00");
    }*/

    std::vector<u8> data(len);

    LOG_DEBUG("gdb: addr: {:016X} len: {:016X}", addr, len);

    IgnoreTLBExceptions = 2;
    GdbHexToMem(data.data(), len_pos + 1, len);
    for (int i = 0; i < len; i++)
        (*DebugMemWrite8)((u32)(addr + i), data[i]);
    IgnoreTLBExceptions = 0;

    SendReply("OK");
}

/// Tell the CPU that it should perform a single step.
static void Step() {
    /*if (command_length > 1) {
        RegWrite(PC_REGISTER, GdbHexToLong(command_buffer + 1));
    }*/
    (*DebugStep)();
}

/// Tell the CPU to continue executing.
static void Continue() {
    (*DebugSetRunState)(M64P_DBG_RUNSTATE_RUNNING);
}

u32 func_osVirtualToPhysical(VAddr vaddr) {
    if (vaddr >= 0x80000000U) {
        if (vaddr < 0xA0000000U) {
            return vaddr & 0x1FFFFFFF;
        }
    }

    if (vaddr >= 0xA0000000U) {
        if (vaddr < 0xC0000000U) {
            return vaddr & 0x1FFFFFFF;
        }
    }

    VAddr probedAddr = virtual_to_physical_address(&g_dev.r4300, vaddr, 0);
    if(probedAddr)
        return probedAddr;
    else
        return vaddr;
}

/**
 * Commit breakpoint to list of breakpoints.
 *
 * @param type Type of breakpoint.
 * @param addr Address of breakpoint.
 * @param len Length of breakpoint.
 */
static bool CommitBreakpoint(BreakpointType type, VAddr addr, u64 len) {
    BreakpointMap& p = GetBreakpointMap(type);
    
    switch (type) {
    case BreakpointType::Read:
    case BreakpointType::Write:
    case BreakpointType::Access:
        addr = func_osVirtualToPhysical(addr);
    case BreakpointType::Execute:
    default:
        break;
    }
    
    Breakpoint breakpoint;
    breakpoint.active = true;
    breakpoint.addr = addr;
    breakpoint.len = len;

    m64p_breakpoint bkpt;
    bkpt.address = (u32)addr;
    bkpt.endaddr = (u32)(addr + len);
    bkpt.flags = M64P_BKP_FLAG_ENABLED | M64P_BKP_FLAG_LOG;

    switch (type) {
    case BreakpointType::Execute:
        bkpt.flags |= M64P_BKP_FLAG_EXEC;
        break;
    case BreakpointType::Write:
        bkpt.flags |= M64P_BKP_FLAG_WRITE;
        break;
    case BreakpointType::Read:
        bkpt.flags |= M64P_BKP_FLAG_READ;
        break;
    case BreakpointType::Access:
        bkpt.flags |= M64P_BKP_FLAG_WRITE | M64P_BKP_FLAG_READ;
        break;
    default:
        return false;
    }

    int numBkps = (*DebugBreakpointCommand)(M64P_BKP_CMD_ADD_STRUCT, 0, &bkpt);

    if (numBkps == -1) {
        LOG_ERROR("Maximum breakpoint limit already reached.\n");
        return false;
    }

    breakpoint.index = numBkps;
    p.insert({addr, breakpoint});

    LOG_DEBUG("gdb: added {} breakpoint: {:016X} bytes at {:016X}",
              static_cast<int>(type), breakpoint.len, breakpoint.addr);

    return true;
}

/// Handle add breakpoint command from gdb client.
static void AddBreakpoint() {
    BreakpointType type;

    u8 type_id = HexCharToValue(command_buffer[1]);
    switch (type_id) {
    case 0:
    case 1:
        type = BreakpointType::Execute;
        break;
    case 2:
        type = BreakpointType::Write;
        break;
    case 3:
        type = BreakpointType::Read;
        break;
    case 4:
        type = BreakpointType::Access;
        break;
    default:
        return SendReply("E01");
    }

    auto start_offset = command_buffer + 3;
    auto addr_pos = std::find(start_offset, command_buffer + command_length, ',');
    VAddr addr = HexToLong(start_offset, static_cast<u64>(addr_pos - start_offset));

    start_offset = addr_pos + 1;
    u64 len =
        HexToLong(start_offset, static_cast<u64>((command_buffer + command_length) - start_offset));

    if (!CommitBreakpoint(type, addr, len)) {
        return SendReply("E02");
    }

    SendReply("OK");
}

/// Handle remove breakpoint command from gdb client.
static void RemoveBreakpoint() {
    BreakpointType type;

    u8 type_id = HexCharToValue(command_buffer[1]);
    switch (type_id) {
    case 0:
    case 1:
        type = BreakpointType::Execute;
        break;
    case 2:
        type = BreakpointType::Write;
        break;
    case 3:
        type = BreakpointType::Read;
        break;
    case 4:
        type = BreakpointType::Access;
        break;
    default:
        return SendReply("E01");
    }

    auto start_offset = command_buffer + 3;
    auto addr_pos = std::find(start_offset, command_buffer + command_length, ',');
    VAddr addr = HexToLong(start_offset, static_cast<u64>(addr_pos - start_offset));
    
    switch (type) {
    case BreakpointType::Read:
    case BreakpointType::Write:
    case BreakpointType::Access:
        addr = func_osVirtualToPhysical(addr);
    case BreakpointType::Execute:
    default:
        break;
    }
    
    RemoveBreakpoint(type, addr);
    SendReply("OK");
}

//TODO: handle pause via ctrl-c?
void HandlePacket() {
    if (!first_exception)
        SendSignal(SIGTRAP);

    first_exception = false;

    while (1)
    {
        if (gdbserver_socket < 0)
            continue;

        if (!IsDataAvailable()) {
            continue;
        }

        ReadCommand();
        if (command_length == 0) {
            continue;
        }

        LOG_DEBUG("Packet: {}", command_buffer);

        switch (command_buffer[0]) {
        case 'q':
            HandleQuery();
            break;
        case 'H':
            HandleSetThread();
            break;
        case '!':
            SendReply("OK");
            break;
        case '?':
            SendSignal(latest_signal);
            break;
        case 'k':
            Shutdown();
            LOG_INFO("killed by gdb");
            return;
        case 'g':
            ReadRegisters();
            break;
        case 'G':
            WriteRegisters();
            break;
        case 'p':
            ReadRegister();
            break;
        case 'P':
            WriteRegister();
            break;
        case 'm':
            ReadMemory();
            break;
        case 'M':
            WriteMemory();
            break;
        case 's':
            Step();
            return;
        case 'C':
        case 'c':
            Continue();
            return;
        case 'z':
            RemoveBreakpoint();
            break;
        case 'Z':
            AddBreakpoint();
            break;
        case 'T':
            HandleThreadAlive();
            break;
        case 'v':
            if(!strcmp((char*)command_buffer, "vCont?"))
                SendReply("vCont;s;c");
            else
            {
                if(!strcmp((char*)command_buffer, "vCont;s:1"))
                {
                    Step();
                    return;
                }else if(strstr((char*)command_buffer, "vCont;c"))
                {
                    Continue();
                    return;
                }
            }
            
        default:
            SendReply("");
            break;
        }
    }
}
}; // namespace GDBStub

extern "C" {
    void GDBStub_Init(int port) {
        GDBStub::Init(port);
    }

    void GDBStub_Shutdown() {
        GDBStub::Shutdown();
    }

    static void debugger_init(void)
    {
        LOG_DEBUG("Debugger initialized.");
    }

    void debugger_update(unsigned int pc)
    {
        LOG_DEBUG("PC at 0x%08X.", pc);
        GDBStub::HandlePacket();
    }
    
    static void debugger_vi(void)
    {
        LOG_DEBUG("Vertical int");
    }
}