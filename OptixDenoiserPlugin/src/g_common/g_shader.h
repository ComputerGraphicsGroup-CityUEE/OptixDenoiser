#ifndef G_SHADER_H
#define G_SHADER_H

#include <map>
#include <string>
using namespace std;

class GShader
{
  public:
    unsigned int program;
    map<string, int> id;
    char vspath[256];
    char fspath[256];

    GShader();
    ~GShader();
    void load(const char *vertex_path, const char *fragment_path);
    void use();
    void unuse();
    void reload();

    int& operator[]( const char *key );
};

#endif
