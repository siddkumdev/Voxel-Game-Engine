#include "serializer.h"
#include <fstream>
#include <iostream>
#include "voxelizer.h" // Needed to reload models

bool LevelSerializer::SaveLevel(const std::string& path, const std::vector<Chunk*>& chunks) {
    std::ofstream out(path, std::ios::binary);
    if (!out.is_open()) return false;

    int count = chunks.size();
    out.write((char*)&count, sizeof(int));

    for (Chunk* c : chunks) {
        // 1. Name
        int nameLen = c->name.length();
        out.write((char*)&nameLen, sizeof(int));
        out.write(c->name.c_str(), nameLen);

        // 2. Resolution
        int res = c->GetResolution(); 
        out.write((char*)&res, sizeof(int));

        // 3. Physics & Transform
        out.write((char*)&c->physicsBody.position, sizeof(glm::vec3));
        out.write((char*)&c->physicsBody.rotation, sizeof(glm::vec3));
        out.write((char*)&c->physicsBody.scale, sizeof(glm::vec3)); 
        
        out.write((char*)&c->physicsBody.isStatic, sizeof(bool));
        out.write((char*)&c->physicsBody.useGravity, sizeof(bool));
        out.write((char*)&c->physicsBody.resistance, sizeof(float)); 
        
        // 4. Visuals
        out.write((char*)&c->color, sizeof(glm::vec3));

        // 5. Generation Data
        int typeInt = (int)c->m_Type;
        out.write((char*)&typeInt, sizeof(int));
        out.write((char*)&c->m_Radius, sizeof(int));
        out.write((char*)&c->m_Height, sizeof(int));

        // 6. Model Path (NEW)
        // We always save this, even if empty, to keep the format simple
        int pathLen = c->m_ModelPath.length();
        out.write((char*)&pathLen, sizeof(int));
        if (pathLen > 0) {
            out.write(c->m_ModelPath.c_str(), pathLen);
        }
    }

    out.close();
    return true;
}

bool LevelSerializer::LoadLevel(const std::string& path, std::vector<Chunk*>& chunks) {
    std::ifstream in(path, std::ios::binary);
    if (!in.is_open()) return false;

    int count;
    in.read((char*)&count, sizeof(int));

    for (int i = 0; i < count; i++) {
        // 1. Name
        int nameLen;
        in.read((char*)&nameLen, sizeof(int));
        std::string name(nameLen, '\0');
        in.read(&name[0], nameLen);

        // 2. Resolution
        int res;
        in.read((char*)&res, sizeof(int));

        Chunk* c = new Chunk(res);
        c->name = name;

        // 3. Physics & Transform
        in.read((char*)&c->physicsBody.position, sizeof(glm::vec3));
        in.read((char*)&c->physicsBody.rotation, sizeof(glm::vec3));
        in.read((char*)&c->physicsBody.scale, sizeof(glm::vec3)); 
        
        in.read((char*)&c->physicsBody.isStatic, sizeof(bool));
        in.read((char*)&c->physicsBody.useGravity, sizeof(bool));
        in.read((char*)&c->physicsBody.resistance, sizeof(float)); 

        // 4. Visuals
        in.read((char*)&c->color, sizeof(glm::vec3));

        // 5. Generation Data
        int typeInt;
        in.read((char*)&typeInt, sizeof(int));
        c->m_Type = (ChunkType)typeInt;
        
        in.read((char*)&c->m_Radius, sizeof(int));
        in.read((char*)&c->m_Height, sizeof(int));

        // 6. Model Path (NEW)
        int pathLen;
        in.read((char*)&pathLen, sizeof(int));
        if (pathLen > 0) {
            std::string modelPath(pathLen, '\0');
            in.read(&modelPath[0], pathLen);
            c->m_ModelPath = modelPath;
        }

        // 7. Rebuild Logic
        if (c->m_Type == ChunkType::Model && !c->m_ModelPath.empty()) {
            // RELOAD THE MODEL FROM DISK
            Voxelizer::LoadAndVoxelize(c->m_ModelPath, *c);
        } else {
            // Rebuild procedural shape
            c->Rebuild();
        }
        
        chunks.push_back(c);
    }

    in.close();
    return true;
}