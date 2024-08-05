#ifndef RLIGHTS_H
#define RLIGHTS_H

#include "raylib.h"

#ifndef GL_HEADER
#   if defined(GRAPHICS_API_OPENGL_ES2)
#       define GL_HEADER "external/glad_gles2.h"    // Required for: OpenGL functionality
#   else
#       if defined(__APPLE__)
#           define GL_SILENCE_DEPRECATION           // Silence Opengl API deprecation warnings 
#           define GL_HEADER <OpenGL/gl3.h>         // OpenGL 3 library for OSX
#           define GL_EXT_HEADER <OpenGL/gl3ext.h>  // OpenGL 3 extensions library for OSX
#       else
#           define GL_HEADER "external/glad.h"      // Required for: OpenGL functionality 
#       endif
#   endif
#endif

#include GL_HEADER

#ifdef GL_EXT_HEADER
#   include GL_EXT_HEADER
#endif

#ifndef GLSL_VERSION
#   ifdef PLATFORM_DESKTOP
#       define GLSL_VERSION 330
#   else
#       define GLSL_VERSION 100
#   endif //PLATFORM
#endif //GLSL_VERSION

/**
 * @brief Enum representing different types of lights.
 */
typedef enum {
    RLG_DIRLIGHT = 0,                       ///< Enum representing a directional light type.
    RLG_OMNILIGHT,                          ///< Enum representing an omnilight type.
    RLG_SPOTLIGHT                           ///< Enum representing a spotlight type.
} RLG_LightType;

/**
 * @brief Enum representing different types of shaders.
 */
typedef enum {
    RLG_SHADER_LIGHTING = 0,                ///< Enum representing the main lighting shader.
    RLG_SHADER_DEPTH,                       ///< Enum representing the depth writing shader for shadow maps.
    RLG_SHADER_DEPTH_CUBEMAP,               ///< Enum representing the depth writing shader for shadow cubemaps.
    RLG_SHADER_EQUIRECTANGULAR_TO_CUBEMAP,  ///< Enum representing the shader for generating skyboxes from HDR textures.
    RLG_SHADER_IRRADIANCE_CONVOLUTION,      ///< Enum representing the shader for generating irradiance maps from skyboxes.
    RLG_SHADER_SKYBOX                       ///< Enum representing the shader for rendering skyboxes.
} RLG_Shader;

/**
 * @brief Enum representing different properties of a light.
 */
typedef enum {
    RLG_LIGHT_POSITION = 0,                 ///< Position of the light.
    RLG_LIGHT_DIRECTION,                    ///< Direction of the light.
    RLG_LIGHT_COLOR,                        ///< Diffuse color of the light.
    RLG_LIGHT_ENERGY,                       ///< Energy factor of the light.
    RLG_LIGHT_SPECULAR,                     ///< Specular tint color of the light.
    RLG_LIGHT_SIZE,                         ///< Light size, affects fade and shadow blur (spotlight, omnilight only).
    RLG_LIGHT_INNER_CUTOFF,                 ///< Inner cutoff angle of a spotlight.
    RLG_LIGHT_OUTER_CUTOFF,                 ///< Outer cutoff angle of a spotlight.
    RLG_LIGHT_ATTENUATION_CLQ,              ///< Attenuation coefficients (constant, linear, quadratic) of the light.
    RLG_LIGHT_ATTENUATION_CONSTANT,         ///< Constant attenuation coefficient of the light.
    RLG_LIGHT_ATTENUATION_LINEAR,           ///< Linear attenuation coefficient of the light.
    RLG_LIGHT_ATTENUATION_QUADRATIC         ///< Quadratic attenuation coefficient of the light.
} RLG_LightProperty;

/**
 * @brief Enum representing all shader locations used by rlights.
 */
typedef enum {

    /* Same as raylib */

    RLG_LOC_VERTEX_POSITION = 0,
    RLG_LOC_VERTEX_TEXCOORD01,
    RLG_LOC_VERTEX_TEXCOORD02,
    RLG_LOC_VERTEX_NORMAL,
    RLG_LOC_VERTEX_TANGENT,
    RLG_LOC_VERTEX_COLOR,
    RLG_LOC_MATRIX_MVP,
    RLG_LOC_MATRIX_VIEW,
    RLG_LOC_MATRIX_PROJECTION,
    RLG_LOC_MATRIX_MODEL,
    RLG_LOC_MATRIX_NORMAL,
    RLG_LOC_VECTOR_VIEW,
    RLG_LOC_COLOR_DIFFUSE,
    RLG_LOC_COLOR_SPECULAR,
    RLG_LOC_COLOR_AMBIENT,
    RLG_LOC_MAP_ALBEDO,
    RLG_LOC_MAP_METALNESS,
    RLG_LOC_MAP_NORMAL,
    RLG_LOC_MAP_ROUGHNESS,
    RLG_LOC_MAP_OCCLUSION,
    RLG_LOC_MAP_EMISSION,
    RLG_LOC_MAP_HEIGHT,
    RLG_LOC_MAP_CUBEMAP,
    RLG_LOC_MAP_IRRADIANCE,
    RLG_LOC_MAP_PREFILTER,
    RLG_LOC_MAP_BRDF,

    /* Specific to rlights.h */

    RLG_LOC_COLOR_EMISSION,
    RLG_LOC_METALNESS_SCALE,
    RLG_LOC_ROUGHNESS_SCALE,
    RLG_LOC_AO_LIGHT_AFFECT,
    RLG_LOC_HEIGHT_SCALE,

    /* Internal use */

    RLG_COUNT_LOCS

} RLG_ShaderLocationIndex;

/**
 * @brief Structure representing a skybox with associated textures and buffers.
 *
 * This structure contains the textures and buffer IDs necessary for rendering
 * a skybox. It includes the cubemap texture, the irradiance texture, vertex
 * buffer object (VBO) IDs, vertex array object (VAO) ID, and a flag indicating
 * whether the skybox is in high dynamic range (HDR).
 */
typedef struct {
    TextureCubemap cubemap;       ///< The cubemap texture representing the skybox.
    TextureCubemap irradiance;    ///< The irradiance cubemap texture for diffuse lighting.
    int vboPostionsID;            ///< The ID of the vertex buffer object for positions.
    int vboIndicesID;             ///< The ID of the vertex buffer object for indices.
    int vaoID;                    ///< The ID of the vertex array object.
    bool isHDR;                   ///< Flag indicating if the skybox is HDR (high dynamic range).
} RLG_Skybox;

/**
 * @brief Opaque type for a lighting context handle.
 * 
 * This type represents a handle to a lighting context,
 * which is used to manage and update lighting-related data.
 */
typedef void* RLG_Context;

/**
 * @brief Type definition for a rendering function.
 * 
 * This function type is used for rendering purposes,
 * particularly for updating the shadow map in the RLG_UpdateShadowMap function.
 * 
 * @param shader The shader to use for rendering.
 */
typedef void (*RLG_DrawFunc)(Shader);


