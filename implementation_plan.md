# Bread Land Implementation Plan



# Mason Work: Visual Effects & Animation (100 points)

## Feature 1: Normal Mapping (40 points)
[Tutorial](https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/)

### Task M1.1: Add Tangent/Bitangent to Vertex Data
**Files to modify:**
- `src/shapes/Cube.cpp`, `Cube.h`
- `src/shapes/Sphere.cpp`, `Sphere.h`
- `src/shapes/Cylinder.cpp`, `Cylinder.h`
- `src/shapes/Cone.cpp`, `Cone.h`

**Changes:**
- Update vertex format from 8 floats to 14 floats: `[x, y, z, nx, ny, nz, u, v, tx, ty, tz, bx, by, bz]`
- Calculate tangent along U direction, bitangent along V direction
- Add `insertVec3()` calls for tangent and bitangent after UV insertion
- Ensure orthonormality using cross products

### Task M1.2: Update ShapeManager for 14-float Vertices
**Files to modify:**
- `src/shapes/ShapeManager.cpp`, `ShapeManager.h`

**Changes:**
- Update `createVAO()`: change stride from `8 * sizeof(float)` to `14 * sizeof(float)`
- Add vertex attribute locations 3 and 4 for tangent and bitangent
- Update vertex count calculation from `/8` to `/14`

### Task M1.3: Create TextureManager Class
**New files:**
- `src/rendering/TextureManager.cpp`
- `src/rendering/TextureManager.h`

**Implementation:**
- Use Qt QImage to load PNG/JPG textures
- Implement texture caching with `std::map<std::string, GLuint>`
- Methods: `loadTexture()`, `bindTexture()`, `cleanup()`

### Task M1.4: Update Shaders for Normal Mapping
**Files to modify:**
- `resources/shaders/default.vert`
- `resources/shaders/default.frag`

**Changes:**
- Vertex shader: accept tangent/bitangent at locations 3 and 4, transform to world space
- Fragment shader: construct TBN matrix, sample normal map, apply to lighting calculations
- Add uniforms: `sampler2D normalMap`, `float normalMapIntensity`

### Task M1.5: Integrate TextureManager into Realtime
**Files to modify:**
- `src/realtime.cpp`, `realtime.h`
- `src/rendering/ShaderManager.cpp`, `ShaderManager.h`
- `CMakeLists.txt` (add TextureManager.cpp)

**Changes:**
- Add `TextureManager m_textureManager` member
- Load normal map textures in `sceneChanged()`
- Bind textures before rendering each shape
- Add `setUniformSampler2D()` method to ShaderManager

---

