//
//  Created by Sam Gateau on 2017/04/13
//  Copyright 2013-2017 High Fidelity, Inc.
//
//  Distributed under the Apache License, Version 2.0.
//  See the accompanying file LICENSE or http://www.apache.org/licenses/LICENSE-2.0.html
//
#include "GLBackend.h"
#include "GLShader.h"
#include <gl/GLShaders.h>

using namespace gpu;
using namespace gpu::gl;
using CachedShader = ::gl::CachedShader;

// Shader domain
static const size_t NUM_SHADER_DOMAINS = 3;
static_assert(Shader::Type::NUM_DOMAINS == NUM_SHADER_DOMAINS, "GL shader domains must equal defined GPU shader domains");

// GL Shader type enums
// Must match the order of type specified in gpu::Shader::Type
static const std::array<GLenum, NUM_SHADER_DOMAINS> SHADER_DOMAINS{ {
    GL_VERTEX_SHADER,
    GL_FRAGMENT_SHADER,
    GL_GEOMETRY_SHADER,
} };

// Domain specific defines
// Must match the order of type specified in gpu::Shader::Type
static const std::array<std::string, NUM_SHADER_DOMAINS> DOMAIN_DEFINES{ {
    "#define GPU_VERTEX_SHADER",
    "#define GPU_PIXEL_SHADER",
    "#define GPU_GEOMETRY_SHADER",
} };

// Stereo specific defines
static const std::string stereoVersion{
#ifdef GPU_STEREO_DRAWCALL_INSTANCED
R"SHADER(
#define GPU_TRANSFORM_IS_STEREO
#define GPU_TRANSFORM_STEREO_CAMERA
#define GPU_TRANSFORM_STEREO_CAMERA_INSTANCED
#define GPU_TRANSFORM_STEREO_SPLIT_SCREEN
)SHADER"
#endif
#ifdef GPU_STEREO_DRAWCALL_DOUBLED
#ifdef GPU_STEREO_CAMERA_BUFFER
R"SHADER(
#define GPU_TRANSFORM_IS_STEREO
#define GPU_TRANSFORM_STEREO_CAMERA
#define GPU_TRANSFORM_STEREO_CAMERA_ATTRIBUTED
)SHADER"
#else
R"SHADER(
#define GPU_TRANSFORM_IS_STEREO
)SHADER"
#endif
#endif
};

// TextureTable specific defines
static const std::string textureTableVersion {
    "#extension GL_ARB_bindless_texture : require\n#define GPU_TEXTURE_TABLE_BINDLESS\n"
};

// Versions specific of the shader
static const std::array<std::string, GLShader::NumVersions> VERSION_DEFINES { {
    "",
    stereoVersion
} };

static std::string getShaderTypeString(Shader::Type type) {
    switch (type) {
    case Shader::Type::VERTEX:
        return "vertex";
    case Shader::Type::PIXEL:
        return "pixel";
    case Shader::Type::GEOMETRY:
        return "geometry";
    case Shader::Type::PROGRAM:
        return "program";
    default:
        qFatal("Unexpected shader type %d", type);
        Q_UNREACHABLE();
    }
}

std::string GLBackend::getShaderSource(const Shader& shader, int version) {
    if (shader.isProgram()) {
        std::string result;
        result.append("// VERSION " + std::to_string(version));
        for (const auto& subShader : shader.getShaders()) {
            result.append("//-------- ");
            result.append(getShaderTypeString(subShader->getType()));
            result.append("\n");
            result.append(subShader->getSource().getCode());
        }
        return result;
    } 

    std::string shaderDefines = getBackendShaderHeader() + "\n"
        + (supportsBindless() ? textureTableVersion : "\n")
        + DOMAIN_DEFINES[shader.getType()] + "\n"
        + VERSION_DEFINES[version];

    return shaderDefines + "\n" + shader.getSource().getCode();
}

