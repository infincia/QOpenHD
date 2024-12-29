//
// Created by consti10 on 08.09.22.
//

#include <cstdio>
#include <string>
#include <sstream>
#include <iostream>
#include <cassert>
#include <vector>
#include "gl_shaders.h"

static const char *GlErrorString(GLenum error ){
  switch ( error ){
	case GL_NO_ERROR:						return "GL_NO_ERROR";
	case GL_INVALID_ENUM:					return "GL_INVALID_ENUM";
	case GL_INVALID_VALUE:					return "GL_INVALID_VALUE";
	case GL_INVALID_OPERATION:				return "GL_INVALID_OPERATION";
	case GL_INVALID_FRAMEBUFFER_OPERATION:	return "GL_INVALID_FRAMEBUFFER_OPERATION";
	case GL_OUT_OF_MEMORY:					return "GL_OUT_OF_MEMORY";
	  //
	case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
	case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
	default: return "unknown";
  }
}
 void GL_shaders::checkGlError(const std::string& caller) {
  GLenum error;
  std::stringstream ss;
  ss<<"GLError:"<<caller.c_str();
  ss<<__FILE__<<__LINE__;
  bool anyError=false;
  while ((error = glGetError()) != GL_NO_ERROR) {
	ss<<" |"<<GlErrorString(error);
	anyError=true;
  }
  if(anyError){
	std::cout<<ss.str()<<"\n";
  }
}

// NOTE: The lowest we need to suport is generally the RPI and OpenGL ES 2.0
 // Which is why we use v100 here -  GLES20 uses shading language v100

// We always use the same vertex shader code - full screen texture.
// (Adjust ratio by setting the OpenGL viewport)
static const GLchar* vertex_shader_source_all =
     "#version 100\n"
     "attribute vec3 position;\n"
     "attribute vec2 tex_coords;\n"
     "varying vec2 v_texCoord;\n"
    "void main() {  \n"
    "	gl_Position = vec4(position, 1.0);\n"
    "	v_texCoord = tex_coords;\n"
    "}\n";

// All the different fragment shader
static const GLchar* fragment_shader_source_GL_OES_EGL_IMAGE_EXTERNAL =
    "#version 100\n"
	"#extension GL_OES_EGL_image_external : require\n"
	"precision mediump float;\n"
	"uniform samplerExternalOES texture;\n"
    "varying vec2 v_texCoord;\n"
	"void main() {	\n"
    "	gl_FragColor = texture2D( texture, v_texCoord );\n"
	"}\n";
static const GLchar* fragment_shader_source_RGB =
    "#version 100\n"
	"precision mediump float;\n"
	"uniform sampler2D s_texture;\n"
    "varying vec2 v_texCoord;\n"
	"void main() {	\n"
    "	gl_FragColor = texture2D( s_texture, v_texCoord );\n"
    "	gl_FragColor.a = 1.0;\n"
	"}\n";
// I think we always have BT601 ?
// Copy paste constants from SDL
static const GLchar* fragment_shader_source_YUV420P =
    "#version 100\n"
	"precision highp float;\n"
	"uniform sampler2D s_texture_y;\n"
	"uniform sampler2D s_texture_u;\n"
	"uniform sampler2D s_texture_v;\n"
    "varying highp vec2 v_texCoord;\n"
	"void main() {	\n"
	"	const vec3 offset = vec3(-0.0627451017, -0.501960814, -0.501960814);\n"
	"	const vec3 Rcoeff = vec3(1.1644,  0.000,  1.596);\n"
	"	const vec3 Gcoeff = vec3(1.1644, -0.3918, -0.813);\n"
	"	const vec3 Bcoeff = vec3(1.1644,  2.0172,  0.000);\n"
	"	float Y = texture2D(s_texture_y, v_texCoord).r;\n"
	"	float U = texture2D(s_texture_u, v_texCoord).r;\n"
	"	float V = texture2D(s_texture_v, v_texCoord).r;\n"
	"	vec3 yuv=vec3(Y,U,V);\n"
	"	vec3 rgb;\n"
	"	// Do the color transform \n"
	"	yuv += offset;\n"
	"	rgb.r = dot(yuv, Rcoeff);\n"
	"	rgb.g = dot(yuv, Gcoeff);\n"
	"	rgb.b = dot(yuv, Bcoeff);\n"
    "	gl_FragColor = vec4(rgb, 1.0);\n"
	"}\n";
static const GLchar* fragment_shader_source_NV12 =
    "#version 100\n"
	"precision mediump float;\n"
	"uniform sampler2D s_texture_y;\n"
	"uniform sampler2D s_texture_uv;\n"
    "varying vec2 v_texCoord;\n"
	"void main() {	\n"
	"	vec3 YCbCr = vec3(\n"
	"		texture2D(s_texture_y, v_texCoord).x,\n"
	"		texture2D(s_texture_uv,v_texCoord).xy\n"
	"	);"
	"	mat3 colorMatrix = mat3(\n"
    "		1.1644, 1.1644, 1.1644,\n"
    "        0.0, -0.3917, 2.0172,\n"
    "        1.5960, -0.8129, 0.0"
	"		);\n"
    "	gl_FragColor = vec4(clamp(YCbCr*colorMatrix,0.0,1.0), 1.0);\n"
	"}\n";

