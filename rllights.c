#include "rlights.h"

RLG_Context RLG_CreateContext(unsigned int count)
{
    // On-heap allocation for the context's core structure, initializing it with zeros
    struct RLG_Core *rlgCtx = (struct RLG_Core*)calloc(1, sizeof(struct RLG_Core));

    if (!rlgCtx)
    {
        TraceLog(LOG_FATAL, "Heap allocation for RLG context failed!");
        return NULL;
    }

    // NOTE: The limit of 99 is because we measure the size of `rlgLightingFS` with '%s'
    if (count > 99)
    {
        TraceLog(LOG_WARNING, "The limit of lights supported by rlights is 99."
                              "The number of lights has therefore been adjusted to this value.");
        count = 99;
    }

    // We check if all the shader codes are well defined
    if (rlgCachedLightingVS == NULL) TraceLog(LOG_WARNING, "The lighting vertex shader has not been defined.");
    if (rlgCachedLightingFS == NULL) TraceLog(LOG_WARNING, "The lighting fragment shader has not been defined.");
    if (rlgCachedDepthVS == NULL) TraceLog(LOG_WARNING, "The depth vertex shader has not been defined.");
    if (rlgCachedDepthFS == NULL) TraceLog(LOG_WARNING, "The depth fragment shader has not been defined.");

    const char *lightVS = rlgCachedLightingVS;
    const char *lightFS = rlgCachedLightingFS;

#   ifndef NO_EMBEDDED_SHADERS

#       if GLSL_VERSION > 100
        bool vsFormated = (lightVS == rlgLightingVS);
        if (vsFormated)
        {
            // Format frag shader with lights count
            char *fmtVert = (char*)malloc(sizeof(rlgLightingVS));
            snprintf(fmtVert, sizeof(rlgLightingVS), rlgLightingVS, count);
            lightVS = fmtVert;
        }
#       endif

        bool fsFormated = (lightFS == rlgLightingFS);
        if (fsFormated)
        {
            // Format frag shader with lights count
            char *fmtFrag = (char*)malloc(sizeof(rlgLightingFS));
            snprintf(fmtFrag, sizeof(rlgLightingFS), rlgLightingFS, count);
            lightFS = fmtFrag;
        }

#   endif //NO_EMBEDDED_SHADERS

    // Load shader and get locations
    Shader lightShader = { 0 };
    lightShader.id = rlLoadShaderCode(lightVS, lightFS);

    // After shader loading, we TRY to set default location names
    if (lightShader.id > 0)
    {
        // NOTE: Locations that cannot be retrieved are set to -1 by 'rlGetLocationAttrib'
        lightShader.locs = (int*)malloc(RLG_COUNT_LOCS*sizeof(int));

        // Get handles to GLSL input attribute locations
        lightShader.locs[RLG_LOC_VERTEX_POSITION]    = rlGetLocationAttrib(lightShader.id, RLG_SHADER_LIGHTING_ATTRIB_POSITION);
        lightShader.locs[RLG_LOC_VERTEX_TEXCOORD01]  = rlGetLocationAttrib(lightShader.id, RLG_SHADER_LIGHTING_ATTRIB_TEXCOORD);
        lightShader.locs[RLG_LOC_VERTEX_TEXCOORD02]  = rlGetLocationAttrib(lightShader.id, RLG_SHADER_LIGHTING_ATTRIB_TEXCOORD2);
        lightShader.locs[RLG_LOC_VERTEX_NORMAL]      = rlGetLocationAttrib(lightShader.id, RLG_SHADER_LIGHTING_ATTRIB_NORMAL);
        lightShader.locs[RLG_LOC_VERTEX_TANGENT]     = rlGetLocationAttrib(lightShader.id, RLG_SHADER_LIGHTING_ATTRIB_TANGENT);
        lightShader.locs[RLG_LOC_VERTEX_COLOR]       = rlGetLocationAttrib(lightShader.id, RLG_SHADER_LIGHTING_ATTRIB_COLOR);

        // Get handles to GLSL uniform locations (vertex shader)
        lightShader.locs[RLG_LOC_MATRIX_MVP]         = rlGetLocationUniform(lightShader.id, RLG_SHADER_LIGHTING_UNIFORM_MATRIX_MVP);
        lightShader.locs[RLG_LOC_MATRIX_VIEW]        = rlGetLocationUniform(lightShader.id, RLG_SHADER_LIGHTING_UNIFORM_MATRIX_VIEW);
        lightShader.locs[RLG_LOC_MATRIX_PROJECTION]  = rlGetLocationUniform(lightShader.id, RLG_SHADER_LIGHTING_UNIFORM_MATRIX_PROJECTION);
        lightShader.locs[RLG_LOC_MATRIX_MODEL]       = rlGetLocationUniform(lightShader.id, RLG_SHADER_LIGHTING_UNIFORM_MATRIX_MODEL);
        lightShader.locs[RLG_LOC_MATRIX_NORMAL]      = rlGetLocationUniform(lightShader.id, RLG_SHADER_LIGHTING_UNIFORM_MATRIX_NORMAL);

        // Get handles to GLSL uniform locations (fragment shader)
        lightShader.locs[RLG_LOC_COLOR_AMBIENT]      = rlGetLocationUniform(lightShader.id, RLG_SHADER_LIGHTING_UNIFORM_COLOR_AMBIENT);
        lightShader.locs[RLG_LOC_VECTOR_VIEW]        = rlGetLocationUniform(lightShader.id, RLG_SHADER_LIGHTING_UNIFORM_VIEW_POSITION);

        lightShader.locs[RLG_LOC_COLOR_DIFFUSE]      = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].color", MATERIAL_MAP_ALBEDO));
        lightShader.locs[RLG_LOC_COLOR_SPECULAR]     = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].color", MATERIAL_MAP_METALNESS));
        lightShader.locs[RLG_LOC_COLOR_EMISSION]     = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].color", MATERIAL_MAP_EMISSION));

        lightShader.locs[RLG_LOC_MAP_ALBEDO]         = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].texture", MATERIAL_MAP_ALBEDO));
        lightShader.locs[RLG_LOC_MAP_METALNESS]      = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].texture", MATERIAL_MAP_METALNESS));
        lightShader.locs[RLG_LOC_MAP_NORMAL]         = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].texture", MATERIAL_MAP_NORMAL));
        lightShader.locs[RLG_LOC_MAP_ROUGHNESS]      = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].texture", MATERIAL_MAP_ROUGHNESS));
        lightShader.locs[RLG_LOC_MAP_OCCLUSION]      = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].texture", MATERIAL_MAP_OCCLUSION));
        lightShader.locs[RLG_LOC_MAP_EMISSION]       = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].texture", MATERIAL_MAP_EMISSION));
        lightShader.locs[RLG_LOC_MAP_HEIGHT]         = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].texture", MATERIAL_MAP_HEIGHT));
        lightShader.locs[RLG_LOC_MAP_BRDF]           = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].texture", MATERIAL_MAP_HEIGHT + 1));

        lightShader.locs[RLG_LOC_MAP_CUBEMAP]        = rlGetLocationUniform(lightShader.id, TextFormat("cubemaps[%i].texture", 0));
        lightShader.locs[RLG_LOC_MAP_IRRADIANCE]     = rlGetLocationUniform(lightShader.id, TextFormat("cubemaps[%i].texture", 1));
        lightShader.locs[RLG_LOC_MAP_PREFILTER]      = rlGetLocationUniform(lightShader.id, TextFormat("cubemaps[%i].texture", 2));

        lightShader.locs[RLG_LOC_METALNESS_SCALE]    = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].value", MATERIAL_MAP_METALNESS));
        lightShader.locs[RLG_LOC_ROUGHNESS_SCALE]    = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].value", MATERIAL_MAP_ROUGHNESS));
        lightShader.locs[RLG_LOC_AO_LIGHT_AFFECT]    = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].value", MATERIAL_MAP_OCCLUSION));
        lightShader.locs[RLG_LOC_HEIGHT_SCALE]       = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].value", MATERIAL_MAP_HEIGHT));

        // Definition of the lighting shader once initialization is successful
        rlgCtx->shaders[RLG_SHADER_LIGHTING] = lightShader;
    }

    // Frees up space allocated for string formatting
#   ifndef NO_EMBEDDED_SHADERS
#   if GLSL_VERSION > 100
    if (vsFormated) free((void*)lightVS);
#   endif
    if (fsFormated) free((void*)lightFS);
