#include "platform/android/egl_renderer.hpp"

#include <GLES3/gl3.h>
#include <android/log.h>
#include <android/native_window.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <limits>
#include <string_view>
#include <utility>
#include <vector>

namespace tmnf::android {
namespace {

constexpr char kLogTag[] = "TMNF";
constexpr float kPi = 3.14159265358979323846F;

struct Vec3 final {
  float x;
  float y;
  float z;
};

struct Vec2 final {
  float x;
  float y;
};

struct GpuVertex final {
  Vec3 position;
  Vec3 normal;
  Vec3 color;
  Vec2 textureCoordinate;
};

struct HudVertex final {
  Vec2 position;
  Vec2 textureCoordinate;
};

struct HudGlyph final {
  char character;
  float left;
  float highV;
  float right;
  float lowV;
  float advance;
};

// Exact records recovered from Interface\Media\Font\Led.Font.Gbx chunk 0x0904E007.
constexpr std::array kHudGlyphs{
    HudGlyph{' ', 44.015625F, 254.0F, 62.015625F, 192.0F, 18.0F},
    HudGlyph{'A', 65.015625F, 254.0F, 95.015625F, 192.0F, 30.0F},
    HudGlyph{'C', 131.015625F, 254.0F, 159.015625F, 192.0F, 27.0F},
    HudGlyph{'L', 403.015625F, 254.0F, 433.015625F, 192.0F, 30.0F},
    HudGlyph{'M', 436.015625F, 254.0F, 485.015625F, 192.0F, 49.0F},
    HudGlyph{'P', 68.015625F, 191.015625F, 98.015625F, 129.015625F, 30.0F},
    HudGlyph{'0', 445.015625F, 191.015625F, 475.015625F, 129.015625F, 30.0F},
    HudGlyph{'1', 478.015625F, 191.015625F, 508.015625F, 129.015625F, 30.0F},
    HudGlyph{'2', 2.0F, 128.015625F, 32.0F, 66.015625F, 30.0F},
    HudGlyph{'3', 35.015625F, 128.015625F, 65.015625F, 66.015625F, 30.0F},
    HudGlyph{'4', 68.015625F, 128.015625F, 98.015625F, 66.015625F, 30.0F},
    HudGlyph{'5', 101.015625F, 128.015625F, 131.015625F, 66.015625F, 30.0F},
    HudGlyph{'6', 134.015625F, 128.015625F, 164.015625F, 66.015625F, 30.0F},
    HudGlyph{'7', 167.015625F, 128.015625F, 197.015625F, 66.015625F, 30.0F},
    HudGlyph{'8', 200.015625F, 128.015625F, 230.015625F, 66.015625F, 30.0F},
    HudGlyph{'9', 233.015625F, 128.015625F, 263.015625F, 66.015625F, 30.0F},
    HudGlyph{':', 266.015625F, 128.015625F, 277.015625F, 66.015625F, 11.0F},
    HudGlyph{'.', 280.015625F, 128.015625F, 293.015625F, 66.015625F, 13.0F},
    HudGlyph{'/', 144.015625F, 65.015625F, 181.015625F, 3.015625F, 37.0F},
};

constexpr std::size_t kMaximumHudVertices = 384U;

const HudGlyph* hudGlyph(const char character) noexcept {
  const auto found = std::ranges::find_if(
      kHudGlyphs, [character](const HudGlyph& glyph) { return glyph.character == character; });
  return found == kHudGlyphs.end() ? nullptr : &*found;
}

float hudTextWidth(const std::string_view text) noexcept {
  float width = 0.0F;
  for (const char character : text) {
    if (const HudGlyph* glyph = hudGlyph(character); glyph != nullptr) {
      width += glyph->advance;
    }
  }
  return width;
}

void appendHudText(std::array<HudVertex, kMaximumHudVertices>& vertices, std::size_t& count,
                   const std::string_view text, float cursorX, const float top, const float scale,
                   const int width, const int height) noexcept {
  constexpr float kAtlasWidth = 512.0F;
  constexpr float kAtlasHeight = 256.0F;
  const auto screenX = [width](const float pixel) {
    return pixel * 2.0F / static_cast<float>(width) - 1.0F;
  };
  const auto screenY = [height](const float pixel) {
    return 1.0F - pixel * 2.0F / static_cast<float>(height);
  };
  for (const char character : text) {
    const HudGlyph* const glyph = hudGlyph(character);
    if (glyph == nullptr) {
      continue;
    }
    if (character != ' ' && count + 6U <= vertices.size()) {
      const float left = screenX(cursorX);
      const float right = screenX(cursorX + (glyph->right - glyph->left) * scale);
      const float topPosition = screenY(top);
      const float bottom = screenY(top + (glyph->highV - glyph->lowV) * scale);
      const float leftU = glyph->left / kAtlasWidth;
      const float rightU = glyph->right / kAtlasWidth;
      const float topV = 1.0F - glyph->highV / kAtlasHeight;
      const float bottomV = 1.0F - glyph->lowV / kAtlasHeight;
      vertices[count++] = {{left, bottom}, {leftU, bottomV}};
      vertices[count++] = {{right, bottom}, {rightU, bottomV}};
      vertices[count++] = {{right, topPosition}, {rightU, topV}};
      vertices[count++] = {{left, bottom}, {leftU, bottomV}};
      vertices[count++] = {{right, topPosition}, {rightU, topV}};
      vertices[count++] = {{left, topPosition}, {leftU, topV}};
    }
    cursorX += glyph->advance * scale;
  }
}

using Mat4 = std::array<float, 16>;

Vec3 operator+(const Vec3 a, const Vec3 b) { return {a.x + b.x, a.y + b.y, a.z + b.z}; }

Vec3 operator-(const Vec3 a, const Vec3 b) { return {a.x - b.x, a.y - b.y, a.z - b.z}; }

Vec3 operator*(const Vec3 value, const float scalar) {
  return {value.x * scalar, value.y * scalar, value.z * scalar};
}

float dot(const Vec3 a, const Vec3 b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

Vec3 cross(const Vec3 a, const Vec3 b) {
  return {a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x};
}

Vec3 normalize(const Vec3 value) {
  const float lengthSquared = dot(value, value);
  if (lengthSquared <= 1.0e-12F) {
    return {0.0F, 1.0F, 0.0F};
  }
  return value * (1.0F / std::sqrt(lengthSquared));
}

Vec3 transformPosition(const tmnf::gbx::SolidVisual& visual, const tmnf::gbx::Float3 position) {
  const auto& matrix = visual.transform;
  return {
      matrix[0] * position.x + matrix[1] * position.y + matrix[2] * position.z + matrix[9],
      matrix[3] * position.x + matrix[4] * position.y + matrix[5] * position.z + matrix[10],
      matrix[6] * position.x + matrix[7] * position.y + matrix[8] * position.z + matrix[11],
  };
}

Vec3 transformNormal(const tmnf::gbx::SolidVisual& visual, const tmnf::gbx::Float3 normal) {
  const auto& matrix = visual.transform;
  return {
      matrix[0] * normal.x + matrix[1] * normal.y + matrix[2] * normal.z,
      matrix[3] * normal.x + matrix[4] * normal.y + matrix[5] * normal.z,
      matrix[6] * normal.x + matrix[7] * normal.y + matrix[8] * normal.z,
  };
}

Vec3 rotateBlockVector(const Vec3 value, const std::uint8_t direction) {
  switch (direction & 3U) {
  case 1:
    return {-value.z, value.y, value.x};
  case 2:
    return {-value.x, value.y, -value.z};
  case 3:
    return {value.z, value.y, -value.x};
  default:
    return value;
  }
}

Vec3 placeBlockPosition(const Vec3 value, const gbx::ChallengeBlock& block) {
  constexpr float kHorizontalCellSize = 32.0F;
  constexpr float kVerticalCellSize = 8.0F;
  constexpr Vec3 cellCenter{16.0F, 0.0F, 16.0F};
  const Vec3 rotated = rotateBlockVector(value - cellCenter, block.direction) + cellCenter;
  return rotated + Vec3{static_cast<float>(block.coordinate[0]) * kHorizontalCellSize,
                        static_cast<float>(block.coordinate[1]) * kVerticalCellSize,
                        static_cast<float>(block.coordinate[2]) * kHorizontalCellSize};
}

Mat4 multiply(const Mat4& left, const Mat4& right) {
  Mat4 result{};
  for (int column = 0; column < 4; ++column) {
    for (int row = 0; row < 4; ++row) {
      for (int inner = 0; inner < 4; ++inner) {
        result[static_cast<std::size_t>(column * 4 + row)] +=
            left[static_cast<std::size_t>(inner * 4 + row)] *
            right[static_cast<std::size_t>(column * 4 + inner)];
      }
    }
  }
  return result;
}

Mat4 perspective(const float verticalFieldOfView, const float aspect, const float nearPlane,
                 const float farPlane) {
  const float scale = 1.0F / std::tan(verticalFieldOfView * 0.5F);
  Mat4 result{};
  result[0] = scale / aspect;
  result[5] = scale;
  result[10] = (farPlane + nearPlane) / (nearPlane - farPlane);
  result[11] = -1.0F;
  result[14] = (2.0F * farPlane * nearPlane) / (nearPlane - farPlane);
  return result;
}

Mat4 lookAt(const Vec3 eye, const Vec3 target, const Vec3 up) {
  const Vec3 forward = normalize(target - eye);
  const Vec3 side = normalize(cross(forward, up));
  const Vec3 correctedUp = cross(side, forward);
  return {
      side.x,          correctedUp.x,          -forward.x,        0.0F,
      side.y,          correctedUp.y,          -forward.y,        0.0F,
      side.z,          correctedUp.z,          -forward.z,        0.0F,
      -dot(side, eye), -dot(correctedUp, eye), dot(forward, eye), 1.0F,
  };
}

Mat4 vehicleModelMatrix(const Vec3 center, const float heading, const float scale) {
  const float cosine = std::cos(heading);
  const float sine = std::sin(heading);
  return {cosine * scale, 0.0F, -sine * scale,  0.0F, 0.0F,     scale,    0.0F,     0.0F,
          sine * scale,   0.0F, cosine * scale, 0.0F, center.x, center.y, center.z, 1.0F};
}

void logEglFailure(const char* operation) {
  __android_log_print(ANDROID_LOG_ERROR, kLogTag, "%s failed with EGL error 0x%x", operation,
                      eglGetError());
}

unsigned int compileShader(const GLenum type, const char* const source) {
  const GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &source, nullptr);
  glCompileShader(shader);
  GLint compiled = GL_FALSE;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
  if (compiled == GL_TRUE) {
    return shader;
  }
  GLint length = 0;
  glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
  std::vector<char> message(static_cast<std::size_t>(std::max(length, 1)));
  glGetShaderInfoLog(shader, length, nullptr, message.data());
  __android_log_print(ANDROID_LOG_ERROR, kLogTag, "shader compilation failed: %s", message.data());
  glDeleteShader(shader);
  return 0;
}

unsigned int createImageProgram() {
  constexpr char vertexSource[] = R"glsl(#version 300 es
uniform vec2 uScale;
uniform vec2 uOffset;
layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aUv;
out vec2 vUv;
void main() {
  gl_Position = vec4(aPosition * uScale + uOffset, 0.0, 1.0);
  vUv = aUv;
}
)glsl";
  constexpr char fragmentSource[] = R"glsl(#version 300 es
precision mediump float;
uniform sampler2D uTexture;
in vec2 vUv;
out vec4 outColor;
void main() { outColor = texture(uTexture, vUv); }
)glsl";