/// negative x,y is bottom left and first vertex
/// 3 ---- 4
/// |      |
/// 1 ---- 2
static const GLfloat vertices[4][3] =
	{
        {-1.0, -1.0, 0.0}, { 1.0, -1.0, 0.0}, {-1.0, 1.0, 0.0}, {1.0, 1.0, 0.0}
	};

// Consti10: Video was flipped horizontally (at least big buck bunny), fixed
static const GLfloat uv_coords[][4][2] =
	{
		//{ {0.0, 0.0}, {1.0, 0.0}, {0.0, 1.0}, {1.0, 1.0} }
		//{ {1.0, 1.0}, {0.0, 1.0}, {1.0, 0.0}}//, {0.0, 0.0} }
		{ {0.0, 1.0}, {1.0, 1.0}, {0.0, 0.0}, {1.0, 0.0} }
	};
static const GLfloat uv_coords_rotated_90_degree[][4][2] =
    {
        { {1.0, 1.0}, {1.0, 0.0}, {0.0, 1.0}, {0.0, 0.0} }
    };

static GLint common_get_shader_program(const char *vertex_shader_source, const char *fragment_shader_source) {
  enum Consts {INFOLOG_LEN = 512};
  GLchar infoLog[INFOLOG_LEN];
  GLint fragment_shader;
  GLint shader_program;
  GLint success;
  GLint vertex_shader;

  /* Vertex shader */
  vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
	glGetShaderInfoLog(vertex_shader, INFOLOG_LEN, NULL, infoLog);
    printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
  }

  /* Fragment shader */
  fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &success);
  if (!success) {
	glGetShaderInfoLog(fragment_shader, INFOLOG_LEN, NULL, infoLog);
	printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
  }

  /* Link shaders */
  shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
  if (!success) {
	glGetProgramInfoLog(shader_program, INFOLOG_LEN, NULL, infoLog);
    fprintf(stderr,"ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
  }

  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  //assert(shader_program!=0);
  return shader_program;
}

static GLint checked_glGetAttribLocation(GLuint program, const GLchar *name){
  auto ret=glGetAttribLocation(program,name);
  //assert(ret>=0);
  return ret;
}
static GLint checked_glGetUniformLocation(GLuint program, const GLchar *name){
  auto ret= glGetUniformLocation(program,name);
  //assert(ret>=0);
  return ret;
}

void GL_shaders::initialize() {
  // Shader 1
  rgba_shader.program = common_get_shader_program(vertex_shader_source_all,fragment_shader_source_RGB);
  //assert(rgba_shader.program!=0);
  rgba_shader.pos = checked_glGetAttribLocation(rgba_shader.program, "position");
  rgba_shader.uvs = checked_glGetAttribLocation(rgba_shader.program, "tex_coords");
  rgba_shader.sampler = checked_glGetUniformLocation(rgba_shader.program, "s_texture" );
  checkGlError("Create shader RGBA");
  // Shader 2
  egl_shader.program = common_get_shader_program(vertex_shader_source_all, fragment_shader_source_GL_OES_EGL_IMAGE_EXTERNAL);
  egl_shader.pos = checked_glGetAttribLocation(egl_shader.program, "position");
  egl_shader.uvs = checked_glGetAttribLocation(egl_shader.program, "tex_coords");
  checkGlError("Create shader EGL");
  // Shader 3
  yuv_420P_shader.program= common_get_shader_program(vertex_shader_source_all, fragment_shader_source_YUV420P);
  yuv_420P_shader.pos = checked_glGetAttribLocation(yuv_420P_shader.program, "position");
  yuv_420P_shader.uvs = checked_glGetAttribLocation(yuv_420P_shader.program, "tex_coords");
  yuv_420P_shader.s_texture_y=checked_glGetUniformLocation(yuv_420P_shader.program, "s_texture_y");
  yuv_420P_shader.s_texture_u=checked_glGetUniformLocation(yuv_420P_shader.program, "s_texture_u");
  yuv_420P_shader.s_texture_v=checked_glGetUniformLocation(yuv_420P_shader.program, "s_texture_v");
  checkGlError("Create shader YUV420P");
  // Shader 4
  nv12_shader.program= common_get_shader_program(vertex_shader_source_all, fragment_shader_source_NV12);
  nv12_shader.pos = checked_glGetAttribLocation(nv12_shader.program, "position");
  nv12_shader.uvs = checked_glGetAttribLocation(nv12_shader.program, "tex_coords");
  nv12_shader.s_texture_y=checked_glGetUniformLocation(nv12_shader.program, "s_texture_y");
  nv12_shader.s_texture_uv=checked_glGetUniformLocation(nv12_shader.program, "s_texture_uv");
  checkGlError("Create shader NV12");
  //
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(uv_coords), 0, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(uv_coords), uv_coords);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  // For now, we only do 90°
  glGenBuffers(1, &vbo_rotated_90_degree);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_rotated_90_degree);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices)+sizeof(uv_coords), 0, GL_STATIC_DRAW);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
  glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(uv_coords), uv_coords_rotated_90_degree);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  checkGlError("Create VBO");
}