#   endif //NO_EMBEDDED_SHADERS

    // Init default view position and ambient color
    rlgCtx->colAmbient = (Vector3){0.1f, 0.1f, 0.1f};
    rlgCtx->viewPos = (Vector3){0, 0, 0};

    SetShaderValue(lightShader,
        lightShader.locs[RLG_LOC_COLOR_AMBIENT],
        &rlgCtx->colAmbient, RL_SHADER_UNIFORM_VEC3);

    // Retrieving lighting shader uniforms indicating which textures we should sample
    for (int i = 0, mapID = 0, cubemapID = 0; i < RLG_COUNT_MATERIAL_MAPS; i++)
    {
        if (i == MATERIAL_MAP_CUBEMAP || i == MATERIAL_MAP_IRRADIANCE || i == MATERIAL_MAP_PREFILTER)
        {
            rlgCtx->material.locs.useMaps[i] = rlGetLocationUniform(lightShader.id, TextFormat("cubemaps[%i].active", cubemapID));
            cubemapID++;
        }
        else
        {
            rlgCtx->material.locs.useMaps[i] = rlGetLocationUniform(lightShader.id, TextFormat("maps[%i].active", mapID));
            mapID++;
        }
    }

    // Default activation of diffuse texture sampling
    rlgCtx->material.data.useMaps[MATERIAL_MAP_ALBEDO] = true;
    SetShaderValue(lightShader, rlgCtx->material.locs.useMaps[MATERIAL_MAP_ALBEDO],
        &rlgCtx->material.data.useMaps[MATERIAL_MAP_ALBEDO], SHADER_UNIFORM_INT);

    // Recovery of “special” lighting shader uniforms
    rlgCtx->material.locs.parallaxMinLayers = rlGetLocationUniform(lightShader.id, "parallaxMinLayers");
    rlgCtx->material.locs.parallaxMaxLayers = rlGetLocationUniform(lightShader.id, "parallaxMaxLayers");
    rlgCtx->locLightingFar = rlGetLocationUniform(lightShader.id, "farPlane");

    // Allocation and initialization of the desired number of lights
    rlgCtx->lights = (struct RLG_Light*)calloc(count, sizeof(struct RLG_Light));
    for (unsigned int i = 0; i < count; i++)
    {
        struct RLG_Light *light = &rlgCtx->lights[i];

        light->data.shadowMap      = (struct RLG_ShadowMap){0};
        light->data.position       = (Vector3){0};
        light->data.direction      = (Vector3){0};
        light->data.color          = (Vector3){ 1.0f, 1.0f, 1.0f};
        light->data.energy         = 1.0f;
        light->data.specular       = 1.0f;
        light->data.size           = 0.0f;
        light->data.innerCutOff    = -1.0f;
        light->data.outerCutOff    = -1.0f;
        light->data.constant       = 1.0f;
        light->data.linear         = 0.0f;
        light->data.quadratic      = 0.0f;
        light->data.shadowMapTxlSz = 0.0f;
        light->data.depthBias      = 0.0f;
        light->data.type           = RLG_DIRLIGHT;
        light->data.shadow         = 0;
        light->data.enabled        = 0;

        light->locs.vpMatrix       = rlGetLocationUniform(lightShader.id, TextFormat("matLights[%i]", i));
        light->locs.shadowCubemap  = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].shadowCubemap", i));
        light->locs.shadowMap      = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].shadowMap", i));
        light->locs.position       = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].position", i));
        light->locs.direction      = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].direction", i));
        light->locs.color          = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].color", i));
        light->locs.energy         = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].energy", i));
        light->locs.specular       = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].specular", i));
        light->locs.size           = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].size", i));
        light->locs.innerCutOff    = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].innerCutOff", i));
        light->locs.outerCutOff    = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].outerCutOff", i));
        light->locs.constant       = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].constant", i));
        light->locs.linear         = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].linear", i));
        light->locs.quadratic      = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].quadratic", i));
        light->locs.shadowMapTxlSz = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].shadowMapTxlSz", i));
        light->locs.depthBias      = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].depthBias", i));
        light->locs.type           = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].type", i));
        light->locs.shadow         = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].shadow", i));
        light->locs.enabled        = rlGetLocationUniform(lightShader.id, TextFormat("lights[%i].enabled", i));

        SetShaderValue(lightShader, light->locs.color, &light->data.color, SHADER_UNIFORM_VEC3);
        SetShaderValue(lightShader, light->locs.energy, &light->data.energy, SHADER_UNIFORM_FLOAT);
        SetShaderValue(lightShader, light->locs.specular, &light->data.specular, SHADER_UNIFORM_FLOAT);
        SetShaderValue(lightShader, light->locs.innerCutOff, &light->data.innerCutOff, SHADER_UNIFORM_FLOAT);
        SetShaderValue(lightShader, light->locs.outerCutOff, &light->data.outerCutOff, SHADER_UNIFORM_FLOAT);
        SetShaderValue(lightShader, light->locs.constant, &light->data.constant, SHADER_UNIFORM_FLOAT);
    }

    // Set light count
    rlgCtx->lightCount = count;

    // Init default material maps
    Texture defaultTexture  = (Texture){0};
    defaultTexture.id       = rlGetTextureIdDefault();
    defaultTexture.width    = 1;
    defaultTexture.height   = 1;
    defaultTexture.mipmaps  = 1;
    defaultTexture.format   = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    rlgCtx->defaultMaps[MATERIAL_MAP_ALBEDO].texture = defaultTexture;
    rlgCtx->defaultMaps[MATERIAL_MAP_ALBEDO].color = WHITE;
    rlgCtx->defaultMaps[MATERIAL_MAP_METALNESS].texture = defaultTexture;
    rlgCtx->defaultMaps[MATERIAL_MAP_METALNESS].value = 0.5f;
    rlgCtx->defaultMaps[MATERIAL_MAP_ROUGHNESS].texture = defaultTexture;
    rlgCtx->defaultMaps[MATERIAL_MAP_ROUGHNESS].value = 0.5f;
    rlgCtx->defaultMaps[MATERIAL_MAP_OCCLUSION].texture = defaultTexture;
    rlgCtx->defaultMaps[MATERIAL_MAP_OCCLUSION].value = 0.0f;
    rlgCtx->defaultMaps[MATERIAL_MAP_EMISSION].texture = defaultTexture;
    rlgCtx->defaultMaps[MATERIAL_MAP_EMISSION].color = BLACK;
    rlgCtx->defaultMaps[MATERIAL_MAP_HEIGHT].texture = defaultTexture;
    rlgCtx->defaultMaps[MATERIAL_MAP_HEIGHT].value = 0.05f;

    // Load depth shader (used for shadow casting)
    rlgCtx->shaders[RLG_SHADER_DEPTH] = LoadShaderFromMemory(rlgCachedDepthVS, rlgCachedDepthFS);

    // Load depth cubemap shader (used for omnilight shadow casting)
    rlgCtx->shaders[RLG_SHADER_DEPTH_CUBEMAP] = LoadShaderFromMemory(rlgCachedDepthCubemapVS, rlgCachedDepthCubemapFS);
    rlgCtx->locDepthCubemapLightPos = rlGetLocationUniform(rlgCtx->shaders[RLG_SHADER_DEPTH_CUBEMAP].id, "lightPos");
    rlgCtx->locDepthCubemapFar = rlGetLocationUniform(rlgCtx->shaders[RLG_SHADER_DEPTH_CUBEMAP].id, "farPlane");

    // Get Near/Far render values
    rlgCtx->zNear = 0.01f;  // TODO: replace with rlGetCullDistanceNear()
    rlgCtx->zFar = 1000.0f; // TODO: replace with rlGetCullDistanceFar()

    // Send Near/Far to shaders who need it
    SetShaderValue(rlgCtx->shaders[RLG_SHADER_DEPTH_CUBEMAP],
        rlgCtx->locDepthCubemapFar, &rlgCtx->zFar, SHADER_UNIFORM_FLOAT);

    // Load equirectangular to cubemap shader (used for skybox cubemap generation)
    rlgCtx->shaders[RLG_SHADER_EQUIRECTANGULAR_TO_CUBEMAP] = LoadShaderFromMemory(
        rlgCachedEquirectangularToCubemapVS, rlgCachedEquirectangularToCubemapFS);

    // Load irradiance convolution shader (used to generate irradiance map of the skybox cubemap)
    rlgCtx->shaders[RLG_SHADER_IRRADIANCE_CONVOLUTION] = LoadShaderFromMemory(
        rlgCachedIrradianceConvolutionVS, rlgCachedIrradianceConvolutionFS);

    // Load skybox shader
    rlgCtx->shaders[RLG_SHADER_SKYBOX] = LoadShaderFromMemory(rlgCachedSkyboxVS, rlgCachedSkyboxFS);
    rlgCtx->skybox.locDoGamma = rlGetLocationUniform(rlgCtx->shaders[RLG_SHADER_SKYBOX].id, "doGamma");

    return (RLG_Context)rlgCtx;
}

void RLG_DestroyContext(RLG_Context ctx)
{
    struct RLG_Core *pCtx = (struct RLG_Core*)ctx;

    for (int i = 0; i < RLG_COUNT_SHADERS; i++)
    {
        if (IsShaderReady(pCtx->shaders[i]))
        {
            UnloadShader(pCtx->shaders[i]);
            pCtx->shaders[i] = (Shader){0};
        }
    }

    if (pCtx->lights != NULL)
    {
        for (unsigned int i = 0; i < pCtx->lightCount; i++)
        {
            struct RLG_Light *light = &pCtx->lights[i];

            if (light->data.shadowMap.id != 0)
            {
                rlUnloadTexture(light->data.shadowMap.depth.id);
                rlUnloadFramebuffer(light->data.shadowMap.id);
            }
        }

        free(pCtx->lights);
        pCtx->lights = NULL;
    }

    pCtx->lightCount = 0;
}

void RLG_SetContext(RLG_Context ctx)
{
    rlgCtx = (struct RLG_Core*)ctx;
}

RLG_Context RLG_GetContext(void)
{
    return (RLG_Context)rlgCtx;
}

void RLG_SetCustomShaderCode(RLG_Shader shader, const char *vsCode, const char *fsCode)
{
    switch (shader)
    {
        case RLG_SHADER_LIGHTING:
            rlgCachedLightingVS = vsCode;
            rlgCachedLightingFS = fsCode;
            break;

        case RLG_SHADER_DEPTH:
            rlgCachedDepthVS = vsCode;
            rlgCachedDepthFS = fsCode;
            break;

        case RLG_SHADER_DEPTH_CUBEMAP:
            rlgCachedDepthCubemapVS = vsCode;
            rlgCachedDepthCubemapFS = fsCode;
            break;

        case RLG_SHADER_EQUIRECTANGULAR_TO_CUBEMAP:
            rlgCachedEquirectangularToCubemapVS = vsCode;
            rlgCachedEquirectangularToCubemapFS = fsCode;
            break;

        case RLG_SHADER_IRRADIANCE_CONVOLUTION:
            rlgCachedIrradianceConvolutionVS = vsCode;
            rlgCachedIrradianceConvolutionFS = fsCode;
            break;

        case RLG_SHADER_SKYBOX:
            rlgCachedSkyboxVS = vsCode;
            rlgCachedSkyboxFS = fsCode;
            break;

        default:
            TraceLog(LOG_WARNING, "Unsupported 'shader' passed to 'RLG_SetCustomShader'");
            break;
    }
}

const Shader* RLG_GetShader(RLG_Shader shader)
{
    if (shader < 0 || shader >= RLG_COUNT_SHADERS)
    {
        return NULL;
    }

    return &rlgCtx->shaders[shader];
}

void RLG_SetViewPosition(float x, float y, float z)
{
    RLG_SetViewPositionV((Vector3){ x, y, z});
}

void RLG_SetViewPositionV(Vector3 position)
{
    int loc = rlgCtx->shaders[RLG_SHADER_LIGHTING].locs[RLG_LOC_VECTOR_VIEW];

    if (loc != -1)
    {
        rlgCtx->viewPos = position;
        SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING],
            loc, &rlgCtx->viewPos, SHADER_UNIFORM_VEC3);
    }

}

Vector3 RLG_GetViewPosition(void)
{
    return rlgCtx->viewPos;
}

void RLG_SetAmbientColor(Color color)
{
    int loc = rlgCtx->shaders[RLG_SHADER_LIGHTING].locs[RLG_LOC_COLOR_AMBIENT];

    if (loc != -1)
    {
        rlgCtx->colAmbient.x = (float)color.r/255.0f;
        rlgCtx->colAmbient.y = (float)color.g/255.0f;
        rlgCtx->colAmbient.z = (float)color.b/255.0f;

        SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING],
            loc, &rlgCtx->colAmbient, SHADER_UNIFORM_VEC3);
    }
}
Color RLG_GetAmbientColor(void)
{
    Color color = { 0 };

    color.r = rlgCtx->colAmbient.x*255.0f;
    color.g = rlgCtx->colAmbient.y*255.0f;
    color.b = rlgCtx->colAmbient.z*255.0f;
    color.a = 255;

    return color;
}

void RLG_SetParallaxLayers(int min, int max)
{
    if (rlgCtx->material.locs.parallaxMinLayers != -1 &&
        min != rlgCtx->material.data.parallaxMinLayers)
    {
        rlgCtx->material.data.parallaxMinLayers = min;
        SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING],
            rlgCtx->material.locs.parallaxMinLayers,
            &min, RL_SHADER_UNIFORM_INT);
    }

    if (rlgCtx->material.locs.parallaxMaxLayers != -1 &&
        max != rlgCtx->material.data.parallaxMaxLayers)
    {
        rlgCtx->material.data.parallaxMaxLayers = max;
        SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING],
            rlgCtx->material.locs.parallaxMaxLayers,
            &max, RL_SHADER_UNIFORM_INT);
    }
}

