#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <bits/this_thread_sleep.h> // for sleep functionality

// SensorNode class represents a sensor with capabilities to generate audio data, compress it, and send it to the cloud.
class SensorNode {
private:
    std::string id;             // Unique identifier for the sensor node.
    int sample_rate;            // Sample rate for audio data generation.
    int bit_depth;              // Bit depth of the audio data.
    int duration;               // Duration of audio data generation (in seconds).
    int sleep_interval;         // Interval for sleep mode (not used in this case but can be extended).

public:
    // Constructor initializing the sensor node with a given ID and default values for other parameters.
    SensorNode(const std::string& node_id)
        : id(node_id), sample_rate(400000), bit_depth(24), duration(1), sleep_interval(3600) {
        std::srand(static_cast<unsigned>(std::time(nullptr))); // Seed for random number generation
    }

    // Function to log messages with timestamps.
    void log(const std::string& msg) {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::cout << "[" << std::put_time(std::localtime(&now), "%F %T") << "] " << msg << std::endl;
    }

    // Function to generate dummy audio data based on the sample rate and bit depth.
    std::vector<int> generateAudioData() {
        log("Wake: Generating dummy audio data...");
        int range = (1 << (bit_depth - 1)); // Calculate range based on bit depth
        std::vector<int> data(sample_rate * duration); // Audio data vector

        // Populate the data vector with random values within the specified range
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = (std::rand() % (2 * range)) - range;
        }

        return data;
    }

    // Function to compress the audio data (simple 4:1 compression for demonstration).
    std::vector<int> compressData(const std::vector<int>& data) {
        log("Processing: Compressing audio data...");
        std::vector<int> compressed;

        // Take every 4th sample for compression (4:1 ratio)
        for (size_t i = 0; i < data.size(); i += 4) {
            compressed.push_back(data[i]); // Add the 4th element to the compressed data
        }

        return compressed;
    }

    // Function to simulate sending the compressed data to the cloud.
    bool sendToCloud(const std::vector<int>& data) {
        log("Transmit: Preparing payload...");

        std::ostringstream payload;
        payload << "{ \"sensor_id\": \"" << id << "\", "
                << "\"timestamp\": " << std::time(nullptr) << ", "
                << "\"audio_data\": [";

        // Add the first 100 audio data samples to the payload
        for (size_t i = 0; i < std::min<size_t>(100, data.size()); ++i) {
            if (i > 0) payload << ", ";
            payload << data[i];
        }

        payload << "] }";

        // Simulate a 90% success rate for the transmission
        bool success = (std::rand() % 10) < 9;

        if (success) {
            log("Transmit: Sending data to cloud...");
            std::cout << payload.str().substr(0, 120) << " ..." << std::endl; // Display a part of the payload
        } else {
            log("Transmit: Failed to send data.");
        }

        return success;
    }

    // The main sensor cycle that generates, compresses, and sends data to the cloud.
    void runCycle() {
        log("---- Sensor Cycle Start ----");

        // Generate audio data
        std::vector<int> audio = generateAudioData();
        
        // Compress the audio data
        std::vector<int> compressed = compressData(audio);

        // Try to send the compressed data to the cloud
        if (!sendToCloud(compressed)) {
            log("Error: Transmission failed. Logging for retry.");
        }

        log("Sleep: Entering sleep mode...\n");
        
        // Simulate sleep mode for 2 seconds before starting the next cycle
        std::this_thread::sleep_for(std::chrono::seconds(2)); 
    }
};

// Main function, entry point of the program
int main() {
    // Create a sensor node with a unique ID
    SensorNode node("sensor_001");

    // Run the sensor cycle 3 times
    for (int i = 0; i < 3; ++i) {
        node.runCycle();
    }

    return 0;
}
