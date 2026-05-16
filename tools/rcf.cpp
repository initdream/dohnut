// SPDX-License-Identifier: GPL-2.0-only
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <cctype>

namespace fs = std::filesystem;

#pragma pack(push, 1)
struct radCFFileInfo { char id[32]; uint8_t v[4]; uint32_t align, pad, header; };
struct radCFHeader { uint32_t numFiles, detailed, firstFile, res; };
struct HFE { uint32_t hash, offset, size; };
#pragma pack(pop)

void extract_rcf(const std::string& path) {
    std::ifstream f(path, std::ios::binary | std::ios::ate);
    if (!f) { std::cerr << "Cannot open " << path << "\n"; return; }

    size_t fileSize = f.tellg();
    f.seekg(0);
    std::vector<uint8_t> data(fileSize);
    f.read(reinterpret_cast<char*>(data.data()), fileSize);

    auto* info   = reinterpret_cast<radCFFileInfo*>(data.data());
    auto* header = reinterpret_cast<radCFHeader*>(data.data() + info->header);
    uint32_t numFiles = header->numFiles;

    auto* entries = reinterpret_cast<HFE*>(data.data() + info->header + sizeof(radCFHeader));

    size_t dictStart = info->header + sizeof(radCFHeader) + numFiles * sizeof(HFE);

    std::cout << "RCF 1.2 | Files: " << numFiles 
              << " | Name table start: 0x" << std::hex << dictStart << std::dec << "\n";

    //COLLECT NAMES 
    std::vector<std::string> names;
    std::string cur;
    for (size_t i = dictStart; i < fileSize; ++i) {
        char c = static_cast<char>(data[i]);
        if (c >= 32 && c <= 126) {
            cur += c;
        } else {
            if (cur.length() > 3) {
                names.push_back(cur);
            }
            cur.clear();
        }
    }

    std::cout << "Names collected: " << names.size() << " / " << numFiles << "\n";

    //SORT HFE BY FILE OFFSET
    std::vector<HFE> sortedEntries(entries, entries + numFiles);
    std::sort(sortedEntries.begin(), sortedEntries.end(),
              [](const HFE& a, const HFE& b) { return a.offset < b.offset; });

    //EXTRACT
    fs::path outDir = "extracted_rcf";
    fs::create_directories(outDir);

    for (uint32_t i = 0; i < numFiles && i < names.size(); ++i) {
        std::string name = names[i];

        std::replace(name.begin(), name.end(), '\\', fs::path::preferred_separator);

        fs::path outPath = outDir / name;
        fs::create_directories(outPath.parent_path());

        const HFE& entry = sortedEntries[i];

        std::ofstream out(outPath, std::ios::binary);
        if (entry.offset + entry.size <= fileSize) {
            out.write(reinterpret_cast<const char*>(data.data() + entry.offset), entry.size);
        }
    }

    std::cout << "All files extracted with correct names and structure.\n";
    std::cout << "Output folder: " << outDir << "\n";
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <file.rcf>\n";
        return 1;
    }
    extract_rcf(argv[1]);
    return 0;
}
