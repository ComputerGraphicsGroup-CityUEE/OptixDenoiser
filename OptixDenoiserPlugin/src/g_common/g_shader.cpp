#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <GL/glew.h>
#include "g_common.h"

#include "g_shader.h"


GShader::GShader()
{
  //memset(this, 0, sizeof(*this));
  program = 0;
  strcpy(vspath,"");
  strcpy(fspath,"");
}
GShader::~GShader()
{
  if (program != 0)
    glDeleteProgram(program);
}

void GShader::load(const char *vertex_path, const char *fragment_path)
{
  strcpy(vspath, vertex_path);
  strcpy(fspath, fragment_path);
  reload();
}

void GShader::reload()
{
  if (program != 0)
    glDeleteProgram(program);

  char* vertex_src = (char*)freadall(vspath);
  char* fragment_src = (char*)freadall(fspath);
  char infoLog[512];
  GLint success;

  GLuint vertex, fragment;

  vertex = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex, 1, (const char**)&vertex_src, NULL);
  glCompileShader(vertex);
  glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
  if (!success)
  {
    glGetShaderInfoLog(vertex, 512, NULL, infoLog);
	printf("[%s] %s", vspath, infoLog);
  }
  fragment = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment, 1, (const char**)&fragment_src, NULL);
  glCompileShader(fragment);
  glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
  if (!success)
  {
	  glGetShaderInfoLog(fragment, 512, NULL, infoLog);
	  printf("[%s] %s", fspath, infoLog);
  }

  program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success)
  {
	  glGetShaderInfoLog(program, 512, NULL, infoLog);
	  printf("[LinkProgram] %s", infoLog);
  }

  glDeleteShader(vertex);
  glDeleteShader(fragment);

  free(vertex_src);
  free(fragment_src);


  for( auto xi=id.begin(); xi!=id.end(); )
  {
    int ui = glGetUniformLocation(program, (*xi).first.c_str() );
    int ai = glGetAttribLocation( program, (*xi).first.c_str() );
    if( ui==-1 && ai==-1 )
      xi = id.erase(xi);
    else
    {
      (*xi).second = ui!=-1 ? ui : ai;
      xi++;
    }
  }
}

void GShader::use()
{
  glUseProgram(program);
}

void GShader::unuse()
{
  glUseProgram(0);
}

int& GShader::operator[]( const char *key )
{
  auto xi=id.find(key);
  if( xi!=id.end() )
    return (*xi).second;

  int &val = id[key];
  int ui = glGetUniformLocation(program, key);
  int ai = glGetAttribLocation( program, key);
  val = ui!=-1 ? ui : ai;
  return val;
}

