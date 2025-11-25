## Phase 0: Base Infrastructure Setup

### Ticket 0.1: Project Setup
Copy proj5 codebase to bread-final directory. Verify project compiles and runs with existing features.

### Ticket 0.2: Create Basic Bread Land Scene
Create bread_land.json scene file with terrain mesh (large scaled cube or plane for now), camera positioned above landscape, and warm directional lighting (golden hour simulation).

### Ticket 0.3: Texture Loading System
Create TextureManager class using Qt QImage to load PNG/JPG files. Implement OpenGL texture generation with glGenTextures, glTexImage2D, glTexParameteri for wrapping and filtering. Add texture caching (std::map) to avoid redundant loads.

### Ticket 0.4: UV Coordinate Generation
Add UV coordinates to all shape generators (Cube, Sphere, Cylinder, Cone). Implement UV mapping: planar for cube faces, spherical for sphere (atan2/asin), cylindrical for cylinder/cone. Update VAO attribute pointers to include location 2 for UV. Update vertex shader to pass UVs to fragment shader.

### Ticket 0.5: UI Controls for New Features
Add UI widgets: fog controls (enable checkbox, density slider, start/end distance sliders, color picker), normal mapping controls (enable checkbox, intensity slider), scrolling controls (speed slider, direction inputs), instancing controls (count slider, type toggles).

---

## Phase 1: Realtime Fog (20 points)

### Ticket 1.1: Make Depth Buffer Accessible in Fragment Shader
Ensure depth testing is enabled (already should be in proj5). Pass camera position uniform to fragment shader. Calculate fragment-to-camera distance using: `float distance = length(cameraPos - fragWorldPos)`.

### Ticket 1.2: Implement Fog Calculation Functions
Add fog uniforms: `bool enableFog`, `vec3 fogColor`, `float fogStart`, `float fogEnd`, `float fogDensity`, `int fogMode`. Implement linear fog: `fogFactor = clamp((distance - fogStart) / (fogEnd - fogStart), 0.0, 1.0)`. Implement exponential fog: `fogFactor = 1.0 - exp(-fogDensity * distance)`. Blend final color: `finalColor = mix(litColor, fogColor, fogFactor)`. Pass uniforms from Realtime::paintGL().

### Ticket 1.3: Set Fog Color to Be Like Steam Coming out of Oven
Set default fog color to RGB(0.9, 0.7, 0.4) matching bread theme. Add color picker in UI to adjust fog color. Verify fog color blends naturally with scene lighting.

### Ticket 1.4: Add UI Controls for Fog Parameters
Wire up UI sliders to fog uniforms: density, start distance, end distance. Add keyboard toggle (F key) for quick fog enable/disable. Display current parameter values in UI.

---

## Phase 2: Normal Mapping (40 points)

### Ticket 2.1: Generate or Obtain Normal Map Textures
Find/create bread crust normal map texture (512x512 or 1024x1024) showing bumpy crust detail. Should be in tangent space (bluish overall with RGB variations). Save to resources/textures/bread_crust_normal.png. Optionally create variants: fine crust, coarse crust, sourdough bubbles. Also obtain bread diffuse texture for testing.

### Ticket 2.2: Calculate Tangent and Bitangent Vectors
For each shape generator, compute tangent and bitangent aligned with UV directions. Cube: per-face tangent/bitangent aligned with face edges. Sphere: tangent along longitude circles, bitangent along latitude circles. Cylinder/Cone: tangent around circumference, bitangent along height. Add to vertex data (now 14 floats: pos[3] + normal[3] + UV[2] + tangent[3] + bitangent[3]). Update VAO attributes: location 3 for tangent, location 4 for bitangent. Ensure tangent space is orthonormal.