#if defined(__cplusplus)
extern "C" {
#endif

/**
 * @brief Create a new lighting context with the desired number of lights.
 * 
 * @param lightCount The number of lights to initialize within the context.
 * @return A new RLG_Context object representing the created lighting context.
 */
RLG_Context RLG_CreateContext(unsigned int lightCount);

/**
 * @brief Destroy a previously created lighting context and release associated resources.
 * 
 * @param ctx The lighting context to destroy.
 */
void RLG_DestroyContext(RLG_Context ctx);

/**
 * @brief Set the active lighting context for rlights.
 * 
 * @param ctx The lighting context to set as active.
 */
void RLG_SetContext(RLG_Context ctx);

/**
 * @brief Get the currently active lighting context for rlights.
 * 
 * @return The current RLG_Context object representing the active lighting context.
 */
RLG_Context RLG_GetContext(void);

/**
 * @brief Set custom shader code for a specific shader type.
 * 
 * @note This function should be called before RLG_Init to define your own shaders.
 * 
 * @param shader The type of shader to set the custom code for.
 * @param vsCode Vertex shader code for the specified shader type.
 * @param fsCode Fragment shader code for the specified shader type.
 */
void RLG_SetCustomShaderCode(RLG_Shader shader, const char *vsCode, const char *fsCode);

/**
 * @brief Get the current shader of the specified type.
 * 
 * @param shader The type of shader to retrieve.
 * 
 * @return A pointer to the current Shader object used for the specified shader type.
 *         Returns NULL if the shader type is not loaded.
 */
const Shader* RLG_GetShader(RLG_Shader shader);

/**
 * @brief Set the view position, corresponds to the position of your camera.
 * 
 * @param x The x-coordinate of the view position.
 * @param y The y-coordinate of the view position.
 * @param z The z-coordinate of the view position.
 */
void RLG_SetViewPosition(float x, float y, float z);

/**
 * @brief Set the view position using a Vector3 structure.
 * 
 * @param position The view position as a Vector3 structure.
 */
void RLG_SetViewPositionV(Vector3 position);

/**
 * @brief Get the current view position.
 * 
 * @return The current view position as a Vector3 structure.
 */
Vector3 RLG_GetViewPosition(void);

/**
 * @brief Set the ambient color.
 * 
 * @param color The ambient color as a Color structure.
 */
void RLG_SetAmbientColor(Color color);

/**
 * @brief Get the current ambient color.
 * 
 * @return The current ambient color as a Color structure.
 */
Color RLG_GetAmbientColor(void);

/**
 * @brief Set the minimum and maximum layers for parallax mapping.
 * 
 * @param min The minimum layer index.
 * @param max The maximum layer index.
 */
void RLG_SetParallaxLayers(int min, int max);

/**
 * @brief Get the current minimum and maximum layers for parallax mapping.
 * 
 * @param min Pointer to store the minimum layer index.
 * @param max Pointer to store the maximum layer index.
 */
void RLG_GetParallaxLayers(int* min, int* max);

/**
 * @brief Activate or deactivate texture sampling in the materials of models.
 * 
 * @param mapIndex The material map to modify.
 * @param active Boolean value indicating whether to activate (true) or deactivate (false) texture sampling.
 */
void RLG_UseMap(MaterialMapIndex mapIndex, bool active);

/**
 * @brief Check if texture sampling is enabled for a given material map.
 * 
 * @param mapIndex The material map to check.
 * @return True if texture sampling is enabled, false otherwise.
 */
bool RLG_IsMapUsed(MaterialMapIndex mapIndex);

/**
 * @brief Use the default material map if true, otherwise use the material maps defined in the models.
 * 
 * @param mapIndex The material map to modify.
 * @param active Boolean value indicating whether to use the default material map (true) or the material maps defined in the models (false).
 */
void RLG_UseDefaultMap(MaterialMapIndex mapIndex, bool active);

/**
 * @brief Set the default material map for a given material map index.
 * 
 * @param mapIndex The material map index to set.
 * @param map The material map to set as default.
 */
void RLG_SetDefaultMap(MaterialMapIndex mapIndex, MaterialMap map);

/**
 * @brief Get the default material map for a given material map index.
 * 
 * @param mapIndex The material map index to retrieve.
 * @return The default material map.
 */
MaterialMap RLG_GetDefaultMap(MaterialMapIndex mapIndex);

/**
 * @brief Check if the default material map is used for a given material map index.
 * 
 * @param mapIndex The material map index to check.
 * @return True if the default material map is used, false otherwise.
 */
bool RLG_IsDefaultMapUsed(MaterialMapIndex mapIndex);

/**
 * @brief Get the number of lights initialized sets in the shader.
 * 
 * @return The number of lights as an unsigned integer.
 */
unsigned int RLG_GetLightcount(void);

/**
 * @brief Activate or deactivate a specific light.
 * 
 * @param light The index of the light to modify.
 * @param active Boolean value indicating whether to activate (true) or deactivate (false) the light.
 */
void RLG_UseLight(unsigned int light, bool active);

/**
 * @brief Check if a specific light is enabled.
 * 
 * @param light The index of the light to check.
 * @return true if the light is enabled, false otherwise.
 */
bool RLG_IsLightUsed(unsigned int light);

/**
 * @brief Toggle the state of a specific light.
 * 
 * @param light The index of the light to toggle.
 */
void RLG_ToggleLight(unsigned int light);

/**
 * @brief Set the type of a specific light.
 * 
 * @param light The index of the light to set the type for.
 * @param type The type of light to set.
 */
void RLG_SetLightType(unsigned int light, RLG_LightType type);

/**
 * @brief Get the type of a specific light.
 * 
 * @param light The index of the light to get the type for.
 * @return The type of light as RLG_LightType enumeration.
 */
RLG_LightType RLG_GetLightType(unsigned int light);

/**
 * @brief Set a float value for a specific light property.
 * 
 * @param light The index of the light to modify.
 * @param property The light property to set the value for.
 * @param value The float value to assign to the light property.
 */
void RLG_SetLightValue(unsigned int light, RLG_LightProperty property, float value);

/**
 * @brief Set XYZ coordinates for a specific light property.
 * 
 * @param light The index of the light to modify.
 * @param property The light property to set the coordinates for.
 * @param x The X coordinate value.
 * @param y The Y coordinate value.
 * @param z The Z coordinate value.
 */
void RLG_SetLightXYZ(unsigned int light, RLG_LightProperty property, float x, float y, float z);

/**
 * @brief Set a Vector3 value for a specific light property.
 * 
 * @param light The index of the light to modify.
 * @param property The light property to set the Vector3 value for.
 * @param value The Vector3 value to assign to the light property.
 */
void RLG_SetLightVec3(unsigned int light, RLG_LightProperty property, Vector3 value);

/**
 * @brief Set a color value for a specific light.
 * 
 * @param light The index of the light to modify.
 * @param color The color to assign to the light.
 */
void RLG_SetLightColor(unsigned int light, Color color);

/**
 * @brief Get the float value of a specific light property.
 * 
 * @param light The index of the light to retrieve the value from.
 * @param property The light property to retrieve the value for.
 * @return The float value of the specified light property.
 */
float RLG_GetLightValue(unsigned int light, RLG_LightProperty property);

/**
 * @brief Get the Vector3 value of a specific light property.
 * 
 * @param light The index of the light to retrieve the value from.
 * @param property The light property to retrieve the Vector3 value for.
 * @return The Vector3 value of the specified light property.
 */
Vector3 RLG_GetLightVec3(unsigned int light, RLG_LightProperty property);

/**
 * @brief Get the color value of a specific light.
 * 
 * @param light The index of the light to retrieve the value from.
 * @return The color value of the specified light.
 */
Color RLG_GetLightColor(unsigned int light);

/**
 * @brief Translate the position of a specific light by the given offsets.
 * 
 * This function adjusts the position of the specified light by adding the
 * provided x, y, and z offsets to its current position.
 *
 * @param light The index of the light to translate.
 * @param x The offset to add to the x-coordinate of the light position.
 * @param y The offset to add to the y-coordinate of the light position.
 * @param z The offset to add to the z-coordinate of the light position.
 */
void RLG_LightTranslate(unsigned int light, float x, float y, float z);

/**
 * @brief Translate the position of a specific light by the given vector.
 * 
 * This function adjusts the position of the specified light by adding the
 * provided vector to its current position.
 *
 * @param light The index of the light to translate.
 * @param v The vector to add to the light position.
 */
void RLG_LightTranslateV(unsigned int light, Vector3 v);

/**
 * @brief Rotate the direction of a specific light around the X-axis.
 * 
 * This function rotates the direction vector of the specified light by the given
 * degrees around the X-axis.
 *
 * @param light The index of the light to rotate.
 * @param degrees The angle in degrees to rotate the light direction.
 */
void RLG_LightRotateX(unsigned int light, float degrees);

/**
 * @brief Rotate the direction of a specific light around the Y-axis.
 * 
 * This function rotates the direction vector of the specified light by the given
 * degrees around the Y-axis.
 *
 * @param light The index of the light to rotate.
 * @param degrees The angle in degrees to rotate the light direction.
 */
void RLG_LightRotateY(unsigned int light, float degrees);

/**
 * @brief Rotate the direction of a specific light around the Z-axis.
 * 
 * This function rotates the direction vector of the specified light by the given
 * degrees around the Z-axis.
 *
 * @param light The index of the light to rotate.
 * @param degrees The angle in degrees to rotate the light direction.
 */
void RLG_LightRotateZ(unsigned int light, float degrees);

/**
 * @brief Rotate the direction of a specific light around an arbitrary axis.
 * 
 * This function rotates the direction vector of the specified light by the given
 * degrees around the specified axis.
 *
 * @param light The index of the light to rotate.
 * @param axis The axis to rotate around.
 * @param degrees The angle in degrees to rotate the light direction.
 */
void RLG_LightRotate(unsigned int light, Vector3 axis, float degrees);

/**
 * @brief Set the target position of a specific light.
 * 
 * @param light The index of the light to set the target position for.
 * @param x The x-coordinate of the target position.
 * @param y The y-coordinate of the target position.
 * @param z The z-coordinate of the target position.
 */
void RLG_SetLightTarget(unsigned int light, float x, float y, float z);

/**
 * @brief Set the target position of a specific light using a Vector3 structure.
 * 
 * @param light The index of the light to set the target position for.
 * @param targetPosition The target position of the light as a Vector3 structure.
 */
void RLG_SetLightTargetV(unsigned int light, Vector3 targetPosition);

/**
 * @brief Get the target position of a specific light.
 * 
 * @param light The index of the light to get the target position for.
 * @return The target position of the light as a Vector3 structure.
 */
Vector3 RLG_GetLightTarget(unsigned int light);

/**
 * @brief Enable shadow casting for a light.
 *
 * @warning Shadow casting is not fully functional for omnilights yet. Please specify the light direction.
 * 
 * @param light The index of the light to enable shadow casting for.
 * @param shadowMapResolution The resolution of the shadow map.
 * 
 * @todo Implement shadow casting feature for omnilights using cubemaps.
 *       Previous attempts have been made, but it might be optimal to
 *       directly call OpenGL functions bypassing RLGL, though this
 *       approach could pose issues for some users...
 */
void RLG_EnableShadow(unsigned int light, int shadowMapResolution);

/**
 * @brief Disable shadow casting for a light.
 * 
 * @param light The index of the light to disable shadow casting for.
 */
void RLG_DisableShadow(unsigned int light);

/**
 * @brief Check if shadow casting is enabled for a light.
 * 
 * @param light The index of the light to check for shadow casting.
 * @return true if shadow casting is enabled, false otherwise.
 */
bool RLG_IsShadowEnabled(unsigned int light);

/**
 * @brief Set the bias value for shadow mapping of a light.
 * 
 * @param light The index of the light to set the shadow bias for.
 * @param value The bias value to set.
 */
void RLG_SetShadowBias(unsigned int light, float value);

/**
 * @brief Get the bias value for shadow mapping of a light.
 * 
 * @param light The index of the light to get the shadow bias for.
 * @return The shadow bias value.
 */
float RLG_GetShadowBias(unsigned int light);

/**
 * @brief Updates the shadow map for a given light source.
 * 
 * This function updates the shadow map for the specified light by calling the provided draw function.
 * It sets the active shadow map for the light and uses the draw function to render the scene.
 * 
 * @param light The identifier of the light source for which to update the shadow map.
 * @param drawFunc The function to draw the scene for shadow rendering.
 */
void RLG_UpdateShadowMap(unsigned int light, RLG_DrawFunc drawFunc);

/**
 * @brief Retrieves the shadow map texture for a given light source.
 * 
 * @param light The identifier of the light source for which to retrieve the shadow map.
 * @return The shadow map texture for the specified light source.
 */
Texture RLG_GetShadowMap(unsigned int light);

/**
 * @brief Casts a mesh for shadow rendering.
 * 
 * @param shader The shader to use for rendering the mesh.
 * @param mesh The mesh to cast.
 * @param transform The transformation matrix to apply to the mesh.
 */
void RLG_CastMesh(Shader shader, Mesh mesh, Matrix transform);

/**
 * @brief Casts a model for shadow rendering.
 * 
 * @param shader The shader to use for rendering the model.
 * @param model The model to cast.
 * @param position The position at which to cast the model for shadow rendering.
 * @param scale The scale at which to cast the model for shadow rendering.
 */
void RLG_CastModel(Shader shader, Model model, Vector3 position, float scale);

/**
 * @brief Casts a model for shadow rendering with extended parameters.
 * 
 * @param shader The shader to use for rendering the model.
 * @param model The model to cast.
 * @param position The position at which to cast the model for shadow rendering.
 * @param rotationAxis The axis around which to rotate the model for shadow rendering.
 * @param rotationAngle The angle by which to rotate the model for shadow rendering.
 * @param scale The scale at which to cast the model for shadow rendering.
 */
void RLG_CastModelEx(Shader shader, Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale);

/**
 * @brief Draw a mesh with a specified material and transformation.
 * 
 * This function draws a mesh with the specified material and transformation.
 * 
 * @param mesh The mesh to draw.
 * @param material The material to apply to the mesh.
 * @param transform The transformation matrix to apply to the mesh.
 */
void RLG_DrawMesh(Mesh mesh, Material material, Matrix transform);

/**
 * @brief Draw a model at a specified position with a specified scale and tint.
 * 
 * This function draws a model at the specified position with the specified scale and tint.
 * 
 * @param model The model to draw.
 * @param position The position at which to draw the model.
 * @param scale The scale at which to draw the model.
 * @param tint The tint color to apply to the model.
 */
void RLG_DrawModel(Model model, Vector3 position, float scale, Color tint);

/**
 * @brief Draw a model at a specified position with a specified rotation, scale, and tint.
 * 
 * This function draws a model at the specified position with the specified rotation, scale, and tint.
 * 
 * @param model The model to draw.
 * @param position The position at which to draw the model.
 * @param rotationAxis The axis around which to rotate the model.
 * @param rotationAngle The angle by which to rotate the model.
 * @param scale The scale at which to draw the model.
 * @param tint The tint color to apply to the model.
 */
void RLG_DrawModelEx(Model model, Vector3 position, Vector3 rotationAxis, float rotationAngle, Vector3 scale, Color tint);

/**
 * @brief Loads a skybox from a file.
 *
 * This function loads a skybox texture from the specified file and returns
 * a RLG_Skybox structure containing the cubemap texture.
 *
 * @param skyboxFileName The path to the skybox texture file.
 * @return RLG_Skybox The loaded skybox.
 */
RLG_Skybox RLG_LoadSkybox(const char* skyboxFileName);

/**
 * @brief Loads a HDR skybox from a file with specified size and format.
 *
 * This function loads a high dynamic range (HDR) skybox texture from the
 * specified file. It creates a cubemap texture with the given size and format.
 *
 * @note On some Android devices with WebGL, framebuffer objects (FBO)
 * do not properly support a FLOAT-based attachment, so the function uses
 * PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 instead of PIXELFORMAT_UNCOMPRESSED_R32G32B32A32.
 *
 * @param skyboxFileName The path to the skybox texture file.
 * @param size The size of the cubemap texture.
 * @param format The pixel format of the cubemap texture.
 * @return RLG_Skybox The loaded HDR skybox.
 */
RLG_Skybox RLG_LoadSkyboxHDR(const char* skyboxFileName, int size, int format);

/**
 * @brief Unloads a skybox.
 *
 * This function unloads the specified skybox, freeing the associated
 * resources.
 *
 * @param skybox The skybox to be unloaded.
 */
void RLG_UnloadSkybox(RLG_Skybox skybox);

/**
 * @brief Draws a skybox.
 *
 * This function renders the specified skybox.
 *
 * @param skybox The skybox to be drawn.
 */
void RLG_DrawSkybox(RLG_Skybox skybox);


#if defined(__cplusplus)
}
#endif