GLShader* GLBackend::compileBackendShader(const Shader& shader, const Shader::CompilationHandler& handler) {
    // Any GLSLprogram ? normally yes...
    GLenum shaderDomain = SHADER_DOMAINS[shader.getType()];
    GLShader::ShaderObjects shaderObjects;
    Shader::CompilationLogs compilationLogs(GLShader::NumVersions);
    shader.incrementCompilationAttempt();

    for (int version = 0; version < GLShader::NumVersions; version++) {
        auto& shaderObject = shaderObjects[version];
        auto shaderSource = getShaderSource(shader, version);
        if (handler) {
            bool retest = true;
            std::string currentSrc = shaderSource;
            // When a Handler is specified, we can try multiple times to build the shader and let the handler change the source if the compilation fails.
            // The retest bool is set to false as soon as the compilation succeed to wexit the while loop.
            // The handler tells us if we should retry or not while returning a modified version of the source.
            while (retest) {
                bool result = ::gl::compileShader(shaderDomain, currentSrc, shaderObject.glshader, compilationLogs[version].message);
                compilationLogs[version].compiled = result;
                if (!result) {
                    std::string newSrc;
                    retest = handler(shader, currentSrc, compilationLogs[version], newSrc);
                    currentSrc = newSrc;
                } else {
                    retest = false;
                }
            }
        } else {
            compilationLogs[version].compiled = ::gl::compileShader(shaderDomain, shaderSource, shaderObject.glshader, compilationLogs[version].message);
        }

        if (!compilationLogs[version].compiled) {
            qCWarning(gpugllogging) << "GLBackend::compileBackendProgram - Shader didn't compile:\n" << compilationLogs[version].message.c_str();
            shader.setCompilationLogs(compilationLogs);
            return nullptr;
        }
    }
    // Compilation feedback
    shader.setCompilationLogs(compilationLogs);

    // So far so good, the shader is created successfully
    GLShader* object = new GLShader(this->shared_from_this());
    object->_shaderObjects = shaderObjects;

    return object;
}

std::atomic<size_t> gpuBinaryShadersLoaded;

GLShader* GLBackend::compileBackendProgram(const Shader& program, const Shader::CompilationHandler& handler) {
    if (!program.isProgram()) {
        return nullptr;
    }

    GLShader::ShaderObjects programObjects;
    program.incrementCompilationAttempt();
    Shader::CompilationLogs compilationLogs(GLShader::NumVersions);

    for (int version = 0; version < GLShader::NumVersions; version++) {
        auto& programObject = programObjects[version];
        auto programSource = getShaderSource(program, version);
        auto hash = ::gl::getShaderHash(programSource);

        CachedShader cachedBinary;
        {
            Lock shaderCacheLock{ _shaderBinaryCache._mutex };
            if (_shaderBinaryCache._binaries.count(hash) != 0) {
                cachedBinary = _shaderBinaryCache._binaries[hash];
            }
        }

        GLuint glprogram = 0;
        // If we have a cached binary program, try to load it instead of compiling the individual shaders
        if (cachedBinary) {
            glprogram = ::gl::buildProgram(cachedBinary);
            if (0 != glprogram) {
                ++gpuBinaryShadersLoaded;
            } else {
                cachedBinary = CachedShader();
                std::unique_lock<std::mutex> shaderCacheLock{ _shaderBinaryCache._mutex };
                _shaderBinaryCache._binaries.erase(hash);
            }
        }

        // If we have no program, then either no cached binary, or the binary failed to load 
        // (perhaps a GPU driver update invalidated the cache)
        if (0 == glprogram) {
            // Let's go through every shaders and make sure they are ready to go
            std::vector<GLuint> shaderGLObjects;
            shaderGLObjects.reserve(program.getShaders().size());
            for (auto subShader : program.getShaders()) {
                auto object = GLShader::sync((*this), *subShader, handler);
                if (object) {
                    shaderGLObjects.push_back(object->_shaderObjects[version].glshader);
                } else {
                    qCWarning(gpugllogging) << "GLBackend::compileBackendProgram - One of the shaders of the program is not compiled?";
                    compilationLogs[version].compiled = false;
                    compilationLogs[version].message = std::string("Failed to compile, one of the shaders of the program is not compiled ?");
                    program.setCompilationLogs(compilationLogs);
                    return nullptr;
                }
            }

            glprogram = ::gl::buildProgram(shaderGLObjects);

            if (!::gl::linkProgram(glprogram, compilationLogs[version].message)) {
                glDeleteProgram(glprogram);
                glprogram = 0;
                return nullptr;
            }

            if (!cachedBinary) {
                ::gl::getProgramBinary(glprogram, cachedBinary);
                cachedBinary.source = programSource;
                std::unique_lock<std::mutex> shaderCacheLock{ _shaderBinaryCache._mutex };
                _shaderBinaryCache._binaries[hash] = cachedBinary;
            }
        }

        if (glprogram == 0) {
            qCWarning(gpugllogging) << "GLBackend::compileBackendProgram - Program didn't link:\n" << compilationLogs[version].message.c_str(); 
            program.setCompilationLogs(compilationLogs);
            return nullptr;
        }

        compilationLogs[version].compiled = true;
        programObject.glprogram = glprogram;
        postLinkProgram(programObject, program);
    }
    // Compilation feedback
    program.setCompilationLogs(compilationLogs);

    // So far so good, the program versions have all been created successfully
    GLShader* object = new GLShader(this->shared_from_this());
    object->_shaderObjects = programObjects;
    return object;
}


