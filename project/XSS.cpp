#include <iostream> //Provides input and output functionality like cin,cout,endl.

#include <fstream>  //Provides file input and output functionality like ifstream, ofstream,fstream.

#include <string>   // Provides the std::string class and related functions for manupulation.

#include <algorithm> // Provides a collection of algorithms for working with containers e.g. array. 

#include <cctype> // Provides functions for character classification and transformation. 
// for eg. std::isalpha(): Checks if a character is alphabetic.
// std::isdigit(): Checks if a character is a digit.
// std::tolower(): Converts a character to lowercase.
// std::toupper(): Converts a character to uppercase.

#include <cstring> // Provides C-style string manipulation functions.
		   // std::strlen(): Returns the length of a C-style string.
// std::strcpy(): Copies one string to another.
// std::strcmp(): Compares two strings.

#include <sys/socket.h> //  Provides functions and structures for socket programming (networking) 
			//  socket(): Creates a socket.
// bind(): Binds a socket to an address.
//listen(): Listens for incoming connections.
//accept(): Accepts a connection.

#include <netinet/in.h> //Provides definitions for internet addresses and structures.
			//struct sockaddr_in: Structure for IPv4 addresses.
// htons(): Converts a port number to network byte order.
// inet_addr(): Converts an IP address string to binary format.

#include <unistd.h> // Provides access to POSIX operating system API.
		    // close(): Closes a file descriptor.
// read(): Reads from a file descriptor.
// write(): Writes to a file descriptor.
// sleep(): Suspends execution for a specified number of seconds.

const int PORT = 8080;
const int BUFFER_SIZE = 4096;

// Function to convert a string to lowercase
std::string to_lowercase(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(),
                   [](unsigned char c){ return std::tolower(c); });
    return lower_str;
}

// Function to check for the presence of <script> tags
bool contains_script_tag(const std::string& data) {
    std::string lower_data = to_lowercase(data);
    return lower_data.find("script") != std::string::npos;
}

// Function to read the HTML file content
std::string read_html_file(const std::string& file_path) {
    std::ifstream file(file_path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return "";
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    file.close();
    return content;
}

// Function to send the HTTP response
void send_response(int client_socket, const std::string& content) {
    std::string response = "HTTP/1.1 200 OK\r\n";
    response += "Content-Type: text/html\r\n";
    response += "Content-Length: " + std::to_string(content.size()) + "\r\n";
    response += "\r\n";
    response += content;
    send(client_socket, response.c_str(), response.size(), 0);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    // Create socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the network address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::cout << "Server is running on http://localhost:" << PORT << std::endl;

    while (true) {
        // Accept an incoming connection
        if ((client_socket = accept(server_fd, (struct sockaddr *)&address,
                                 (socklen_t*)&addrlen)) < 0) {
            perror("Accept failed");
            continue;
        }

        // Read the incoming request
        ssize_t valread = read(client_socket, buffer, BUFFER_SIZE - 1);
        if (valread < 0) {
            perror("Read failed");
            close(client_socket);
            continue;
        }
        buffer[valread] = '\0';

        // Convert buffer to std::string for easier manipulation
        std::string request(buffer);

        // Check for <script> tags in the entire request
        if (contains_script_tag(request)) {
            std::cout << "XSS detected in the request." << std::endl;
            // Respond with an error message
            std::string error_message = "<html><body><h1>400 Bad Request</h1><h3>XSS Detectected</h3></body></html>";
            send_response(client_socket, error_message);
        } else {
            // Check if the request is a GET request for the login page
            if (request.find("GET /login") == 0) {
                std::string html_content = read_html_file("login.html");
                if (!html_content.empty()) {
                    send_response(client_socket, html_content);
                } else {
                    std::string error_message = "<html><body><h1>404 Not Found</h1></body></html>";
                    send_response(client_socket, error_message);
                }
            }
            // Check if the request is a POST request for login
            else if (request.find("POST /login") == 0) {
                // Find the end of the headers
                size_t pos = request.find("\r\n\r\n");
                if (pos != std::string::npos) {
                    // Extract the body of the request
                    std::string body = request.substr(pos + 4);



                    // Check for <script> tags in the body
                    if (contains_script_tag(body)) {
                        std::cout << "XSS detected in the request body." << std::endl;
                    } else {
                        std::cout << "No XSS detected in the request body." << std::endl;
                    }

                    // Respond with a simple message
                    std::string response = "<html><body><h1>Login successful</h1></body></html>";
                    send_response(client_socket, response);
                }
            }
        }

        // Close the client socket
        close(client_socket);
    }

    return 0;
}