  const GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
  const GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
  if (vertex == 0 || fragment == 0) {
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return 0;
  }
  const GLuint program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  glDeleteShader(vertex);
  glDeleteShader(fragment);
  GLint linked = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (linked != GL_TRUE) {
    glDeleteProgram(program);
    return 0;
  }
  return program;
}

unsigned int createMeshProgram() {
  constexpr char vertexSource[] = R"glsl(#version 300 es
uniform mat4 uMvp;
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;
layout(location = 3) in vec2 aUv;
out vec3 vNormal;
out vec3 vColor;
out vec2 vUv;
void main() {
  gl_Position = uMvp * vec4(aPosition, 1.0);
  vNormal = aNormal;
  vColor = aColor;
  vUv = aUv;
}
)glsl";
  constexpr char fragmentSource[] = R"glsl(#version 300 es
precision mediump float;
uniform sampler2D uTexture;
in vec3 vNormal;
in vec3 vColor;
in vec2 vUv;
out vec4 outColor;
void main() {
  vec4 texel = texture(uTexture, vUv);
  if (texel.a < 0.2) {
    discard;
  }
  vec3 lightDirection = normalize(vec3(0.35, 0.85, 0.4));
  float diffuse = abs(dot(normalize(vNormal), lightDirection));
  float lighting = 0.32 + 0.68 * diffuse;
  outColor = vec4(texel.rgb * vColor * lighting, 1.0);
}
)glsl";

  const GLuint vertex = compileShader(GL_VERTEX_SHADER, vertexSource);
  const GLuint fragment = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
  if (vertex == 0 || fragment == 0) {
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return 0;
  }
  const GLuint program = glCreateProgram();
  glAttachShader(program, vertex);
  glAttachShader(program, fragment);
  glLinkProgram(program);
  glDeleteShader(vertex);
  glDeleteShader(fragment);
  GLint linked = GL_FALSE;
  glGetProgramiv(program, GL_LINK_STATUS, &linked);
  if (linked != GL_TRUE) {
    glDeleteProgram(program);
    return 0;
  }
  return program;
}

} // namespace

