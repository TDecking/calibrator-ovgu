#include <icp/ICP.h>
#include <icp.h>

Eigen::Matrix4d iterative_closest_point(const std::vector<Eigen::Vector3d>& src, const std::vector<Eigen::Vector3d>& target) {
    std::vector<gs::Point*> src_conv;
    std::vector<gs::Point*> target_conv;

    // Allocate
    gs::Point* src_buf = new gs::Point[src.size()];
    gs::Point* target_buf= new gs::Point[target.size()];

    for (int i = 0; i < src.size(); i++) {
        Eigen::Vector3d p = src.at(i);
        gs::Point* p1 = src_buf + i;
        *p1 = gs::Point(p(0), p(1), p(2));
        src_conv.push_back(p1);
    }

    for (int i = 0; i < target.size(); i++) {
        Eigen::Vector3d p = target.at(i);
        gs::Point* p1 = target_buf + i;
        *p1 = gs::Point(p(0), p(1), p(2));
        target_conv.push_back(p1);
    }


    // Since the library merely performs the transformation
    // without returning a matrix or something comparable,
    // we need to determine it ourselves.
    // Observing the transformation applied to zero, we can
    // determine the translation. Unit vectors along the
    // cartesian axes determine the rotation.
    gs::Point inference[4] = {
        gs::Point(1.0, 0.0, 0.0),
        gs::Point(0.0, 1.0, 0.0),
        gs::Point(0.0, 0.0, 1.0),
        gs::Point(0.0, 0.0, 0.0)
    };

    src_conv.push_back(inference);
    src_conv.push_back(inference + 1);
    src_conv.push_back(inference + 2);
    src_conv.push_back(inference + 3);

    // Perform transformation
    gs::icp(src_conv, target_conv);

    // Release
    delete[] src_buf;
    delete[] target_buf;

    // Determine result
    inference[0] = inference[0] - inference[3];
    inference[1] = inference[1] - inference[3];
    inference[2] = inference[2] - inference[3];

    Eigen::Matrix4d result = Eigen::Matrix4d::Zero();

    result(3, 3) = 1.0;

    result(0, 0) = (double)inference[0].pos[0],
    result(1, 0) = (double)inference[0].pos[1],
    result(2, 0) = (double)inference[0].pos[2],

    result(0, 1) = (double)inference[1].pos[0],
    result(1, 1) = (double)inference[1].pos[1],
    result(2, 1) = (double)inference[1].pos[2],

    result(0, 2) = (double)inference[2].pos[0],
    result(1, 2) = (double)inference[2].pos[1],
    result(2, 2) = (double)inference[2].pos[2],

    result(0, 3) = (double)inference[3].pos[0];
    result(1, 3) = (double)inference[3].pos[1];
    result(2, 3) = (double)inference[3].pos[2];

    return result;
}