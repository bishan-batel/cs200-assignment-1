// AffineDemo.cpp
// -- graphical demo/test of 'Affine' package
// cs200 9/20
//
// To compile under Windows:
//   cl /EHsc AffineDemo.cpp Affine.cpp SDL2.lib SDL2main.lib\
//      opengl32.lib glew32.lib /link /subsystem:console
//   [you can substitute 'Affine.cpp' with 'Affine.lib']
// To compile under Linux:
//   g++ AffineDemo.cpp Affine.cpp -lSDL2 -lGL -lGLEW

#include <iostream>
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <GL/gl.h>
#include "Affine.h"
using namespace std;


const float PI = 4.0f*atan(1.0f);
const glm::vec4 O = cs200::point(0,0);

namespace {
  float frand(float a=0, float b=1) {
    return a + (b-a)*float(rand())/float(RAND_MAX);
  }
}


class Client {
  public:
    Client(void);
    ~Client(void);
    void draw(double dt);
    void keypress(SDL_Keycode kc);
    void resize(int W, int H);
  private:
    double time;
    GLuint program,
           vao_edges,
           vao_faces,
           vertex_buffer,
           edge_buffer,
           face_buffer;
    GLint ucolor,
          utransform;
    int square_count;
    glm::vec4 center;
    float rot_rate,
          rsize,
          rscale;
};


Client::Client(void)
    : time(0) {

  // compile fragment shader
  const char *fragment_shader_text =
    "#version 130\n\
     uniform vec3 color;\
     out vec4 frag_color;\
     void main(void) {\
       frag_color = vec4(color,1);\
     }";
  GLuint fshader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fshader,1,&fragment_shader_text,0);
  glCompileShader(fshader);

  // compile vertex shader
  const char *vertex_shader_text =
    "#version 130\n\
     in vec4 position;\
     uniform mat4 transform;\
     void main() {\
       gl_Position = transform * position;\
     }";
  GLuint vshader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vshader,1,&vertex_shader_text,0);
  glCompileShader(vshader);

  // link shader program
  program = glCreateProgram();
  glAttachShader(program,fshader);
  glAttachShader(program,vshader);
  glLinkProgram(program);
  glDeleteShader(fshader);
  glDeleteShader(vshader);

  // store vertex array on GPU
  glGenBuffers(1,&vertex_buffer);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  const glm::vec4 vertices[4] = {
     cs200::point( 0.5f, 0.5f), cs200::point(-0.5f, 0.5f),
     cs200::point(-0.5f,-0.5f), cs200::point( 0.5f,-0.5f)
  };
  glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),&(vertices[0].x),GL_STATIC_DRAW);

  // store edge list on GPU
  glGenBuffers(1,&edge_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,edge_buffer);
  const int edges[4][2] = { {0,1}, {1,2}, {2,3}, {3,0} };
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(edges),edges,GL_STATIC_DRAW);

  // store face list on GPU
  glGenBuffers(1,&face_buffer);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,face_buffer);
  const int faces[2][3] = { {0,1,2}, {0,2,3} };
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(faces),faces,GL_STATIC_DRAW);

  // get shader uniform variable locations
  ucolor = glGetUniformLocation(program,"color");
  utransform = glGetUniformLocation(program,"transform");

  // create a VAO for drawing the faces
  GLint aposition = glGetAttribLocation(program,"position");
  glGenVertexArrays(1,&vao_faces);
  // start recording
  glBindVertexArray(vao_faces);
  // associate position attribute with vertex array buffer
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  glVertexAttribPointer(aposition,4,GL_FLOAT,false,sizeof(glm::vec4),0);
  glEnableVertexAttribArray(aposition);
  // select the face buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,face_buffer);
  // stop recording
  glBindVertexArray(0);
  
  // create a VAO for drawing the edges
  glGenVertexArrays(1,&vao_edges);
  glBindVertexArray(vao_edges);
  glBindBuffer(GL_ARRAY_BUFFER,vertex_buffer);
  glVertexAttribPointer(aposition,4,GL_FLOAT,false,sizeof(glm::vec4),0);
  glEnableVertexAttribArray(aposition);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,edge_buffer);
  glBindVertexArray(0);

  // set modeling transform parameters
  center = O;
  rot_rate = 360/frand(5,10);
  square_count = 3;
  rsize = 1.0f/square_count;
  rscale = 0.8f;
}