void RLG_GetParallaxLayers(int* min, int* max)
{
    if (min != NULL) *min = rlgCtx->material.data.parallaxMinLayers;
    if (max != NULL) *max = rlgCtx->material.data.parallaxMaxLayers;
}

void RLG_UseMap(MaterialMapIndex mapIndex, bool active)
{
    if (mapIndex >= 0 && mapIndex < RLG_COUNT_MATERIAL_MAPS)
    {
        if (rlgCtx->material.locs.useMaps[mapIndex] != -1 &&
            active != rlgCtx->material.data.useMaps[mapIndex])
        {
            int v = (int)active;
            rlgCtx->material.data.useMaps[mapIndex] = active;
            SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], rlgCtx->material.locs.useMaps[mapIndex], &v, SHADER_UNIFORM_INT);
        }
    }
}

bool RLG_IsMapUsed(MaterialMapIndex mapIndex)
{
    bool result = false;

    if (mapIndex >= 0 && mapIndex < RLG_COUNT_MATERIAL_MAPS)
    {
        result = rlgCtx->material.data.useMaps[mapIndex];
    }

    return result;
}

void RLG_UseDefaultMap(MaterialMapIndex mapIndex, bool active)
{
    if (mapIndex >= 0 || mapIndex < RLG_COUNT_MATERIAL_MAPS)
    {
        rlgCtx->usedDefaultMaps[mapIndex] = active;
    }
}

void RLG_SetDefaultMap(MaterialMapIndex mapIndex, MaterialMap map)
{
    if (mapIndex >= 0 || mapIndex < RLG_COUNT_MATERIAL_MAPS)
    {
        rlgCtx->defaultMaps[mapIndex] = map;
    }
}

MaterialMap RLG_GetDefaultMap(MaterialMapIndex mapIndex)
{
    MaterialMap map = { 0 };

    if (mapIndex >= 0 || mapIndex < RLG_COUNT_MATERIAL_MAPS)
    {
        map = rlgCtx->defaultMaps[mapIndex];
    }

    return map;
}

bool RLG_IsDefaultMapUsed(MaterialMapIndex mapIndex)
{
    return rlgCtx->usedDefaultMaps[mapIndex];
}

unsigned int RLG_GetLightcount(void)
{
    return rlgCtx->lightCount;
}

void RLG_UseLight(unsigned int light, bool active)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_UseLight' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];

    if (active != l->data.enabled)
    {
        l->data.enabled = (int)active;
        SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING],
            l->locs.enabled, &l->data.enabled, SHADER_UNIFORM_INT);
    }
}

bool RLG_IsLightUsed(unsigned int light)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_IsLightUsed' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return false;
    }

    return (bool)rlgCtx->lights[light].data.enabled;
}

void RLG_ToggleLight(unsigned int light)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_ToggleLight' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];

    l->data.enabled = !l->data.enabled;
    SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.enabled,
        &l->data.enabled, SHADER_UNIFORM_INT);
}

void RLG_SetLightType(unsigned int light, RLG_LightType type)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_SetLightType' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];

    if (l->data.type != type)
    {
        if (l->data.shadow)
        {
            int shadowMapResolution = l->data.shadowMap.width;

            RLG_DisableShadow(light);
            RLG_EnableShadow(light, shadowMapResolution);
        }

        l->data.type = (int)type;
        SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.type,
            &l->data.type, SHADER_UNIFORM_INT);
    }
}

RLG_LightType RLG_GetLightType(unsigned int light)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_GetLightType' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return (RLG_LightType)0;
    }

    return (RLG_LightType)rlgCtx->lights[light].data.type;
}

void RLG_SetLightValue(unsigned int light, RLG_LightProperty property, float value)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_SetLightValue' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];

    switch (property)
    {
        case RLG_LIGHT_COLOR:
            l->data.color = (Vector3){ value, value, value};
            SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.color,
                &l->data.color, SHADER_UNIFORM_VEC3);
            break;

        case RLG_LIGHT_ENERGY:
            if (value != l->data.energy)
            {
                l->data.energy = value;
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.energy,
                    &l->data.energy, SHADER_UNIFORM_FLOAT);
            }
            break;

        case RLG_LIGHT_SPECULAR:
            if (value != l->data.specular)
            {
                l->data.specular = value;
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.specular,
                    &l->data.specular, SHADER_UNIFORM_FLOAT);
            }
            break;

        case RLG_LIGHT_SIZE:
            if (value != l->data.size)
            {
                l->data.size = value;
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.size,
                    &l->data.size, SHADER_UNIFORM_FLOAT);
            }
            break;

        case RLG_LIGHT_INNER_CUTOFF:
            if (value != l->data.innerCutOff)
            {
                l->data.innerCutOff = value;
                float c = cosf(value*DEG2RAD);
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.innerCutOff,
                    &c, SHADER_UNIFORM_FLOAT);
            }
            break;

        case RLG_LIGHT_OUTER_CUTOFF:
            if (value != l->data.outerCutOff)
            {
                l->data.outerCutOff = value;
                float c = cosf(value*DEG2RAD);
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.outerCutOff,
                    &c, SHADER_UNIFORM_FLOAT);
            }
            break;

        case RLG_LIGHT_ATTENUATION_CONSTANT:
            if (value != l->data.constant)
            {
                l->data.constant = value;
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.constant, &value, SHADER_UNIFORM_FLOAT);
            }
            break;

        case RLG_LIGHT_ATTENUATION_LINEAR:
            if (value != l->data.linear)
            {
                l->data.linear = value;
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.linear, &value, SHADER_UNIFORM_FLOAT);
            }
            break;

        case RLG_LIGHT_ATTENUATION_QUADRATIC:
            if (value != l->data.quadratic)
            {
                l->data.quadratic = value;
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.quadratic, &value, SHADER_UNIFORM_FLOAT);
            }
            break;

        default:
            break;
    }
}

void RLG_SetLightXYZ(unsigned int light, RLG_LightProperty property, float x, float y, float z)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_SetLightXYZ' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];
    Vector3 value = { x, y, z };

    switch (property)
    {
        case RLG_LIGHT_POSITION:
            l->data.position = value;
            SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.position,
                &value, SHADER_UNIFORM_VEC3);
            break;

        case RLG_LIGHT_DIRECTION:
            l->data.direction = value;
            SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.direction,
                &value, SHADER_UNIFORM_VEC3);
            break;

        case RLG_LIGHT_COLOR:
            l->data.color = value;
            SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.color,
                &value, SHADER_UNIFORM_VEC3);
            break;

        case RLG_LIGHT_ATTENUATION_CLQ:
            if (x != l->data.constant)
            {
                l->data.constant = x;
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.constant, &x, SHADER_UNIFORM_FLOAT);
            }
            if (y != l->data.linear)
            {
                l->data.linear = y;
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.linear, &y, SHADER_UNIFORM_FLOAT);
            }
            if (z != l->data.quadratic)
            {
                l->data.quadratic = z;
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.quadratic, &z, SHADER_UNIFORM_FLOAT);
            }
            break;

        default:
            break;
    }
}

void RLG_SetLightVec3(unsigned int light, RLG_LightProperty property, Vector3 value)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_SetLightVec3' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];

    switch (property)
    {
        case RLG_LIGHT_POSITION:
            l->data.position = value;
            SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.position,
                &value, SHADER_UNIFORM_VEC3);
            break;

        case RLG_LIGHT_DIRECTION:
            l->data.direction = value;
            SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.direction,
                &value, SHADER_UNIFORM_VEC3);
            break;

        case RLG_LIGHT_COLOR:
            l->data.color = value;
            SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.color,
                &value, SHADER_UNIFORM_VEC3);
            break;

        case RLG_LIGHT_ATTENUATION_CLQ:
            if (value.x != l->data.constant)
            {
                l->data.constant = value.x;
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.constant, &value.x, SHADER_UNIFORM_FLOAT);
            }
            if (value.y != l->data.linear)
            {
                l->data.linear = value.y;
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.linear, &value.y, SHADER_UNIFORM_FLOAT);
            }
            if (value.z != l->data.quadratic)
            {
                l->data.quadratic = value.z;
                SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.quadratic, &value.z, SHADER_UNIFORM_FLOAT);
            }
            break;

        default:
            break;
    }
}

void RLG_SetLightColor(unsigned int light, Color color)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_SetLightColor' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    Vector3 nCol = { 0 };
    nCol.x = color.r/255.0f;
    nCol.y = color.g/255.0f;
    nCol.z = color.b/255.0f;

    struct RLG_Light *l = &rlgCtx->lights[light];

    l->data.color = nCol;
    SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.color,
        &nCol, SHADER_UNIFORM_VEC3);
}

float RLG_GetLightValue(unsigned int light, RLG_LightProperty property)
{
    float result = 0;

    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_GetLightValue' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return result;
    }

    const struct RLG_Light *l = &rlgCtx->lights[light];

    switch (property)
    {
        case RLG_LIGHT_ENERGY:
            result = l->data.energy;
            break;

        case RLG_LIGHT_SPECULAR:
            result = l->data.specular;
            break;

        case RLG_LIGHT_SIZE:
            result = l->data.size;
            break;

        case RLG_LIGHT_INNER_CUTOFF:
            result = l->data.innerCutOff;
            break;

        case RLG_LIGHT_OUTER_CUTOFF:
            result = l->data.outerCutOff;
            break;

        case RLG_LIGHT_ATTENUATION_CONSTANT:
            result = l->data.constant;
            break;

        case RLG_LIGHT_ATTENUATION_LINEAR:
            result = l->data.linear;
            break;

        case RLG_LIGHT_ATTENUATION_QUADRATIC:
            result = l->data.quadratic;
            break;

        default:
            break;
    }

    return result;
}

Vector3 RLG_GetLightVec3(unsigned int light, RLG_LightProperty property)
{
    Vector3 result = { 0 };

    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_GetLightVec3' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return result;
    }

    const struct RLG_Light *l = &rlgCtx->lights[light];

    switch (property)
    {
        case RLG_LIGHT_POSITION:
            result = l->data.position;
            break;

        case RLG_LIGHT_DIRECTION:
            result = l->data.direction;
            break;

        case RLG_LIGHT_COLOR:
            result = l->data.color;
            break;

        case RLG_LIGHT_ATTENUATION_CLQ:
            result.y = l->data.linear;
            result.x = l->data.constant;
            result.z = l->data.quadratic;
            break;

        default:
            break;
    }

    return result;
}

Color RLG_GetLightColor(unsigned int light)
{
    Color result = BLACK;

    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_GetLightColor' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return result;
    }

    const struct RLG_Light *l = &rlgCtx->lights[light];

    result.r = 255*l->data.color.x;
    result.g = 255*l->data.color.y;
    result.b = 255*l->data.color.z;

    return result;
}

void RLG_LightTranslate(unsigned int light, float x, float y, float z)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_LightTranslate' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];

    l->data.position.x += x;
    l->data.position.y += y;
    l->data.position.z += z;

    SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.position,
        &l->data.position, SHADER_UNIFORM_VEC3);
}

void RLG_LightTranslateV(unsigned int light, Vector3 v)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_LightTranslateV' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];
    l->data.position.x += v.x;
    l->data.position.y += v.y;
    l->data.position.z += v.z;

    SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.position,
        &l->data.position, SHADER_UNIFORM_VEC3);
}

