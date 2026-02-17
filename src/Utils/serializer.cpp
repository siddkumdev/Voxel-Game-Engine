#include "serializer.h"
#include <fstream>
#include <vector>
#include "voxelizer.h"
#include <iostream>

template<typename T>
void Write(std::ofstream& out, const T& val) {
    out.write((const char*)&val, sizeof(T));
}

template<typename T>
void Read(std::ifstream& in, T& val) {
    in.read((char*)&val, sizeof(T));
}

void WriteString(std::ofstream& out, const std::string& str) {
    int len = (int)str.length();
    Write(out, len);
    if (len > 0) out.write(str.c_str(), len);
}

std::string ReadString(std::ifstream& in) {
    int len;
    Read(in, len);
    if (len <= 0) return "";
    std::string str(len, '\0');
    in.read(&str[0], len);
    return str;
}

bool LevelSerializer::SaveLevel(const std::string& path, const std::vector<Chunk*>& chunks) {
    std::ofstream out(path, std::ios::binary);
    if (!out) return false;

    int count = (int)chunks.size();
    Write(out, count);

    for (Chunk* c : chunks) {
        WriteString(out, c->name);
        Write(out, c->GetResolution());
        
        Write(out, c->position);
        Write(out, c->rotation);
        Write(out, c->scale);
        Write(out, c->isStatic);
        Write(out, c->useGravity);
        Write(out, c->resistance);
        
        Write(out, c->color);
        Write(out, (int)c->m_Type);
        Write(out, c->m_Radius);
        Write(out, c->m_Height);

        WriteString(out, c->m_ModelPath);
    }
    return true;
}

bool LevelSerializer::LoadLevel(const std::string& path, std::vector<Chunk*>& chunks) {
    std::ifstream in(path, std::ios::binary);
    if (!in) return false;

    int count;
    Read(in, count);

    for (int i = 0; i < count; i++) {
        std::string name = ReadString(in);
        int res;
        Read(in, res);

        Chunk* c = new Chunk(res);
        c->name = name;

        Read(in, c->position);
        Read(in, c->rotation);
        Read(in, c->scale);
        Read(in, c->isStatic);
        Read(in, c->useGravity);
        Read(in, c->resistance);
        
        Read(in, c->color);
        
        int type;
        Read(in, type);
        c->m_Type = (ChunkType)type;

        Read(in, c->m_Radius);
        Read(in, c->m_Height);

        c->m_ModelPath = ReadString(in);

        if (c->m_Type == ChunkType::Model && !c->m_ModelPath.empty()) {
            Voxelizer::LoadAndVoxelize(c->m_ModelPath, *c);
        } else {
            c->Rebuild();
        }
        chunks.push_back(c);
    }
    return true;
}
