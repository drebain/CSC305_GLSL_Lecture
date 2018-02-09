// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <OpenGP/util/cuda_support.h>
#include <OpenGP/SphereMesh/SphereMesh.h>


//=============================================================================
namespace OpenGP {
//=============================================================================

inline Vec2 pill_tangent(const Vec4 &s0_3D, const Vec4 &s1_3D) {

    /// Adapted from pill_dist.m

    Scalar dist_3D = (s1_3D.block<3, 1>(0, 0) - s0_3D.block<3, 1>(0, 0)).norm();

    Vec3 s0(0, 0, s0_3D(3));
    Vec3 s1(dist_3D, 0, s1_3D(3));

    Vec3 a = s1 - s0;
    Scalar a2 = (a(0)*a(0)) + (a(1)*a(1));
    Scalar ad = std::sqrt(a2);
    Vec2 ab = a.block<2, 1>(0, 0);

    Scalar beta = ab.dot(Vec2(std::sqrt((ad*ad)-(a(2)*a(2))), a(2))) / (ad * ad);
    Scalar alpha = std::sqrt(1 - (beta * beta));

    return Vec2(beta, alpha);

}

OPENGP_DEVICE_FUNC inline float stop_nan(float t) {
    return cuda_support::isfinite(t) ? t : 0;
}

OPENGP_DEVICE_FUNC inline float clamp01(float t) {
    return fmax(fmin(t, 1), 0);
}

OPENGP_DEVICE_FUNC inline Vec3 get_barycentric(Vec3 v0, Vec3 v1, Vec3 v2, Vec3 p) {
    Vec3 A0 = (v1 - v0).cross(v2 - v0);
    Vec3 A1 = (v2 - v1).cross(v0 - v1);
    Vec3 A2 = (v0 - v2).cross(v1 - v2);
    Vec3 l0 = v0 - p;
    Vec3 l1 = v1 - p;
    Vec3 l2 = v2 - p;
    float A_inv = stop_nan(1.0f / A0.squaredNorm());
    return A_inv * Vec3(
        l1.cross(l2).dot(A0),
        l2.cross(l0).dot(A1),
        l0.cross(l1).dot(A2)
    );
}

OPENGP_DEVICE_FUNC inline Vec3 wedge_normal(const Vec4 &s0, const Vec4 &s1, const Vec4 &s2) {

    Vec3 c0c1 = s1.head<3>() - s0.head<3>();
    Vec3 c0c2 = s2.head<3>() - s0.head<3>();

    float beta = asin((s2(3) - s0(3)) / c0c2.norm());

    Vec3 n_tri = c0c1.cross(c0c2).normalized();
    Vec3 a = -c0c2.normalized();
    Vec3 b = a.cross(n_tri);

    float sb = sin(beta);
    float cb = cos(beta);

    float alpha = asin((s0(3) - s1(3) - sb * c0c1.dot(a)) / (cb * c0c1.dot(b)));

    float sa = sin(alpha);
    float ca = cos(alpha);

    return a * sb + cb * (n_tri * ca + b * sa);
}

OPENGP_DEVICE_FUNC inline Vec3 wedge_normal_toward(const Vec4 &s0, const Vec4 &s1, const Vec4 &s2, const Vec3& p) {

    Vec3 c0c1 = s1.head<3>() - s0.head<3>();
    Vec3 c0c2 = s2.head<3>() - s0.head<3>();

    float beta = asin((s2(3) - s0(3)) / c0c2.norm());

    Vec3 n_tri = c0c1.cross(c0c2).normalized();
    n_tri *= copysign(1.0f, n_tri.dot(p - s0.head<3>()));
    Vec3 a = -c0c2.normalized();
    Vec3 b = a.cross(n_tri);

    float sb = sin(beta);
    float cb = cos(beta);

    float alpha = asin((s0(3) - s1(3) - sb * c0c1.dot(a)) / (cb * c0c1.dot(b)));

    float sa = sin(alpha);
    float ca = cos(alpha);

    return a * sb + cb * (n_tri * ca + b * sa);

}

OPENGP_DEVICE_FUNC inline Vec3 wedge_normal(const Vec4 &s0, const Vec4 &s1, const Vec4 &s2, bool flipped) {

    Vec3 c0c1 = s1.head<3>() - s0.head<3>();
    Vec3 c0c2 = s2.head<3>() - s0.head<3>();

    Vec3 n_tri = c0c1.cross(c0c2);

    Vec3 result = wedge_normal(s0, s1, s2);
    if (flipped) result -= 2 * n_tri * n_tri.dot(result) / n_tri.squaredNorm();
    return result;
}

OPENGP_DEVICE_FUNC inline Vec3 sphere_project(const Vec3& p, const Vec4& s, float* sdf_out=nullptr) {
    Vec3 d = p - s.head<3>();
    float spdist = d.norm();
    if (sdf_out != nullptr) *sdf_out = spdist - s(3);
    return s.head<3>() + s(3) * d / spdist;
}

OPENGP_DEVICE_FUNC inline Vec3 pill_project(const Vec3 &p, const Vec4 &s0, const Vec4 &s1, float *sdf_out=nullptr) {

    // Abbreviations

    Vec3 c0 = s0.head<3>();

    Vec4 a = s1 - s0;

    Vec3 d = a.head<3>();
    float l = d.norm();

    // Direction of axis
    Vec3 an = d / l;

    // Offset of p from c0
    Vec3 d2 = p - c0;

    // Projection onto skeleton
    Vec3 p_proj = d2 - an * d2.dot(an);

    // Angle of bitangent
    float beta = asin(a(3) / l);

    // Offset along axis
    Vec3 offset = an * p_proj.norm() * tan(beta);

    // Position along axis
    float t = an.dot(p + p_proj + offset - c0) / l;
    // Clamped between 0 and 1
    t = clamp01(stop_nan(t));

    // Lerped position + radius
    Vec4 s2 = (1 - t) * s0 + t * s1;

    return sphere_project(p, s2, sdf_out);

}

OPENGP_DEVICE_FUNC inline Vec3 wedge_project(const Vec3 &p, const Vec4 &s0, const Vec4 &s1, const Vec4 &s2, float *sdf_out=nullptr) {

    Vec3 n_wedge = wedge_normal_toward(s0, s1, s2, p);
    n_wedge(0) = stop_nan(n_wedge(0));
    n_wedge(1) = stop_nan(n_wedge(1));
    n_wedge(2) = stop_nan(n_wedge(2));

    Vec3 tt0 = s0.head<3>() + n_wedge * s0(3);
    Vec3 tt1 = s1.head<3>() + n_wedge * s1(3);
    Vec3 tt2 = s2.head<3>() + n_wedge * s2(3);
    Vec3 ttp = p - n_wedge * (p - tt0).dot(n_wedge);

    Vec3 baryc = get_barycentric(tt0, tt1, tt2, ttp).cwiseAbs();
    if (fabs(baryc.sum() - 1) > 0.001) {
        baryc = Vec3(1, 0, 0);
    }

    Vec4 s3 = baryc(0) * s0 + baryc(1) * s1 + baryc(2) * s2;

    float best_sdf;
    Vec3 best_proj = sphere_project(p, s3, &best_sdf);

    float sdf0, sdf1, sdf2;

    Vec3 proj0 = pill_project(p, s0, s1, &sdf0);
    Vec3 proj1 = pill_project(p, s1, s2, &sdf1);
    Vec3 proj2 = pill_project(p, s2, s0, &sdf2);

    if (sdf0 < best_sdf) { best_sdf = sdf0; best_proj = proj0; }
    if (sdf1 < best_sdf) { best_sdf = sdf1; best_proj = proj1; }
    if (sdf2 < best_sdf) { best_sdf = sdf2; best_proj = proj2; }

    if (sdf_out != nullptr) *sdf_out = best_sdf;

    return best_proj;

}

//=============================================================================
} // namespace OpenGP
//=============================================================================