void RLG_LightRotateX(unsigned int light, float degrees)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_LightRotateX' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];
    float radians = DEG2RAD*degrees;
    float c = cosf(radians);
    float s = sinf(radians);

    l->data.direction.y = l->data.direction.y*c + l->data.direction.z*s;
    l->data.direction.z = -l->data.direction.y*s + l->data.direction.z*c;

    SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.direction,
        &l->data.direction, SHADER_UNIFORM_VEC3);
}

void RLG_LightRotateY(unsigned int light, float degrees)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_LightRotateY' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];
    float radians = DEG2RAD*degrees;
    float c = cosf(radians);
    float s = sinf(radians);

    l->data.direction.x = l->data.direction.x*c - l->data.direction.z*s;
    l->data.direction.z = l->data.direction.x*s + l->data.direction.z*c;

    SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.direction,
        &l->data.direction, SHADER_UNIFORM_VEC3);
}

void RLG_LightRotateZ(unsigned int light, float degrees)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_LightRotateZ' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];
    float radians = DEG2RAD*degrees;
    float c = cosf(radians);
    float s = sinf(radians);

    l->data.direction.x = l->data.direction.x*c + l->data.direction.y*s;
    l->data.direction.y = -l->data.direction.x*s + l->data.direction.y*c;

    SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.direction,
        &l->data.direction, SHADER_UNIFORM_VEC3);
}

void RLG_LightRotate(unsigned int light, Vector3 axis, float degrees)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_LightRotate' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];
    float radians = -DEG2RAD*degrees;
    float halfTheta = radians*0.5f;

    float sinHalfTheta = sinf(halfTheta);
    Quaternion rotationQuat = {
        axis.x * sinHalfTheta,
        axis.y * sinHalfTheta,
        axis.z * sinHalfTheta,
        cosf(halfTheta)
    };

    // Convert the current direction vector to a quaternion
    Quaternion directionQuat = {
        l->data.direction.x,
        l->data.direction.y,
        l->data.direction.z,
        0.0f
    };

    // Calculate the rotated direction quaternion
    Quaternion rotatedQuat = QuaternionMultiply(
        QuaternionMultiply(rotationQuat, directionQuat),
        QuaternionInvert(rotationQuat));

    // Update the light direction with the rotated direction
    l->data.direction = Vector3Normalize((Vector3){
        rotatedQuat.x, rotatedQuat.y, rotatedQuat.z}
    );

    SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.direction,
        &l->data.direction, SHADER_UNIFORM_VEC3);
}

void RLG_SetLightTarget(unsigned int light, float x, float y, float z)
{
    RLG_SetLightTargetV(light, (Vector3){ x, y, z});
}

void RLG_SetLightTargetV(unsigned int light, Vector3 targetPosition)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_SetLightTarget' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];

    l->data.direction = Vector3Normalize(Vector3Subtract(
        targetPosition, l->data.position));

    SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.direction,
        &l->data.direction, SHADER_UNIFORM_VEC3);
}

Vector3 RLG_GetLightTarget(unsigned int light)
{
    Vector3 result = (Vector3){0};

    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_GetLightTarget' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return result;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];
    result = Vector3Add(l->data.position, l->data.direction);

    return result;
}

void RLG_EnableShadow(unsigned int light, int shadowMapResolution)
{
    // Check if the specified light ID is within the valid range
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_EnableShadow' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    // Get a pointer to the specified light structure
    struct RLG_Light *l = &rlgCtx->lights[light];

    // Check if the current shadow map resolution is different from the desired resolution
    if (l->data.shadowMap.width != shadowMapResolution)
    {
        // If the shadow map is already initialized, unload the existing texture and framebuffer
        if (l->data.shadowMap.id != 0)
        {
            rlUnloadTexture(l->data.shadowMap.depth.id);
            rlUnloadFramebuffer(l->data.shadowMap.id);
        }

        // Get a pointer to the shadow map structure of the light
        struct RLG_ShadowMap *sm = &l->data.shadowMap;

        // If the light is an omnidirectional light, set up a cube map for shadows
        if (l->data.type == RLG_OMNILIGHT)
        {
            glGenFramebuffers(1, &sm->id);
            glGenTextures(1, &sm->depth.id);

            glBindTexture(GL_TEXTURE_CUBE_MAP, sm->depth.id);
            for (unsigned int i = 0; i < 6; ++i)
            {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_DEPTH_COMPONENT,
                    shadowMapResolution, shadowMapResolution, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
            }

            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

            glBindFramebuffer(GL_FRAMEBUFFER, sm->id);
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, sm->depth.id, 0);
            glDrawBuffer(GL_NONE);
            glReadBuffer(GL_NONE);

            // Check if the framebuffer is complete
            if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            {
                // Log an error if the framebuffer is not complete
                TraceLog(LOG_ERROR, "Framebuffer is not complete for omnidirectional shadow map");
            }

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            // Configure the shadow map parameters
            sm->depth.width = sm->depth.height = shadowMapResolution;
            sm->width = sm->height = shadowMapResolution;
            sm->depth.format = 19, sm->depth.mipmaps = 1;
        }
        else
        {
            // Set up a 2D texture for shadow map for other light types
            sm->id = rlLoadFramebuffer(shadowMapResolution, shadowMapResolution);
            sm->width = sm->height = shadowMapResolution;
            rlEnableFramebuffer(sm->id);

            sm->depth.id = rlLoadTextureDepth(shadowMapResolution, shadowMapResolution, false);
            sm->depth.width = sm->depth.height = shadowMapResolution;
            sm->depth.format = 19, sm->depth.mipmaps = 1;

            rlTextureParameters(sm->depth.id, RL_TEXTURE_WRAP_S, RL_TEXTURE_WRAP_CLAMP);
            rlTextureParameters(sm->depth.id, RL_TEXTURE_WRAP_T, RL_TEXTURE_WRAP_CLAMP);
            rlFramebufferAttach(sm->id, sm->depth.id, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_TEXTURE2D, 0);
        }

        // REVIEW: Should this value be modifiable by the user?
        float texelSize = 1.0f/shadowMapResolution;
        SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.shadowMapTxlSz,
            &texelSize, SHADER_UNIFORM_FLOAT);

        // Set the depth bias value based on the light type
        l->data.depthBias = (l->data.type == RLG_OMNILIGHT) ? 0.05f : 0.0002f;
        SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.depthBias,
            &l->data.depthBias, SHADER_UNIFORM_FLOAT);
    }

    // Enable shadows for the light and send the information to the shader
    l->data.shadow = true;
    SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.shadow,
        &l->data.shadow, SHADER_UNIFORM_INT);
}

void RLG_DisableShadow(unsigned int light)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_DisableShadow' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];

    if (l->data.shadow)
    {
        // Unload depth texture and framebuffer
        rlUnloadTexture(l->data.shadowMap.depth.id);
        rlUnloadFramebuffer(l->data.shadowMap.id);

        // Fill shadow map struct with zeroes
        l->data.shadowMap = (struct RLG_ShadowMap){0};

        // Send info to the shader
        l->data.shadow = false;
        SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.shadow,
            &l->data.shadow, SHADER_UNIFORM_INT);
    }
}

bool RLG_IsShadowEnabled(unsigned int light)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_IsShadowEnabled' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return false;
    }

    return rlgCtx->lights[light].data.shadow;
}

void RLG_SetShadowBias(unsigned int light, float value)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_SetShadowBias' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    struct RLG_Light *l = &rlgCtx->lights[light];

    l->data.depthBias = value;
    SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.depthBias,
        &value, SHADER_UNIFORM_FLOAT);
}

float RLG_GetShadowBias(unsigned int light)
{
    if (light >= rlgCtx->lightCount)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_GetShadowBias' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return 0;
    }

    return rlgCtx->lights[light].data.depthBias;
}

void RLG_UpdateShadowMap(unsigned int light, RLG_DrawFunc drawFunc)
{
    // Directions and up vectors for the 6 faces of the cubemap
    static const Vector3 dirs[6] = {
        {  1.0,  0.0,  0.0 }, // +X
        { -1.0,  0.0,  0.0 }, // -X
        {  0.0,  1.0,  0.0 }, // +Y
        {  0.0, -1.0,  0.0 }, // -Y
        {  0.0,  0.0,  1.0 }, // +Z
        {  0.0,  0.0, -1.0 }  // -Z
    };

    static const Vector3 ups[6] = {
        {  0.0, -1.0,  0.0 }, // +X
        {  0.0, -1.0,  0.0 }, // -X
        {  0.0,  0.0,  1.0 }, // +Y
        {  0.0,  0.0, -1.0 }, // -Y
        {  0.0, -1.0,  0.0 }, // +Z
        {  0.0, -1.0,  0.0 }  // -Z
    };

    // Safety checks
    if (!drawFunc)
    {
        // Log an error if the draw function pointer is NULL
        TraceLog(LOG_ERROR, "The drawing function pointer specified to 'RLG_UpdateShadowMap' is NULL");
        return;  
    }

    if (light >= rlgCtx->lightCount)
    {
        // Log an error if the light ID exceeds the number of allocated lights
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_UpdateShadowMap' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return;
    }

    // Get a pointer to the specified light structure
    struct RLG_Light *l = &rlgCtx->lights[light];
    if (!l->data.shadow)
    {
        TraceLog(LOG_ERROR, "Light [ID %i] does not support shadow casting", light);
        return;
    }

    // Flush the rendering batch and enable the shadow map framebuffer
    rlDrawRenderBatchActive();
    rlEnableFramebuffer(l->data.shadowMap.id);

    // Configure the projection for the shadow map
    rlViewport(0, 0, l->data.shadowMap.width, l->data.shadowMap.height);
    rlMatrixMode(RL_PROJECTION);
    rlPushMatrix();
    rlLoadIdentity();

    // Near and far clipping planes for shadow map rendering
    rlgCtx->zNear = 0.01f;      // TODO: replace with rlGetCullDistanceNear()
    rlgCtx->zFar = 1000.0f;     // TODO: replace with rlGetCullDistanceFar()

    // Set up projection matrix based on the light type
    switch (l->data.type)
    {
        case RLG_DIRLIGHT:
        case RLG_SPOTLIGHT:
            // Orthographic projection for directional and spotlight
            rlOrtho(-10.0, 10.0, -10.0, 10.0, rlgCtx->zNear, rlgCtx->zFar);
            break;

        case RLG_OMNILIGHT:
            // Perspective projection for omnidirectional light
            rlMultMatrixf(MatrixToFloat(MatrixPerspective(90*DEG2RAD, 1.0, rlgCtx->zNear, rlgCtx->zFar)));
            break;
    }

    // Switch to modelview matrix mode
    rlMatrixMode(RL_MODELVIEW);

    // Enable depth test and disable color blending
    rlEnableDepthTest();
    rlDisableColorBlend();

    // Select the appropriate depth shader
    Shader shader = { 0 };
    if (l->data.type == RLG_OMNILIGHT)
    {
        shader = rlgCtx->shaders[RLG_SHADER_DEPTH_CUBEMAP];

        // Send the light position to the depth shader
        SetShaderValue(shader, rlgCtx->locDepthCubemapLightPos,
            &l->data.position, SHADER_UNIFORM_VEC3);

        // Send zFar to the depth shader to scale depth from [0..zFar] to [0..1]
        SetShaderValue(shader, rlgCtx->locDepthCubemapFar,
            &rlgCtx->zFar, SHADER_UNIFORM_FLOAT);

        // Send zFar to the lighting shader to scale depth from [0..1] to [0..zFar]
        SetShaderValue(rlgCtx->shaders[RLG_SHADER_LIGHTING],
            rlgCtx->locLightingFar, &rlgCtx->zFar,
            SHADER_UNIFORM_FLOAT);
    }
    else
    {
        shader = rlgCtx->shaders[RLG_SHADER_DEPTH];
    }

    // Determine the number of iterations for omnidirectional light
    int iterationCount = (l->data.type == RLG_OMNILIGHT) ? 6 : 1;
    for (int i = 0; i < iterationCount; i++)
    {
        // Configure the ModelView matrix
        Matrix matView = { 0 };
        if (l->data.type == RLG_OMNILIGHT)
        {
            // Attach the depth texture of the i-th face
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                l->data.shadowMap.depth.id, 0);

            // Calculate the view matrix
            matView = MatrixLookAt(l->data.position, Vector3Add(l->data.position, dirs[i]), ups[i]);
        }
        else
        {
            // Calculate the view matrix for directional and spotlight
            matView = MatrixLookAt(l->data.position, Vector3Add(l->data.position, l->data.direction), (Vector3){ 0, 1, 0});

            // Calculate and send the view-projection matrix to the lighting shader for later rendering
            Matrix viewProj = MatrixMultiply(matView, rlGetMatrixProjection());
            SetShaderValueMatrix(rlgCtx->shaders[RLG_SHADER_LIGHTING], l->locs.vpMatrix, viewProj);
        }

        // Apply the view matrix for rendering into the depth texture
        rlLoadIdentity();
        rlMultMatrixf(MatrixToFloat(matView));

        // Clear the previous state of the depth texture
        rlClearScreenBuffers();

        // Render objects in the light's context
        drawFunc(shader);

        // Flush the rendering batch
        rlDrawRenderBatchActive();
    }

    // End rendering
    rlEnableColorBlend();
    rlDisableFramebuffer();

    // Reset the viewport and projection matrix to the screen settings
    rlViewport(0, 0, GetScreenWidth(), GetScreenHeight());
    rlMatrixMode(RL_PROJECTION);
    rlPopMatrix();
    rlMatrixMode(RL_MODELVIEW);
    rlLoadIdentity();
}