EglRenderer::~EglRenderer() { detach(); }

bool EglRenderer::attach(ANativeWindow* window) {
  detach();
  if (window == nullptr) {
    return false;
  }

  display_ = eglGetDisplay(EGL_DEFAULT_DISPLAY);
  if (display_ == EGL_NO_DISPLAY || eglInitialize(display_, nullptr, nullptr) != EGL_TRUE) {
    logEglFailure("eglInitialize");
    detach();
    return false;
  }

  constexpr EGLint attributes[] = {
      EGL_SURFACE_TYPE,
      EGL_WINDOW_BIT,
      EGL_RENDERABLE_TYPE,
      EGL_OPENGL_ES3_BIT,
      EGL_RED_SIZE,
      8,
      EGL_GREEN_SIZE,
      8,
      EGL_BLUE_SIZE,
      8,
      EGL_ALPHA_SIZE,
      8,
      EGL_DEPTH_SIZE,
      24,
      EGL_NONE,
  };

  EGLConfig config = nullptr;
  EGLint configCount = 0;
  if (eglChooseConfig(display_, attributes, &config, 1, &configCount) != EGL_TRUE ||
      configCount == 0) {
    logEglFailure("eglChooseConfig");
    detach();
    return false;
  }

  EGLint nativeFormat = 0;
  eglGetConfigAttrib(display_, config, EGL_NATIVE_VISUAL_ID, &nativeFormat);
  ANativeWindow_setBuffersGeometry(window, 0, 0, nativeFormat);

  constexpr EGLint contextAttributes[] = {
      EGL_CONTEXT_CLIENT_VERSION,
      3,
      EGL_NONE,
  };
  context_ = eglCreateContext(display_, config, EGL_NO_CONTEXT, contextAttributes);
  surface_ = eglCreateWindowSurface(display_, config, window, nullptr);
  if (context_ == EGL_NO_CONTEXT || surface_ == EGL_NO_SURFACE ||
      eglMakeCurrent(display_, surface_, surface_, context_) != EGL_TRUE) {
    logEglFailure("EGL context creation");
    detach();
    return false;
  }

  eglQuerySurface(display_, surface_, EGL_WIDTH, &width_);
  eglQuerySurface(display_, surface_, EGL_HEIGHT, &height_);
  eglSwapInterval(display_, 1);

  createImageResources();
  createMeshResources();
  createHudResources();

  __android_log_print(ANDROID_LOG_INFO, kLogTag, "OpenGL ES bootstrap ready: %dx%d, renderer=%s",
                      width_, height_, reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
  // Present immediately so startup is visibly verified even if Android
  // delivered its first focus event before the native callback was installed.
  draw(0.0);
  return true;
}

void EglRenderer::detach() {
  if (display_ != EGL_NO_DISPLAY) {
    if (imageTexture_ != 0) {
      glDeleteTextures(1, &imageTexture_);
    }
    if (imageVertexArray_ != 0) {
      glDeleteVertexArrays(1, &imageVertexArray_);
    }
    if (imageVertexBuffer_ != 0) {
      glDeleteBuffers(1, &imageVertexBuffer_);
    }
    if (imageProgram_ != 0) {
      glDeleteProgram(imageProgram_);
    }
    if (hudTexture_ != 0) {
      glDeleteTextures(1, &hudTexture_);
    }
    if (hudVertexArray_ != 0) {
      glDeleteVertexArrays(1, &hudVertexArray_);
    }
    if (hudVertexBuffer_ != 0) {
      glDeleteBuffers(1, &hudVertexBuffer_);
    }
    if (meshVertexArray_ != 0) {
      glDeleteVertexArrays(1, &meshVertexArray_);
    }
    if (meshVertexBuffer_ != 0) {
      glDeleteBuffers(1, &meshVertexBuffer_);
    }
    if (meshIndexBuffer_ != 0) {
      glDeleteBuffers(1, &meshIndexBuffer_);
    }
    if (meshFallbackTexture_ != 0) {
      glDeleteTextures(1, &meshFallbackTexture_);
    }
    if (vehicleVertexArray_ != 0) {
      glDeleteVertexArrays(1, &vehicleVertexArray_);
    }
    if (vehicleVertexBuffer_ != 0) {
      glDeleteBuffers(1, &vehicleVertexBuffer_);
    }
    if (vehicleIndexBuffer_ != 0) {
      glDeleteBuffers(1, &vehicleIndexBuffer_);
    }
    if (!meshMaterialTextures_.empty()) {
      glDeleteTextures(static_cast<GLsizei>(meshMaterialTextures_.size()),
                       meshMaterialTextures_.data());
    }
    if (meshProgram_ != 0) {
      glDeleteProgram(meshProgram_);
    }
    eglMakeCurrent(display_, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
    if (context_ != EGL_NO_CONTEXT) {
      eglDestroyContext(display_, context_);
    }
    if (surface_ != EGL_NO_SURFACE) {
      eglDestroySurface(display_, surface_);
    }
    eglTerminate(display_);
  }

  display_ = EGL_NO_DISPLAY;
  surface_ = EGL_NO_SURFACE;
  context_ = EGL_NO_CONTEXT;
  width_ = 0;
  height_ = 0;
  imageTexture_ = 0;
  imageVertexArray_ = 0;
  imageVertexBuffer_ = 0;
  imageProgram_ = 0;
  imageScaleUniform_ = -1;
  imageOffsetUniform_ = -1;
  hudTexture_ = 0;
  hudVertexArray_ = 0;
  hudVertexBuffer_ = 0;
  meshProgram_ = 0;
  meshVertexArray_ = 0;
  meshVertexBuffer_ = 0;
  meshIndexBuffer_ = 0;
  meshFallbackTexture_ = 0;
  vehicleVertexArray_ = 0;
  vehicleVertexBuffer_ = 0;
  vehicleIndexBuffer_ = 0;
  meshMaterialTextures_.clear();
  meshDraws_.clear();
  vehicleDraws_.clear();
  meshMvpUniform_ = -1;
  meshTextureUniform_ = -1;
  meshIndexCount_ = 0;
  vehicleIndexCount_ = 0;
}

void EglRenderer::resize() {
  if (!ready()) {
    return;
  }
  eglQuerySurface(display_, surface_, EGL_WIDTH, &width_);
  eglQuerySurface(display_, surface_, EGL_HEIGHT, &height_);
}

void EglRenderer::draw(const double seconds) {
  if (!ready()) {
    return;
  }

  const float pulse = 0.5F + 0.5F * static_cast<float>(std::sin(seconds * 0.8));
  glViewport(0, 0, width_, height_);
  glClearColor(0.015F + pulse * 0.01F, 0.055F + pulse * 0.025F, 0.12F + pulse * 0.06F, 1.0F);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (meshProgram_ != 0 && meshVertexArray_ != 0 && meshIndexCount_ > 0 && width_ > 0 &&
      height_ > 0) {
    Vec3 normalizedVehicle{};
    Vec3 eye{};
    Vec3 target{};
    if (vehicleVisible_) {
      normalizedVehicle = (Vec3{vehiclePosition_.x, vehiclePosition_.y, vehiclePosition_.z} -
                           Vec3{meshWorldCenter_.x, meshWorldCenter_.y, meshWorldCenter_.z}) *
                          meshWorldInverseExtent_;
      const Vec3 forward{std::sin(vehicleHeading_), 0.0F, std::cos(vehicleHeading_)};
      eye = normalizedVehicle - forward * (14.0F * meshWorldInverseExtent_) +
            Vec3{0.0F, 7.0F * meshWorldInverseExtent_, 0.0F};
      target = normalizedVehicle + forward * (7.0F * meshWorldInverseExtent_) +
               Vec3{0.0F, 1.0F * meshWorldInverseExtent_, 0.0F};
    } else {
      const float angle = static_cast<float>(seconds) * 0.12F + imageOffsetX_ * 3.2F;
      const float elevation = 1.15F + imageOffsetY_ * 2.4F;
      constexpr float distance = 2.65F;
      eye = {std::sin(angle) * distance, elevation, std::cos(angle) * distance};
      target = {0.0F, 0.0F, 0.0F};
    }
    const Mat4 view = lookAt(eye, target, {0.0F, 1.0F, 0.0F});
    const Mat4 projection =
        perspective(54.0F * kPi / 180.0F, static_cast<float>(width_) / static_cast<float>(height_),
                    0.001F, 20.0F);
    const Mat4 mvp = multiply(projection, view);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);
    glDisable(GL_BLEND);
    glUseProgram(meshProgram_);
    glUniformMatrix4fv(meshMvpUniform_, 1, GL_FALSE, mvp.data());
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(meshVertexArray_);
    for (const MeshDraw& draw : meshDraws_) {
      glBindTexture(GL_TEXTURE_2D, draw.texture == 0 ? meshFallbackTexture_ : draw.texture);
      const auto byteOffset = static_cast<std::uintptr_t>(draw.firstIndex) * sizeof(std::uint32_t);
      glDrawElements(GL_TRIANGLES, draw.indexCount, GL_UNSIGNED_INT,
                     reinterpret_cast<const void*>(byteOffset));
    }
    if (vehicleVisible_ && vehicleVertexArray_ != 0 && vehicleIndexCount_ > 0) {
      const Mat4 vehicleModel =
          vehicleModelMatrix(normalizedVehicle, vehicleHeading_, meshWorldInverseExtent_);
      const Mat4 vehicleMvp = multiply(mvp, vehicleModel);
      glUniformMatrix4fv(meshMvpUniform_, 1, GL_FALSE, vehicleMvp.data());
      glBindVertexArray(vehicleVertexArray_);
      for (const MeshDraw& draw : vehicleDraws_) {
        glBindTexture(GL_TEXTURE_2D, draw.texture == 0 ? meshFallbackTexture_ : draw.texture);
        const auto byteOffset =
            static_cast<std::uintptr_t>(draw.firstIndex) * sizeof(std::uint32_t);
        glDrawElements(GL_TRIANGLES, draw.indexCount, GL_UNSIGNED_INT,
                       reinterpret_cast<const void*>(byteOffset));
      }
    }
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
  } else if (imageProgram_ != 0 && imageTexture_ != 0) {
    const float verticalScale = 0.55F;
    const float horizontalScale =
        verticalScale * static_cast<float>(height_) / static_cast<float>(width_);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glDisable(GL_SCISSOR_TEST);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glUseProgram(imageProgram_);
    glUniform2f(imageScaleUniform_, horizontalScale, verticalScale);
    glUniform2f(imageOffsetUniform_, imageOffsetX_, imageOffsetY_);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, imageTexture_);
    glBindVertexArray(imageVertexArray_);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glDisable(GL_BLEND);
  }

  drawHud();

  if (eglSwapBuffers(display_, surface_) != EGL_TRUE) {
    logEglFailure("eglSwapBuffers");
  }
}