#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include "rlgl.h"

/* Helper macros */

#define STRINGIFY(x) #x             ///< NOTE: Undefined at the end of the header
#define TOSTRING(x) STRINGIFY(x)    ///< NOTE: Undefined at the end of the header

// General macro to initialize a structure with specific values
// NOTE: Undefine at the end of the header
// In C++, use the brace initializer list syntax: type{...}
// In C, use the designated initializer syntax: (type){...}
#ifdef __cplusplus
    #define INIT_STRUCT(type, ...) { __VA_ARGS__ }
#else
    #define INIT_STRUCT(type, ...) (type) { __VA_ARGS__ }
#endif

// Macro to initialize a structure with zeros
// NOTE: Undefine at the end of the header
// In C++, use the brace initializer list syntax: type{}
// In C, use the designated initializer syntax: (type){0}
#ifdef __cplusplus
    #define INIT_STRUCT_ZERO(type) {}
#else
    #define INIT_STRUCT_ZERO(type) (type) { 0 }
#endif

/* Helper defintions */

#define RLG_COUNT_MATERIAL_MAPS 12  ///< Same as MAX_MATERIAL_MAPS defined in raylib/config.h
#define RLG_COUNT_SHADERS 6         ///< Total shader used by rlights.h internally

/* Uniform names definitions */