Texture RLG_GetShadowMap(unsigned int light)
{
    if (light >= rlgCtx->lightCount)
    {
        // Log an error if the light ID exceeds the number of allocated lights
        TraceLog(LOG_ERROR, "Light [ID %i] specified to 'RLG_GetShadowMap' exceeds allocated number [MAX %i]", light, rlgCtx->lightCount);
        return (Texture){0};
    }

    return rlgCtx->lights[light].data.shadowMap.depth;
}

void RLG_CastMesh(Shader shader, Mesh mesh, Matrix transform)
{
    // Bind shader program
    rlEnableShader(shader.id);

    // Get a copy of current matrices to work with,
    // just in case stereo render is required, and we need to modify them
    // NOTE: At this point the modelview matrix just contains the view matrix (camera)
    // That's because BeginMode3D() sets it and there is no model-drawing function
    // that modifies it, all use rlPushMatrix() and rlPopMatrix()
    Matrix matModel = MatrixIdentity();
    Matrix matView = rlGetMatrixModelview();
    Matrix matModelView = MatrixIdentity();
    Matrix matProjection = rlGetMatrixProjection();

    // Model transformation matrix is sent to shader uniform location: RLG_LOC_MATRIX_MODEL
    if (shader.locs[RLG_LOC_MATRIX_MODEL] != -1)
        rlSetUniformMatrix(shader.locs[RLG_LOC_MATRIX_MODEL], transform);

    // Accumulate several model transformations:
    //    transform: model transformation provided (includes DrawModel() params combined with model.transform)
    //    rlGetMatrixTransform(): rlgl internal transform matrix due to push/pop matrix stack
    matModel = MatrixMultiply(transform, rlGetMatrixTransform());

    // Get model-view matrix
    matModelView = MatrixMultiply(matModel, matView);

    // Try binding vertex array objects (VAO) or use VBOs if not possible
    if (!rlEnableVertexArray(mesh.vaoId))
    {
        // Bind mesh VBO data: vertex position (shader-location = 0)
        rlEnableVertexBuffer(mesh.vboId[0]);
        rlSetVertexAttribute(shader.locs[RLG_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(shader.locs[RLG_LOC_VERTEX_POSITION]);

        // If vertex indices exist, bine the VBO containing the indices
        if (mesh.indices != NULL) rlEnableVertexBufferElement(mesh.vboId[6]);
    }

    int eyeCount = rlIsStereoRenderEnabled() ? 2 : 1;

    for (int eye = 0; eye < eyeCount; eye++)
    {
        // Calculate model-view-projection matrix (MVP)
        Matrix matModelViewProjection = MatrixIdentity();
        if (eyeCount == 1) matModelViewProjection = MatrixMultiply(matModelView, matProjection);
        else
        {
            // Setup current eye viewport (half screen width)
            rlViewport(eye*rlGetFramebufferWidth()/2, 0, rlGetFramebufferWidth()/2, rlGetFramebufferHeight());
            matModelViewProjection = MatrixMultiply(MatrixMultiply(matModelView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye));
        }

        // Send combined model-view-projection matrix to shader
        rlSetUniformMatrix(shader.locs[RLG_LOC_MATRIX_MVP], matModelViewProjection);

        // Draw mesh
        if (mesh.indices != NULL) rlDrawVertexArrayElements(0, mesh.triangleCount*3, 0);
        else rlDrawVertexArray(0, mesh.vertexCount);
    }

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Disable shader program
    rlDisableShader();

    // Restore rlgl internal modelview and projection matrices
    rlSetMatrixModelview(matView);
    rlSetMatrixProjection(matProjection);
}

void RLG_CastModel(Shader shader, Model model, Vector3 position, float scale)
{
    Vector3 vScale = { scale, scale, scale };
    Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };

    RLG_CastModelEx(shader, model, position, rotationAxis, 0.0f, vScale);
}

void RLG_CastModelEx(Shader shader, Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale)
{
    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle*DEG2RAD);
    Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);
    model.transform = MatrixMultiply(model.transform, matTransform);

    for (int i = 0; i < model.meshCount; i++)
    {
        RLG_CastMesh(shader, model.meshes[i], model.transform);
    }
}