bool EglRenderer::ready() const noexcept {
  return display_ != EGL_NO_DISPLAY && surface_ != EGL_NO_SURFACE && context_ != EGL_NO_CONTEXT;
}

void EglRenderer::setImage(tmnf::image::RgbaImage image) {
  image_ = std::move(image);
  if (ready()) {
    createImageResources();
  }
}

void EglRenderer::setHudFont(tmnf::image::RgbaImage image) {
  hudFont_ = std::move(image);
  if (ready()) {
    createHudResources();
  }
}

void EglRenderer::setScene(tmnf::content::TrackScene scene, tmnf::gbx::SolidModel vehicleSolid,
                           tmnf::content::SolidMaterialSet vehicleMaterials) {
  scene_ = std::move(scene);
  vehicleSolid_ = std::move(vehicleSolid);
  vehicleMaterials_ = std::move(vehicleMaterials);
  if (ready()) {
    createMeshResources();
  }
}

void EglRenderer::setVehiclePose(const tmnf::gbx::Float3 position, const float heading) noexcept {
  vehiclePosition_ = position;
  vehicleHeading_ = heading;
  vehicleVisible_ = true;
}

void EglRenderer::setRaceHud(const float raceTimeSeconds, const std::uint32_t checkpointCount,
                             const std::uint32_t checkpointTotal, const std::uint32_t completedLaps,
                             const std::uint32_t lapTotal, const bool finished) noexcept {
  hudRaceTimeSeconds_ =
      std::isfinite(raceTimeSeconds) && raceTimeSeconds >= 0.0F ? raceTimeSeconds : 0.0F;
  hudCheckpointCount_ = checkpointCount;
  hudCheckpointTotal_ = checkpointTotal;
  hudCompletedLaps_ = completedLaps;
  hudLapTotal_ = std::max(lapTotal, 1U);
  hudFinished_ = finished;
  hudVisible_ = true;
}

