#include "SlangShaderCompiler.h"
#include "Engine/Logger/SynLog.h"
#include <slang.h>
#include <slang-com-ptr.h>
#include <vector>

namespace Syn {

    std::vector<uint32_t> SlangShaderCompiler::Compile(const CookedShader& shader) {
        using namespace slang;

        Slang::ComPtr<IGlobalSession> globalSession;
        createGlobalSession(globalSession.writeRef());

        TargetDesc targetDesc = {};
        targetDesc.format = SLANG_SPIRV;
        targetDesc.profile = globalSession->findProfile("sm_6_5");
        targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;

        const char* searchPaths[] = { "Assets/Shaders", "Engine/Shaders" };

        std::vector<PreprocessorMacroDesc> macros;
        macros.reserve(shader.defines.size());
        for (const auto& def : shader.defines) {
            macros.push_back({ def.c_str(), "1" });
        }

        SessionDesc sessionDesc = {};
        sessionDesc.targets = &targetDesc;
        sessionDesc.targetCount = 1;
        sessionDesc.searchPaths = searchPaths;
        sessionDesc.searchPathCount = 2;
        sessionDesc.preprocessorMacros = macros.data();
        sessionDesc.preprocessorMacroCount = static_cast<uint32_t>(macros.size());

        Slang::ComPtr<ISession> session;
        globalSession->createSession(sessionDesc, session.writeRef());

        Slang::ComPtr<IBlob> diagnosticBlob;
        IModule* slangModule = session->loadModuleFromSourceString(
            shader.identifier.c_str(),
            shader.identifier.c_str(),
            shader.sourceCode.c_str(),
            diagnosticBlob.writeRef()
        );

        if (diagnosticBlob && diagnosticBlob->getBufferSize() > 0) {
            Error("Slang Compilation Output for {}:\n{}", shader.identifier, (const char*)diagnosticBlob->getBufferPointer());
        }

        if (!slangModule) {
            SYN_ASSERT(false, "Slang shader module compilation failed!");
            return {};
        }

        Slang::ComPtr<IEntryPoint> entryPoint;
        slangModule->findEntryPointByName("main", entryPoint.writeRef());
        if (!entryPoint) {
            Error("Failed to find 'main' entry point in slang shader: {}", shader.identifier);
            return {};
        }

        IComponentType* componentTypes[] = { slangModule, entryPoint.get() };
        Slang::ComPtr<IComponentType> linkedProgram;
        session->createCompositeComponentType(
            componentTypes, 2, linkedProgram.writeRef(), diagnosticBlob.writeRef()
        );

        if (diagnosticBlob && diagnosticBlob->getBufferSize() > 0) {
            Error("Slang Linking Output for {}:\n{}", shader.identifier, (const char*)diagnosticBlob->getBufferPointer());
        }

        Slang::ComPtr<IBlob> spirvBlob;
        linkedProgram->getEntryPointCode(0, 0, spirvBlob.writeRef(), diagnosticBlob.writeRef());

        if (!spirvBlob) {
            Error("Failed to generate SPIR-V for: {}", shader.identifier);
            return {};
        }

        const uint32_t* spirvData = reinterpret_cast<const uint32_t*>(spirvBlob->getBufferPointer());
        size_t spirvSize = spirvBlob->getBufferSize() / sizeof(uint32_t);

        return std::vector<uint32_t>(spirvData, spirvData + spirvSize);
    }

    SlangStage SlangShaderCompiler::MapStageToSlang(VkShaderStageFlagBits stage) const {
        switch (stage) {
        case VK_SHADER_STAGE_VERTEX_BIT: return SLANG_STAGE_VERTEX;
        case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT: return SLANG_STAGE_HULL;
        case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT: return SLANG_STAGE_DOMAIN;
        case VK_SHADER_STAGE_GEOMETRY_BIT: return SLANG_STAGE_GEOMETRY;
        case VK_SHADER_STAGE_FRAGMENT_BIT: return SLANG_STAGE_PIXEL;
        case VK_SHADER_STAGE_COMPUTE_BIT: return SLANG_STAGE_COMPUTE;
        case VK_SHADER_STAGE_TASK_BIT_EXT: return SLANG_STAGE_AMPLIFICATION;
        case VK_SHADER_STAGE_MESH_BIT_EXT: return SLANG_STAGE_MESH;
        case VK_SHADER_STAGE_RAYGEN_BIT_KHR: return SLANG_STAGE_RAY_GENERATION;
        case VK_SHADER_STAGE_ANY_HIT_BIT_KHR: return SLANG_STAGE_ANY_HIT;
        case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR: return SLANG_STAGE_CLOSEST_HIT;
        case VK_SHADER_STAGE_MISS_BIT_KHR: return SLANG_STAGE_MISS;
        case VK_SHADER_STAGE_INTERSECTION_BIT_KHR: return SLANG_STAGE_INTERSECTION;
        case VK_SHADER_STAGE_CALLABLE_BIT_KHR: return SLANG_STAGE_CALLABLE;
        default: return SLANG_STAGE_NONE;
        }
    }
}