void RLG_DrawMesh(Mesh mesh, Material material, Matrix transform)
{
    const Shader *shader = &rlgCtx->shaders[RLG_SHADER_LIGHTING];

    // Bind shader program
    rlEnableShader(shader->id);

    // Send required data to shader (matrices, values)
    //-----------------------------------------------------
    // Upload to shader material.data.colDiffuse
    if (shader->locs[RLG_LOC_COLOR_DIFFUSE] != -1)
    {
        float values[4];

        if (rlgCtx->usedDefaultMaps[MATERIAL_MAP_ALBEDO])
        {
            values[0] = (float)rlgCtx->defaultMaps[MATERIAL_MAP_ALBEDO].color.r/255.0f;
            values[1] = (float)rlgCtx->defaultMaps[MATERIAL_MAP_ALBEDO].color.g/255.0f;
            values[2] = (float)rlgCtx->defaultMaps[MATERIAL_MAP_ALBEDO].color.b/255.0f;
            values[3] = (float)rlgCtx->defaultMaps[MATERIAL_MAP_ALBEDO].color.a/255.0f;
        }
        else
        {
            values[0] = (float)material.maps[MATERIAL_MAP_ALBEDO].color.r/255.0f;
            values[1] = (float)material.maps[MATERIAL_MAP_ALBEDO].color.g/255.0f;
            values[2] = (float)material.maps[MATERIAL_MAP_ALBEDO].color.b/255.0f;
            values[3] = (float)material.maps[MATERIAL_MAP_ALBEDO].color.a/255.0f;
        }

        rlSetUniform(shader->locs[RLG_LOC_COLOR_DIFFUSE], values, SHADER_UNIFORM_VEC4, 1);
    }

    // Upload to shader material.data.colSpecular (if location available)
    if (shader->locs[RLG_LOC_COLOR_SPECULAR] != -1)
    {
        float values[4];

        if (rlgCtx->usedDefaultMaps[MATERIAL_MAP_METALNESS])
        {
            values[0] = (float)rlgCtx->defaultMaps[MATERIAL_MAP_METALNESS].color.r/255.0f;
            values[1] = (float)rlgCtx->defaultMaps[MATERIAL_MAP_METALNESS].color.g/255.0f;
            values[2] = (float)rlgCtx->defaultMaps[MATERIAL_MAP_METALNESS].color.b/255.0f;
            values[3] = (float)rlgCtx->defaultMaps[MATERIAL_MAP_METALNESS].color.a/255.0f;
        }
        else
        {
            values[0] = (float)material.maps[MATERIAL_MAP_METALNESS].color.r/255.0f;
            values[1] = (float)material.maps[MATERIAL_MAP_METALNESS].color.g/255.0f;
            values[2] = (float)material.maps[MATERIAL_MAP_METALNESS].color.b/255.0f;
            values[3] = (float)material.maps[MATERIAL_MAP_METALNESS].color.a/255.0f;
        }

        rlSetUniform(shader->locs[RLG_LOC_COLOR_SPECULAR], values, SHADER_UNIFORM_VEC4, 1);
    }

    // Upload to shader material.data.colEmission (if location available)
    if (shader->locs[RLG_LOC_COLOR_EMISSION] != -1)
    {
        float values[4];

        if (rlgCtx->usedDefaultMaps[MATERIAL_MAP_EMISSION])
        {
            values[0] = (float)rlgCtx->defaultMaps[MATERIAL_MAP_EMISSION].color.r/255.0f;
            values[1] = (float)rlgCtx->defaultMaps[MATERIAL_MAP_EMISSION].color.g/255.0f;
            values[2] = (float)rlgCtx->defaultMaps[MATERIAL_MAP_EMISSION].color.b/255.0f;
            values[3] = (float)rlgCtx->defaultMaps[MATERIAL_MAP_EMISSION].color.a/255.0f;
        }
        else
        {
            values[0] = (float)material.maps[MATERIAL_MAP_EMISSION].color.r/255.0f;
            values[1] = (float)material.maps[MATERIAL_MAP_EMISSION].color.g/255.0f;
            values[2] = (float)material.maps[MATERIAL_MAP_EMISSION].color.b/255.0f;
            values[3] = (float)material.maps[MATERIAL_MAP_EMISSION].color.a/255.0f;
        }

        rlSetUniform(shader->locs[RLG_LOC_COLOR_EMISSION], values, SHADER_UNIFORM_VEC4, 1);
    }

    // Upload to shader material.data.metalness (if location available)
    if (shader->locs[RLG_LOC_METALNESS_SCALE] != -1)
    {
        if (rlgCtx->usedDefaultMaps[MATERIAL_MAP_METALNESS])
        {
            rlSetUniform(shader->locs[RLG_LOC_METALNESS_SCALE],
                &rlgCtx->defaultMaps[MATERIAL_MAP_METALNESS].value, SHADER_UNIFORM_FLOAT, 1);
        }
        else
        {
            rlSetUniform(shader->locs[RLG_LOC_METALNESS_SCALE],
                &material.maps[MATERIAL_MAP_METALNESS].value, SHADER_UNIFORM_FLOAT, 1);
        }
    }

    // Upload to shader material.data.roughness (if location available)
    if (shader->locs[RLG_LOC_ROUGHNESS_SCALE] != -1)
    {
        if (rlgCtx->usedDefaultMaps[MATERIAL_MAP_ROUGHNESS])
        {
            rlSetUniform(shader->locs[RLG_LOC_ROUGHNESS_SCALE],
                &rlgCtx->defaultMaps[MATERIAL_MAP_ROUGHNESS].value, SHADER_UNIFORM_FLOAT, 1);
        }
        else
        {
            rlSetUniform(shader->locs[RLG_LOC_ROUGHNESS_SCALE],
                &material.maps[MATERIAL_MAP_ROUGHNESS].value, SHADER_UNIFORM_FLOAT, 1);
        }
    }

    // Upload to shader material.data.aoLightAffect (if location available)
    if (shader->locs[RLG_LOC_AO_LIGHT_AFFECT] != -1)
    {
        if (rlgCtx->usedDefaultMaps[MATERIAL_MAP_OCCLUSION])
        {
            rlSetUniform(shader->locs[RLG_LOC_AO_LIGHT_AFFECT],
                &rlgCtx->defaultMaps[MATERIAL_MAP_OCCLUSION].value, SHADER_UNIFORM_FLOAT, 1);
        }
        else
        {
            rlSetUniform(shader->locs[RLG_LOC_AO_LIGHT_AFFECT],
                &material.maps[MATERIAL_MAP_OCCLUSION].value, SHADER_UNIFORM_FLOAT, 1);
        }
    }

    // Upload to shader material.data.heightScale (if location available)
    if (shader->locs[RLG_LOC_HEIGHT_SCALE] != -1)
    {
        if (rlgCtx->usedDefaultMaps[MATERIAL_MAP_HEIGHT])
        {
            rlSetUniform(shader->locs[RLG_LOC_HEIGHT_SCALE],
                &rlgCtx->defaultMaps[MATERIAL_MAP_HEIGHT].value, SHADER_UNIFORM_FLOAT, 1);
        }
        else
        {
            rlSetUniform(shader->locs[RLG_LOC_HEIGHT_SCALE],
                &material.maps[MATERIAL_MAP_HEIGHT].value, SHADER_UNIFORM_FLOAT, 1);
        }
    }

    // Get a copy of current matrices to work with,
    // just in case stereo render is required, and we need to modify them
    // NOTE: At this point the modelview matrix just contains the view matrix (camera)
    // That's because BeginMode3D() sets it and there is no model-drawing function
    // that modifies it, all use rlPushMatrix() and rlPopMatrix()
    Matrix matModel = MatrixIdentity();
    Matrix matView = rlGetMatrixModelview();
    Matrix matModelView = MatrixIdentity();
    Matrix matProjection = rlGetMatrixProjection();

    // Upload view matrix (if location available)
    if (shader->locs[RLG_LOC_MATRIX_VIEW] != -1)
        rlSetUniformMatrix(shader->locs[RLG_LOC_MATRIX_VIEW], matView);

    // Upload projection matrix (if location available)
    if (shader->locs[RLG_LOC_MATRIX_PROJECTION] != -1)
        rlSetUniformMatrix(shader->locs[RLG_LOC_MATRIX_PROJECTION], matProjection);

    // Model transformation matrix is sent to shader uniform location: RLG_LOC_MATRIX_MODEL
    if (shader->locs[RLG_LOC_MATRIX_MODEL] != -1)
        rlSetUniformMatrix(shader->locs[RLG_LOC_MATRIX_MODEL], transform);

    // Accumulate several model transformations:
    //    transform: model transformation provided (includes DrawModel() params combined with model.transform)
    //    rlGetMatrixTransform(): rlgl internal transform matrix due to push/pop matrix stack
    matModel = MatrixMultiply(transform, rlGetMatrixTransform());

    // Get model-view matrix
    matModelView = MatrixMultiply(matModel, matView);

    // Upload model normal matrix (if locations available)
    if (shader->locs[RLG_LOC_MATRIX_NORMAL] != -1)
        rlSetUniformMatrix(shader->locs[RLG_LOC_MATRIX_NORMAL], MatrixTranspose(MatrixInvert(matModel)));
    //-----------------------------------------------------

    // Bind active texture maps (if available)
    for (int i = 0; i < 11; i++)
    {
        if (rlgCtx->material.data.useMaps[i])
        {
            int textureID = (rlgCtx->usedDefaultMaps[i])
                ? rlgCtx->defaultMaps[i].texture.id
                : material.maps[i].texture.id;

            if (textureID > 0)
            {
                // Select current shader texture slot
                rlActiveTextureSlot(i);

                // Enable texture for active slot
                if (i == MATERIAL_MAP_IRRADIANCE ||
                    i == MATERIAL_MAP_PREFILTER ||
                    i == MATERIAL_MAP_CUBEMAP)
                {
                    rlEnableTextureCubemap(textureID);
                }
                else
                {
                    rlEnableTexture(textureID);
                }

                rlSetUniform(shader->locs[RLG_LOC_MAP_ALBEDO + i], &i, SHADER_UNIFORM_INT, 1);
            }
        }
    }

    // Bind depth textures for shadow mapping
    for (unsigned int i = 0; i < rlgCtx->lightCount; i++)
    {
        const struct RLG_Light *l = &rlgCtx->lights[i];

        if (l->data.enabled && l->data.shadow)
        {
            int j = 11 + i;
            rlActiveTextureSlot(j);

            if (l->data.type == RLG_OMNILIGHT)
            {
                rlEnableTextureCubemap(l->data.shadowMap.depth.id);
                rlSetUniform(l->locs.shadowCubemap, &j, SHADER_UNIFORM_INT, 1);
            }
            else
            {
                rlEnableTexture(l->data.shadowMap.depth.id);
                rlSetUniform(l->locs.shadowMap, &j, SHADER_UNIFORM_INT, 1);
            }
        }
    }

    // Try binding vertex array objects (VAO) or use VBOs if not possible
    // WARNING: UploadMesh() enables all vertex attributes available in mesh and sets default attribute values
    // for shader expected vertex attributes that are not provided by the mesh (i.e. colors)
    // This could be a dangerous approach because different meshes with different shaders can enable/disable some attributes
    if (!rlEnableVertexArray(mesh.vaoId))
    {
        // Bind mesh VBO data: vertex position (shader-location = 0)
        rlEnableVertexBuffer(mesh.vboId[0]);
        rlSetVertexAttribute(shader->locs[RLG_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(shader->locs[RLG_LOC_VERTEX_POSITION]);

        // Bind mesh VBO data: vertex texcoords (shader-location = 1)
        rlEnableVertexBuffer(mesh.vboId[1]);
        rlSetVertexAttribute(shader->locs[RLG_LOC_VERTEX_TEXCOORD01], 2, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(shader->locs[RLG_LOC_VERTEX_TEXCOORD01]);

        if (shader->locs[RLG_LOC_VERTEX_NORMAL] != -1)
        {
            // Bind mesh VBO data: vertex normals (shader-location = 2)
            rlEnableVertexBuffer(mesh.vboId[2]);
            rlSetVertexAttribute(shader->locs[RLG_LOC_VERTEX_NORMAL], 3, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(shader->locs[RLG_LOC_VERTEX_NORMAL]);
        }

        // Bind mesh VBO data: vertex colors (shader-location = 3, if available)
        if (shader->locs[RLG_LOC_VERTEX_COLOR] != -1)
        {
            if (mesh.vboId[3] != 0)
            {
                rlEnableVertexBuffer(mesh.vboId[3]);
                rlSetVertexAttribute(shader->locs[RLG_LOC_VERTEX_COLOR], 4, RL_UNSIGNED_BYTE, 1, 0, 0);
                rlEnableVertexAttribute(shader->locs[RLG_LOC_VERTEX_COLOR]);
            }
            else
            {
                // Set default value for defined vertex attribute in shader but not provided by mesh
                // WARNING: It could result in GPU undefined behaviour
                float value[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
                rlSetVertexAttributeDefault(shader->locs[RLG_LOC_VERTEX_COLOR], value, SHADER_ATTRIB_VEC4, 4);
                rlDisableVertexAttribute(shader->locs[RLG_LOC_VERTEX_COLOR]);
            }
        }

        // Bind mesh VBO data: vertex tangents (shader-location = 4, if available)
        if (shader->locs[RLG_LOC_VERTEX_TANGENT] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[4]);
            rlSetVertexAttribute(shader->locs[RLG_LOC_VERTEX_TANGENT], 4, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(shader->locs[RLG_LOC_VERTEX_TANGENT]);
        }

        // Bind mesh VBO data: vertex texcoords2 (shader-location = 5, if available)
        if (shader->locs[RLG_LOC_VERTEX_TEXCOORD02] != -1)
        {
            rlEnableVertexBuffer(mesh.vboId[5]);
            rlSetVertexAttribute(shader->locs[RLG_LOC_VERTEX_TEXCOORD02], 2, RL_FLOAT, 0, 0, 0);
            rlEnableVertexAttribute(shader->locs[RLG_LOC_VERTEX_TEXCOORD02]);
        }

        if (mesh.indices != NULL) rlEnableVertexBufferElement(mesh.vboId[6]);
    }

    int eyeCount = 1;
    if (rlIsStereoRenderEnabled()) eyeCount = 2;

    for (int eye = 0; eye < eyeCount; eye++)
    {
        // Calculate model-view-projection matrix (MVP)
        Matrix matModelViewProjection = MatrixIdentity();
        if (eyeCount == 1) matModelViewProjection = MatrixMultiply(matModelView, matProjection);
        else
        {
            // Setup current eye viewport (half screen width)
            rlViewport(eye*rlGetFramebufferWidth()/2, 0, rlGetFramebufferWidth()/2, rlGetFramebufferHeight());
            matModelViewProjection = MatrixMultiply(MatrixMultiply(matModelView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye));
        }

        // Send combined model-view-projection matrix to shader
        rlSetUniformMatrix(shader->locs[RLG_LOC_MATRIX_MVP], matModelViewProjection);

        // Draw mesh
        if (mesh.indices != NULL) rlDrawVertexArrayElements(0, mesh.triangleCount*3, 0);
        else rlDrawVertexArray(0, mesh.vertexCount);
    }

    // Unbind all bound texture maps
    for (int i = 0; i < 11; i++)
    {
        if (rlgCtx->material.data.useMaps[i])
        {
            // Select current shader texture slot
            rlActiveTextureSlot(i);

            // Disable texture for active slot
            if (i == MATERIAL_MAP_IRRADIANCE ||
                i == MATERIAL_MAP_PREFILTER ||
                i == MATERIAL_MAP_CUBEMAP)
            {
                rlDisableTextureCubemap();
            }
            else
            {
                rlDisableTexture();
            }
        }
    }

    // Unbind depth textures
    for (unsigned int i = 0; i < rlgCtx->lightCount; i++)
    {
        const struct RLG_Light *l = &rlgCtx->lights[i];

        if (l->data.enabled && l->data.shadow)
        {
            rlActiveTextureSlot(11 + i);

            if (l->data.type == RLG_OMNILIGHT)
            {
                rlDisableTextureCubemap();
            }
            else
            {
                rlDisableTexture();
            }
        }
    }

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Disable shader program
    rlDisableShader();

    // Restore rlgl internal modelview and projection matrices
    rlSetMatrixModelview(matView);
    rlSetMatrixProjection(matProjection);
}

void RLG_DrawModel(Model model, Vector3 position, float scale, Color tint)
{
    Vector3 vScale = { scale, scale, scale };
    Vector3 rotationAxis = { 0.0f, 1.0f, 0.0f };

    RLG_DrawModelEx(model, position, rotationAxis, 0.0f, vScale, tint);
}

void RLG_DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint)
{
    Matrix matScale = MatrixScale(scale.x, scale.y, scale.z);
    Matrix matRotation = MatrixRotate(rotationAxis, rotationAngle*DEG2RAD);
    Matrix matTranslation = MatrixTranslate(position.x, position.y, position.z);

    Matrix matTransform = MatrixMultiply(MatrixMultiply(matScale, matRotation), matTranslation);
    model.transform = MatrixMultiply(model.transform, matTransform);

    for (int i = 0; i < model.meshCount; i++)
    {
        Color color = model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color;

        Color colorTint = WHITE;
        colorTint.r = (unsigned char)((color.r*tint.r)/255);
        colorTint.g = (unsigned char)((color.g*tint.g)/255);
        colorTint.b = (unsigned char)((color.b*tint.b)/255);
        colorTint.a = (unsigned char)((color.a*tint.a)/255);

        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = colorTint;
        RLG_DrawMesh(model.meshes[i], model.materials[model.meshMaterial[i]], model.transform);
        model.materials[model.meshMaterial[i]].maps[MATERIAL_MAP_DIFFUSE].color = color;
    }
}

RLG_Skybox RLG_LoadSkybox(const char* skyboxFileName)
{
    // Define the positions of the vertices for a cube
    static const float positions[] =
    {
        // Front face
        -0.5f, -0.5f,  0.5f,    // Vertex 0
         0.5f, -0.5f,  0.5f,    // Vertex 1
         0.5f,  0.5f,  0.5f,    // Vertex 2
        -0.5f,  0.5f,  0.5f,    // Vertex 3

        // Back face
        -0.5f, -0.5f, -0.5f,    // Vertex 4
         0.5f, -0.5f, -0.5f,    // Vertex 5
         0.5f,  0.5f, -0.5f,    // Vertex 6
        -0.5f,  0.5f, -0.5f     // Vertex 7
    };

    // Define the indices for drawing the cube faces
    const unsigned short indices[] =
    {
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Right face
        1, 5, 6,
        6, 2, 1,

        // Back face
        5, 4, 7,
        7, 6, 5,

        // Left face
        4, 0, 3,
        3, 7, 4,

        // Top face
        3, 2, 6,
        6, 7, 3,

        // Bottom face
        4, 5, 1,
        1, 0, 4
    };

    RLG_Skybox skybox = { 0 };

    // Load vertex array object (VAO) and bind it
    skybox.vaoID = rlLoadVertexArray();
    rlEnableVertexArray(skybox.vaoID);
    {
        // Load vertex buffer object (VBO) for positions and bind it
        skybox.vboPostionsID = rlLoadVertexBuffer(positions, sizeof(positions), false);
        rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(0);

        // Load element buffer object (EBO) for indices and bind it
        skybox.vboIndicesID = rlLoadVertexBufferElement(indices, sizeof(indices), false);
    }
    rlDisableVertexArray();

    // Load the cubemap texture from the image file
    Image img = LoadImage(skyboxFileName);
    skybox.cubemap = LoadTextureCubemap(img, CUBEMAP_LAYOUT_AUTO_DETECT);
    UnloadImage(img);

    // Generate Irradiance Cubemap
    {
        int size = skybox.cubemap.width / 16;
        size = (size < 8) ? 8 : size;

        // Create a renderbuffer for depth attachment
        unsigned int rbo = rlLoadTextureDepth(size, size, true);

        // Create a cubemap texture to hold the HDR data
        skybox.irradiance.id = rlLoadTextureCubemap(NULL, size, skybox.cubemap.format);
        rlCubemapParameters(skybox.irradiance.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
        rlCubemapParameters(skybox.irradiance.id, GL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);

        // Create and configure the framebuffer
        unsigned int fbo = rlLoadFramebuffer(size, size);
        rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
        rlFramebufferAttach(fbo, skybox.irradiance.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

        // Validate the framebuffer configuration
        if (rlFramebufferComplete(fbo))
        {
            TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", fbo);
        }  

        // Enable the shader for converting HDR equirectangular environment map to cubemap faces
        rlEnableShader(rlgCtx->shaders[RLG_SHADER_IRRADIANCE_CONVOLUTION].id);

        // Set the projection matrix for the shader
        Matrix matFboProjection = MatrixPerspective(90.0 * DEG2RAD, 1.0, 0.1, 10.0);
        rlSetUniformMatrix(rlgCtx->shaders[RLG_SHADER_IRRADIANCE_CONVOLUTION].locs[SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

        // Define view matrices for each cubemap face
        Matrix fboViews[6] = {
            MatrixLookAt((Vector3){0}, (Vector3){  1.0f,  0.0f,  0.0f}, (Vector3) {0.0f, -1.0f,  0.0f}),
            MatrixLookAt((Vector3){0}, (Vector3){ -1.0f,  0.0f,  0.0f}, (Vector3) {0.0f, -1.0f,  0.0f}),
            MatrixLookAt((Vector3){0}, (Vector3){  0.0f,  1.0f,  0.0f}, (Vector3) {0.0f,  0.0f,  1.0f}),
            MatrixLookAt((Vector3){0}, (Vector3){  0.0f, -1.0f,  0.0f}, (Vector3) {0.0f,  0.0f, -1.0f}),
            MatrixLookAt((Vector3){0}, (Vector3){  0.0f,  0.0f,  1.0f}, (Vector3) {0.0f, -1.0f,  0.0f}),
            MatrixLookAt((Vector3){0}, (Vector3){  0.0f,  0.0f, -1.0f}, (Vector3) {0.0f, -1.0f,  0.0f})
        };

        // Set the viewport to match the framebuffer dimensions
        rlViewport(0, 0, size, size);
        rlDisableBackfaceCulling();

        // Activate the panorama texture
        rlActiveTextureSlot(0);
        rlEnableTextureCubemap(skybox.cubemap.id);

        for (int i = 0; i < 6; i++)
        {
            // Set the view matrix for the current cubemap face
            rlSetUniformMatrix(rlgCtx->shaders[RLG_SHADER_IRRADIANCE_CONVOLUTION].locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);
            
            // Attach the current cubemap face to the framebuffer
            rlFramebufferAttach(fbo, skybox.irradiance.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);
            rlEnableFramebuffer(fbo);

            // Clear the framebuffer and draw the cube face
            rlClearScreenBuffers();
            rlLoadDrawCube();
        }

        // Disable the shader and textures
        rlDisableShader();
        rlDisableTextureCubemap();
        rlDisableFramebuffer();

        // Unload the framebuffer and its attachments
        rlUnloadFramebuffer(fbo);

        // Reset the viewport to default dimensions
        rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
        rlEnableBackfaceCulling();

        // Set the cubemap properties
        skybox.cubemap.width = size;
        skybox.cubemap.height = size;
        skybox.cubemap.mipmaps = 1;
        skybox.cubemap.format = skybox.cubemap.format;
    }

    return skybox;
}

RLG_Skybox RLG_LoadSkyboxHDR(const char* skyboxFileName, int size, int format)
{
    // Define the positions of the vertices for a cube
    static const float positions[] =
    {
        // Front face
        -0.5f, -0.5f,  0.5f,    // Vertex 0
         0.5f, -0.5f,  0.5f,    // Vertex 1
         0.5f,  0.5f,  0.5f,    // Vertex 2
        -0.5f,  0.5f,  0.5f,    // Vertex 3

        // Back face
        -0.5f, -0.5f, -0.5f,    // Vertex 4
         0.5f, -0.5f, -0.5f,    // Vertex 5
         0.5f,  0.5f, -0.5f,    // Vertex 6
        -0.5f,  0.5f, -0.5f     // Vertex 7
    };

    // Define the indices for drawing the cube faces
    const unsigned short indices[] =
    {
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Right face
        1, 5, 6,
        6, 2, 1,

        // Back face
        5, 4, 7,
        7, 6, 5,

        // Left face
        4, 0, 3,
        3, 7, 4,

        // Top face
        3, 2, 6,
        6, 7, 3,

        // Bottom face
        4, 5, 1,
        1, 0, 4
    };

    RLG_Skybox skybox = { 0 };

    // Generate a vertex array object (VAO) for the skybox
    skybox.vaoID = rlLoadVertexArray();
    rlEnableVertexArray(skybox.vaoID);
    {
        // Load the vertex positions into a vertex buffer object (VBO)
        skybox.vboPostionsID = rlLoadVertexBuffer(positions, sizeof(positions), false);
        rlSetVertexAttribute(0, 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(0);

        // Load the indices into an element buffer object (EBO)
        skybox.vboIndicesID = rlLoadVertexBufferElement(indices, sizeof(indices), false);
    }
    rlDisableVertexArray();

    // Create a framebuffer object (FBO) to generate the skybox and irradiance map
    unsigned int fbo = rlLoadFramebuffer(0, 0);

    // Generate the cubemap for the skybox
    {
        // Load the HDR panorama texture
        Texture2D panorama = LoadTexture(skyboxFileName);

        // Create a renderbuffer for depth attachment
        unsigned int rbo = rlLoadTextureDepth(size, size, true);

        // Create a cubemap texture to hold the HDR data
        skybox.cubemap.id = rlLoadTextureCubemap(NULL, size, format);

        // Configure the framebuffer with the renderbuffer and cubemap texture
        rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
        rlFramebufferAttach(fbo, skybox.cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

        // Validate the framebuffer configuration
        if (rlFramebufferComplete(fbo))
        {
            TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", fbo);
        }

        // Enable the shader to convert the HDR equirectangular map to cubemap faces
        rlEnableShader(rlgCtx->shaders[RLG_SHADER_EQUIRECTANGULAR_TO_CUBEMAP].id);

        // Set the projection matrix for the shader
        Matrix matFboProjection = MatrixPerspective(90.0 * DEG2RAD, 1.0, 0.1, 10.0);
        rlSetUniformMatrix(rlgCtx->shaders[RLG_SHADER_EQUIRECTANGULAR_TO_CUBEMAP].locs[SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

        // Define view matrices for each cubemap face
        Matrix fboViews[6] = {
            MatrixLookAt((Vector3){0}, (Vector3) { 1.0f,  0.0f,  0.0f}, (Vector3){ 0.0f, -1.0f,  0.0f}),
            MatrixLookAt((Vector3){0}, (Vector3) {-1.0f,  0.0f,  0.0f}, (Vector3){ 0.0f, -1.0f,  0.0f}),
            MatrixLookAt((Vector3){0}, (Vector3) { 0.0f,  1.0f,  0.0f}, (Vector3){ 0.0f,  0.0f,  1.0f}),
            MatrixLookAt((Vector3){0}, (Vector3) { 0.0f, -1.0f,  0.0f}, (Vector3){ 0.0f,  0.0f, -1.0f}),
            MatrixLookAt((Vector3){0}, (Vector3) { 0.0f,  0.0f,  1.0f}, (Vector3){ 0.0f, -1.0f,  0.0f}),
            MatrixLookAt((Vector3){0}, (Vector3) { 0.0f,  0.0f, -1.0f}, (Vector3){ 0.0f, -1.0f,  0.0f})
        };

        // Set the viewport to match the framebuffer dimensions
        rlViewport(0, 0, size, size);
        rlDisableBackfaceCulling();

        // Activate the panorama texture
        rlActiveTextureSlot(0);
        rlEnableTexture(panorama.id);

        for (int i = 0; i < 6; i++)
        {
            // Set the view matrix for the current cubemap face
            rlSetUniformMatrix(rlgCtx->shaders[RLG_SHADER_EQUIRECTANGULAR_TO_CUBEMAP].locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);
            
            // Attach the current cubemap face to the framebuffer
            rlFramebufferAttach(fbo, skybox.cubemap.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);
            rlEnableFramebuffer(fbo);

            // Clear the framebuffer and draw the cube
            rlClearScreenBuffers();
            rlLoadDrawCube();
        }

        // Disable the shader and textures
        rlDisableShader();
        rlDisableTexture();
        rlDisableFramebuffer();

        // Reset the viewport to default dimensions
        rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
        rlEnableBackfaceCulling();

        // Set the cubemap properties
        skybox.cubemap.width = size;
        skybox.cubemap.height = size;
        skybox.cubemap.mipmaps = 1;
        skybox.cubemap.format = format;

        // Unload the panorama texture as it's no longer needed
        UnloadTexture(panorama);
    }

    // Generate the irradiance cubemap
    {
        int irrSize = skybox.cubemap.width / 16;
        irrSize = (irrSize < 8) ? 8 : irrSize;

        // Create a renderbuffer for depth attachment
        unsigned int rbo = rlLoadTextureDepth(irrSize, irrSize, true);

        // Create a cubemap texture to hold the irradiance data
        skybox.irradiance.id = rlLoadTextureCubemap(NULL, irrSize, skybox.cubemap.format);
        rlCubemapParameters(skybox.irradiance.id, RL_TEXTURE_MIN_FILTER, RL_TEXTURE_FILTER_LINEAR);
        rlCubemapParameters(skybox.irradiance.id, GL_TEXTURE_MAG_FILTER, RL_TEXTURE_FILTER_LINEAR);

        // Create and configure the framebuffer
        unsigned int fbo = rlLoadFramebuffer(irrSize, irrSize);
        rlFramebufferAttach(fbo, rbo, RL_ATTACHMENT_DEPTH, RL_ATTACHMENT_RENDERBUFFER, 0);
        rlFramebufferAttach(fbo, skybox.irradiance.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X, 0);

        // Validate the framebuffer configuration
        if (rlFramebufferComplete(fbo))
        {
            TraceLog(LOG_INFO, "FBO: [ID %i] Framebuffer object created successfully", fbo);
        }

        // Enable the shader for irradiance convolution
        rlEnableShader(rlgCtx->shaders[RLG_SHADER_IRRADIANCE_CONVOLUTION].id);

        // Set the projection matrix for the shader
        Matrix matFboProjection = MatrixPerspective(90.0 * DEG2RAD, 1.0, 0.1, 10.0);
        rlSetUniformMatrix(rlgCtx->shaders[RLG_SHADER_IRRADIANCE_CONVOLUTION].locs[SHADER_LOC_MATRIX_PROJECTION], matFboProjection);

        // Define view matrices for each cubemap face
        Matrix fboViews[6] = {
            MatrixLookAt((Vector3){0}, (Vector3){  1.0f,  0.0f,  0.0f}, (Vector3){ 0.0f, -1.0f,  0.0f}),
            MatrixLookAt((Vector3){0}, (Vector3){ -1.0f,  0.0f,  0.0f}, (Vector3){ 0.0f, -1.0f,  0.0f}),
            MatrixLookAt((Vector3){0}, (Vector3){  0.0f,  1.0f,  0.0f}, (Vector3){ 0.0f,  0.0f,  1.0f}),
            MatrixLookAt((Vector3){0}, (Vector3){  0.0f, -1.0f,  0.0f}, (Vector3){ 0.0f,  0.0f, -1.0f}),
            MatrixLookAt((Vector3){0}, (Vector3){  0.0f,  0.0f,  1.0f}, (Vector3){ 0.0f, -1.0f,  0.0f}),
            MatrixLookAt((Vector3){0}, (Vector3){  0.0f,  0.0f, -1.0f}, (Vector3){ 0.0f, -1.0f,  0.0f})
        };

        // Set the viewport to match the framebuffer dimensions
        rlViewport(0, 0, irrSize, irrSize);
        rlDisableBackfaceCulling();

        // Activate the cubemap texture
        rlActiveTextureSlot(0);
        rlEnableTextureCubemap(skybox.cubemap.id);

        for (int i = 0; i < 6; i++)
        {
            // Set the view matrix for the current cubemap face
            rlSetUniformMatrix(rlgCtx->shaders[RLG_SHADER_IRRADIANCE_CONVOLUTION].locs[SHADER_LOC_MATRIX_VIEW], fboViews[i]);
            
            // Attach the current cubemap face to the framebuffer
            rlFramebufferAttach(fbo, skybox.irradiance.id, RL_ATTACHMENT_COLOR_CHANNEL0, RL_ATTACHMENT_CUBEMAP_POSITIVE_X + i, 0);
            rlEnableFramebuffer(fbo);

            // Clear the framebuffer and draw the cube
            rlClearScreenBuffers();
            rlLoadDrawCube();
        }

        // Disable the shader and textures
        rlDisableShader();
        rlDisableTextureCubemap();
        rlDisableFramebuffer();

        // Reset the viewport to default dimensions
        rlViewport(0, 0, rlGetFramebufferWidth(), rlGetFramebufferHeight());
        rlEnableBackfaceCulling();

        // Set the cubemap properties
        skybox.cubemap.width = irrSize;
        skybox.cubemap.height = irrSize;
        skybox.cubemap.mipmaps = 1;
        skybox.cubemap.format = skybox.cubemap.format;
    }

    // Unload the framebuffer
    rlUnloadFramebuffer(fbo);

    // Indicate that the texture used is HDR
    skybox.isHDR = true;

    return skybox;
}

void RLG_UnloadSkybox(RLG_Skybox skybox)
{
    UnloadTexture(skybox.cubemap);
    UnloadTexture(skybox.irradiance);

    rlUnloadVertexArray(skybox.vaoID);
    rlUnloadVertexBuffer(skybox.vboIndicesID);
    rlUnloadVertexBuffer(skybox.vboPostionsID);
}

void RLG_DrawSkybox(RLG_Skybox skybox)
{
    Shader *shader = &rlgCtx->shaders[RLG_SHADER_SKYBOX];

    // Bind shader program
    rlEnableShader(shader->id);

    if (rlgCtx->skybox.previousCubemapID != skybox.cubemap.id)
    {
        int isHDR = (int)skybox.isHDR;
        rlSetUniform(rlgCtx->skybox.locDoGamma, &isHDR, SHADER_UNIFORM_INT, 1);
        rlgCtx->skybox.previousCubemapID = skybox.cubemap.id;
    }

    rlDisableBackfaceCulling();
    rlDisableDepthMask();

    // Get current view/projection matrices
    Matrix matView = rlGetMatrixModelview();
    Matrix matProjection = rlGetMatrixProjection();

    // Upload view and projection matrices (if locations available)
    if (shader->locs[SHADER_LOC_MATRIX_VIEW] != -1) rlSetUniformMatrix(shader->locs[SHADER_LOC_MATRIX_VIEW], matView);
    if (shader->locs[SHADER_LOC_MATRIX_PROJECTION] != -1) rlSetUniformMatrix(shader->locs[SHADER_LOC_MATRIX_PROJECTION], matProjection);

    // Bind cubemap texture (if available)
    if (skybox.cubemap.id > 0)
    {
        rlActiveTextureSlot(0);
        rlEnableTextureCubemap(skybox.cubemap.id);
    }

    // Try binding vertex array objects (VAO) or use VBOs if not possible
    if (!rlEnableVertexArray(skybox.vaoID))
    {
        // Bind mesh VBO data: vertex position (shader-location = 0)
        rlEnableVertexBuffer(skybox.vboPostionsID);
        rlSetVertexAttribute(shader->locs[SHADER_LOC_VERTEX_POSITION], 3, RL_FLOAT, 0, 0, 0);
        rlEnableVertexAttribute(shader->locs[SHADER_LOC_VERTEX_POSITION]);

        if (skybox.vboIndicesID != 0)
        {
            rlEnableVertexBufferElement(skybox.vboIndicesID);
        }
    }

    int eyeCount = 1;
    if (rlIsStereoRenderEnabled()) eyeCount = 2;

    for (int eye = 0; eye < eyeCount; eye++)
    {
        // Calculate model-view-projection matrix (MVP)
        Matrix matModelViewProjection = MatrixIdentity();
        if (eyeCount == 1)
        {
            matModelViewProjection = MatrixMultiply(matView, matProjection);
        }
        else
        {
            // Setup current eye viewport (half screen width)
            rlViewport(eye*rlGetFramebufferWidth()/2, 0, rlGetFramebufferWidth()/2, rlGetFramebufferHeight());
            matModelViewProjection = MatrixMultiply(MatrixMultiply(matView, rlGetMatrixViewOffsetStereo(eye)), rlGetMatrixProjectionStereo(eye));
        }

        // Send combined model-view-projection matrix to shader
        rlSetUniformMatrix(shader->locs[SHADER_LOC_MATRIX_MVP], matModelViewProjection);

        // Draw mesh
        if (skybox.vboIndicesID != 0)
        {
            rlDrawVertexArrayElements(0, 36, 0);
        }
        else
        {
            rlDrawVertexArray(0, 36);
        }
    }

    // Unbind bound cubemap texture
    if (skybox.cubemap.id > 0)
    {
        rlActiveTextureSlot(0);
        rlDisableTextureCubemap();
    }

    // Disable all possible vertex array objects (or VBOs)
    rlDisableVertexArray();
    rlDisableVertexBuffer();
    rlDisableVertexBufferElement();

    // Disable shader program
    rlDisableShader();

    // Restore rlgl internal modelview and projection matrices
    rlSetMatrixModelview(matView);
    rlSetMatrixProjection(matProjection);

    rlEnableBackfaceCulling();
    rlEnableDepthMask();
}