#ifndef GUI_IMAGE_LOAD_H
#define GUI_IMAGE_LOAD_H

#define _CRT_SECURE_NO_WARNINGS
#include <stb/stb_image.h>
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_opengl.h>


struct display_image_data {
  std::vector<float> output_image_data;
  GLuint out_texture = 0;
  int last_render_width = 0;
  int last_render_height = 0;
};

typedef std::vector<unsigned char> sdl_image;

bool LoadRenderedTexture(display_image_data &d_imdata, int width,
int height) {

  if (d_imdata.output_image_data.empty()) return false;

  bool resolution_changed = (width != d_imdata.last_render_width ||
                             height != d_imdata.last_render_height);

  // Create the texture anew if it's the first time or the resolution has changed.
  if (d_imdata.out_texture == 0 || resolution_changed) {
    glGenTextures(1, &d_imdata.out_texture);
    glBindTexture(GL_TEXTURE_2D, d_imdata.out_texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA,
    GL_FLOAT, 0);
    // GLenum err = glGetError();
    // std::cout << err << std::flush;

    d_imdata.last_render_height = height;
    d_imdata.last_render_width = width;
  }

  // Upload the latest pixels.
  else {
    glBindTexture(GL_TEXTURE_2D, d_imdata.out_texture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, width, height, GL_RGBA, GL_FLOAT,
    d_imdata.output_image_data.data());
    // GLenum err = glGetError();
    // std::cout << "GLERROR: "<< err << std::flush;

  }

  return true;
}

#endif