#define RLG_SHADER_LIGHTING_ATTRIB_POSITION             "vertexPosition"
#define RLG_SHADER_LIGHTING_ATTRIB_TEXCOORD             "vertexTexCoord"
#define RLG_SHADER_LIGHTING_ATTRIB_TEXCOORD2            "vertexTexCoord2"
#define RLG_SHADER_LIGHTING_ATTRIB_NORMAL               "vertexNormal"
#define RLG_SHADER_LIGHTING_ATTRIB_TANGENT              "vertexTangent"
#define RLG_SHADER_LIGHTING_ATTRIB_COLOR                "vertexColor"

#define RLG_SHADER_LIGHTING_UNIFORM_MATRIX_MVP          "mvp"
#define RLG_SHADER_LIGHTING_UNIFORM_MATRIX_VIEW         "matView"
#define RLG_SHADER_LIGHTING_UNIFORM_MATRIX_PROJECTION   "matProjection"
#define RLG_SHADER_LIGHTING_UNIFORM_MATRIX_MODEL        "matModel"
#define RLG_SHADER_LIGHTING_UNIFORM_MATRIX_NORMAL       "matNormal"

#define RLG_SHADER_LIGHTING_UNIFORM_COLOR_AMBIENT       "colAmbient"
#define RLG_SHADER_LIGHTING_UNIFORM_VIEW_POSITION       "viewPos"

/* Embedded shaders definition */

#ifndef NO_EMBEDDED_SHADERS

#define GLSL_VERSION_DEF \
    "#version " TOSTRING(GLSL_VERSION) "\n"

#if GLSL_VERSION < 330

#   define GLSL_TEXTURE_DEF         "#define TEX texture2D\n"
#   define GLSL_TEXTURE_CUBE_DEF    "#define TEXCUBE textureCube\n"

#   define GLSL_FS_OUT_DEF          ""

#   define GLSL_FINAL_COLOR(x)      "gl_FragColor = " x ";"
#   define GLSL_PRECISION(x)        "precision " x ";"

#   define GLSL_VS_IN(x)            "attribute " x ";"
#   define GLSL_FS_IN(x)            "varying " x ";"
#   define GLSL_VS_OUT(x)           "varying " x ";"

#   define GLSL_FS_FLAT_IN(x)       "varying " x ";"
#   define GLSL_VS_FLAT_OUT(x)      "varying " x ";"

#else

#   define GLSL_TEXTURE_DEF         "#define TEX texture\n"
#   define GLSL_TEXTURE_CUBE_DEF    "#define TEXCUBE texture\n"

#   define GLSL_FS_OUT_DEF          "out vec4 _;"

#   define GLSL_FINAL_COLOR(x)      "_ = " x ";"
#   define GLSL_PRECISION(x)        ""

#   define GLSL_VS_IN(x)            "in " x ";"
#   define GLSL_FS_IN(x)            "in " x ";"
#   define GLSL_VS_OUT(x)           "out " x ";"

#   define GLSL_FS_FLAT_IN(x)       "flat in " x ";"
#   define GLSL_VS_FLAT_OUT(x)      "flat out " x ";"

#endif

/* Shader */

static const char rlgLightingVS[] = GLSL_VERSION_DEF

#   if GLSL_VERSION > 100
    "#define NUM_LIGHTS %i\n"
    "uniform mat4 matLights[NUM_LIGHTS];"
    GLSL_VS_OUT("vec4 fragPosLightSpace[NUM_LIGHTS]")