## Feature 2: Realtime Fog (20 points)
[Tutorial](https://opengl-notes.readthedocs.io/en/latest/topics/texturing/aliasing.html#fog)

### Task M2.1: Implement Fog in Fragment Shader
**Files to modify:**
- `resources/shaders/default.frag`

**Changes:**
- Calculate distance from camera: `float distance = length(cameraPos - fragPosition)`
- Implement linear fog: `fogFactor = clamp((distance - fogStart) / (fogEnd - fogStart), 0.0, 1.0)`
- Blend with fog color: `finalColor = mix(litColor, fogColor, fogFactor)`
- Uniforms already initialized in boilerplate

### Task M2.2: Add UI Controls for Fog
**Files to modify:**
- `src/mainwindow.cpp`, `mainwindow.h`
- `src/settings.h`

**Changes:**
- Add checkbox to enable/disable fog
- Add sliders for fog density, start distance, end distance
- Wire up to `settings.enableFog`, `settings.fogDensity`, etc.

---

## Feature 3: Scrolling Textures (20 points)

### Task M3.1: Add Time Tracking
**Files to modify:**
- `src/realtime.cpp`, `realtime.h`

**Changes:**
- Add `float m_elapsedTime` member
- Update in `timerEvent()`: `m_elapsedTime = m_elapsedTimer.elapsed() / 1000.0f`
- Pass to shader in `paintGL()`: `m_shaderManager.setUniformFloat("time", m_elapsedTime)`

### Task M3.2: Implement UV Scrolling in Shader
**Files to modify:**
- `resources/shaders/default.frag`

**Changes:**
- Calculate scrolled UVs: `vec2 scrolledUV = fragUV + scrollDirection * time * scrollSpeed`
- Use `scrolledUV` when sampling textures if `enableScrolling` is true
- Uniforms already declared in boilerplate

### Task M3.3: Add UI Controls for Scrolling
**Files to modify:**
- `src/mainwindow.cpp`, `mainwindow.h`
- `src/settings.h`

**Changes:**
- Add checkbox to enable/disable scrolling
- Add slider for scroll speed
- Wire up to `settings.enableScrolling`, `settings.scrollSpeed`

---

## Feature 4: Instanced Rendering (20 points)
[Tutorial](https://learnopengl.com/Advanced-OpenGL/Instancing)

### Task M4.1: Create InstanceManager Class
**New files:**
- `src/rendering/InstanceManager.cpp`
- `src/rendering/InstanceManager.h`

**Implementation:**
- Generate random instance transformation matrices
- Store in `std::vector<glm::mat4>`
- Methods: `generateInstances()`, `uploadToGPU()`, `getInstanceVBO()`, `getInstanceCount()`

### Task M4.2: Update Shaders for Instancing
**Files to modify:**
- `resources/shaders/default.vert`

**Changes:**
- Add `layout(location=5) in mat4 instanceMatrix` (uses locations 5-8)
- Use `instanceMatrix` instead of `modelMatrix` when instancing is enabled
- Add `#define USE_INSTANCING` toggle

### Task M4.3: Implement Instanced Drawing
**Files to modify:**
- `src/realtime.cpp`, `realtime.h`
- `src/shapes/ShapeManager.cpp`, `ShapeManager.h`
- `CMakeLists.txt` (add InstanceManager.cpp)

**Changes:**
- Add instance VBO setup in ShapeManager
- Use `glDrawArraysInstanced()` instead of `glDrawArrays()`
- Configure vertex attributes with `glVertexAttribDivisor(location, 1)` for locations 5-8

### Task M4.4: Add UI Controls for Instancing
**Files to modify:**
- `src/mainwindow.cpp`, `mainwindow.h`
- `src/settings.h`

**Changes:**
- Add checkbox to enable/disable instanced rendering
- Add slider for instance count
- Wire up to `settings.enableInstancing`, etc.

---




# Sai Work: lighting and Scene Systems (100 points)

## Feature 1: Shadow Mapping (60 points)
[Tutorial](https://learnopengl.com/Advanced-Lighting/Shadows/Shadow-Mapping)

### Task S1.1: Create Shadow Framebuffer
**New files:**
- `src/rendering/ShadowManager.cpp`
- `src/rendering/ShadowManager.h`

**Implementation:**
- Generate depth texture for shadow map
- Create framebuffer object (FBO) for shadow pass
- Methods: `initialize()`, `bindForWriting()`, `bindForReading()`, `cleanup()`
- Recommended shadow map size: 2048x2048

### Task S1.2: Create Shadow Mapping Shaders
**New files:**
- `resources/shaders/shadow.vert`
- `resources/shaders/shadow.frag`

**Implementation:**
- Simple depth-only shaders for shadow pass
- Transform vertices to light space
- Output depth values

### Task S1.3: Implement Two-Pass Rendering
**Files to modify:**
- `src/realtime.cpp`, `realtime.h`
- `src/rendering/ShaderManager.cpp`, `ShaderManager.h`
- `CMakeLists.txt` (add ShadowManager.cpp, add shadow shaders to resources)

**Changes:**
- First pass: render scene from light's perspective to shadow map
- Second pass: normal rendering with shadow map sampling
- Calculate light-space transformation matrix
- Add shadow shader program management

### Task S1.4: Update Main Shaders for Shadow Sampling
**Files to modify:**
- `resources/shaders/default.vert`
- `resources/shaders/default.frag`

**Changes:**
- Vertex shader: calculate light-space position, pass to fragment shader
- Fragment shader: sample shadow map, apply shadow factor to lighting
- Add uniform: `sampler2D shadowMap`, `mat4 lightSpaceMatrix`
- Implement PCF (Percentage Closer Filtering) for soft shadows

### Task S1.5: Add UI Controls for Shadows
**Files to modify:**
- `src/mainwindow.cpp`, `mainwindow.h`
- `src/settings.h`

**Changes:**
- Add checkbox to enable/disable shadows
- Add slider for shadow bias
- Wire up to `settings.enableShadows`, etc.

---

## Feature 2: Procedural Terrain Generation (40 points)
OR
## Feature 2: Post-Processing Pipeline (40 points)

### Option A: Procedural Terrain

#### Task S2A.1: Create TerrainGenerator Class
**New files:**
- `src/terrain/TerrainGenerator.cpp`
- `src/terrain/TerrainGenerator.h`

**Implementation:**
- Generate heightmap using Perlin noise or sine waves
- Create grid of vertices (100x100 or 200x200)
- Calculate normals from height gradients
- Generate UVs with tiling
- Output vertex data in 14-float format (including tangent/bitangent)

#### Task S2A.2: Integrate Terrain into Scene
**Files to modify:**
- `src/realtime.cpp`, `realtime.h`
- `CMakeLists.txt` (add TerrainGenerator.cpp)

**Changes:**
- Generate terrain in `sceneChanged()`
- Add terrain VAO/VBO management
- Render terrain as first object in scene
- Apply bread crust material and textures

#### Task S2A.3: Add UI Controls for Terrain
**Files to modify:**
- `src/mainwindow.cpp`, `mainwindow.h`
- `src/settings.h`

**Changes:**
- Add sliders for terrain parameters (scale, height multiplier, octaves)
- Add button to regenerate terrain
- Wire up to terrain generator

---

### Option B: Post-Processing Pipeline

#### Task S2B.1: Create Post-Process Framebuffer
**New files:**
- `src/rendering/PostProcessManager.cpp`
- `src/rendering/PostProcessManager.h`

**Implementation:**
- Create FBO with color and depth attachments
- Generate screen-aligned quad for rendering
- Methods: `initialize()`, `bindForWriting()`, `renderToScreen()`

#### Task S2B.2: Create Post-Process Shaders
**New files:**
- `resources/shaders/screen.vert`
- `resources/shaders/screen.frag`

**Implementation:**
- Simple pass-through vertex shader
- Fragment shader with effects: bloom, color grading, edge detection, etc.

#### Task S2B.3: Implement Post-Processing Pass
**Files to modify:**
- `src/realtime.cpp`, `realtime.h`
- `CMakeLists.txt` (add PostProcessManager.cpp, add screen shaders to resources)

**Changes:**
- Render scene to texture instead of screen
- Apply post-processing effects
- Render final result to screen

#### Task S2B.4: Add UI Controls for Post-Processing
**Files to modify:**
- `src/mainwindow.cpp`, `mainwindow.h`
- `src/settings.h`

**Changes:**
- Add checkboxes for different effects
- Add sliders for effect intensity
- Wire up to post-process settings

---



### We'll both be modifying these:
- `src/realtime.cpp`, `realtime.h`
- `resources/shaders/default.vert`, `default.frag`
- `src/settings.h` - 
- `CMakeLists.txt` - 

### Mason exclusive Files:
- `src/shapes/*.cpp`, `src/shapes/*.h` - Mason owns shape modifications
- `src/rendering/TextureManager.*`
- `src/rendering/InstanceManager.*`

### Sai Exclusive Files:
- `src/rendering/ShadowManager.*`
- `src/terrain/TerrainGenerator.*` OR `src/rendering/PostProcessManager.*`
- `resources/shaders/shadow.*`
- `resources/shaders/screen.*` (if doing post-processing)

