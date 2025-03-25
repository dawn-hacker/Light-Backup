#include <iostream>
#include <csignal>
#include <cstdlib>
#include <unistd.h>
#include <sys/wait.h>

// Global variable to store the TShark process ID
pid_t tshark_pid = -1;

// Signal handler to handle SIGINT (Ctrl+C)
void signalHandler(int signum) {
    std::cout << "\nInterrupt signal (" << signum << ") received.\n";
    if (tshark_pid != -1) {
        std::cout << "Terminating TShark process (PID: " << tshark_pid << ")...\n";
        // Send SIGTERM to the TShark process
        kill(tshark_pid, SIGTERM);
        // Wait for the TShark process to terminate
        waitpid(tshark_pid, nullptr, 0);
        std::cout << "TShark process terminated.\n";
    }
    exit(signum);
}

int main() {
    // Register the signal handler for SIGINT
    signal(SIGINT, signalHandler);

    // Specify the network interface to capture from (e.g., "eth0")
    std::string interface = "eth0";
    // Specify the output file name
    std::string output_file = "network_capture.pcap";
    // Construct the TShark command
    std::string command = "tshark -i " + interface + " -w " + output_file;

    // Fork the process to create a child process
    tshark_pid = fork();
    if (tshark_pid == -1) {
        std::cerr << "Failed to fork process.\n";
        return 1;
    } else if (tshark_pid == 0) {
        // Child process: Execute TShark
        execl("/bin/sh", "sh", "-c", command.c_str(), (char *)nullptr);
        // If execl returns, there was an error
        std::cerr << "Failed to execute TShark.\n";
        return 1;
    } else {
        // Parent process: Wait for the child process to complete
        std::cout << "TShark is running with PID: " << tshark_pid << "\n";
        int status;
        waitpid(tshark_pid, &status, 0);
        if (WIFEXITED(status)) {
            std::cout << "TShark exited with status " << WEXITSTATUS(status) << ".\n";
        } else if (WIFSIGNALED(status)) {
            std::cout << "TShark was terminated by signal " << WTERMSIG(status) << ".\n";
        }
    }

    std::cout << "Network traffic has been captured and saved to " << output_file << "\n";
    return 0;
}