#   endif

    GLSL_VS_IN("vec3 " RLG_SHADER_LIGHTING_ATTRIB_POSITION)
    GLSL_VS_IN("vec2 " RLG_SHADER_LIGHTING_ATTRIB_TEXCOORD)
    GLSL_VS_IN("vec4 " RLG_SHADER_LIGHTING_ATTRIB_TANGENT)
    GLSL_VS_IN("vec3 " RLG_SHADER_LIGHTING_ATTRIB_NORMAL)
    GLSL_VS_IN("vec4 " RLG_SHADER_LIGHTING_ATTRIB_COLOR)

    "uniform lowp int useNormalMap;"
    "uniform mat4 " RLG_SHADER_LIGHTING_UNIFORM_MATRIX_NORMAL ";"
    "uniform mat4 " RLG_SHADER_LIGHTING_UNIFORM_MATRIX_MODEL ";"
    "uniform mat4 " RLG_SHADER_LIGHTING_UNIFORM_MATRIX_MVP ";"

    GLSL_VS_OUT("vec3 fragPosition")
    GLSL_VS_OUT("vec2 fragTexCoord")
    GLSL_VS_OUT("vec3 fragNormal")
    GLSL_VS_OUT("vec4 fragColor")
    GLSL_VS_FLAT_OUT("mat3 TBN")

    "void main()"
    "{"
        "fragPosition = vec3(" RLG_SHADER_LIGHTING_UNIFORM_MATRIX_MODEL "*vec4(" RLG_SHADER_LIGHTING_ATTRIB_POSITION ", 1.0));"
        "fragNormal = (" RLG_SHADER_LIGHTING_UNIFORM_MATRIX_NORMAL "*vec4(" RLG_SHADER_LIGHTING_ATTRIB_NORMAL ", 0.0)).xyz;"

        "fragTexCoord = " RLG_SHADER_LIGHTING_ATTRIB_TEXCOORD ";"
        "fragColor = " RLG_SHADER_LIGHTING_ATTRIB_COLOR ";"

        // The TBN matrix is used to transform vectors from tangent space to world space
        // It is currently used to transform normals from a normal map to world space normals
        "vec3 T = normalize(vec3(" RLG_SHADER_LIGHTING_UNIFORM_MATRIX_MODEL "*vec4(" RLG_SHADER_LIGHTING_ATTRIB_TANGENT ".xyz, 0.0)));"
        "vec3 B = cross(fragNormal, T)*" RLG_SHADER_LIGHTING_ATTRIB_TANGENT ".w;"
        "TBN = mat3(T, B, fragNormal);"

#       if GLSL_VERSION > 100
        "for (int i = 0; i < NUM_LIGHTS; i++)"
        "{"
            "fragPosLightSpace[i] = matLights[i]*vec4(fragPosition, 1.0);"
        "}"
#       endif

        "gl_Position = " RLG_SHADER_LIGHTING_UNIFORM_MATRIX_MVP "*vec4(" RLG_SHADER_LIGHTING_ATTRIB_POSITION ", 1.0);"
    "}";

static const char rlgLightingFS[] = GLSL_VERSION_DEF
    GLSL_TEXTURE_DEF GLSL_TEXTURE_CUBE_DEF

    "#define NUM_LIGHTS"                " %i\n"
    "#define NUM_MATERIAL_MAPS"         " 7\n"
    "#define NUM_MATERIAL_CUBEMAPS"     " 2\n"

    "#define DIRLIGHT"                  " 0\n"
    "#define OMNILIGHT"                 " 1\n"
    "#define SPOTLIGHT"                 " 2\n"

    "#define ALBEDO"                    " 0\n"
    "#define METALNESS"                 " 1\n"
    "#define NORMAL"                    " 2\n"
    "#define ROUGHNESS"                 " 3\n"
    "#define OCCLUSION"                 " 4\n"
    "#define EMISSION"                  " 5\n"
    "#define HEIGHT"                    " 6\n"

    "#define CUBEMAP"                   " 0\n"
    "#define IRRADIANCE"                " 1\n"

    "#define PI 3.1415926535897932384626433832795028\n"

    GLSL_PRECISION("mediump float")

#   if GLSL_VERSION > 100
    GLSL_FS_IN("vec4 fragPosLightSpace[NUM_LIGHTS]")
#   else
    "uniform mat4 matLights[NUM_LIGHTS];"
#   endif

    GLSL_FS_IN("vec3 fragPosition")
    GLSL_FS_IN("vec2 fragTexCoord")
    GLSL_FS_IN("vec3 fragNormal")
    GLSL_FS_IN("vec4 fragColor")
    GLSL_FS_FLAT_IN("mat3 TBN")

    GLSL_FS_OUT_DEF

    "struct MaterialMap {"
        "sampler2D texture;"
        "mediump vec4 color;"
        "mediump float value;"
        "lowp int active;"
    "};"

    "struct MaterialCubemap {"
        "samplerCube texture;"
        "mediump vec4 color;"
        "mediump float value;"
        "lowp int active;"
    "};"

    "struct Light {"
        "samplerCube shadowCubemap;"    ///< Sampler for the shadow map texture
        "sampler2D shadowMap;"          ///< Sampler for the shadow map texture
        "vec3 position;"                ///< Position of the light in world coordinates
        "vec3 direction;"               ///< Direction vector of the light (for directional and spotlights)
        "vec3 color;"                   ///< Diffuse color of the light
        "float energy;"                 ///< Energy factor of the diffuse light color
        "float specular;"               ///< Specular amount of the light
        "float size;"                   ///< Light size (spotlight, omnilight only)
        "float innerCutOff;"            ///< Inner cutoff angle for spotlights (cosine of the angle)
        "float outerCutOff;"            ///< Outer cutoff angle for spotlights (cosine of the angle)
        "float constant;"               ///< Constant attenuation factor
        "float linear;"                 ///< Linear attenuation factor
        "float quadratic;"              ///< Quadratic attenuation factor
        "float shadowMapTxlSz;"         ///< Texel size of the shadow map
        "float depthBias;"              ///< Bias value to avoid self-shadowing artifacts
        "lowp int type;"                ///< Type of the light (e.g., point, directional, spotlight)
        "lowp int shadow;"              ///< Indicates if the light casts shadows (1 for true, 0 for false)
        "lowp int enabled;"             ///< Indicates if the light is active (1 for true, 0 for false)
    "};"

    "uniform MaterialCubemap cubemaps[NUM_MATERIAL_CUBEMAPS];"
    "uniform MaterialMap maps[NUM_MATERIAL_MAPS];"
    "uniform Light lights[NUM_LIGHTS];"

    "uniform lowp int parallaxMinLayers;"
    "uniform lowp int parallaxMaxLayers;"

    "uniform float farPlane;"   ///< Used to scale depth values ​​when reading the depth cubemap (point shadows)

    "uniform vec3 " RLG_SHADER_LIGHTING_UNIFORM_COLOR_AMBIENT ";"
    "uniform vec3 " RLG_SHADER_LIGHTING_UNIFORM_VIEW_POSITION ";"

    "float DistributionGGX(float cosTheta, float alpha)"
    "{"
        "float a = cosTheta*alpha;"
        "float k = alpha/(1.0 - cosTheta*cosTheta + a*a);"
        "return k*k*(1.0/PI);"
    "}"

    // From Earl Hammon, Jr. "PBR Diffuse Lighting for GGX+Smith Microsurfaces"
    // SEE: https://www.gdcvault.com/play/1024478/PBR-Diffuse-Lighting-for-GGX
    "float GeometrySmith(float NdotL, float NdotV, float alpha)"
    "{"
        "return 0.5/mix(2.0*NdotL*NdotV, NdotL + NdotV, alpha);"
    "}"

    "float SchlickFresnel(float u)"
    "{"
        "float m = 1.0 - u;"
        "float m2 = m*m;"
        "return m2*m2*m;" // pow(m,5)
    "}"

    "vec3 ComputeF0(float metallic, float specular, vec3 albedo)"
    "{"
        "float dielectric = 0.16*specular*specular;"
        // use albedo*metallic as colored specular reflectance at 0 angle for metallic materials
        // SEE: https://google.github.io/filament/Filament.md.html
        "return mix(vec3(dielectric), albedo, vec3(metallic));"
    "}"

    "vec2 Parallax(vec2 uv, vec3 V)"
    "{"
        "float height = 1.0 - TEX(maps[HEIGHT].texture, uv).r;"
        "return uv - vec2(V.xy/V.z)*height*maps[HEIGHT].value;"
    "}"

    "vec2 DeepParallax(vec2 uv, vec3 V)"
    "{"
        "float numLayers = mix("
            "float(parallaxMaxLayers),"
            "float(parallaxMinLayers),"
            "abs(dot(vec3(0.0, 0.0, 1.0), V)));"

        "float layerDepth = 1.0/numLayers;"
        "float currentLayerDepth = 0.0;"

        "vec2 P = V.xy/V.z*maps[HEIGHT].value;"
        "vec2 deltaTexCoord = P/numLayers;"
    
        "vec2 currentUV = uv;"
        "float currentDepthMapValue = 1.0 - TEX(maps[HEIGHT].texture, currentUV).y;"
        
        "while(currentLayerDepth < currentDepthMapValue)"
        "{"
            "currentUV += deltaTexCoord;"
            "currentLayerDepth += layerDepth;"
            "currentDepthMapValue = 1.0 - TEX(maps[HEIGHT].texture, currentUV).y;"
        "}"

        "vec2 prevTexCoord = currentUV - deltaTexCoord;"
        "float afterDepth  = currentDepthMapValue + currentLayerDepth;"
        "float beforeDepth = 1.0 - TEX(maps[HEIGHT].texture,"
            "prevTexCoord).y - currentLayerDepth - layerDepth;"

        "float weight = afterDepth/(afterDepth - beforeDepth);"
        "return prevTexCoord*weight + currentUV*(1.0 - weight);"
    "}"

    "float ShadowOmni(int i, float cNdotL)"
    "{"
        "vec3 fragToLight = fragPosition - lights[i].position;"
        "float closestDepth = TEXCUBE(lights[i].shadowCubemap, fragToLight).r;"
        "closestDepth *= farPlane;" // Rescale depth
        "float currentDepth = length(fragToLight);"
        "float bias = lights[i].depthBias*max(1.0 - cNdotL, 0.05);"
        "return currentDepth - bias > closestDepth ? 0.0 : 1.0;"
    "}"

    "float Shadow(int i, float cNdotL)"
    "{"
