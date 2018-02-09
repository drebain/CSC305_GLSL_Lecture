// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <OpenGP/SurfaceMesh/bounding_box.h>
#include <OpenGP/SphereMesh/SphereMesh.h>


//=============================================================================
namespace OpenGP{
//=============================================================================

inline Box3 bounding_box(const SphereMesh& mesh) {
    auto vpoints = mesh.get_vertex_property<Vec4>("v:point");
    Box3 bbox;
    bbox.setNull();
    for(auto v: mesh.vertices()) {
        Vec4 point = vpoints[v];
        bbox.extend( point.block<3,1>(0,0) + Vec3(1, 1, 1) * point(3) );
        bbox.extend( point.block<3,1>(0,0) - Vec3(1, 1, 1) * point(3) );
    }
    return bbox;
}

//=============================================================================
} // namespace OpenGP
//=============================================================================
