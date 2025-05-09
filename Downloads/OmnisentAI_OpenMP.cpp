#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <random>
#include <iomanip>
#include <sstream>  // Required for std::ostringstream
#include <omp.h>    // OpenMP header for parallel constructs
#include <thread>  // Required for std::this_thread


// SensorNode class simulating a sensor device
class SensorNode {
private:
    std::string id;            // Sensor ID
    int sample_rate;           // Sample rate for data generation
    int bit_depth;             // Bit depth of the audio data
    int duration;              // Duration of the audio data
    int sleep_interval;        // Sleep interval between sensor cycles

public:
    // Constructor to initialize the sensor node with default parameters
    SensorNode(const std::string& node_id)
        : id(node_id), sample_rate(400000), bit_depth(24), duration(1), sleep_interval(3600) {
        std::srand(std::time(nullptr)); // Seed the random number generator
    }

    // Method for logging messages with timestamps
    void log(const std::string& msg) {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::cout << "[" << std::put_time(std::localtime(&now), "%F %T") << "] " << msg << std::endl;
    }

    // Method to generate dummy audio data
    std::vector<int> generateAudioData() {
        log("Wake: Generating dummy audio data...");
        int range = (1 << (bit_depth - 1)); // Calculate the range for bit depth
        std::vector<int> data(sample_rate * duration); // Create a vector to hold the audio data

        // Parallelize the audio data generation using OpenMP
        #pragma omp parallel for
        for (size_t i = 0; i < data.size(); ++i) {
            // Fill the data vector with random values within the given range
            data[i] = (std::rand() % (2 * range)) - range;
        }

        return data;
    }

    // Method to compress audio data by reducing the size (4:1 compression)
    std::vector<int> compressData(const std::vector<int>& data) {
        log("Processing: Compressing audio data...");
        std::vector<int> compressed;
        compressed.reserve(data.size() / 4); // Reserve space for the compressed data

        // Parallelize the compression using OpenMP
        #pragma omp parallel for
        for (size_t i = 0; i < data.size(); i += 4) {
            #pragma omp critical // Ensure thread-safe access to the 'compressed' vector
            compressed.push_back(data[i]); // Add every 4th element for 4:1 compression
        }

        return compressed;
    }

    // Method to simulate sending data to the cloud
    bool sendToCloud(const std::vector<int>& data) {
        log("Transmit: Preparing payload...");

        // Manually create a JSON-like payload structure
        std::ostringstream payload;
        payload << "{\n"
                << "  \"sensor_id\": \"" << id << "\",\n"
                << "  \"timestamp\": " << static_cast<long long>(std::time(nullptr)) << ",\n"
                << "  \"audio_data\": [";

        // Append some audio data to the payload (up to 100 elements)
        for (size_t i = 0; i < std::min<size_t>(100, data.size()); ++i) {
            payload << data[i];
            if (i < 99 && i < data.size() - 1) {
                payload << ", ";
            }
        }
        payload << "]\n}";

        // Simulate a 90% success rate for transmission
        bool success = (std::rand() % 10) < 9;

        if (success) {
            log("Transmit: Sending data to cloud...");
            std::cout << payload.str().substr(0, 120) << " ..." << std::endl; // Print part of the payload
        } else {
            log("Transmit: Failed to send data.");
        }

        return success;
    }

    // Method to run a complete cycle for the sensor
    void runCycle() {
        log("---- Sensor Cycle Start ----");

        // Generate audio data and compress it
        std::vector<int> audio = generateAudioData();
        std::vector<int> compressed = compressData(audio);

        // Try to send the compressed data to the cloud
        if (!sendToCloud(compressed)) {
            log("Error: Transmission failed. Logging for retry.");
        }

        log("Sleep: Entering sleep mode...\n");
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulate sleep mode
    }
};

// Main function to simulate running multiple sensor cycles concurrently
int main() {
    // Parallelize the execution of multiple sensor cycles using OpenMP
    #pragma omp parallel for
    for (int i = 0; i < 3; ++i) {
        // Each thread creates and runs a separate sensor node
        SensorNode node("sensor_00" + std::to_string(i+1));
        node.runCycle(); // Run the sensor cycle for each node
    }

    return 0;
}