void EglRenderer::setMapSelector(const std::uint32_t mapIndex, const std::uint32_t mapTotal,
                                 const std::int32_t personalBestMilliseconds,
                                 const std::int32_t authorMilliseconds,
                                 const bool visible) noexcept {
  selectorMapTotal_ = mapTotal;
  selectorMapIndex_ = mapTotal == 0U ? 0U : std::min(mapIndex, mapTotal - 1U);
  selectorPersonalBestMilliseconds_ = personalBestMilliseconds;
  selectorAuthorMilliseconds_ = authorMilliseconds;
  selectorVisible_ = visible && mapTotal != 0U;
}

void EglRenderer::setImageOffset(const float x, const float y) noexcept {
  imageOffsetX_ = std::clamp(x, -0.75F, 0.75F);
  imageOffsetY_ = std::clamp(y, -0.35F, 0.35F);
}

bool EglRenderer::createImageResources() {
  if (image_.width <= 0 || image_.height <= 0 || image_.pixels.empty()) {
    return false;
  }
  if (imageTexture_ != 0) {
    glDeleteTextures(1, &imageTexture_);
    imageTexture_ = 0;
  }
  if (imageProgram_ == 0) {
    imageProgram_ = createImageProgram();
    if (imageProgram_ == 0) {
      return false;
    }
    imageScaleUniform_ = glGetUniformLocation(imageProgram_, "uScale");
    imageOffsetUniform_ = glGetUniformLocation(imageProgram_, "uOffset");
    glUseProgram(imageProgram_);
    glUniform1i(glGetUniformLocation(imageProgram_, "uTexture"), 0);
    glUseProgram(0);
  }
  if (imageVertexArray_ == 0) {
    glGenVertexArrays(1, &imageVertexArray_);
    glGenBuffers(1, &imageVertexBuffer_);
    constexpr std::array<float, 24> vertices{
        -1.0F, -1.0F, 0.0F, 1.0F, 1.0F, -1.0F, 1.0F, 1.0F, 1.0F,  1.0F, 1.0F, 0.0F,
        -1.0F, -1.0F, 0.0F, 1.0F, 1.0F, 1.0F,  1.0F, 0.0F, -1.0F, 1.0F, 0.0F, 0.0F,
    };
    glBindVertexArray(imageVertexArray_);
    glBindBuffer(GL_ARRAY_BUFFER, imageVertexBuffer_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          reinterpret_cast<const void*>(2 * sizeof(float)));
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
  }

  glGenTextures(1, &imageTexture_);
  glBindTexture(GL_TEXTURE_2D, imageTexture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, image_.width, image_.height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, image_.pixels.data());
  glBindTexture(GL_TEXTURE_2D, 0);
  __android_log_print(ANDROID_LOG_INFO, kLogTag, "Uploaded %dx%d texture extracted from NadeoPak",
                      image_.width, image_.height);
  return true;
}