GLBackend::ElementResource GLBackend::getFormatFromGLUniform(GLenum gltype) {
    switch (gltype) {
    case GL_FLOAT:
        return ElementResource(Element(SCALAR, gpu::FLOAT, UNIFORM), Resource::BUFFER);
    case GL_FLOAT_VEC2:
        return ElementResource(Element(VEC2, gpu::FLOAT, UNIFORM), Resource::BUFFER);
    case GL_FLOAT_VEC3:
        return ElementResource(Element(VEC3, gpu::FLOAT, UNIFORM), Resource::BUFFER);
    case GL_FLOAT_VEC4:
        return ElementResource(Element(VEC4, gpu::FLOAT, UNIFORM), Resource::BUFFER);

    case GL_INT:
        return ElementResource(Element(SCALAR, gpu::INT32, UNIFORM), Resource::BUFFER);
    case GL_INT_VEC2:
        return ElementResource(Element(VEC2, gpu::INT32, UNIFORM), Resource::BUFFER);
    case GL_INT_VEC3:
        return ElementResource(Element(VEC3, gpu::INT32, UNIFORM), Resource::BUFFER);
    case GL_INT_VEC4:
        return ElementResource(Element(VEC4, gpu::INT32, UNIFORM), Resource::BUFFER);

    case GL_UNSIGNED_INT:
        return ElementResource(Element(SCALAR, gpu::UINT32, UNIFORM), Resource::BUFFER);
    case GL_UNSIGNED_INT_VEC2:
        return ElementResource(Element(VEC2, gpu::UINT32, UNIFORM), Resource::BUFFER);
    case GL_UNSIGNED_INT_VEC3:
        return ElementResource(Element(VEC3, gpu::UINT32, UNIFORM), Resource::BUFFER);
    case GL_UNSIGNED_INT_VEC4:
        return ElementResource(Element(VEC4, gpu::UINT32, UNIFORM), Resource::BUFFER);

    case GL_BOOL:
        return ElementResource(Element(SCALAR, gpu::BOOL, UNIFORM), Resource::BUFFER);
    case GL_BOOL_VEC2:
        return ElementResource(Element(VEC2, gpu::BOOL, UNIFORM), Resource::BUFFER);
    case GL_BOOL_VEC3:
        return ElementResource(Element(VEC3, gpu::BOOL, UNIFORM), Resource::BUFFER);
    case GL_BOOL_VEC4:
        return ElementResource(Element(VEC4, gpu::BOOL, UNIFORM), Resource::BUFFER);

    case GL_FLOAT_MAT2:
        return ElementResource(Element(gpu::MAT2, gpu::FLOAT, UNIFORM), Resource::BUFFER);
    case GL_FLOAT_MAT3:
        return ElementResource(Element(MAT3, gpu::FLOAT, UNIFORM), Resource::BUFFER);
    case GL_FLOAT_MAT4:
        return ElementResource(Element(MAT4, gpu::FLOAT, UNIFORM), Resource::BUFFER);

    case GL_SAMPLER_2D:
        return ElementResource(Element(SCALAR, gpu::FLOAT, SAMPLER), Resource::TEXTURE_2D);
    case GL_SAMPLER_3D:
        return ElementResource(Element(SCALAR, gpu::FLOAT, SAMPLER), Resource::TEXTURE_3D);
    case GL_SAMPLER_CUBE:
        return ElementResource(Element(SCALAR, gpu::FLOAT, SAMPLER), Resource::TEXTURE_CUBE);
    case GL_SAMPLER_2D_MULTISAMPLE:
        return ElementResource(Element(SCALAR, gpu::FLOAT, SAMPLER_MULTISAMPLE), Resource::TEXTURE_2D);
    case GL_SAMPLER_2D_ARRAY:
        return ElementResource(Element(SCALAR, gpu::FLOAT, SAMPLER), Resource::TEXTURE_2D_ARRAY);
    case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
        return ElementResource(Element(SCALAR, gpu::FLOAT, SAMPLER_MULTISAMPLE), Resource::TEXTURE_2D_ARRAY);
    case GL_SAMPLER_2D_SHADOW:
        return ElementResource(Element(SCALAR, gpu::FLOAT, SAMPLER_SHADOW), Resource::TEXTURE_2D);
    case GL_SAMPLER_CUBE_SHADOW:
        return ElementResource(Element(SCALAR, gpu::FLOAT, SAMPLER_SHADOW), Resource::TEXTURE_CUBE);
    case GL_SAMPLER_2D_ARRAY_SHADOW:
        return ElementResource(Element(SCALAR, gpu::FLOAT, SAMPLER_SHADOW), Resource::TEXTURE_2D_ARRAY);
    case GL_SAMPLER_BUFFER:
        return ElementResource(Element(SCALAR, gpu::FLOAT, RESOURCE_BUFFER), Resource::BUFFER);
    case GL_INT_SAMPLER_2D:
        return ElementResource(Element(SCALAR, gpu::INT32, SAMPLER), Resource::TEXTURE_2D);
    case GL_INT_SAMPLER_2D_MULTISAMPLE:
        return ElementResource(Element(SCALAR, gpu::INT32, SAMPLER_MULTISAMPLE), Resource::TEXTURE_2D);
    case GL_INT_SAMPLER_3D:
        return ElementResource(Element(SCALAR, gpu::INT32, SAMPLER), Resource::TEXTURE_3D);
    case GL_INT_SAMPLER_CUBE:
        return ElementResource(Element(SCALAR, gpu::INT32, SAMPLER), Resource::TEXTURE_CUBE);
    case GL_INT_SAMPLER_2D_ARRAY:
        return ElementResource(Element(SCALAR, gpu::INT32, SAMPLER), Resource::TEXTURE_2D_ARRAY);
    case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        return ElementResource(Element(SCALAR, gpu::INT32, SAMPLER_MULTISAMPLE), Resource::TEXTURE_2D_ARRAY);
    case GL_UNSIGNED_INT_SAMPLER_2D:
        return ElementResource(Element(SCALAR, gpu::UINT32, SAMPLER), Resource::TEXTURE_2D);
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
        return ElementResource(Element(SCALAR, gpu::UINT32, SAMPLER_MULTISAMPLE), Resource::TEXTURE_2D);
    case GL_UNSIGNED_INT_SAMPLER_3D:
        return ElementResource(Element(SCALAR, gpu::UINT32, SAMPLER), Resource::TEXTURE_3D);
    case GL_UNSIGNED_INT_SAMPLER_CUBE:
        return ElementResource(Element(SCALAR, gpu::UINT32, SAMPLER), Resource::TEXTURE_CUBE);
    case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
        return ElementResource(Element(SCALAR, gpu::UINT32, SAMPLER), Resource::TEXTURE_2D_ARRAY);
    case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
        return ElementResource(Element(SCALAR, gpu::UINT32, SAMPLER_MULTISAMPLE), Resource::TEXTURE_2D_ARRAY);

#if !defined(USE_GLES)
    case GL_SAMPLER_1D:
        return ElementResource(Element(SCALAR, gpu::FLOAT, SAMPLER), Resource::TEXTURE_1D);
    case GL_SAMPLER_1D_ARRAY:
        return ElementResource(Element(SCALAR, gpu::FLOAT, SAMPLER), Resource::TEXTURE_1D_ARRAY);
    case GL_INT_SAMPLER_1D:
        return ElementResource(Element(SCALAR, gpu::INT32, SAMPLER), Resource::TEXTURE_1D);
    case GL_INT_SAMPLER_1D_ARRAY:
        return ElementResource(Element(SCALAR, gpu::INT32, SAMPLER), Resource::TEXTURE_1D_ARRAY);
    case GL_UNSIGNED_INT_SAMPLER_1D:
        return ElementResource(Element(SCALAR, gpu::UINT32, SAMPLER), Resource::TEXTURE_1D);
    case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY:
        return ElementResource(Element(SCALAR, gpu::UINT32, SAMPLER), Resource::TEXTURE_1D_ARRAY);
#endif

    default:
        return ElementResource(Element(), Resource::BUFFER);
    }

    // Non-covered types
    //{GL_FLOAT_MAT2x3    mat2x3},
    //{GL_FLOAT_MAT2x4    mat2x4},
    //{GL_FLOAT_MAT3x2    mat3x2},
    //{GL_FLOAT_MAT3x4    mat3x4},
    //{GL_FLOAT_MAT4x2    mat4x2},
    //{GL_FLOAT_MAT4x3    mat4x3},
    //{GL_DOUBLE_MAT2    dmat2},
    //{GL_DOUBLE_MAT3    dmat3},
    //{GL_DOUBLE_MAT4    dmat4},
    //{GL_DOUBLE_MAT2x3    dmat2x3},
    //{GL_DOUBLE_MAT2x4    dmat2x4},
    //{GL_DOUBLE_MAT3x2    dmat3x2},
    //{GL_DOUBLE_MAT3x4    dmat3x4},
    //{GL_DOUBLE_MAT4x2    dmat4x2},
    //{GL_DOUBLE_MAT4x3    dmat4x3},
    //{GL_SAMPLER_1D_SHADOW    sampler1DShadow},
    //{GL_SAMPLER_1D_ARRAY_SHADOW    sampler1DArrayShadow},
    //{GL_SAMPLER_2D_RECT    sampler2DRect},
    //{GL_SAMPLER_2D_RECT_SHADOW    sampler2DRectShadow},
    //{GL_INT_SAMPLER_BUFFER    isamplerBuffer},
    //{GL_INT_SAMPLER_2D_RECT    isampler2DRect},
    //{GL_UNSIGNED_INT_SAMPLER_BUFFER    usamplerBuffer},
    //{GL_UNSIGNED_INT_SAMPLER_2D_RECT    usampler2DRect},
    //{GL_IMAGE_1D    image1D},
    //{GL_IMAGE_2D    image2D},
    //{GL_IMAGE_3D    image3D},
    //{GL_IMAGE_2D_RECT    image2DRect},
    //{GL_IMAGE_CUBE    imageCube},
    //{GL_IMAGE_BUFFER    imageBuffer},
    //{GL_IMAGE_1D_ARRAY    image1DArray},
    //{GL_IMAGE_2D_ARRAY    image2DArray},
    //{GL_IMAGE_2D_MULTISAMPLE    image2DMS},
    //{GL_IMAGE_2D_MULTISAMPLE_ARRAY    image2DMSArray},
    //{GL_INT_IMAGE_1D    iimage1D},
    //{GL_INT_IMAGE_2D    iimage2D},
    //{GL_INT_IMAGE_3D    iimage3D},
    //{GL_INT_IMAGE_2D_RECT    iimage2DRect},
    //{GL_INT_IMAGE_CUBE    iimageCube},
    //{GL_INT_IMAGE_BUFFER    iimageBuffer},
    //{GL_INT_IMAGE_1D_ARRAY    iimage1DArray},
    //{GL_INT_IMAGE_2D_ARRAY    iimage2DArray},
    //{GL_INT_IMAGE_2D_MULTISAMPLE    iimage2DMS},
    //{GL_INT_IMAGE_2D_MULTISAMPLE_ARRAY    iimage2DMSArray},
    //{GL_UNSIGNED_INT_IMAGE_1D    uimage1D},
    //{GL_UNSIGNED_INT_IMAGE_2D    uimage2D},
    //{GL_UNSIGNED_INT_IMAGE_3D    uimage3D},
    //{GL_UNSIGNED_INT_IMAGE_2D_RECT    uimage2DRect},
    //{GL_UNSIGNED_INT_IMAGE_CUBE    uimageCube},
    //{GL_UNSIGNED_INT_IMAGE_BUFFER    uimageBuffer},
    //{GL_UNSIGNED_INT_IMAGE_1D_ARRAY    uimage1DArray},
    //{GL_UNSIGNED_INT_IMAGE_2D_ARRAY    uimage2DArray},
    //{GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE    uimage2DMS},
    //{GL_UNSIGNED_INT_IMAGE_2D_MULTISAMPLE_ARRAY    uimage2DMSArray},
    //{GL_UNSIGNED_INT_ATOMIC_COUNTER    atomic_uint}
};

void GLBackend::initShaderBinaryCache() {
    GLint numBinFormats = 0;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &numBinFormats);
    if (numBinFormats > 0) {
        _shaderBinaryCache._formats.resize(numBinFormats);
        glGetIntegerv(GL_PROGRAM_BINARY_FORMATS, _shaderBinaryCache._formats.data());
    }
    ::gl::loadShaderCache(_shaderBinaryCache._binaries);
}

void GLBackend::killShaderBinaryCache() {
    ::gl::saveShaderCache(_shaderBinaryCache._binaries);
}
