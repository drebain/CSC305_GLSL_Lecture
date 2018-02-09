// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <algorithm>

#include "SphereMeshRenderer.h"


//=============================================================================
namespace OpenGP {
//=============================================================================

const char *SphereMeshRenderer::sphere_vshader() {
    return R"GLSL(

        // Per primitive
        in vec4 vsphere;

        // Per vertex
        in vec3 vposition;
        in vec3 vnormal;

        out vec3 fposition;
        out vec3 fnormal;

        void vertex() {
            vec3 real_pos = vposition * vsphere.w + vsphere.xyz;
            gl_Position = get_MVP() * vec4(real_pos, 1.0);
            fposition = (get_M() * vec4(real_pos, 1.0)).xyz;
            fnormal = normalize( inverse(transpose(mat3(get_M()))) * vnormal );
        }

    )GLSL";
}

const char *SphereMeshRenderer::sphere_fshader() {
    return R"GLSL(

        in vec3 fnormal;
        in vec3 fposition;

        void fragment() {
            set_normal(normalize(fnormal));
            set_position(fposition);
            set_wire_weight(0);
        }

    )GLSL";
}

const char *SphereMeshRenderer::cone_vshader() {
    return R"GLSL(

        // Per primitive
        in vec4 v0;
        in vec4 v1;

        // Per vertex
        in float vtheta;
        in float vt;

        out vec3 fposition;
        out vec3 fnormal;

        void vertex() {

            vec4 s = mix(v0, v1, vt);

            vec3 saxis = v1.xyz - v0.xyz;
            float l = length(saxis);
            saxis /= l;
            vec3 a = cross(saxis, vec3(1, 0, 0));
            vec3 b = cross(saxis, vec3(0, 1, 0));
            vec3 ihat = normalize(a + b);
            vec3 jhat = cross(saxis, ihat);

            float beta = asin((v1.w - v0.w) / l);

            vec3 radial = ihat * cos(vtheta) + jhat * sin(vtheta);

            vec3 vnormal = radial * cos(beta) - saxis * sin(beta);
            vec3 vposition = s.xyz + s.w * vnormal;

            gl_Position = get_MVP() * vec4(vposition, 1.0);
            fposition = (get_M() * vec4(vposition, 1.0)).xyz;
            fnormal = normalize( inverse(transpose(mat3(get_M()))) * vnormal );

        }

    )GLSL";
}

const char *SphereMeshRenderer::cone_fshader() {
    return R"GLSL(

        in vec3 fnormal;
        in vec3 fposition;

        void fragment() {
            set_normal(normalize(fnormal));
            set_position(fposition);
            set_wire_weight(0);
        }

    )GLSL";
}

const char *SphereMeshRenderer::triangle_vshader() {
    return R"GLSL(

        in vec3 vposition;
        in vec3 vnormal;

        out vec3 fposition;
        out vec3 fnormal;

        void vertex() {
            gl_Position = get_MVP() * vec4(vposition, 1.0);
            fposition = (get_M() * vec4(vposition, 1.0)).xyz;
            fnormal = normalize( inverse(transpose(mat3(get_M()))) * vnormal );
        }

    )GLSL";
}

const char *SphereMeshRenderer::triangle_fshader() {
    return R"GLSL(

        in vec3 fnormal;
        in vec3 fposition;

        void fragment() {
            set_normal(normalize(fnormal));
            set_position(fposition);
            set_wire_weight(0);
        }

    )GLSL";
}

SphereMeshRenderer::SphereMeshRenderer() {

    int segments = 64;

    { // Sphere generation
        std::vector<Vec3> vpoint;
        std::vector<Vec3> vnormal;
        std::vector<unsigned int> triangles;

        int n = 0;
        for (int i = 0;i <= segments;i++) {
            for (int j = 0;j <= segments;j++) {
                Scalar lon = 2 * PI * ((Scalar)i) / segments;
                Scalar lat = PI * (((Scalar)j) / segments - 0.5);
                Vec3 vert(std::cos(lon) * std::cos(lat), std::sin(lon) * std::cos(lat), std::sin(lat));
                vpoint.push_back(vert);
                vnormal.push_back(vert);
                if (i > 0 && j > 0) {
                    int v0 = n;
                    int v1 = v0 - 1;
                    int v2 = v0 - segments - 1;
                    int v3 = v1 - segments - 1;
                    triangles.push_back(v0);
                    triangles.push_back(v1);
                    triangles.push_back(v2);
                    triangles.push_back(v1);
                    triangles.push_back(v2);
                    triangles.push_back(v3);
                }
                n++;
            }
        }

        sphere.set_vpoint(vpoint);
        sphere.set_vnormal(vnormal);
        sphere.set_triangles(triangles);
    }

    { // Cone generation
        std::vector<float> vtheta;
        std::vector<float> vt;
        std::vector<unsigned int> triangles;

        for (int i = 0;i <= segments;i++) {
            Scalar theta = 2 * PI * ((Scalar)i) / segments;
            vtheta.push_back(theta);
            vt.push_back(0);
            vtheta.push_back(theta);
            vt.push_back(1);
            if (i > 0) {
                int v0 = 1 + 2 * i;
                int v1 = v0 - 1;
                int v2 = v0 - 2;
                int v3 = v0 - 3;
                triangles.push_back(v0);
                triangles.push_back(v1);
                triangles.push_back(v2);
                triangles.push_back(v1);
                triangles.push_back(v2);
                triangles.push_back(v3);
            }
        }

        cone.set_vbo<float>("vtheta", vtheta);
        cone.set_vbo<float>("vt", vt);
        cone.set_triangles(triangles);
    }

    rebuild();
}

