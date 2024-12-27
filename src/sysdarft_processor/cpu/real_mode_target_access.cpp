#include <cpu.h>
#include <cpu.h>
#include <cpu.h>

#define __illegal_instruction_assert__(Expression)                              \
if (!(Expression))                                                              \
{                                                                               \
    throw IllegalInstruction("Assertion failed during instruction decoding at " \
        __FILE__ ":" + std::to_string(__LINE__)                                 \
        + ": " + std::string(__FUNCTION__) + ": " #Expression);                 \
} __asm__("")

void processor::Target::do_setup_register_info()
{
    const uint8_t width = CPU.pop<8>();
    const auto register_index = CPU.pop<8>();
    TargetInformation.RegisterIndex = register_index;
    TargetType = TypeRegister;
    TargetWidth = width;

    switch (width) {
    case 0x08: literal = "%R"   + std::to_string(register_index); break;
    case 0x16: literal = "%EXR" + std::to_string(register_index); break;
    case 0x32: literal = "%HER" + std::to_string(register_index); break;
    case 0x64: literal = "%FER" + std::to_string(register_index); break;
    case 0xFC: literal = "%XMM" + std::to_string(register_index); break;
    default: __illegal_instruction_assert__(false);
    }
}

void processor::Target::do_setup_constant_info()
{
    __illegal_instruction_assert__(CPU.pop<8>() == 0x64);
    const auto constant_value = CPU.pop<64>();
    TargetType = TypeConstant;
    TargetInformation.ConstantValue = constant_value;
    literal = "$(" + std::to_string(constant_value) + ")";
}

void processor::Target::do_setup_memory_info()
{
    uint64_t BaseAddress, Offset1, Offset2, Ratio;
    std::string literal_base, literal_off1, literal_off2;
    const uint8_t width = CPU.pop<8>();

    const uint8_t PrefixBaseAddress = CPU.pop<8>();
    __illegal_instruction_assert__(PrefixBaseAddress == REGISTER_PREFIX
        || PrefixBaseAddress == CONSTANT_PREFIX);
    do_decode_via_prefix(PrefixBaseAddress);
    BaseAddress = get_target_content_in_u64bit_t();
    literal_base = literal;

    const uint8_t PrefixOffset1 = CPU.pop<8>();
    __illegal_instruction_assert__(PrefixOffset1 == REGISTER_PREFIX
        || PrefixOffset1 == CONSTANT_PREFIX);
    do_decode_via_prefix(PrefixOffset1);
    Offset1 = get_target_content_in_u64bit_t();
    literal_off1 = literal;

    const uint8_t PrefixOffset2 = CPU.pop<8>();
    __illegal_instruction_assert__(PrefixOffset2 == REGISTER_PREFIX
        || PrefixOffset2 == CONSTANT_PREFIX);
    do_decode_via_prefix(PrefixOffset2);
    Offset2 = get_target_content_in_u64bit_t();
    literal_off2 = literal;

    Ratio = CPU.pop<8>();

    TargetType = TypeMemory;
    TargetInformation.MemoryAddress = (BaseAddress + Offset1 + Offset2) * Ratio;
    TargetWidth = width;
    literal = "*" + std::to_string(Ratio) +
        "(" + literal_base +
        ", " + literal_off1 +
        ", " + literal_off2 + ")";
}

void processor::Target::do_decode_via_prefix(const uint8_t prefix)
{
    switch (prefix)
    {
    case REGISTER_PREFIX: do_setup_register_info(); break;
    case CONSTANT_PREFIX: do_setup_constant_info(); break;
    case MEMORY_PREFIX: do_setup_memory_info(); break;
    default: throw IllegalInstruction("Unknown Target prefix");
    }
}

uint64_t processor::Target::do_get_register()
{
    std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
    switch (TargetWidth) {
    case 0x08:
        switch (TargetInformation.RegisterIndex) {
        case 0x00: return CPU.Registers.R0;
        case 0x01: return CPU.Registers.R1;
        case 0x02: return CPU.Registers.R2;
        case 0x03: return CPU.Registers.R3;
        case 0x04: return CPU.Registers.R4;
        case 0x05: return CPU.Registers.R5;
        case 0x06: return CPU.Registers.R6;
        case 0x07: return CPU.Registers.R7;
        default: __illegal_instruction_assert__(false);
        }
    case 0x16:
        switch (TargetInformation.RegisterIndex) {
        case 0x00: return CPU.Registers.ExtendedRegister0;
        case 0x01: return CPU.Registers.ExtendedRegister1;
        case 0x02: return CPU.Registers.ExtendedRegister2;
        case 0x03: return CPU.Registers.ExtendedRegister3;
        case 0x04: return CPU.Registers.ExtendedRegister4;
        case 0x05: return CPU.Registers.ExtendedRegister5;
        case 0x06: return CPU.Registers.ExtendedRegister6;
        case 0x07: return CPU.Registers.ExtendedRegister7;
        default: __illegal_instruction_assert__(false);
        }
    case 0x32:
        switch (TargetInformation.RegisterIndex) {
        case 0x00: return CPU.Registers.HalfExtendedRegister0;
        case 0x01: return CPU.Registers.HalfExtendedRegister1;
        case 0x02: return CPU.Registers.HalfExtendedRegister2;
        case 0x03: return CPU.Registers.HalfExtendedRegister3;
        case 0x04: return CPU.Registers.HalfExtendedRegister4;
        case 0x05: return CPU.Registers.HalfExtendedRegister5;
        case 0x06: return CPU.Registers.HalfExtendedRegister6;
        case 0x07: return CPU.Registers.HalfExtendedRegister7;
        default: __illegal_instruction_assert__(false);
        }

    case 0x64:
        switch (TargetInformation.RegisterIndex) {
        case 0x00: return CPU.Registers.FullyExtendedRegister0;
        case 0x01: return CPU.Registers.FullyExtendedRegister1;
        case 0x02: return CPU.Registers.FullyExtendedRegister2;
        case 0x03: return CPU.Registers.FullyExtendedRegister3;
        case 0x04: return CPU.Registers.FullyExtendedRegister4;
        case 0x05: return CPU.Registers.FullyExtendedRegister5;
        case 0x06: return CPU.Registers.FullyExtendedRegister6;
        case 0x07: return CPU.Registers.FullyExtendedRegister7;
        case R_StackPointer: return CPU.Registers.StackPointer;
        case R_DataPointer: return CPU.Registers.DataPointer;
        case R_ExtendedSegmentPointer: return CPU.Registers.ExtendedSegmentPointer;
        default: __illegal_instruction_assert__(false);
        }
    default: __illegal_instruction_assert__(false);
    }
}

void processor::Target::do_set_register(uint64_t reg)
{
    std::lock_guard<std::mutex> lock(CPU.RegisterAccessMutex);
    switch (TargetWidth) {
    case 0x08:
        switch (TargetInformation.RegisterIndex) {
        case 0x00: CPU.Registers.R0 = (uint8_t)reg; return;
        case 0x01: CPU.Registers.R1 = (uint8_t)reg; return;
        case 0x02: CPU.Registers.R2 = (uint8_t)reg; return;
        case 0x03: CPU.Registers.R3 = (uint8_t)reg; return;
        case 0x04: CPU.Registers.R4 = (uint8_t)reg; return;
        case 0x05: CPU.Registers.R5 = (uint8_t)reg; return;
        case 0x06: CPU.Registers.R6 = (uint8_t)reg; return;
        case 0x07: CPU.Registers.R7 = (uint8_t)reg; return;
        default: __illegal_instruction_assert__(false);
        }
    case 0x16:
        switch (TargetInformation.RegisterIndex) {
        case 0x00: CPU.Registers.ExtendedRegister0 = (uint16_t)reg; return;
        case 0x01: CPU.Registers.ExtendedRegister1 = (uint16_t)reg; return;
        case 0x02: CPU.Registers.ExtendedRegister2 = (uint16_t)reg; return;
        case 0x03: CPU.Registers.ExtendedRegister3 = (uint16_t)reg; return;
        case 0x04: CPU.Registers.ExtendedRegister4 = (uint16_t)reg; return;
        case 0x05: CPU.Registers.ExtendedRegister5 = (uint16_t)reg; return;
        case 0x06: CPU.Registers.ExtendedRegister6 = (uint16_t)reg; return;
        case 0x07: CPU.Registers.ExtendedRegister7 = (uint16_t)reg; return;
        default: __illegal_instruction_assert__(false);
        }
    case 0x32:
        switch (TargetInformation.RegisterIndex) {
        case 0x00: CPU.Registers.HalfExtendedRegister0 = (uint32_t)reg; return;
        case 0x01: CPU.Registers.HalfExtendedRegister1 = (uint32_t)reg; return;
        case 0x02: CPU.Registers.HalfExtendedRegister2 = (uint32_t)reg; return;
        case 0x03: CPU.Registers.HalfExtendedRegister3 = (uint32_t)reg; return;
        case 0x04: CPU.Registers.HalfExtendedRegister4 = (uint32_t)reg; return;
        case 0x05: CPU.Registers.HalfExtendedRegister5 = (uint32_t)reg; return;
        case 0x06: CPU.Registers.HalfExtendedRegister6 = (uint32_t)reg; return;
        case 0x07: CPU.Registers.HalfExtendedRegister7 = (uint32_t)reg; return;
        default: __illegal_instruction_assert__(false);
        }

    case 0x64:
        switch (TargetInformation.RegisterIndex) {
        case 0x00: CPU.Registers.FullyExtendedRegister0 = reg; return;
        case 0x01: CPU.Registers.FullyExtendedRegister1 = reg; return;
        case 0x02: CPU.Registers.FullyExtendedRegister2 = reg; return;
        case 0x03: CPU.Registers.FullyExtendedRegister3 = reg; return;
        case 0x04: CPU.Registers.FullyExtendedRegister4 = reg; return;
        case 0x05: CPU.Registers.FullyExtendedRegister5 = reg; return;
        case 0x06: CPU.Registers.FullyExtendedRegister6 = reg; return;
        case 0x07: CPU.Registers.FullyExtendedRegister7 = reg; return;
        case 0x08: CPU.Registers.FullyExtendedRegister8 = reg; return;
        case 0x09: CPU.Registers.FullyExtendedRegister9 = reg; return;
        case 0x0a: CPU.Registers.FullyExtendedRegister10 = reg; return;
        case 0x0b: CPU.Registers.FullyExtendedRegister11 = reg; return;
        case 0x0c: CPU.Registers.FullyExtendedRegister12 = reg; return;
        case 0x0d: CPU.Registers.FullyExtendedRegister13 = reg; return;
        case 0x0e: CPU.Registers.FullyExtendedRegister14 = reg; return;
        case 0x0f: CPU.Registers.FullyExtendedRegister15 = reg; return;
        case R_StackPointer: CPU.Registers.StackPointer = reg; return;
        case R_DataPointer: CPU.Registers.DataPointer = reg; return;
        case R_ExtendedSegmentPointer: CPU.Registers.ExtendedSegmentPointer = reg; return;
        default: __illegal_instruction_assert__(false);
        }
    default: __illegal_instruction_assert__(false);
    }
}

void processor::Target::set_target_content_in_u64bit_t(uint64_t val)
{
    uint8_t actual_width = 8;
    switch (TargetWidth) {
    case 0x08: actual_width = 1; break;
    case 0x16: actual_width = 2; break;
    case 0x32: actual_width = 4; break;
    case 0x64: actual_width = 8; break;
    default: __illegal_instruction_assert__(false);
    }

    switch (TargetType)
    {
    case TypeRegister: do_set_register(val); break;
    case TypeConstant: __illegal_instruction_assert__(false);
    case TypeMemory: CPU.write_memory(TargetInformation.MemoryAddress, (char*)&val, actual_width); break;
    default: __illegal_instruction_assert__(false);
    }
}

uint64_t processor::Target::get_target_content_in_u64bit_t()
{
    switch (TargetType)
    {
    case TypeRegister: return do_get_register();
    case TypeConstant: return TargetInformation.ConstantValue;
    case TypeMemory:
        uint64_t Value;
        CPU.get_memory(TargetInformation.MemoryAddress, (char*)&Value, sizeof(Value));
        return Value;
    default: __illegal_instruction_assert__(false);
    }
}

processor::Target::Target(processor & _CPU) : CPU(_CPU)
{
    const auto prefix = CPU.pop<8>();
    do_decode_via_prefix(prefix);
    literal = "<" + literal + ">";
}