bool EglRenderer::createHudResources() {
  if (imageProgram_ == 0 || hudFont_.width != 512 || hudFont_.height != 256 ||
      hudFont_.pixels.empty()) {
    return false;
  }
  if (hudTexture_ != 0) {
    glDeleteTextures(1, &hudTexture_);
    hudTexture_ = 0;
  }
  if (hudVertexArray_ != 0) {
    glDeleteVertexArrays(1, &hudVertexArray_);
    hudVertexArray_ = 0;
  }
  if (hudVertexBuffer_ != 0) {
    glDeleteBuffers(1, &hudVertexBuffer_);
    hudVertexBuffer_ = 0;
  }

  glGenVertexArrays(1, &hudVertexArray_);
  glGenBuffers(1, &hudVertexBuffer_);
  glBindVertexArray(hudVertexArray_);
  glBindBuffer(GL_ARRAY_BUFFER, hudVertexBuffer_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(HudVertex) * kMaximumHudVertices, nullptr, GL_DYNAMIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(HudVertex),
                        reinterpret_cast<const void*>(offsetof(HudVertex, position)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(HudVertex),
                        reinterpret_cast<const void*>(offsetof(HudVertex, textureCoordinate)));
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glGenTextures(1, &hudTexture_);
  glBindTexture(GL_TEXTURE_2D, hudTexture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, hudFont_.width, hudFont_.height, 0, GL_RGBA,
               GL_UNSIGNED_BYTE, hudFont_.pixels.data());
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glBindTexture(GL_TEXTURE_2D, 0);
  __android_log_print(ANDROID_LOG_INFO, kLogTag, "Original LED race HUD ready");
  return true;
}

void EglRenderer::drawHud() {
  if ((!hudVisible_ && !selectorVisible_) || hudTexture_ == 0 || hudVertexArray_ == 0 ||
      imageProgram_ == 0 || width_ <= 0 || height_ <= 0) {
    return;
  }

  std::array<char, 32> timer{};
  std::array<char, 32> checkpoint{};
  std::array<char, 32> lap{};
  const auto formatTime = [](std::array<char, 32>& output, const std::uint64_t totalMilliseconds,
                             const char* const prefix) {
    std::snprintf(output.data(), output.size(), "%s%02llu:%02llu.%03llu", prefix,
                  static_cast<unsigned long long>(totalMilliseconds / 60'000U),
                  static_cast<unsigned long long>(totalMilliseconds / 1'000U % 60U),
                  static_cast<unsigned long long>(totalMilliseconds % 1'000U));
  };
  const double boundedSeconds =
      std::clamp(static_cast<double>(hudRaceTimeSeconds_), 0.0, 359'999.999);
  formatTime(timer, static_cast<std::uint64_t>(boundedSeconds * 1000.0), "");
  std::snprintf(checkpoint.data(), checkpoint.size(), "CP %u/%u", hudCheckpointCount_,
                hudCheckpointTotal_);
  const std::uint32_t currentLap =
      hudFinished_ ? hudLapTotal_ : std::min(hudCompletedLaps_ + 1U, hudLapTotal_);
  std::snprintf(lap.data(), lap.size(), "LAP %u/%u", currentLap, hudLapTotal_);

  const float scale = std::clamp(static_cast<float>(height_) / 1080.0F, 0.65F, 1.25F);
  const float margin = 22.0F * scale;
  std::array<HudVertex, kMaximumHudVertices> vertices{};
  std::size_t vertexCount = 0;
  appendHudText(vertices, vertexCount, "M", margin, margin, scale, width_, height_);
  if (hudVisible_) {
    const std::string_view timerText(timer.data());
    const std::string_view checkpointText(checkpoint.data());
    appendHudText(vertices, vertexCount, timerText,
                  (static_cast<float>(width_) - hudTextWidth(timerText) * scale) * 0.5F, margin,
                  scale, width_, height_);
    appendHudText(vertices, vertexCount, checkpointText,
                  static_cast<float>(width_) - margin - hudTextWidth(checkpointText) * scale,
                  margin, scale, width_, height_);
    if (hudLapTotal_ > 1U) {
      const std::string_view lapText(lap.data());
      appendHudText(vertices, vertexCount, lapText,
                    static_cast<float>(width_) - margin - hudTextWidth(lapText) * scale,
                    margin + 68.0F * scale, scale, width_, height_);
    }
  }
  if (selectorVisible_) {
    std::array<char, 32> selectedMap{};
    std::array<char, 32> personalBest{};
    std::array<char, 32> authorTime{};
    std::snprintf(selectedMap.data(), selectedMap.size(), "MAP %02u/%02u", selectorMapIndex_ + 1U,
                  selectorMapTotal_);
    if (selectorPersonalBestMilliseconds_ > 0) {
      formatTime(personalBest, static_cast<std::uint64_t>(selectorPersonalBestMilliseconds_), "P ");
    }
    if (selectorAuthorMilliseconds_ > 0) {
      formatTime(authorTime, static_cast<std::uint64_t>(selectorAuthorMilliseconds_), "A ");
    }
    const float selectorTop = static_cast<float>(height_) * 0.34F;
    for (const auto [text, line] : {std::pair{std::string_view(selectedMap.data()), 0U},
                                    std::pair{std::string_view(personalBest.data()), 1U},
                                    std::pair{std::string_view(authorTime.data()), 2U}}) {
      if (text.empty()) {
        continue;
      }
      appendHudText(vertices, vertexCount, text,
                    (static_cast<float>(width_) - hudTextWidth(text) * scale) * 0.5F,
                    selectorTop + static_cast<float>(line) * 70.0F * scale, scale, width_, height_);
    }
  }
  if (vertexCount == 0U) {
    return;
  }

  glBindBuffer(GL_ARRAY_BUFFER, hudVertexBuffer_);
  glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<GLsizeiptr>(vertexCount * sizeof(HudVertex)),
                  vertices.data());
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glUseProgram(imageProgram_);
  glUniform2f(imageScaleUniform_, 1.0F, 1.0F);
  glUniform2f(imageOffsetUniform_, 0.0F, 0.0F);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, hudTexture_);
  glBindVertexArray(hudVertexArray_);
  glDrawArrays(GL_TRIANGLES, 0, static_cast<GLsizei>(vertexCount));
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glUseProgram(0);
  glDisable(GL_BLEND);
}

bool EglRenderer::createMeshResources() {
  if (meshVertexArray_ != 0) {
    glDeleteVertexArrays(1, &meshVertexArray_);
    meshVertexArray_ = 0;
  }
  if (meshVertexBuffer_ != 0) {
    glDeleteBuffers(1, &meshVertexBuffer_);
    meshVertexBuffer_ = 0;
  }
  if (meshIndexBuffer_ != 0) {
    glDeleteBuffers(1, &meshIndexBuffer_);
    meshIndexBuffer_ = 0;
  }
  if (meshFallbackTexture_ != 0) {
    glDeleteTextures(1, &meshFallbackTexture_);
    meshFallbackTexture_ = 0;
  }
  if (vehicleVertexArray_ != 0) {
    glDeleteVertexArrays(1, &vehicleVertexArray_);
    vehicleVertexArray_ = 0;
  }
  if (vehicleVertexBuffer_ != 0) {
    glDeleteBuffers(1, &vehicleVertexBuffer_);
    vehicleVertexBuffer_ = 0;
  }
  if (vehicleIndexBuffer_ != 0) {
    glDeleteBuffers(1, &vehicleIndexBuffer_);
    vehicleIndexBuffer_ = 0;
  }
  if (!meshMaterialTextures_.empty()) {
    glDeleteTextures(static_cast<GLsizei>(meshMaterialTextures_.size()),
                     meshMaterialTextures_.data());
    meshMaterialTextures_.clear();
  }
  meshDraws_.clear();
  vehicleDraws_.clear();
  meshIndexCount_ = 0;
  vehicleIndexCount_ = 0;
  if (scene_.models.empty() || scene_.placements.empty()) {
    return false;
  }

  if (meshProgram_ == 0) {
    meshProgram_ = createMeshProgram();
    if (meshProgram_ == 0) {
      return false;
    }
    meshMvpUniform_ = glGetUniformLocation(meshProgram_, "uMvp");
    meshTextureUniform_ = glGetUniformLocation(meshProgram_, "uTexture");
    glUseProgram(meshProgram_);
    glUniform1i(meshTextureUniform_, 0);
    glUseProgram(0);
  }

  glGenTextures(1, &meshFallbackTexture_);
  glBindTexture(GL_TEXTURE_2D, meshFallbackTexture_);
  constexpr std::array<std::uint8_t, 4> white{255, 255, 255, 255};
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white.data());

  const std::size_t trackTextureCount = scene_.textures.size();
  meshMaterialTextures_.resize(trackTextureCount + vehicleMaterials_.textures.size());
  if (!meshMaterialTextures_.empty()) {
    glGenTextures(static_cast<GLsizei>(meshMaterialTextures_.size()), meshMaterialTextures_.data());
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    for (std::size_t index = 0; index < meshMaterialTextures_.size(); ++index) {
      const image::RgbaImage& materialImage =
          index < trackTextureCount ? scene_.textures[index].image
                                    : vehicleMaterials_.textures[index - trackTextureCount].image;
      if (materialImage.width <= 0 || materialImage.height <= 0 || materialImage.pixels.empty()) {
        continue;
      }
      glBindTexture(GL_TEXTURE_2D, meshMaterialTextures_[index]);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
      glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, materialImage.width, materialImage.height, 0,
                   GL_RGBA, GL_UNSIGNED_BYTE, materialImage.pixels.data());
      glGenerateMipmap(GL_TEXTURE_2D);
    }
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  }
  glBindTexture(GL_TEXTURE_2D, 0);

  std::vector<GpuVertex> vertices;
  std::vector<bool> needsNormal;
  std::vector<std::uint32_t> indices;
  Vec3 minimum{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(),
               std::numeric_limits<float>::max()};
  Vec3 maximum{std::numeric_limits<float>::lowest(), std::numeric_limits<float>::lowest(),
               std::numeric_limits<float>::lowest()};

  for (const content::TrackPlacement& placement : scene_.placements) {
    if (placement.blockIndex >= scene_.challenge.blocks.size() ||
        placement.modelIndex >= scene_.models.size()) {
      continue;
    }
    const gbx::ChallengeBlock& block = scene_.challenge.blocks[placement.blockIndex];
    const content::TrackModel& model = scene_.models[placement.modelIndex];
    for (std::size_t visualIndex = 0; visualIndex < model.solid.visuals.size(); ++visualIndex) {
      const gbx::SolidVisual& visual = model.solid.visuals[visualIndex];
      if (!visual.renderEnabled || visual.positions.empty() || visual.indices.empty()) {
        continue;
      }
      const std::uint32_t vertexBase = static_cast<std::uint32_t>(vertices.size());
      const int firstIndex = static_cast<int>(indices.size());
      for (std::size_t vertexIndex = 0; vertexIndex < visual.positions.size(); ++vertexIndex) {
        const Vec3 localPosition = transformPosition(visual, visual.positions[vertexIndex]);
        const Vec3 position = placeBlockPosition(localPosition, block);
        minimum.x = std::min(minimum.x, position.x);
        minimum.y = std::min(minimum.y, position.y);
        minimum.z = std::min(minimum.z, position.z);
        maximum.x = std::max(maximum.x, position.x);
        maximum.y = std::max(maximum.y, position.y);
        maximum.z = std::max(maximum.z, position.z);

        Vec3 transformedNormal{0.0F, 0.0F, 0.0F};
        if (vertexIndex < visual.normals.size()) {
          transformedNormal = rotateBlockVector(
              transformNormal(visual, visual.normals[vertexIndex]), block.direction);
        }
        const bool hasNormal = dot(transformedNormal, transformedNormal) > 1.0e-12F;
        const Vec3 normal = hasNormal ? normalize(transformedNormal) : Vec3{0.0F, 0.0F, 0.0F};
        Vec2 textureCoordinate{0.0F, 0.0F};
        if (!visual.textureCoordinates.empty() &&
            vertexIndex < visual.textureCoordinates.front().size()) {
          const gbx::Float2 source = visual.textureCoordinates.front()[vertexIndex];
          // TMF DDS rows and Direct3D-authored UVs use a top-left image origin. GLES
          // samples uploaded texture rows from the opposite V origin, so reflect only V.
          textureCoordinate = {source.x, 1.0F - source.y};
        }
        vertices.push_back({position, normal, {1.0F, 1.0F, 1.0F}, textureCoordinate});
        needsNormal.push_back(!hasNormal);
      }
      for (const std::uint32_t index : visual.indices) {
        if (index < visual.positions.size()) {
          indices.push_back(vertexBase + index);
        }
      }
      MeshDraw draw;
      draw.firstIndex = firstIndex;
      draw.indexCount = static_cast<int>(indices.size()) - firstIndex;
      if (visualIndex < model.visualTextureIndices.size()) {
        const std::size_t textureIndex = model.visualTextureIndices[visualIndex];
        if (textureIndex < meshMaterialTextures_.size()) {
          draw.texture = meshMaterialTextures_[textureIndex];
        }
      }
      if (draw.indexCount > 0) {
        meshDraws_.push_back(draw);
      }
    }
  }
  if (vertices.empty() || indices.empty()) {
    return false;
  }

  const Vec3 center = (minimum + maximum) * 0.5F;
  const Vec3 size = maximum - minimum;
  const float halfExtent = std::max({size.x, size.y, size.z}) * 0.5F;
  const float inverseExtent = halfExtent > 1.0e-6F ? 1.0F / halfExtent : 1.0F;
  meshWorldCenter_ = {center.x, center.y, center.z};
  meshWorldInverseExtent_ = inverseExtent;
  __android_log_print(ANDROID_LOG_INFO, kLogTag,
                      "Scene normalization: center=(%.2f, %.2f, %.2f), inverseExtent=%.7f",
                      center.x, center.y, center.z, inverseExtent);
  for (GpuVertex& vertex : vertices) {
    vertex.position = (vertex.position - center) * inverseExtent;
  }

  for (std::size_t triangle = 0; triangle + 2 < indices.size(); triangle += 3) {
    const std::uint32_t a = indices[triangle];
    const std::uint32_t b = indices[triangle + 1];
    const std::uint32_t c = indices[triangle + 2];
    if (a >= vertices.size() || b >= vertices.size() || c >= vertices.size()) {
      continue;
    }
    const Vec3 face = cross(vertices[b].position - vertices[a].position,
                            vertices[c].position - vertices[a].position);
    if (needsNormal[a]) {
      vertices[a].normal = vertices[a].normal + face;
    }
    if (needsNormal[b]) {
      vertices[b].normal = vertices[b].normal + face;
    }
    if (needsNormal[c]) {
      vertices[c].normal = vertices[c].normal + face;
    }
  }
  for (GpuVertex& vertex : vertices) {
    vertex.normal = normalize(vertex.normal);
  }

  glGenVertexArrays(1, &meshVertexArray_);
  glGenBuffers(1, &meshVertexBuffer_);
  glGenBuffers(1, &meshIndexBuffer_);
  glBindVertexArray(meshVertexArray_);
  glBindBuffer(GL_ARRAY_BUFFER, meshVertexBuffer_);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vertices.size() * sizeof(GpuVertex)),
               vertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, meshIndexBuffer_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(indices.size() * sizeof(std::uint32_t)), indices.data(),
               GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GpuVertex),
                        reinterpret_cast<const void*>(offsetof(GpuVertex, position)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GpuVertex),
                        reinterpret_cast<const void*>(offsetof(GpuVertex, normal)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GpuVertex),
                        reinterpret_cast<const void*>(offsetof(GpuVertex, color)));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(GpuVertex),
                        reinterpret_cast<const void*>(offsetof(GpuVertex, textureCoordinate)));
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  std::vector<GpuVertex> vehicleVertices;
  std::vector<std::uint32_t> vehicleIndices;
  for (std::size_t visualIndex = 0; visualIndex < vehicleSolid_.visuals.size(); ++visualIndex) {
    const gbx::SolidVisual& visual = vehicleSolid_.visuals[visualIndex];
    if (!visual.renderEnabled || visual.positions.empty() || visual.indices.empty()) {
      continue;
    }
    const std::uint32_t vertexBase = static_cast<std::uint32_t>(vehicleVertices.size());
    const int firstIndex = static_cast<int>(vehicleIndices.size());
    for (std::size_t vertexIndex = 0; vertexIndex < visual.positions.size(); ++vertexIndex) {
      Vec3 normal{0.0F, 1.0F, 0.0F};
      if (vertexIndex < visual.normals.size()) {
        normal = normalize(transformNormal(visual, visual.normals[vertexIndex]));
      }
      Vec2 textureCoordinate{0.0F, 0.0F};
      if (!visual.textureCoordinates.empty() &&
          vertexIndex < visual.textureCoordinates.front().size()) {
        const gbx::Float2 source = visual.textureCoordinates.front()[vertexIndex];
        textureCoordinate = {source.x, 1.0F - source.y};
      }
      vehicleVertices.push_back({transformPosition(visual, visual.positions[vertexIndex]),
                                 normal,
                                 {1.0F, 1.0F, 1.0F},
                                 textureCoordinate});
    }
    for (const std::uint32_t index : visual.indices) {
      if (index < visual.positions.size()) {
        vehicleIndices.push_back(vertexBase + index);
      }
    }
    MeshDraw draw;
    draw.firstIndex = firstIndex;
    draw.indexCount = static_cast<int>(vehicleIndices.size()) - firstIndex;
    if (visualIndex < vehicleMaterials_.visualTextureIndices.size()) {
      const std::size_t textureIndex = vehicleMaterials_.visualTextureIndices[visualIndex];
      if (textureIndex < vehicleMaterials_.textures.size()) {
        draw.texture = meshMaterialTextures_[trackTextureCount + textureIndex];
      }
    }
    if (draw.indexCount > 0) {
      vehicleDraws_.push_back(draw);
    }
  }
  if (vehicleVertices.empty() || vehicleIndices.empty()) {
    return false;
  }
  glGenVertexArrays(1, &vehicleVertexArray_);
  glGenBuffers(1, &vehicleVertexBuffer_);
  glGenBuffers(1, &vehicleIndexBuffer_);
  glBindVertexArray(vehicleVertexArray_);
  glBindBuffer(GL_ARRAY_BUFFER, vehicleVertexBuffer_);
  glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(vehicleVertices.size() * sizeof(GpuVertex)),
               vehicleVertices.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vehicleIndexBuffer_);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER,
               static_cast<GLsizeiptr>(vehicleIndices.size() * sizeof(std::uint32_t)),
               vehicleIndices.data(), GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GpuVertex),
                        reinterpret_cast<const void*>(offsetof(GpuVertex, position)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GpuVertex),
                        reinterpret_cast<const void*>(offsetof(GpuVertex, normal)));
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(GpuVertex),
                        reinterpret_cast<const void*>(offsetof(GpuVertex, color)));
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(GpuVertex),
                        reinterpret_cast<const void*>(offsetof(GpuVertex, textureCoordinate)));
  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  vehicleIndexCount_ = static_cast<int>(vehicleIndices.size());
  meshIndexCount_ = static_cast<int>(indices.size());
  __android_log_print(ANDROID_LOG_INFO, kLogTag,
                      "Uploaded scene: %zu placements, %zu models, %zu vertices, %d indices, "
                      "%zu resolved material textures; StadiumCar low LOD: %zu vertices, %d "
                      "indices, %zu draws",
                      scene_.placements.size(), scene_.models.size(), vertices.size(),
                      meshIndexCount_, meshMaterialTextures_.size(), vehicleVertices.size(),
                      vehicleIndexCount_, vehicleDraws_.size());
  return true;
}

} // namespace tmnf::android