void GL_shaders::beforeDrawVboSetup(GLint pos, GLint uvs,int rotation_degree) const {
  auto vbo_rot = vbo;
  if(rotation_degree==90 || rotation_degree==270){
      // 270° - not technically correct, but at least the user can just use a flip
      vbo_rot= vbo_rotated_90_degree;
  }
  glBindBuffer(GL_ARRAY_BUFFER, vbo_rot);
  glEnableVertexAttribArray(pos);
  glEnableVertexAttribArray(uvs);
  glVertexAttribPointer(pos, 3, GL_FLOAT, GL_FALSE, 0, (GLvoid*)0);
  glVertexAttribPointer(uvs, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid*)sizeof(vertices)); /// last is offset to loc in buf memory
}
void GL_shaders::afterDrawVboCleanup(GLint pos, GLint uvs) {
  glDisableVertexAttribArray(pos);
  glDisableVertexAttribArray(uvs);
  glBindBuffer(GL_ARRAY_BUFFER,0);
}

void GL_shaders::draw_rgb(GLuint texture,int rotation_degree) {
  glUseProgram(rgba_shader.program);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture);
  glUniform1i(rgba_shader.sampler,0);
  beforeDrawVboSetup(rgba_shader.pos,rgba_shader.uvs,rotation_degree);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  glBindTexture(GL_TEXTURE_2D, 0);
  afterDrawVboCleanup(rgba_shader.pos,rgba_shader.uvs);
  checkGlError("Draw RGBA texture");
}

void GL_shaders::draw_egl(GLuint texture,int rotation_degree) {
  glUseProgram(egl_shader.program);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES,texture);
  beforeDrawVboSetup(egl_shader.pos,egl_shader.uvs,rotation_degree);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  afterDrawVboCleanup(egl_shader.pos,egl_shader.uvs);
  glBindTexture(GL_TEXTURE_EXTERNAL_OES,0);
  checkGlError("Draw EGL texture");
}

static void bind_textures(std::vector<GLuint> textures){
    for(int i=0;i<textures.size();i++){
        glActiveTexture(GL_TEXTURE0 + i);
        GLuint texture=textures[i];
        glBindTexture(GL_TEXTURE_2D,texture);
    }
}
static void unbind_textures(int n_textures){
    for(int i=0;i<n_textures;i++){
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D,0);
    }
    // Overkill, but needed for QT
    glActiveTexture(GL_TEXTURE0);
}
// Set texture uniforms such that the first one is bound to uniform 0,
// second one to uniform 1 etc
static void set_uniforms_ascending(std::vector<GLint> uniforms){
  for(unsigned int i=0;i<uniforms.size();i++){
	glUniform1i(uniforms[i], i);
  }
}

void GL_shaders::draw_YUV420P(GLuint textureY, GLuint textureU, GLuint textureV,int rotation_degree) {
  checkGlError("B Draw YUV420 texture");
  glUseProgram(yuv_420P_shader.program);
  const std::vector<GLuint> textures{textureY,textureU,textureV};
  bind_textures(textures);
  const std::vector<GLint> uniforms{yuv_420P_shader.s_texture_y,yuv_420P_shader.s_texture_u,yuv_420P_shader.s_texture_v};
  set_uniforms_ascending(uniforms);
  beforeDrawVboSetup(yuv_420P_shader.pos,yuv_420P_shader.uvs,rotation_degree);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  afterDrawVboCleanup(yuv_420P_shader.pos,yuv_420P_shader.uvs);
  unbind_textures(textures.size());
  checkGlError("Draw YUV420 texture");
}

void GL_shaders::draw_NV12(GLuint textureY, GLuint textureUV,int rotation_degree) {
  glUseProgram(nv12_shader.program);
  const std::vector<GLuint> textures{textureY,textureUV};
  bind_textures(textures);
  const std::vector<GLint> uniforms{nv12_shader.s_texture_y,nv12_shader.s_texture_uv};
  set_uniforms_ascending(uniforms);
  beforeDrawVboSetup(nv12_shader.pos,nv12_shader.uvs,rotation_degree);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  afterDrawVboCleanup(nv12_shader.pos,nv12_shader.uvs);
  unbind_textures(textures.size());
  checkGlError("Draw NV12 texture");
}

void GL_shaders::debug_set_swap_interval(int interval)
{
    auto res=eglSwapInterval(eglGetCurrentDisplay(),interval);
    if(!res){
        std::cerr<<"Cannot set swap interval of "<<interval<<"\n";
    }

}
