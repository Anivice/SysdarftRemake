#ifndef DEBUG_H
#define DEBUG_H

#include <string>
#include <mutex>
#include <regex>
#include <type_traits>
#include <map>
#include <unordered_map>

namespace debug
{
    /**
     * @brief Represents the output and status of a command execution.
     *
     * The `cmd_status` structure is used to encapsulate the results of executing a system command
     * through the `_exec_command` function.
     * It includes separate fields to store the standard output
     * and standard error generated by the command, as well as the exit status returned by the system
     * upon completion of the command.
     *
     * The structure fields are:
     * - `fd_stdout`: A string to hold the standard output (stdout) from the executed command.
     * - `fd_stderr`: A string intended to capture the standard error (stderr) output. This includes
     *   both the stderr directly from the command and any errors encountered during the execution process
     *   by `_exec_command`.
     * - `exit_status`: An integer representing the exit code of the command. A value of 0 typically
     *   indicates success, whereas a non-zero value indicates an error or abnormal termination. If an
     *   error occurs during command execution handling within `_exec_command`, this field is set to 1.
     *
     * Usage Example:
     * @code
     * cmd_status result = _exec_command("ls", {"-l", "/var/log"});
     * std::cout << "STDOUT: " << result.fd_stdout << std::endl;
     * std::cout << "STDERR: " << result.fd_stderr << std::endl;
     * std::cout << "Exit Status: " << result.exit_status << std::endl;
     * @endcode
     *
     * @see _exec_command
     */
    struct cmd_status
    {
        std::string fd_stdout;  // normal output
        std::string fd_stderr;  // error information
        int exit_status{};      // exit status
    };

    /**
     * @brief Executes a system command and captures its output and exit status.
     *
     * This function executes a specified system command with given arguments, capturing the command's
     * standard output (stdout) and standard error (stderr) into separate buffers. It also captures
     * the exit status of the command to determine whether it executed successfully. The command output
     * and status are returned in a structured format defined by `cmd_status`.
     *
     * Usage of pipes and forking ensures that stdout and stderr are captured separately and reliably,
     * even if the command outputs simultaneously to both streams. This function is intended to be used
     * where detailed output from system commands is needed for further processing or error handling.
     *
     * @param cmd The command to execute, specified as a string. This should be a path to the executable or
     *            a system command accessible in the system's PATH.
     * @param args A vector of strings representing the arguments to pass to the command. These are passed
     *             directly to the command as its runtime arguments.
     *
     * @return Returns a `cmd_status` structure containing:
     *         - `fd_stdout`: A string that captures all output directed to stdout by the command.
     *         - `fd_stderr`: A string that captures all output directed to stderr. This includes any
     *           error messages that might occur during the execution of the command or in the setup
     *           of execution environments (e.g., pipes, forks).
     *         - `exit_status`: An integer representing the exit code returned by the command. A value
     *           of 0 typically indicates that the command executed successfully. Any non-zero value
     *           indicates an error or abnormal termination, with -1 indicating an error in capturing
     *           the exit status itself.
     *
     * @note If there is an error in setting up the execution environment (e.g., creating pipes, forking),
     *       `fd_stderr` will contain the error description, and `exit_status` will be set to 1.
     *
     * Example:
     * @code
     * cmd_status result = _exec_command("/usr/bin/ls", {"-l", "/var/log"});
     * std::cout << "STDOUT: " << result.fd_stdout << std::endl;
     * std::cout << "STDERR: " << result.fd_stderr << std::endl;
     * std::cout << "Exit Status: " << result.exit_status << std::endl;
     * @endcode
     */
    cmd_status _exec_command(const std::string& cmd, const std::vector<std::string>& args);

