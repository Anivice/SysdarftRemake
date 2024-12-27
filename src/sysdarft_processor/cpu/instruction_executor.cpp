#include <cpu.h>
#include <cstdint>

std::mutex interruption_vector_address_table_mutex_;
std::map < uint64_t /* Interruption number */, uint64_t /* Interruption vector address */>
    interruption_vector_address_table;

class __initialize_vector__ {
public:
    __initialize_vector__()
    {
        // Define parameters
        constexpr size_t TOTAL_ENTRIES = 512;

        // Initialize the table
        std::lock_guard lock(interruption_vector_address_table_mutex_);
        for (uint64_t i = 0; i < TOTAL_ENTRIES; i++)
        {
            constexpr uint64_t INCREMENT = 8;
            constexpr uint64_t START_ADDRESS = 0xA0000;
            uint64_t address = START_ADDRESS + i * INCREMENT;
            interruption_vector_address_table.insert(std::make_pair(i, address));
        }
    }
} __initialize_vector_instance__;

// Sample operation function
void processor::operation(__uint128_t timestamp)
{
    switch (pop<64>())
    {
        case 0x00: InstructionExecutor.nop(); break;
        case 0x01: InstructionExecutor.add(); break;
        default: soft_interruption_ready(INT_ILLEGAL_INSTRUCTION); break;
    }
}

void processor::soft_interruption_ready(const uint64_t int_code)
{
}

processor::Target processor::__InstructionExecutorType__::pop_target()
{
    return Target(CPU);
}

void processor::__InstructionExecutorType__::nop()
{
    debug::log("[PROCESSOR]:\tNOP\n");
    // No Operation
}

void processor::__InstructionExecutorType__::add()
{
    auto width = CPU.pop<8>();
    auto operand1 = pop_target();
    auto operand2 = pop_target();
    debug::log("[PROCESSOR]:\tADD ", operand1.literal, ", ", operand2.literal, "\n");
    operand1 = operand1.get<uint64_t>() + operand2.get<uint64_t>();
}

void processor::__InstructionExecutorType__::pushall()
{
}