### Ticket 2.3: Implement Tangent-Space Normal Mapping in Fragment Shader
Add uniforms: `sampler2D normalMap`, `bool enableNormalMapping`, `float normalMapIntensity`. In vertex shader, transform tangent and bitangent to world space using normal matrix, pass to fragment shader. In fragment shader, construct TBN matrix: `mat3(normalize(tangent), normalize(bitangent), normalize(normal))`. Sample normal map: `vec3 texNormal = texture(normalMap, fragUV).rgb`. Remap from [0,1] to [-1,1]: `texNormal = texNormal * 2.0 - 1.0`. Apply intensity: `texNormal.xy *= normalMapIntensity`. Transform to world space: `vec3 worldNormal = normalize(TBN * texNormal)`. Use worldNormal for all lighting calculations instead of interpolated normal.

### Ticket 2.4: Apply Normal Mapping to All Geometry
Update scene materials to reference normal map texture. Apply to terrain with tiling (repeat 20x20 times). Apply to mountains with larger tiling (repeat 5x5 times for bigger crust features). Apply to decoration objects (bread instances). Bind normal map texture to GL_TEXTURE1 in rendering loop before drawing each object.

### Ticket 2.5: Test with Multiple Normal Map Intensities and Textures
Test intensity values: 0.0 (flat), 0.5 (subtle), 1.0 (normal), 2.0 (exaggerated). Test different bread crust textures on same geometry. Implement debug visualization: render normals as RGB colors `(normal * 0.5 + 0.5)`. Capture on/off comparison screenshots showing surface detail enhancement. Test on all primitive types (cube, sphere, cylinder, cone).

---

## Phase 3: Scrolling Textures (20 points)

### Ticket 3.1: Add Time Uniform to Shader System
Add time tracking to Realtime class using QElapsedTimer. Pass `uniform float time` (elapsed seconds) to shaders. Update time each frame in paintGL(). Verify time increases continuously.

### Ticket 3.2: Create Butter Texture
Find/create yellow-gold butter texture with flowing/streaky appearance suggesting liquid motion. Should have directional flow pattern. Save to resources/textures/butter_flow.png. Size 512x512 or larger. Ensure texture tiles seamlessly.

### Ticket 3.3: Model River Geometry
Generate river as ribbon mesh (strip of connected quads) following winding path through terrain. Use simple spline or connected line segments. Width ~10 units. Create vertex positions, normals (pointing up), and UVs (U along path 0-1 repeated, V across width 0-1). Position river slightly above terrain (+0.1 units) to avoid z-fighting. Add river geometry to scene as separate renderable object.

### Ticket 3.4: Implement UV Scrolling in Fragment Shader
Add uniforms: `bool enableScrolling`, `vec2 scrollDirection`, `float scrollSpeed`. In fragment shader, calculate scrolled UVs: `vec2 scrolledUV = fragUV + scrollDirection * time * scrollSpeed`. Sample textures with scrolledUV instead of fragUV. Apply to butter river material. Set scrollDirection to flow along river path (e.g., vec2(1.0, 0.0) normalized). Set scrollSpeed to ~0.5 for visible flow.

### Ticket 3.5: Test Different Scroll Speeds and Directions
Test scroll speeds: 0.1x (very slow), 0.5x (slow), 1.0x (medium), 2.0x (fast), 5.0x (very fast). Test directions: horizontal (1,0), vertical (0,1), diagonal (1,1) normalized, reverse (-1,0). Verify UV wrapping at boundaries is seamless. Test multiple objects scrolling at different rates. Capture video demonstrations or animated GIFs.

---

## Phase 4: Instanced Rendering (20 points)

### Ticket 4.1: Create Low-Poly Bread Models
Design bread roll: use existing sphere primitive, radius ~1.5 units, apply golden-brown material. Design baguette: use existing cylinder primitive, length ~6 units, radius ~0.6 units, rotate to horizontal orientation. Design croissant (optional): use bent cylinder or custom mesh, ~3 units length. Prepare materials with bread textures and normal maps for all models.

### Ticket 4.2: Implement Instanced Rendering with glDrawArraysInstanced
Create instance VBO to store transformation matrices (mat4, 16 floats per instance). Allocate buffer with glGenBuffers, glBindBuffer(GL_ARRAY_BUFFER), glBufferData with GL_DYNAMIC_DRAW. Set up vertex attributes for mat4 (requires 4 vec4s at locations 5-8): for each column, glVertexAttribPointer with stride 64 bytes, offsets 0/16/32/48, then glVertexAttribDivisor(location, 1). Update vertex shader to accept `layout(location=5-8) in mat4 instanceMatrix` and use it for transformations instead of modelMatrix. Replace glDrawArrays with glDrawArraysInstanced(GL_TRIANGLES, 0, vertexCount, instanceCount).