void SphereMeshRenderer::render(const RenderContext &context) {

    sphere_shader.bind();

    sphere.set_attributes(sphere_shader);
    update_shader(sphere_shader, context);

    sphere.draw_instanced(sphere_count);

    sphere_shader.unbind();

    cone_shader.bind();

    cone.set_attributes(cone_shader);
    update_shader(cone_shader, context);

    cone.draw_instanced(cone_count);

    cone_shader.unbind();

    triangle_shader.bind();

    triangle.set_attributes(triangle_shader);
    update_shader(triangle_shader, context);

    triangle.draw();

    triangle_shader.unbind();


}

void SphereMeshRenderer::rebuild() {

    build_shader(sphere_shader, sphere_vshader(), sphere_fshader());
    sphere_shader.bind();
    sphere.set_attributes(sphere_shader);
    sphere_shader.unbind();

    build_shader(cone_shader, cone_vshader(), cone_fshader());
    cone_shader.bind();
    cone.set_attributes(cone_shader);
    cone_shader.unbind();

    build_shader(triangle_shader, triangle_vshader(), triangle_fshader());
    triangle_shader.bind();
    triangle.set_attributes(triangle_shader);
    triangle_shader.unbind();

}

void SphereMeshRenderer::upload_mesh(const SphereMesh &mesh) {

    auto vpoint = mesh.get_vertex_property<Vec4>("v:point");

    sphere_count = 0;
    std::vector<Vec4> vsphere;
    for (auto sphere : mesh.spheres()) {
        vsphere.push_back(vpoint[mesh.vertex(sphere)]);
        sphere_count++;
    }
    for (auto edge : mesh.edges()) {
        vsphere.push_back(vpoint[mesh.vertex(edge, 0)]);
        vsphere.push_back(vpoint[mesh.vertex(edge, 1)]);
        sphere_count += 2;
    }
    for (auto face : mesh.faces()) {
        vsphere.push_back(vpoint[mesh.vertex(face, 0)]);
        vsphere.push_back(vpoint[mesh.vertex(face, 1)]);
        vsphere.push_back(vpoint[mesh.vertex(face, 2)]);
        sphere_count += 3;
    }

    sphere.set_vbo<Vec4>("vsphere", vsphere, 1);


    cone_count = 0;
    std::vector<Vec4> vcone0;
    std::vector<Vec4> vcone1;
    for (auto edge : mesh.edges()) {
        vcone0.push_back(vpoint[mesh.vertex(edge, 0)]);
        vcone1.push_back(vpoint[mesh.vertex(edge, 1)]);
        cone_count++;
    }
    for (auto face : mesh.faces()) {
        vcone0.push_back(vpoint[mesh.vertex(face, 0)]);
        vcone1.push_back(vpoint[mesh.vertex(face, 1)]);
        vcone0.push_back(vpoint[mesh.vertex(face, 1)]);
        vcone1.push_back(vpoint[mesh.vertex(face, 2)]);
        vcone0.push_back(vpoint[mesh.vertex(face, 2)]);
        vcone1.push_back(vpoint[mesh.vertex(face, 0)]);
        cone_count += 3;
    }

    cone.set_vbo<Vec4>("v0", vcone0, 1);
    cone.set_vbo<Vec4>("v1", vcone1, 1);


    unsigned int element_count = 0;
    std::vector<Vec3> vtri_point, vtri_normal;
    std::vector<unsigned int> tri_elements;
    for (auto face : mesh.faces()) {

        SphereMesh::Vertex v0, v1, v2;

        v0 = mesh.vertex(face, 0);
        v1 = mesh.vertex(face, 1);
        v2 = mesh.vertex(face, 2);

        Vec4 s0 = vpoint[v0];
        Vec4 s1 = vpoint[v1];
        Vec4 s2 = vpoint[v2];
        Vec3 p0 = s0.block<3, 1>(0, 0);
        Vec3 p1 = s1.block<3, 1>(0, 0);
        Vec3 p2 = s2.block<3, 1>(0, 0);
        Scalar r0 = s0(3);
        Scalar r1 = s1(3);
        Scalar r2 = s2(3);

        Vec3 tn = wedge_normal(s0, s1, s2, 0);

        // tangent point 0
        Vec3 t0 = p0 + r0 * tn;

        // tangent point 1
        Vec3 t1 = p1 + r1 * tn;

        // tangent point 2
        Vec3 t2 = p2 + r2 * tn;

        vtri_point.push_back(t0);
        vtri_normal.push_back(tn);
        vtri_point.push_back(t1);
        vtri_normal.push_back(tn);
        vtri_point.push_back(t2);
        vtri_normal.push_back(tn);
        tri_elements.push_back(element_count++);
        tri_elements.push_back(element_count++);
        tri_elements.push_back(element_count++);

        tn = wedge_normal(s0, s1, s2, 1);

        // tangent point 0
        t0 = p0 + r0 * tn;

        // tangent point 1
        t1 = p1 + r1 * tn;

        // tangent point 2
        t2 = p2 + r2 * tn;

        vtri_point.push_back(t0);
        vtri_normal.push_back(tn);
        vtri_point.push_back(t1);
        vtri_normal.push_back(tn);
        vtri_point.push_back(t2);
        vtri_normal.push_back(tn);
        tri_elements.push_back(element_count++);
        tri_elements.push_back(element_count++);
        tri_elements.push_back(element_count++);

    }

    triangle.set_vbo<Vec3>("vposition", vtri_point);
    triangle.set_vbo<Vec3>("vnormal", vtri_normal);
    triangle.set_triangles(tri_elements);

}

//=============================================================================
} // OpenGP::
//=============================================================================
