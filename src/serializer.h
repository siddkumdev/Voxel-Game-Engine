#pragma once
#include <vector>
#include <string>
#include "chunk.h"

class LevelSerializer {
public:
    static bool SaveLevel(const std::string& filename, const std::vector<Chunk*>& chunks);
    static bool LoadLevel(const std::string& filename, std::vector<Chunk*>& outChunks);
};