    /**
     * @brief Executes a system command with variadic string arguments.
     *
     * This template function simplifies the execution of system commands by allowing direct
     * passing of arguments without manually constructing a vector. It accepts a command and
     * any number of string arguments, which are then forwarded as a vector to the `_exec_command`
     * function for execution. This function is ideal for use cases where the command and its
     * parameters are not predetermined and need to be specified at runtime.
     *
     * @tparam Strings Variadic template parameter pack for any number of string arguments.
     *
     * @param cmd The command to be executed. This should be the path to the executable or a command
     *            accessible from the system's PATH.
     * @param args Variadic parameters, each a std::string or string-literal, representing the arguments
     *             to be passed to the command.
     *
     * @return Returns a `cmd_status` structure containing the outputs and exit status of the executed
     *         command. The structure includes:
     *         - `fd_stdout`: String capturing the standard output.
     *         - `fd_stderr`: String capturing the standard error, which includes errors during command
     *           execution and environment setup.
     *         - `exit_status`: Integer exit code from the command, where 0 typically indicates success.
     *
     * Example Usage:
     * @code
     * cmd_status result = exec_command("/bin/echo", "Hello,", "world!");
     * std::cout << "STDOUT: " << result.fd_stdout << std::endl; // Outputs: Hello, world!
     * std::cout << "Exit Status: " << result.exit_status << std::endl;
     * @endcode
     *
     * @note This function leverages perfect forwarding to efficiently pass arguments to the underlying
     *       execution mechanism, thus it is essential to provide either temporaries or explicitly moved
     *       objects if avoiding unnecessary copies is critical.
     */
    template < typename... Strings >
    cmd_status exec_command(const std::string& cmd, Strings&&... args);

    /**
     * @brief Retrieves the current date and time formatted with high precision.
     *
     * This function captures the current date and time, including milliseconds, and formats it into
     * a standard readable string. The current time is obtained from the system clock, formatted according
     * to the ISO 8601 standard, extended to include milliseconds. This precision is useful for logging
     * and time-stamping events in debugging and monitoring where finer granularity is required.
     *
     * The date and time are formatted as "YYYY-MM-DD HH:MM:SS.sssssssssssss", where the fractional part
     * represents milliseconds extended to a fixed width of 13 digits for consistency. This fixed width
     * includes padding with zeros as necessary.
     *
     * Utilizes the C++ standard library's <chrono> and <iomanip> facilities to handle time and formatting,
     * ensuring portability and accuracy. The function is designed to be thread-safe on platforms where
     * `std::localtime` is thread-safe or if `std::localtime` is replaced with a thread-safe alternative
     * like `localtime_r` in environments where it is available.
     *
     * Example Usage:
     * @code
     * std::string currentDateTime = debug::get_current_date_time();
     * std::cout << "Current Date-Time: " << currentDateTime << std::endl;
     * @endcode
     *
     * @return std::string Formatted current date and time as a string.
     */
    std::string get_current_date_time();

    /**
     * @brief Trait to determine if a type is a container.
     *
     * This template trait checks whether a type is a container by verifying if it has
     * `std::begin` and `std::end` member functions or free functions. It evaluates to
     * `true` for types like `std::vector`, `std::list`, etc., and `false` for non-container types.
     *
     * @tparam T The type to be checked.
     */
    template <typename T, typename = void>
    struct is_container : std::false_type { };

    template <typename T>
    struct is_container<T, std::void_t<decltype(std::begin(std::declval<T>())),
                                       decltype(std::end(std::declval<T>()))>> : std::true_type { };

    template <typename T>
    constexpr bool is_container_v = is_container<T>::value;

    /**
     * @brief Trait to determine if a type is a `std::map`.
     *
     * This specialization checks whether a type is specifically a `std::map` or not.
     * It evaluates to `true` for `std::map<Key, Value>` and `false` for other types.
     *
     * @tparam T The type to be checked.
     */
    template <typename T>
    struct is_map : std::false_type { };

    template <typename Key, typename Value>
    struct is_map<std::map<Key, Value>> : std::true_type { };

    template <typename T>
    constexpr bool is_map_v = is_map<T>::value;

    /**
     * @brief Trait to determine if a type is a `std::unordered_map`.
     *
     * This specialization checks whether a type is specifically a `std::unordered_map` or not.
     * It evaluates to `true` for `std::unordered_map<Key, Value>` and `false` for other types.
     *
     * @tparam T The type to be checked.
     */
    template <typename T>
    struct is_unordered_map : std::false_type { };

    template <typename Key, typename Value, typename Hash, typename KeyEqual, typename Allocator>
    struct is_unordered_map<std::unordered_map<Key, Value, Hash, KeyEqual, Allocator>> : std::true_type { };