### Ticket 4.3: Generate Instance Transformation Matrices
Create function to generate random instance data: positions scattered across terrain area (e.g., -500 to +500 in x/z), random rotations (0 to 2π around y-axis), random scales (0.8 to 1.2 for variety). Build mat4 for each instance combining translation, rotation, and scale. Store in std::vector. Support different instance types (rolls, baguettes) with separate instance buffers.

### Ticket 4.4: Pass Transformations via Vertex Attributes
Upload instance matrices to GPU: glBindBuffer instance VBO, glBufferData or glBufferSubData with matrices data. Before drawing instanced objects, bind shape's VAO and instance VBO, configure attribute pointers, then call glDrawArraysInstanced. Implement toggle between regular rendering and instanced rendering for comparison.

### Ticket 4.5: Populate Landscape with Hundreds of Bread Instances
Generate 500+ bread roll instances scattered across terrain. Generate 200+ baguette instances at various angles. Implement terrain height sampling: for each instance position (x, z), calculate corresponding terrain y-coordinate so instances sit on surface. Distribute instances with some clustering in foreground, sparse in background. Apply bread textures with normal mapping to all instances.

### Ticket 4.6: Measure FPS with Varying Instance Counts
Implement FPS counter: track frame time deltas, calculate average over 1 second, display in UI or console. Test performance at instance counts: 0 (baseline), 100, 500, 1000, 2000, 5000. Record FPS for each count. Compare instanced rendering (single draw call) vs non-instanced (draw each separately). Create performance table and graph. Verify instances are lit correctly and affected by fog based on distance.

---

## Phase 5: Scene Composition and Integration

### Ticket 5.1: Procedural Terrain Generation
Implement heightmap-based terrain: create 100x100 or 200x200 vertex grid. Use noise function (Perlin noise or simplesine waves) to generate height values creating rolling hills. Calculate vertex normals by averaging face normals or using height gradient. Generate UVs with tiling (u = x/terrain_width * tileCount, v = z/terrain_depth * tileCount). Apply bread crust diffuse and normal map textures tiled 20x20. Set material: golden brown diffuse (0.8, 0.6, 0.3), low specular (0.1), low shininess (matte crust).

### Ticket 5.2: Mountain Geometry Generation
Create 3-5 mountain meshes shaped like bread loaves: use scaled spheres (flattened on bottom) or custom geometry with rounded tops. Scale to large size (height 200-400 units, width 300-600 units). Position in background at 800-1200 units from origin. Apply bread crust textures with larger-scale tiling (5x5 or 10x10 for bigger crust features). Verify mountains fade into golden fog at distance for atmospheric depth.

### Ticket 5.3: Camera and Lighting Configuration
Set camera starting position at elevation ~100 units, centered over terrain, angled downward (~-30 degrees pitch). Configure camera movement: WASD for translation, mouse drag for rotation, maintain minimum height constraint (5 units above terrain). Set directional light direction: (-0.3, -0.7, -0.5) normalized (above-front). Set light color: warm white RGB(1.0, 0.95, 0.85). Set scene ambient: golden RGB(0.9, 0.7, 0.4) at low intensity (ka = 0.25). Verify consistent warm atmosphere throughout scene.

### Ticket 5.4: Final Integration and Feature Toggles
Combine all elements into complete scene: procedural terrain, loaf-shaped mountains, butter river with scrolling texture, instanced bread objects (rolls and baguettes), golden fog, warm lighting. Optimize rendering order: render opaque objects front-to-back where possible (terrain, then instances, then mountains). Add UI checkboxes to toggle each of 4 main features independently: fog enable/disable, normal mapping enable/disable, scrolling enable/disable, instancing enable/disable (switch to regular rendering). Verify no z-fighting between terrain and river. Test all features work together correctly. Capture final beauty shots and demonstration videos.