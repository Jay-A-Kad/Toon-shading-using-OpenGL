# Toon-shading-using-OpenGL

#####Project Goals

The project aims to implement a toon shading (cel shading) technique with 4-channel
stylized highlighting and edge detection, aiming to create visually appealing and illustrative
rendering eAects for 3D objects such as spheres, torus, and stylized pineapple which are
rotating along their axis.

#####Project Description

1. Brief Overview of Tasks:
• Developed a 3D rendering pipeline utilizing OpenGL and GLSL shaders.
• Implemented toon shading with quantized lighting intensities and stylized highlights.
• Added edge detection by rendering wireframe outlines on objects.
• Created geometries for a sphere, a torus, and a stylized pineapple with cones for
leaves.
• Enabled dynamic rotation and interactive camera movement with keyboard controls.
• Incorporated Fullscreen toggle and maintained aspect ratio on window resizing.

2. Tools/API/IDE:
• Programming Language: C++
• Graphics Library: OpenGL with GLEW for extensions and GLUT for windowing and
events.
• Math Library: GLM for matrix and vector operations.
• Development Environment: Visual Studio 2022

3. Workflow:
• Initialization: Set up shaders, compiled vertex and fragment shaders for cel shading
and outline rendering.
• Geometry Creation: Used mathematical algorithms to generate sphere, torus, and
pineapple geometries.
• Rendering Pipeline: Applied transformations, configured camera settings, and
implemented custom lighting calculations in shaders.
• Interaction: Added keyboard-based camera controls and window management
features.

4. Technical/Algorithmic Challenges Faced:
• Quantized Lighting: Ensuring smooth yet discrete transitions in light intensity for the
toon shader.
• Complex Geometry Generation: EAiciently constructing and rendering objects like
the torus and pineapple with consistent normal and indices.
• Interaction & Usability: Implementing real-time interaction for camera and
rendering adjustments without performance degradation.

5. Strategies Used to Meet the Challenges:
• Used GLSL if conditions to implement light quantization in the fragment shader.
• Scaled objects slightly for outline rendering to prevent depth conflicts.
• Precomputed vertex positions, normal, and indices for complex geometries.
• Leveraged timer functions for smooth rotation and interaction responsiveness.