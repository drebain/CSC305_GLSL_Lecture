// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <vector>

#include <OpenGP/GL/MaterialRenderer.h>
#include <OpenGP/SphereMesh/SphereMesh.h>
#include <OpenGP/GL/GPUMesh.h>
#include <OpenGP/SphereMesh/bounding_box.h>
#include <OpenGP/SphereMesh/helpers.h>


//=============================================================================
namespace OpenGP {
//=============================================================================

class SphereMeshRenderer : public MaterialRenderer {
private:

    GPUMesh sphere, cone, triangle;

    Shader sphere_shader, cone_shader, triangle_shader;

    int sphere_count = 0, cone_count = 0;

    const float PI = 3.141592653589793238462643383;

    HEADERONLY_INLINE static const char *sphere_vshader();
    HEADERONLY_INLINE static const char *sphere_fshader();

    HEADERONLY_INLINE static const char *cone_vshader();
    HEADERONLY_INLINE static const char *cone_fshader();

    HEADERONLY_INLINE static const char *triangle_vshader();
    HEADERONLY_INLINE static const char *triangle_fshader();

public:

    HEADERONLY_INLINE SphereMeshRenderer();

    virtual ~SphereMeshRenderer() {}

    HEADERONLY_INLINE void render(const RenderContext&);

    HEADERONLY_INLINE void rebuild();

    HEADERONLY_INLINE void upload_mesh(const SphereMesh &mesh);

};

//=============================================================================
} // OpenGP::
//=============================================================================

#ifdef HEADERONLY
    #include "SphereMeshRenderer.cpp"
#endif
