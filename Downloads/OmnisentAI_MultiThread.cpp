#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <thread>
#include <iomanip>
#include <sstream>

class SensorNode {
private:
    std::string id;  // Sensor ID
    int sample_rate; // Sampling rate for audio data
    int bit_depth;   // Bit depth of audio data
    int duration;    // Duration for audio generation (seconds)
    int sleep_interval; // Sleep interval between sensor cycles

public:
    // Constructor to initialize the SensorNode with a given ID and default values
    SensorNode(const std::string& node_id)
        : id(node_id), sample_rate(400000), bit_depth(24), duration(1), sleep_interval(3600) {
        std::srand(static_cast<unsigned>(std::time(nullptr))); // Seed random number generator with current time
    }

    // Log function to print messages with timestamps
    void log(const std::string& msg) {
        auto now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        std::cout << "[" << std::put_time(std::localtime(&now), "%F %T") << "] " << msg << std::endl;
    }

    // Parallel function to generate dummy audio data
    void generateAudioData(std::vector<int>& data) {
        log("Wake: Generating dummy audio data...");
        int range = (1 << (bit_depth - 1)); // Calculate range based on bit depth

        // Fill the data vector with random audio data values
        for (size_t i = 0; i < data.size(); ++i) {
            data[i] = (std::rand() % (2 * range)) - range; // Random values between -range and range
        }
    }

    // Parallel function to compress the audio data (4:1 compression)
    void compressData(const std::vector<int>& data, std::vector<int>& compressed) {
        log("Processing: Compressing audio data...");
        // Compress audio data by picking every 4th sample (4:1 compression)
        for (size_t i = 0; i < data.size(); i += 4) {
            compressed.push_back(data[i]);
        }
    }

    // Simulate sending compressed data to the cloud
    bool sendToCloud(const std::vector<int>& data) {
        log("Transmit: Preparing payload...");

        std::ostringstream payload;  // Create a string stream to build the JSON payload
        payload << "{ \"sensor_id\": \"" << id << "\", "
                << "\"timestamp\": " << std::time(nullptr) << ", "
                << "\"audio_data\": [";

        // Add the first 100 audio data points to the payload
        for (size_t i = 0; i < std::min<size_t>(100, data.size()); ++i) {
            if (i > 0) payload << ", ";
            payload << data[i];
        }

        payload << "] }";

        // Simulate a 90% success rate for sending data
        bool success = (std::rand() % 10) < 9;

        if (success) {
            log("Transmit: Sending data to cloud...");
            std::cout << payload.str().substr(0, 120) << " ..." << std::endl; // Display a portion of the payload
        } else {
            log("Transmit: Failed to send data.");
        }

        return success;
    }

    // Run one cycle of the sensor: generate data, compress, and send to cloud
    void runCycle() {
        log("---- Sensor Cycle Start ----");

        std::vector<int> audio(sample_rate * duration); // Vector to store generated audio data
        std::vector<int> compressed; // Vector to store compressed data

        // Create threads for parallelism: one for data generation and one for data compression
        std::thread t1(&SensorNode::generateAudioData, this, std::ref(audio));
        std::thread t2(&SensorNode::compressData, this, std::cref(audio), std::ref(compressed));

        t1.join(); // Wait for audio data generation to finish
        t2.join(); // Wait for compression to finish

        // Attempt to send compressed data to the cloud and log success/failure
        if (!sendToCloud(compressed)) {
            log("Error: Transmission failed. Logging for retry.");
        }

        log("Sleep: Entering sleep mode...\n");
        std::this_thread::sleep_for(std::chrono::seconds(2)); // Simulate sleep between cycles
    }
};

int main() {
    // Create a sensor node with a specific ID
    SensorNode node("sensor_001");

    // Run 3 sensor cycles
    for (int i = 0; i < 3; ++i) {
        node.runCycle();
    }

    return 0;
}
