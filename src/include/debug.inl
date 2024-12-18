#ifndef DEBUG_INL
#define DEBUG_INL

#include <iostream>
#include <sstream>
#include <cstring>
#include <exception>
#include <concepts>
#include <string_view>

template <typename... Strings>
debug::cmd_status debug::exec_command(const std::string &cmd,
                                      Strings &&...args)
{
    const std::vector<std::string> vec{std::forward<Strings>(args)...};
    return _exec_command(cmd, vec);
}

template <typename Container>
std::enable_if_t < debug::is_container_v<Container>
    && !debug::is_map_v<Container>
    && !debug::is_unordered_map_v<Container>
, void >
debug::print_container(const Container &container)
{
    std::cout << "[";
    for (auto it = std::begin(container); it != std::end(container); ++it)
    {
        std::cout << *it;
        if (std::next(it) != std::end(container)) {
            std::cout << ", ";
        }
    }
    std::cout << "]";
}

template <typename Map>
std::enable_if_t < debug::is_map_v<Map> || debug::is_unordered_map_v<Map>, void >
debug::print_container(const Map &map)
{
    std::cout << "{";
    for (auto it = std::begin(map); it != std::end(map); ++it)
    {
        std::cout << it->first << ": " << it->second;
        if (std::next(it) != std::end(map)) {
            std::cout << ", ";
        }
    }
    std::cout << "}";
}

template <typename ParamType> void debug::_log(const ParamType &param)
{
    if constexpr (debug::is_string_v<ParamType>) {
        // Handle std::string and const char*
        std::cout << param;
    } else if constexpr (debug::is_container_v<ParamType>) {
        // Handle containers
        debug::print_container(param);
    } else {
        // Handle other types
        std::cout << param;
    }
}

template <typename ParamType, typename... Args>
void debug::_log(const ParamType &param, const Args &...args)
{
    debug::_log(param);
    (debug::_log(args), ...);
}

template <typename... Args> void debug::log(const Args &...args)
{
    std::lock_guard lock(log_mutex);

    if (verbose)
    {
        // output timestamp
        _log(_CYAN_, _BOLD_, get_current_date_time(), _REGULAR_, ": ");

        // obtain the stack trace
        auto [backtrace_symbols, backtrace_frames] = debug::obtain_stack_frame();

        // extract the caller's function name (third frame in the backtrace)
        std::string caller_function_backtrace_info, addr2line_caller_info;
        if (!backtrace_symbols.empty() && backtrace_symbols.size() > 2) {
            caller_function_backtrace_info = backtrace_symbols[2];
        }

        if (!caller_function_backtrace_info.empty())
        {
            const std::regex pattern(R"(([^\(]+)\(([^\)]*)\) \[([^\]]+)\])");
            std::smatch matches;
            if (std::regex_search(caller_function_backtrace_info, matches,
                                  pattern) &&
                matches.size() > 3)
            {
                const std::string &executable_path = matches[1].str();
                const std::string &traced_address = matches[2].str();
                const std::string &traced_runtime_address = matches[3].str();

                auto generate_addr2line_trace_info =
                    [&](const std::string &address) -> std::string
                    {
                        auto [fd_stdout, fd_stderr, exit_status] =
                            debug::exec_command("addr2line", "--demangle", "-f", "-e",
                                                executable_path, address);

                        if (exit_status != 0) {
                            return "";
                        }

                        return fd_stdout;
                    };

                if (traced_address.empty()) {
                    addr2line_caller_info =
                        generate_addr2line_trace_info(traced_runtime_address);
                } else {
                    addr2line_caller_info =
                        generate_addr2line_trace_info(traced_address);
                }
            }

            std::string caller = separate_before_slash(addr2line_caller_info);
            std::erase(caller, '\n');

            std::string before;

            if (const size_t pos = caller.find('('); pos != std::string::npos) {
                before = caller.substr(0, pos);
            }

            auto shorten_caller_when_fit = [&](const std::string& input,
                                     const std::size_t max_length = 64,
                                     const std::size_t prefix_length = 5,
                                     const std::string& ellipsis = "...")-> std::string
            {
                if (input.size() <= max_length) {
                    return input;
                }

                const std::size_t suffix_length = max_length - prefix_length - ellipsis.size();

                // Create views for prefix and suffix
                const auto prefix = input.substr(0, prefix_length);
                const auto suffix = input.substr(input.size() - suffix_length, suffix_length);

                // Concatenate to form the shortened string
                return prefix + ellipsis + suffix;
            };

            if (!caller.empty())
            {
                if (before.empty()) {
                    _log(_BLUE_, _BOLD_, shorten_caller_when_fit(caller), _REGULAR_, ": ");
                } else {
                    _log(_BLUE_, _BOLD_, shorten_caller_when_fit(before), _REGULAR_, ": ");
                }
            }
        }
    }

    debug::_log(args...);
}

#endif // DEBUG_INL