Client::~Client(void) {
  glDeleteVertexArrays(1,&vao_edges);
  glDeleteVertexArrays(1,&vao_faces);
  glDeleteBuffers(1,&face_buffer);
  glDeleteBuffers(1,&edge_buffer);
  glDeleteBuffers(1,&vertex_buffer);

  glUseProgram(0);
  glDeleteProgram(program);
}


void Client::draw(double dt) {

  // clear frame buffer
  glClearColor(1,1,1,1);
  glClear(GL_COLOR_BUFFER_BIT);

  // select shader
  glUseProgram(program);

  // draw rotating array of squares
  glm::mat4 R = cs200::rotate(rot_rate*time);
  for (int i=0; i < square_count; ++i) {
    for (int j=0; j < square_count; ++j) {

      // compute modeling transform
      glm::vec4 offset = rsize * (cs200::vector(i,j)
                                  - 0.5f*(square_count-1)*cs200::vector(1,1));
      glm::mat4 obj2world = cs200::translate(center-O)
                            * R
                            * cs200::translate(offset)
                            * cs200::scale(rscale*rsize);
      glUniformMatrix4fv(utransform,1,false,&(obj2world[0][0]));

      // draw the faces (vertex array already bound)
      glUniform3f(ucolor,1-i*rsize,0.5f,1-j*rsize);
      glBindVertexArray(vao_faces);
      glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);
      glBindVertexArray(0);

      // draw the edges
      glUniform3f(ucolor,0,0,0);
      glBindVertexArray(vao_edges);
      glDrawElements(GL_LINES,8,GL_UNSIGNED_INT,0);
      glBindVertexArray(0);

    }
  }

  time += dt;
}


void Client::keypress(SDL_Keycode kc) {
  if (kc == SDLK_SPACE) {
    square_count = 1 + square_count%10;
    rsize = 1.0f/square_count;
  }
}


void Client::resize(int W, int H) {
  int D = min(W,H);
  glViewport(0,0,D,D);
}


/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

int main(int argc, char *argv[]) {
  srand(unsigned(time(0)));

  // SDL: initialize and create a window
  SDL_Init(SDL_INIT_VIDEO);
  SDL_Window *window = SDL_CreateWindow("CS200: Affine Demo",SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,600,600,
                                        SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);
  SDL_GLContext context = SDL_GL_CreateContext(window);

  // GLEW: get function bindings (if possible)
  GLenum value = glewInit();
  if (value != GLEW_OK) {
    cout << glewGetErrorString(value) << endl;
    SDL_GL_DeleteContext(context);
    SDL_Quit();
    return -1;
  }

  // animation loop
  bool done = false;
  Client *client = new Client();
  Uint32 ticks_last = SDL_GetTicks();
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_QUIT:
          done = true;
          break;
        case SDL_KEYDOWN:
          if (event.key.keysym.sym == SDLK_ESCAPE)
            done = true;
          else
            client->keypress(event.key.keysym.sym);
          break;
        case SDL_WINDOWEVENT:
          if (event.window.event == SDL_WINDOWEVENT_RESIZED)
            client->resize(event.window.data1,event.window.data2);
          break;
      }
    }
    Uint32 ticks = SDL_GetTicks();
    double dt = 0.001*(ticks - ticks_last);
    ticks_last = ticks;
    client->draw(dt);
    SDL_GL_SwapWindow(window);
  }

  // clean up
  delete client;
  SDL_GL_DeleteContext(context);
  SDL_Quit();
  return 0;
}

