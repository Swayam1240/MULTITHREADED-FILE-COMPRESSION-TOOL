#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <zlib.h>

using namespace std;

const int CHUNK_SIZE = 262144; 
const int NUM_THREADS = 4;

void compressChunk(const vector<char>& input, vector<char>& output) {
    uLongf compressedSize = compressBound(input.size());
    output.resize(compressedSize);

    if (compress(reinterpret_cast<Bytef*>(output.data()), &compressedSize, 
                 reinterpret_cast<const Bytef*>(input.data()), input.size()) != Z_OK) {
        cout << "COMPRESSED FAILED" << endl;
    }
    output.resize(compressedSize);
}

void decompressChunk(const vector<char>& input, vector<char>& output, uLongf originalSize) {
    output.resize(originalSize);

    if (uncompress(reinterpret_cast<Bytef*>(output.data()), &originalSize, 
                   reinterpret_cast<const Bytef*>(input.data()), input.size()) != Z_OK) {
        cout << "DECOMPRESSED FAILED" << endl;
    }
}

void compressFile(const string& inputFile, const string& outputFile) {
    ifstream in(inputFile, ios::binary);
    ofstream out(outputFile, ios::binary);

    vector<thread> threads;
    vector<vector<char>> chunks, compressedChunks;

    while (!in.eof()) {
        vector<char> chunk(CHUNK_SIZE);
        in.read(chunk.data(), CHUNK_SIZE);
        chunk.resize(in.gcount());

        if (chunk.empty()) break;
        chunks.push_back(move(chunk));
    }

    compressedChunks.resize(chunks.size()); 

    for (size_t i = 0; i < chunks.size(); ++i) {
        threads.emplace_back(compressChunk, ref(chunks[i]), ref(compressedChunks[i]));
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (const auto& compressed : compressedChunks) {
        out.write(compressed.data(), compressed.size());
    }
}

void decompressFile(const string& inputFile, const string& outputFile, uLongf originalSize) {
    ifstream in(inputFile, ios::binary);
    ofstream out(outputFile, ios::binary);

    vector<thread> threads;
    vector<vector<char>> chunks, decompressedChunks;

    while (!in.eof()) {
        vector<char> chunk(CHUNK_SIZE);
        in.read(chunk.data(), CHUNK_SIZE);
        chunk.resize(in.gcount());
        if (chunk.empty()) break;
        chunks.push_back(move(chunk));
    }
    decompressedChunks.resize(chunks.size());

    for (size_t i = 0; i < chunks.size(); ++i) {
        threads.emplace_back(decompressChunk, ref(chunks[i]), ref(decompressedChunks[i]), originalSize);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    for (const auto& decompressed : decompressedChunks) {
        out.write(decompressed.data(), decompressed.size());
    }
}

int main() {
    string input = "input.txt";
    string compressed = "compressed.bin";
    string decompressed = "decompressed.txt";

    compressFile(input, compressed);
    cout << "Compression Done!" << endl;

    uLongf originalSize = 1000000; 
    decompressFile(compressed, decompressed, originalSize);
    cout << "Decompression Done!" << endl;

    return 0;
}
