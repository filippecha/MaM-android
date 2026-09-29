# Emulator-only test patch. The Android emulator tops out at OpenGL ES 3.1.
sed -i '/Use OpenGL ES 3.2/,/GL_PROFILE_ES/ s/opts.versionMinor = 2;/opts.versionMinor = 1;/' src/Engine/Graphics/Renderer/OpenGLRenderer.cpp
sed -i 's/#version 320 es\\n/#version 310 es\\n#extension GL_EXT_texture_buffer : enable\\n/' src/Engine/Graphics/Renderer/OpenGLShader.cpp
grep -n "versionMinor = 1" src/Engine/Graphics/Renderer/OpenGLRenderer.cpp
grep -n "310 es" src/Engine/Graphics/Renderer/OpenGLShader.cpp
