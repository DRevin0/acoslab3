#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <thread>
#include <vector>
#include "ThreadPool.hpp"

struct FileStatus {
    std::string filename;
    int lines = 0;
    int words = 0;
    int chars = 0;
};

FileStatus analyze_file(const std::string& filepath) {
    FileStatus stats;
    stats.filename = filepath;
    
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return stats;
    }

    std::string line;
    while (std::getline(file, line)) {
        stats.lines++;
        stats.chars += line.length();
        
        std::istringstream iss(line);
        std::string word;
        while (iss >> word) {
            stats.words++;
        }
    }
    file.close();
    return stats;
}


int main(int argc, char* argv[]) {
    int numThreads = std::thread::hardware_concurrency();
    if (numThreads == 0) numThreads = 2;
    
    std::vector<std::string> files;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--threads" && i + 1 < argc) {
            numThreads = std::stoi(argv[i + 1]);
            i++;
        } else {
            files.push_back(arg);
        }
    }

    if (files.empty()) {
        for (int i = 1; i <= 5; ++i) {
            files.push_back("test_file_" + std::to_string(i) + ".txt");
        }
    }
    
    std::cout << "Starting parallel text processing with " 
              << numThreads << " threads..." << std::endl;
    std::cout << "Processing " << files.size() << " files..." << std::endl;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    ThreadPool<FileStatus> pool(numThreads, analyze_file); 
    
    for (const auto& file : files) {
        pool.submit(Task{file});
    }
    
    int totalLines = 0;
    int totalWords = 0;
    int totalChars = 0;
    int processedFiles = 0;


    while (processedFiles < files.size()) {
        auto resultOpt = pool.getResult();
        if (resultOpt) {
            if (resultOpt->lines < 100) {
                std::cout << "\033[32m";  // зелёный
            } else if (resultOpt->lines < 1000) {
                std::cout << "\033[33m";  // желтый
            } else {
                std::cout << "\033[31m";  // красный
            }
            
            std::cout << resultOpt->filename << ": "
                      << resultOpt->lines << " lines, "
                      << resultOpt->words << " words, "
                      << resultOpt->chars << " chars"
                      << "\033[0m" << std::endl;
            
            totalLines += resultOpt->lines;
            totalWords += resultOpt->words;
            totalChars += resultOpt->chars;
            processedFiles++;
        }
    }
    
    pool.stop();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    
    std::cout << "\n===== RESULTS =====" << std::endl;
    std::cout << "Total lines: " << totalLines << std::endl;
    std::cout << "Total words: " << totalWords << std::endl;
    std::cout << "Total chars: " << totalChars << std::endl;
    std::cout << "Files processed: " << processedFiles << std::endl;
    std::cout << "Threads used: " << numThreads << std::endl;
    std::cout << "Time taken: " << duration.count() << " ms" << std::endl;
    return 0;
}