    template <typename T>
    constexpr bool is_unordered_map_v = is_unordered_map<T>::value;

    /**
     * @brief Trait to determine if a type is a string.
     *
     * This specialization checks if a type is either a `std::string` or a `const char*`.
     * It evaluates to `true` for string-like types and `false` for all other types.
     *
     * @tparam T The type to be checked.
     */
    template <typename T>
    struct is_string : std::false_type { };

    template <>
    struct is_string<std::string> : std::true_type { };

    template <>
    struct is_string<const char*> : std::true_type { };

    template <typename T>
    constexpr bool is_string_v = is_string<T>::value;

    /**
     * @brief Prints the contents of a generic container to the standard output.
     *
     * This function iterates over the elements of a container (e.g., `std::vector`, `std::list`)
     * and prints each element, separated by commas, enclosed in square brackets (`[ ]`).
     *
     * @tparam Container The container type. Must satisfy the `is_container` trait.
     *
     * @param container The container whose contents will be printed.
     *
     * @note This function is not suitable for `std::map`. Use the specialized version for maps instead.
     */
    template <typename Container>
    std::enable_if_t <
        is_container_v<Container>
        && !is_map_v<Container>
        && !is_unordered_map_v<Container>,
    void > print_container(const Container& container);

    /**
     * @brief Prints the contents of a `std::map` to the standard output.
     *
     * This function iterates over the key-value pairs in a `std::map` and prints each pair
     * in the format `key: value`, separated by commas, enclosed in curly braces (`{ }`).
     *
     * @tparam Map The map type. Must satisfy the `is_map` trait.
     *
     * @param map The map whose contents will be printed.
     */
    template <typename Map>
    std::enable_if_t < is_map_v<Map> || is_unordered_map_v<Map>, void >
    print_container(const Map& map);

    /**
     * @brief Logs a single parameter to the standard output.
     *
     * This function determines the type of the parameter and logs it appropriately:
     * - If the parameter is a string (`std::string` or `const char*`), it is output directly.
     * - If the parameter is a container (e.g., `std::vector`), its contents are printed.
     * - If the parameter is a `std::map`, its key-value pairs are printed.
     * - For all other types, the parameter is output using `std::cout`.
     *
     * @tparam ParamType The type of the parameter to be logged.
     *
     * @param param The parameter to be logged.
     */
    template <typename ParamType>
    void _log(const ParamType& param);

    /**
     * @brief Logs multiple parameters to the standard output.
     *
     * This function recursively logs each parameter to the standard output. It is
     * primarily used internally by the public `log` function to handle variadic arguments.
     *
     * @tparam ParamType The type of the first parameter to be logged.
     * @tparam Args Variadic template parameter pack for the remaining arguments.
     *
     * @param param The first parameter to be logged.
     * @param args The remaining parameters to be logged.
     *
     * @note This function is not intended to be called directly. Use the public `log` function instead.
     */
    template < typename ParamType, typename... Args >
    void _log(const ParamType& param, const Args&... args);

    /**
     * @brief Mutex to guard log operations for multi-thread environment
     */
    extern std::mutex log_mutex;

    /**
     * @brief Struct to hold detailed information about a program's stack trace.
     *
     * This structure is used to encapsulate detailed stack trace information, which includes both
     * the symbolic names of functions in the call stack and the corresponding memory addresses of
     * these stack frames. The `backtrace_info` struct is primarily used in debugging and error logging
     * scenarios to provide a clear and detailed view of the call stack at a specific point in the
     * execution of a program.
     *
     * Fields:
     * - `backtrace_symbols`: A vector of strings, where each string represents the demangled name of
     *   a function at a particular stack frame. These names are extracted using symbol resolution
     *   tools like `backtrace_symbols` in environments that support it, which helps in making the
     *   stack trace human-readable.
     * - `backtrace_frames`: A vector of void pointers, where each pointer addresses a specific stack
     *   frame's location in memory. This allows developers to see not just the function names but
     *   also where in memory these calls are occurring, which can be crucial for low-level debugging
     *   tasks or memory corruption issues.
     *
     * Usage:
     * The `backtrace_info` is typically populated and utilized by functions that capture and process
     * stack traces, such as exception handlers or error logging mechanisms. It can be passed around
     * as a part of exception objects or used directly in logging routines to provide a detailed
     * context for errors and exceptions.
     *
     * Example Usage:
     * @code
     * try {
     *     // Code that might throw
     *     throw std::runtime_error("Forced error for demonstration.");
     * } catch (const std::exception& e) {
     *     backtrace_info bt_info = obtain_stack_frame();
     *     log_backtrace(bt_info);
     *     // Further handling or rethrowing the exception
     * }
     * @endcode
     */
    struct backtrace_info
    {
        std::vector < std::string > backtrace_symbols;
        std::vector < void * > backtrace_frames;
    };