#       if GLSL_VERSION > 100
        "vec4 p = fragPosLightSpace[i];"
#       else
        "vec4 p = matLights[i]*vec4(fragPosition, 1.0);"
#       endif

        "vec3 projCoords = p.xyz/p.w;"
        "projCoords = projCoords*0.5 + 0.5;"

        "float bias = max(lights[i].depthBias*(1.0 - cNdotL), 0.00002) + 0.00001;"
        "projCoords.z -= bias;"

        "if (projCoords.z > 1.0 || projCoords.x > 1.0 || projCoords.y > 1.0)"
        "{"
            "return 1.0;"
        "}"

        "float depth = projCoords.z;"
        "float shadow = 0.0;"

        // NOTE: You can increase iterations to improve PCF quality
        "for (int x = -1; x <= 1; x++)"
        "{"
            "for (int y = -1; y <= 1; y++)"
            "{"
                "float pcfDepth = TEX(lights[i].shadowMap, projCoords.xy + vec2(x, y)*lights[i].shadowMapTxlSz).r;"
                "shadow += step(depth, pcfDepth);"
            "}"
        "}"

        "return shadow/9.0;"
    "}"

    "void main()"
    "{"
        // Compute the view direction vector for this fragment
        "vec3 V = normalize(" RLG_SHADER_LIGHTING_UNIFORM_VIEW_POSITION " - fragPosition);"

        // Compute fragTexCoord (UV), apply parallax if height map is enabled
        "vec2 uv = fragTexCoord;"
        "if (maps[HEIGHT].active != 0)"
        "{"
            "uv = (parallaxMinLayers > 0 && parallaxMaxLayers > 1)"
                "? DeepParallax(uv, V) : Parallax(uv, V);"

            "if (uv.x < 0.0 || uv.y < 0.0 || uv.x > 1.0 || uv.y > 1.0)"
            "{"
                "discard;"
            "}"
        "}"

        // Compute albedo (base color) by sampling the texture and multiplying by the diffuse color
        "vec3 albedo = maps[ALBEDO].color.rgb*fragColor.rgb;"
        "if (maps[ALBEDO].active != 0)"
            "albedo *= TEX(maps[ALBEDO].texture, uv).rgb;"

        // Compute metallic factor; if a metalness map is used, sample it
        "float metalness = maps[METALNESS].value;"
        "if (maps[METALNESS].active != 0)"
            "metalness *= TEX(maps[METALNESS].texture, uv).b;"

        // Compute roughness factor; if a roughness map is used, sample it
        "float roughness = maps[ROUGHNESS].value;"
        "if (maps[ROUGHNESS].active != 0)"
            "roughness *= TEX(maps[ROUGHNESS].texture, uv).g;"

        // Compute F0 (reflectance at normal incidence) based on the metallic factor
        "vec3 F0 = ComputeF0(metalness, 0.5, albedo);"

        // Compute the normal vector; if a normal map is used, transform it to tangent space
        "vec3 N = (maps[NORMAL].active == 0) ? normalize(fragNormal)"
            ": normalize(TBN*(TEX(maps[NORMAL].texture, uv).rgb*2.0 - 1.0));"

        // Compute the dot product of the normal and view direction
        "float NdotV = dot(N, V);"
        "float cNdotV = max(NdotV, 1e-4);"  // Clamped to avoid division by zero

        // Initialize diffuse and specular lighting accumulators
        "vec3 diffLighting = vec3(0.0);"
        "vec3 specLighting = vec3(0.0);"

        // Loop through all lights
        "for (int i = 0; i < NUM_LIGHTS; i++)"
        "{"
            "if (lights[i].enabled != 0)"
            "{"
                "float size_A = 0.0;"
                "vec3 L = vec3(0.0);"

                // Compute the light direction vector
                "if (lights[i].type != DIRLIGHT)"
                "{"
                    "vec3 LV = lights[i].position - fragPosition;"
                    "L = normalize(LV);"

                    // If the light has a size, compute the attenuation factor based on the distance
                    "if (lights[i].size > 0.0)"
                    "{"
                        "float t = lights[i].size/max(0.001, length(LV));"
                        "size_A = max(0.0, 1.0 - 1.0/sqrt(1.0 + t*t));"
                    "}"
                "}"
                "else"
                "{"
                    // For directional lights, use the negative direction as the light direction
                    "L = normalize(-lights[i].direction);"
                "}"

                // Compute the dot product of the normal and light direction, adjusted by size_A
                "float NdotL = min(size_A + dot(N, L), 1.0);"
                "float cNdotL = max(NdotL, 0.0);" // clamped NdotL

                // Compute the halfway vector between the view and light directions
                "vec3 H = normalize(V + L);"
                "float cNdotH = clamp(size_A + dot(N, H), 0.0, 1.0);"
                "float cLdotH = clamp(size_A + dot(L, H), 0.0, 1.0);"

                // Compute light color energy
                "vec3 lightColE = lights[i].color*lights[i].energy;"

                // Compute diffuse lighting (Burley model) if the material is not fully metallic
                "vec3 diffLight = vec3(0.0);"
                "if (metalness < 1.0)"
                "{"
                    "float FD90_minus_1 = 2.0*cLdotH*cLdotH*roughness - 0.5;"
                    "float FdV = 1.0 + FD90_minus_1*SchlickFresnel(cNdotV);"
                    "float FdL = 1.0 + FD90_minus_1*SchlickFresnel(cNdotL);"

                    "float diffBRDF = (1.0/PI)*FdV*FdL*cNdotL;"
                    "diffLight = diffBRDF*lightColE;"
                "}"

                // Compute specular lighting using the Schlick-GGX model
                // NOTE: When roughness is 0, specular light should not be entirely disabled.
                // TODO: Handle perfect mirror reflection when roughness is 0.
                "vec3 specLight = vec3(0.0);"
                "if (roughness > 0.0)"
                "{"
                    "float alphaGGX = roughness*roughness;"
                    "float D = DistributionGGX(cNdotH, alphaGGX);"
                    "float G = GeometrySmith(cNdotL, cNdotV, alphaGGX);"

                    "float cLdotH5 = SchlickFresnel(cLdotH);"
                    "float F90 = clamp(50.0*F0.g, 0.0, 1.0);"
                    "vec3 F = F0 + (F90 - F0)*cLdotH5;"

                    "vec3 specBRDF = cNdotL*D*F*G;"
                    "specLight = specBRDF*lightColE*lights[i].specular;"
                "}"

                // Apply spotlight effect if the light is a spotlight
                "float intensity = 1.0;"
                "if (lights[i].type == SPOTLIGHT)"
                "{"
                    "float theta = dot(L, normalize(-lights[i].direction));"
                    "float epsilon = (lights[i].innerCutOff - lights[i].outerCutOff);"
                    "intensity = smoothstep(0.0, 1.0, (theta - lights[i].outerCutOff)/epsilon);"
                "}"

                // Apply attenuation based on the distance from the light
                "float distance    = length(lights[i].position - fragPosition);"
                "float attenuation = 1.0/(lights[i].constant +"
                                         "lights[i].linear*distance +"
                                         "lights[i].quadratic*(distance*distance));"

                // Apply shadow factor if the light casts shadows
                "float shadow = 1.0;"
                "if (lights[i].shadow != 0)"
                "{"
                    "shadow = (lights[i].type == OMNILIGHT)"
                        "? ShadowOmni(i, cNdotL) : Shadow(i, cNdotL);"
                "}"

                // Compute the final intensity factor combining intensity, attenuation, and shadow
                "float factor = intensity*attenuation*shadow;"

                // Accumulate the diffuse and specular lighting contributions
                "diffLighting += diffLight*factor;"
                "specLighting += specLight*factor;"
            "}"
        "}"

        // Compute ambient
        "vec3 ambient = " RLG_SHADER_LIGHTING_UNIFORM_COLOR_AMBIENT ";"
        "if (cubemaps[IRRADIANCE].active != 0)"
        "{"
            "vec3 kS = F0 + (1.0 - F0)*SchlickFresnel(cNdotV);"
            "vec3 kD = (1.0 - kS)*(1.0 - metalness);"
            "ambient = kD*TEXCUBE(cubemaps[IRRADIANCE].texture, N).rgb;"
        "}"

        // Compute ambient occlusion
        "if (maps[OCCLUSION].active != 0)"
        "{"
            "float ao = TEX(maps[OCCLUSION].texture, uv).r;"
            "ambient *= ao;"

            "float lightAffect = mix(1.0, ao, maps[OCCLUSION].value);"
            "diffLighting *= lightAffect;"
            "specLighting *= lightAffect;"
        "}"

        // Skybox reflection
        "if (cubemaps[CUBEMAP].active != 0)"
        "{"
            "vec3 reflectCol = TEXCUBE(cubemaps[CUBEMAP].texture, reflect(-V, N)).rgb;"
            "specLighting = mix(specLighting, reflectCol, 1.0 - roughness);"
        "}"

        // Compute the final diffuse color, including ambient and diffuse lighting contributions
        "vec3 diffuse = albedo*(ambient + diffLighting);"

        // Compute emission color; if an emissive map is used, sample it
        "vec3 emission = maps[EMISSION].color.rgb;"
        "if (maps[EMISSION].active != 0)"
        "{"
            "emission *= TEX(maps[EMISSION].texture, uv).rgb;"
        "}"

        // Compute the final fragment color by combining diffuse, specular, and emission contributions
        GLSL_FINAL_COLOR("vec4(diffuse + specLighting + emission, 1.0)")
    "}";

