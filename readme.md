# BREAD LAND

## Project Overview

A realtime landscape where everything is made of bread. Features bread-textured terrain, mountains resembling loaves, and atmospheric effects creating a warm, golden bread-themed world.

## Technical Features

### 1. Realtime Fog (20 points)

[tutorial](https://opengl-notes.readthedocs.io/en/latest/topics/texturing/aliasing.html#fog)

- Implementation: Depth-based fog calculation in fragment shader
- Purpose: Create atmospheric golden haze throughout Bread Land
- Dependencies: Depth Buffers (already implemented)
- Why first: Simplest feature, sets the atmospheric tone

### 2. Normal Mapping (40 points)

[tutorial](https://www.opengl-tutorial.org/intermediate-tutorials/tutorial-13-normal-mapping/)

- Implementation: Fragment shader that perturbs surface normals using a texture
- Purpose: Add surface detail to bread crusts on terrain, mountains, and objects
- Dependencies: None
- Why second: Core visual feature that all bread surfaces will use

### 3. Scrolling Textures (20 points)

tutorial

- Implementation: Time-based UV coordinate offset in shader
- Purpose: Animate flowing butter rivers or rotating bread artifacts
- Dependencies: None
- Why third: Simple animation system built on existing texture pipeline

### 4. Instanced Rendering (20 points)

[tutorial](https://learnopengl.com/Advanced-OpenGL/Instancing)

- Implementation: Single draw call to render multiple copies of bread models
- Purpose: Efficiently render fields of bread rolls, baguette forests, or scattered crumbs
- dependencies: None
- Why last: Performance optimization after core visuals are working




## Design Details

### Scene Architecture

Bread Land consists of interconnected bread-themed components:

1. Terrain System: Rolling hills and valleys with bread crust texture and normal mapping
2. Mountain System: Background mountains shaped like loaves with crusty peaks
3. Atmospheric System: steamy fog creating depth and warmth
4. Decorations?: Instanced bread objects scattered throughout (rolls, baguettes, croissants)
5. Animations: Flowing butter rivers or rotating bread monuments using scrolling textures

### Component Interactions

- Terrain and mountains share the same bread crust normal maps but at different scales
- Fog system uses depth buffer to fade distant mountains into golden haze
- Instanced objects (bread rolls, baguettes) are placed on terrain surface using height sampling
- Scrolling textures applied to specific geometry (butter rivers) overlay the terrain
- All components use consistent bread-colored palette (golden browns, warm yellows)

### Visual Hierarchy

1. Foreground: Detailed instanced bread objects with strong normal mapping
2. Midground: Terrain with moderate normal mapping detail
3. Background: Mountains with subtle normal mapping, heavily affected by fog
4. Atmosphere: Fog intensity increases with distance, unifying all layers








## Implementation Plan


### Rough overview

1. Start from existing Realtime project codebase
2. Create terrain mesh (procedural grid or heightmap-based)
3. Model mountain geometry as large triangulated loaves
4. Create or obtain bread crust textures and normal maps
5. Set up camera with ability to move through the landscape
6. Establish warm directional lighting (simulating golden hour)


### Feature Implementation Order

#### Phase 1: Realtime Fog

1. Ensure depth buffer is accessible in fragment shader
2. Implement linear or exponential fog calculation based on fragment depth
3. Set fog color to warm golden/amber tone
4. Add UI controls for fog density, start distance, and end distance
5. Test fog at various distances and densities

Component Integration:

- Fog wraps around all geometry uniformly
- Depth buffer automatically populated by OpenGL during rendering
- Fog color matches overall bread color scheme

#### Phase 2: Normal Mapping

1. Generate or obtain normal map textures for bread crust detail
2. Calculate tangent and bitangent vectors for all geometry vertices
3. Implement tangent-space normal mapping in fragment shader
4. Apply normal mapping to terrain, mountains, and decoration objects
5. Test with multiple normal map intensities and different bread textures

Component Integration:

- All geometry shares the same normal mapping shader code
- Terrain uses tiled normal maps for consistent crust appearance
- Mountains use same normal maps at larger scale for bigger crust features
- Normal-mapped surfaces interact correctly with fog (lighting calculated before fog)

#### Phase 3: Scrolling Textures

1. Add time uniform to shader system
2. Create butter texture (yellow/gold with flowing appearance)
3. Model river geometry cutting through terrain
4. Implement UV scrolling in fragment shader (offset UVs by time * scroll_speed)
5. Test different scroll speeds and directions

Component Integration:

- Rivers placed on top of terrain, slightly elevated to avoid z-fighting
- River geometry uses separate shader with scrolling texture
- Rivers affected by fog like other geometry
- Optional: Rivers can also use normal mapping for butter surface detail

#### Phase 4: Instanced Rendering

1. Create low-poly bread models (rolls, baguettes, croissants)
2. Implement instanced rendering using glDrawArraysInstanced or glDrawElementsInstanced
3. Generate instance transformation matrices (positions, rotations, scales)
4. Pass transformations via vertex attributes or uniform buffer
5. Populate landscape with hundreds of bread instances
6. Measure FPS with varying instance counts

Component Integration:

- Instances positioned on terrain surface (sample terrain height at x,z positions)
- Instances use same normal mapping shader as other geometry
- Instances affected by fog based on their distance from camera
- Instance density decreases with distance for performance
- Each instance type (roll, baguette, croissant) batched separately




## Testing Requirements

### Realtime Fog Tests

- Vary fog density parameter (light haze to heavy golden fog)
- Vary fog start and end distances
- Comparison with fog disabled
- Test fog at different times of day (lighting conditions)

### Normal Mapping Tests

- Comparison renders with normal mapping on/off
- Multiple normal map textures (fine crust, coarse crust, sourdough bubbles)
- Different normal map intensities (subtle to extreme)
- Visualize normal vectors as RGB colors for debugging
- Test on terrain, mountains, and instanced objects

### Scrolling Textures Tests

- Different scroll speeds (0.5x, 1.0x, 2.0x, 5.0x)
- Different scroll directions (downstream, upstream, diagonal)
- Verify UV wrapping behavior at boundaries
- Test with different butter/liquid textures

### Instanced Rendering Tests

- FPS comparison: 0 instances vs 100 instances vs 500 instances vs 1000 instances
- Verify correct transformations for each instance (position, rotation, scale)
- Show both instanced and non-instanced rendering methods
- Test multiple instance types (rolls, baguettes, croissants) rendered simultaneously




## Debugging Strategy

### Realtime Fog

- Render fog factor as grayscale overlay
- Isolate fog calculation by disabling other lighting effects
- Verify depth values are being read correctly from depth buffer
- Toggle fog on/off with keyboard input

### Normal Mapping

- Toggle feature on/off with keyboard input
- Render normal vectors as RGB colors (visualize tangent space)
- Verify tangent and bitangent calculations are correct
- Test with flat normal map (should look identical to no normal mapping)
- Check normal map texture loading and sampling

### Scrolling Textures

- Reduce scroll speed to near-zero to verify direction
- Render UV coordinates as colors on geometry
- Add pause/step functionality for frame-by-frame inspection
- Verify time uniform is updating correctly
- Check for UV seams or wrapping artifacts

### Instanced Rendering

- Draw wireframe or bounding boxes around each instance
- Color-code instances by index to verify count
- Print instance count and verify against expected value
- Test with single instance first, then scale up
- Verify transformation matrices are computed correctly





## Scene Composition

### Geometry

- Terrain: Rolling hills and valleys (procedural grid or heightmap, 100x100 to 200x200 vertices)
- Mountains: 3-5 large loaf-shaped meshes in background (800-1200 units distant)
- Butter Rivers: Winding path geometry cutting through terrain (lower poly, follows curves)
- Bread Objects: Multiple types for instancing
  - Bread rolls (spherical, ~100 triangles each)
  - Baguettes (cylindrical, ~150 triangles each)
  - Croissants (curved, ~200 triangles each)

### Textures

- Bread crust albedo: Golden brown base color with variation
- Bread normal maps: Bumpy crust texture (512x512 or 1024x1024)
  - Fine crust variant for terrain
  - Coarse crust variant for mountains
  - Smooth variant for rolls
- Butter texture: Yellow-gold flowing liquid appearance
- Sky: Warm gradient or solid color (toasted wheat color)

### Materials

- All bread surfaces: Same base material with normal mapping
  - Diffuse: Warm golden brown (RGB: 0.8, 0.6, 0.3)
  - Specular: Low specularity for matte crust (0.1-0.2)
  - Ambient: Warm tone matching fog color
- Butter rivers: More specular for wet appearance (0.4-0.6)

### Lighting

- Directional Light: Warm sunlight from above-front (simulating golden hour)
  - Direction: approximately (-0.3, -0.7, -0.5) normalized
  - Color: Slightly warm white (RGB: 1.0, 0.95, 0.85)
- Ambient: Warm golden ambient matching fog
  - Color: Golden (RGB: 0.9, 0.7, 0.4) at low intensity (0.2-0.3)

### Camera

- Starting position: Elevated view of landscape (y: 50-100 units)
- Movement: Free-fly camera or orbital camera
- Field of view: 60-75 degrees
- far plane: Set beyond mountains (1500-2000 units) to capture full scene

### Layout

- Foreground (0-300 units): Terrain with dense instanced bread objects
- Midground (300-800 units): Terrain with butter rivers, moderate bread instances
- Background (800-1200 units): Mountain range with sparse or no instances
- Sky/Fog: Uniform fog grows denser with distance, obscuring far mountains

## Deliverables

1. Updated codebase building on Realtime project
2. Test cases demonstrating each feature with parameter variations
3. Screenshots or videos showing all four features working
4. performance measurements for instanced rendering
5. Submission template documenting implementation and results
