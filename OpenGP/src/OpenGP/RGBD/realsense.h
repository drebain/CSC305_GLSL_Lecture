// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include <OpenGP/RGBD/Stream.h>

#ifdef OPENGP_REALSENSE1
#include <librealsense/rs.hpp>
#else
#include <librealsense2/rs.hpp>
#endif

//=============================================================================
namespace OpenGP {
//=============================================================================

#ifdef OPENGP_REALSENSE1
    
inline SensorDevice get_realsense_device() {

    std::shared_ptr<rs::context> ctx(new rs::context());
    printf("There are %d connected RealSense devices.\n", ctx->get_device_count());

    std::unordered_map<std::string, SensorStream> streams;

    std::shared_ptr<std::unordered_map<std::string, const void*>> stream_data_ptrs(new std::unordered_map<std::string, const void*>());

    rs::device* dev = ctx->get_device(0);

    std::vector<rs::stream> supported_streams;
    /*for (int i = (int)rs::capabilities::depth; i <= (int)rs::capabilities::fish_eye; i++) {
    if (dev->supports((rs::capabilities)i)) {
    supported_streams.push_back((rs::stream)i);
    }
    }*/
    supported_streams.push_back(rs::stream::depth);

    /*for (auto & stream : supported_streams) {
    dev->enable_stream(stream, rs::preset::best_quality);
    }*/
    dev->enable_stream(rs::stream::depth, 640, 480, rs::format::z16, 60);

    for (auto & stream : supported_streams) {
        if (!dev->is_stream_enabled(stream)) continue;
        auto intrin = dev->get_stream_intrinsics(stream);
        float framerate = dev->get_stream_framerate(stream);

        float d = dev->get_depth_scale();

        StreamIntrinsics intrinsics;
        StreamExtrinsics extrinsics{}; // TODO

        auto vfov = intrin.hfov();

        intrinsics.focal_length = { intrin.fx, intrin.fy };
        intrinsics.pixel_center = { intrin.ppx, intrin.ppy };
        intrinsics.width = intrin.width;
        intrinsics.height = intrin.height;

        std::string name = rs_stream_to_string((rs_stream)stream);

        (*stream_data_ptrs)[name] = nullptr;
        streams.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(name.c_str(), const_cast<const void**>(&stream_data_ptrs->at(name)), intrinsics, extrinsics, framerate)
        );
    }

    try {
        dev->start();
    } catch (rs::error e) {
        puts(e.what());
        throw e;
    }

    return SensorDevice(dev->get_depth_scale(), streams, [=](bool block) {
        ctx;
        if (block) {
            dev->wait_for_frames();
        } else if (!dev->poll_for_frames()) {
            return false;
        }

        for (auto stream : supported_streams) {
            if (!dev->is_stream_enabled(stream)) continue;
            auto name = rs_stream_to_string((rs_stream)stream);
            (*stream_data_ptrs)[name] = const_cast<const void*>(dev->get_frame_data(stream));
        }

        return true;

    });
}

#else

inline SensorDevice get_realsense_device() {

    rs2::pipeline pipeline;

    pipeline.start();

    std::unordered_map<std::string, SensorStream> streams;

    std::shared_ptr<std::unordered_map<std::string, const void*>> stream_data_ptrs(new std::unordered_map<std::string, const void*>());

    for (auto profile : pipeline.get_active_profile().get_streams()) {

        StreamIntrinsics intrinsics;
        StreamExtrinsics extrinsics{}; // TODO

        auto video_stream = profile.as<rs2::video_stream_profile>();
        rs2_intrinsics rs2_int = video_stream.get_intrinsics();
        intrinsics.focal_length = { rs2_int.fx, rs2_int.fy };
        intrinsics.pixel_center = { rs2_int.ppx, rs2_int.ppy };
        intrinsics.width = rs2_int.width;
        intrinsics.height = rs2_int.height;

        auto name = profile.stream_name();

        (*stream_data_ptrs)[name] = nullptr;
        streams.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(name),
            std::forward_as_tuple(name.c_str(), const_cast<const void**>(&stream_data_ptrs->at(name)), intrinsics, extrinsics)
        );

    }

    return SensorDevice(streams, [=](bool block) {

        rs2::frameset data;

        if (block) {
            data = pipeline.wait_for_frames();
        } else if (!pipeline.poll_for_frames(&data)) {
            return false;
        }

        for (auto frame : data) {
            auto name = frame.get_profile().stream_name();
            (*stream_data_ptrs)[name] = const_cast<const void*>(frame.get_data());
        }

        return true;

});

#endif

//=============================================================================
} // OpenGP::
//=============================================================================