    /**
     * @brief Captures and returns the current stack traces up to a specified maximum depth.
     *
     * This function captures the current stack frames and the corresponding symbols to facilitate
     * debugging by providing a readable backtrace. It uses the `backtrace` and `backtrace_symbols`
     * functions from the execinfo.h library to obtain the raw stack frames and their symbolic
     * representations, respectively. It is designed to help diagnose issues or understand program flow
     * at runtime by logging or displaying the call stack.
     *
     * The function excludes the stack frame of itself to focus on the caller and above, improving
     * clarity in the resulting stack trace information. If the retrieval of backtrace symbols fails,
     * it throws a `BacktraceError` exception.
     *
     * Usage:
     * This function is typically used in exception handling or error logging scenarios to provide
     * detailed context about the state of the call stack at the moment of an error or a critical event.
     *
     * Example Usage:
     * @code
     * try {
     *     // Code that may throw
     * } catch (const std::exception& e) {
     *     auto bt_info = obtain_stack_frame();
     *     log_error("Exception caught: " + std::string(e.what()), bt_info);
     * }
     * @endcode
     *
     * @return backtrace_info A struct containing two vectors:
     *         - `backtrace_symbols_ret`: A vector of strings, each representing the demangled name of a function
     *           in the call stack from the point of capture.
     *         - `backtrace_frames_ret`: A vector of void pointers, each pointing to a stack frame address.
     *
     * @throws BacktraceError if the `backtrace_symbols` function fails to provide a symbol table.
     */
    backtrace_info obtain_stack_frame();

    /**
     * @brief Separates the part of the string before the first '/'.
     *
     * This function extracts and returns the portion of the input string
     * that appears before the first '/' character. If '/' is not found
     * in the string, the entire input string is returned.
     *
     * @param input The input string to be processed.
     * @return A substring of `input` containing all characters before the first '/'.
     */
    std::string separate_before_slash(const std::string& input);

    /**
     * @brief Logs a message with a timestamp to the standard output.
     *
     * This function provides a convenient interface for logging messages with a
     * prepended timestamp. It supports variadic arguments, allowing seamless
     * concatenation of multiple parameters into a single log entry.
     *
     * @tparam Args Variadic template parameter pack for any number of arguments.
     *
     * @param args Variadic parameters representing the message components to be logged.
     *
     * Example Usage:
     * @code
     * debug::log("Info: ", "Application started at port ", 8080, "\n");
     * @endcode
     *
     */
    template<typename... Args>
    void log(const Args&... args);
}

/**
 * @brief SysdarftBaseError is a base error class to which all error classes are derivative
 */
class SysdarftBaseError : public std::runtime_error
{
protected:
    int cur_errno;  // system errno
public:
    /**
     * @brief SysdarftBaseError is not meant to be called by user, but automatically
     * invoked by derivatives.
     *
     * @param msg error message
     * @param if_perform_code_backtrace is backtrace will be performed
     */
    explicit SysdarftBaseError(const std::string& msg, bool if_perform_code_backtrace = true);
};

/**
 * @brief Error class BacktraceError, throw when back trace failed
 */
class BacktraceError final : public SysdarftBaseError
{
    public:
    /**
     * @param msg error message
     */
    explicit BacktraceError(const std::string & msg) : SysdarftBaseError(
        std::string("Backtrace Failed: ") + msg,
        false)
    { }
};

#include "debug.inl"

#endif //DEBUG_H