static const char rlgDepthVS[] = GLSL_VERSION_DEF
    GLSL_VS_IN("vec3 vertexPosition")
    "uniform mat4 mvp;"
    "void main()"
    "{"
        "gl_Position = mvp*vec4(vertexPosition, 1.0);"
    "}";

static const char rlgDepthFS[] = GLSL_VERSION_DEF
    GLSL_PRECISION("mediump float")
    "void main()"
    "{}";

static const char rlgDepthCubemapVS[] = GLSL_VERSION_DEF
    GLSL_VS_IN("vec3 vertexPosition")
    GLSL_VS_OUT("vec3 fragPosition")
    "uniform mat4 matModel;"
    "uniform mat4 mvp;"
    "void main()"
    "{"
        "fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));"
        "gl_Position = mvp*vec4(vertexPosition, 1.0);"
    "}";

static const char rlgDepthCubemapFS[] = GLSL_VERSION_DEF
    GLSL_PRECISION("mediump float")
    GLSL_FS_IN("vec3 fragPosition")
    "uniform vec3 lightPos;"
    "uniform float farPlane;"
    "void main()"
    "{"
        "float lightDistance = length(fragPosition - lightPos);"
        "lightDistance = lightDistance/farPlane;"
        "gl_FragDepth = lightDistance;"
    "}";

static const char rlgShadowMapFS[] = GLSL_VERSION_DEF
    GLSL_PRECISION("mediump float")
    GLSL_FS_IN("vec2 fragTexCoord")
    "uniform sampler2D texture0;"
    "uniform float near;"
    "uniform float far;"
    GLSL_FS_OUT_DEF
    "void main()"
    "{"
        "float depth = TEX(texture0, vec2(fragTexCoord.x, 1.0 - fragTexCoord.y)).r;"
        "depth = (2.0*near*far)/(far + near - (depth*2.0 - 1.0)*(far - near));"
        GLSL_FINAL_COLOR("vec4(vec3(depth/far), 1.0)")
    "}";

static const char rlgCubemapVS[] = GLSL_VERSION_DEF
    GLSL_VS_IN("vec3 vertexPosition")
    GLSL_VS_OUT("vec3 fragPosition")

    "uniform mat4 matProjection;"
    "uniform mat4 matView;"

    "void main()"
    "{"
        "fragPosition = vertexPosition;"
        "gl_Position = matProjection*matView*vec4(vertexPosition, 1.0);"
    "}";

static const char rlgEquirectangularToCubemapFS[] = GLSL_VERSION_DEF
    GLSL_TEXTURE_DEF

    GLSL_PRECISION("mediump float")
    GLSL_FS_IN("vec3 fragPosition")
    GLSL_FS_OUT_DEF

    "uniform sampler2D equirectangularMap;"

    "vec2 SampleSphericalMap(vec3 v)"
    "{"
        "vec2 uv = vec2(atan(v.z, v.x), asin(v.y));"
        "uv *= vec2(0.1591, -0.3183);" // negative Y, to flip axis
        "uv += 0.5;"
        "return uv;"
    "}"

    "void main()"
    "{"
        "vec2 uv = SampleSphericalMap(normalize(fragPosition));"
        "vec3 color = TEX(equirectangularMap, uv).rgb;"
        GLSL_FINAL_COLOR("vec4(color, 1.0)")
    "}";

static const char rlgIrradianceConvolutionFS[] = GLSL_VERSION_DEF
    GLSL_TEXTURE_CUBE_DEF

    "#define PI 3.14159265359\n"

    GLSL_PRECISION("mediump float")
    GLSL_FS_IN("vec3 fragPosition")
    GLSL_FS_OUT_DEF

    "uniform samplerCube environmentMap;"

    "void main()"
    "{"
        // The world vector acts as the normal of a tangent surface
        // from the origin, aligned to WorldPos. Given this normal, calculate all
        // incoming radiance of the environment. The result of this radiance
        // is the radiance of light coming from -Normal direction, which is what
        // we use in the PBR shader to sample irradiance.
        "vec3 N = normalize(fragPosition);"

        "vec3 irradiance = vec3(0.0);"
        
        // tangent space calculation from origin point
        "vec3 up = vec3(0.0, 1.0, 0.0);"
        "vec3 right = normalize(cross(up, N));"
        "up = normalize(cross(N, right));"

        "float sampleDelta = 0.025;"
        "float nrSamples = 0.0;"

        "for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta)"
        "{"
            "for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta)"
            "{"
                // spherical to cartesian (in tangent space)
                "vec3 tangentSample = vec3(sin(theta) * cos(phi),  sin(theta) * sin(phi), cos(theta));"

                // tangent space to world
                "vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N; "

                "irradiance += TEXCUBE(environmentMap, sampleVec).rgb * cos(theta) * sin(theta);"
                "nrSamples++;"
            "}"
        "}"

        "irradiance = PI * irradiance * (1.0 / float(nrSamples));"
        GLSL_FINAL_COLOR("vec4(irradiance, 1.0)")
    "}";

static const char rlgSkyboxVS[] = GLSL_VERSION_DEF
    GLSL_VS_IN("vec3 vertexPosition")
    GLSL_VS_OUT("vec3 fragPosition")

    "uniform mat4 matProjection;"
    "uniform mat4 matView;"

    "void main()"
    "{"
        "fragPosition = vertexPosition;"
        "mat4 rotView = mat4(mat3(matView));"
        "vec4 clipPos = matProjection*rotView*vec4(vertexPosition, 1.0);"
        "gl_Position = clipPos;"
    "}";

static const char rlgSkyboxFS[] = GLSL_VERSION_DEF
    GLSL_TEXTURE_CUBE_DEF

    GLSL_PRECISION("mediump float")
    GLSL_FS_IN("vec3 fragPosition")
    GLSL_FS_OUT_DEF

    "uniform samplerCube environmentMap;"
    "uniform bool doGamma;"

    "void main()"
    "{"
        "vec3 color = TEXCUBE(environmentMap, fragPosition).rgb;"

        "if (doGamma)" // Apply gamma correction
        "{"
            "color = color/(color + vec3(1.0));"
            "color = pow(color, vec3(1.0/2.2));"
        "}"

        GLSL_FINAL_COLOR("vec4(color, 1.0)")
    "}";

#endif //NO_EMBEDDED_SHADERS

/* Types definitions */

struct RLG_ShadowMap
{
    Texture2D depth;
    unsigned int id;
    int width, height;
};

struct RLG_Material ///< NOTE: This struct is used to handle data that cannot be stored in the MaterialMap struct of raylib.
{
    struct
    {
        int useMaps[RLG_COUNT_MATERIAL_MAPS];
        int parallaxMinLayers;
        int parallaxMaxLayers;
    }
    locs;

    struct
    {
        int useMaps[RLG_COUNT_MATERIAL_MAPS];
        int parallaxMinLayers;
        int parallaxMaxLayers;
    }
    data;
};

struct RLG_Light
{
    struct
    {
        int vpMatrix;       ///< NOTE: Not present in the Light shader struct but in a separate uniform
        int shadowCubemap;
        int shadowMap;
        int position;
        int direction;
        int color;
        int energy;
        int specular;
        int size;
        int innerCutOff;
        int outerCutOff;
        int constant;
        int linear;
        int quadratic;
        int shadowMapTxlSz;
        int depthBias;
        int type;
        int shadow;
        int enabled;
    }
    locs;

    struct
    {
        struct RLG_ShadowMap shadowMap;
        Vector3 position;
        Vector3 direction;
        Vector3 color;
        float energy;
        float specular;
        float size;
        float innerCutOff;
        float outerCutOff;
        float constant;
        float linear;
        float quadratic;
        float shadowMapTxlSz;
        float depthBias;
        int type;
        int shadow;
        int enabled;
    }
    data;
};

struct RLG_SkyboxHandling
{
    unsigned int previousCubemapID;  /*< Indicates whether to update the data sent to the skybox
                                         shader if different from the ID of the skybox to render */
    int locDoGamma;
};

static struct RLG_Core
{
    /* Default material maps */

    MaterialMap defaultMaps[RLG_COUNT_MATERIAL_MAPS];
    bool usedDefaultMaps[RLG_COUNT_MATERIAL_MAPS];

    /* Shaders */

    Shader shaders[RLG_COUNT_SHADERS];

    /* Skybox handling data */

    struct RLG_SkyboxHandling skybox;

    /* Lighting shader data*/

    struct RLG_Material material;
    struct RLG_Light *lights;
    unsigned int lightCount;

    Vector3 colAmbient;
    Vector3 viewPos;

    /* Special values ​​and uniforms */

    float zNear;
    float zFar;

    int locDepthCubemapLightPos;
    int locDepthCubemapFar;
    int locLightingFar;
}
*rlgCtx = NULL;

#ifndef NO_EMBEDDED_SHADERS
    static const char
        *rlgCachedLightingVS = rlgLightingVS,
        *rlgCachedLightingFS = rlgLightingFS;
    static const char
        *rlgCachedDepthVS = rlgDepthVS,
        *rlgCachedDepthFS = rlgDepthFS;
    static const char
        *rlgCachedDepthCubemapVS = rlgDepthCubemapVS,
        *rlgCachedDepthCubemapFS = rlgDepthCubemapFS;
    static const char
        *rlgCachedIrradianceConvolutionVS = rlgCubemapVS,
        *rlgCachedIrradianceConvolutionFS = rlgIrradianceConvolutionFS;
    static const char
        *rlgCachedEquirectangularToCubemapVS = rlgCubemapVS,
        *rlgCachedSkyboxVS = rlgSkyboxVS,
        *rlgCachedEquirectangularToCubemapFS = rlgEquirectangularToCubemapFS;
    static const char
        *rlgCachedSkyboxFS = rlgSkyboxFS;
#else
    static const char
        *rlgCachedLightingVS                    = NULL,
        *rlgCachedLightingFS                    = NULL,
        *rlgCachedDepthVS                       = NULL,
        *rlgCachedDepthFS                       = NULL,
        *rlgCachedDepthCubemapVS                = NULL,
        *rlgCachedDepthCubemapFS                = NULL,
        *rlgCachedIrradianceConvolutionFS       = NULL,
        *rlgCachedIrradianceConvolutionVS       = NULL,
        *rlgCachedEquirectangularToCubemapVS    = NULL,
        *rlgCachedEquirectangularToCubemapFS    = NULL,
        *rlgCachedSkyboxVS                      = NULL,
        *rlgCachedSkyboxFS                      = NULL;
#endif //NO_EMBEDDED_SHADERS

#undef TOSTRING
#undef STRINGIFY

#undef INIT_STRUCT
#undef INIT_STRUCT_ZERO

#endif
