#include <utils.h>
#include <math.h>

Eigen::Matrix4d make_matrix(double x_rotation, double y_rotation, double z_rotation, double x_translation, double y_translation, double z_translation) {
    double crx = cos(x_rotation);
    double srx = sin(x_rotation);
    Eigen::Matrix4d rx {
        { 1.0, 0.0, 0.0, 0.0 },
        { 0.0, crx, -srx, 0.0 },
        { 0.0, srx, crx, 0.0 },
        { 0.0, 0.0, 0.0, 1.0 }
    };

    double cry = cos(y_rotation);
    double sry = sin(y_rotation);
    Eigen::Matrix4d ry {
        { cry, 0.0, -sry, 0.0 },
        { 0.0, 1.0, 0.0, 0.0 },
        { sry, 0.0, cry, 0.0 },
        { 0.0, 0.0, 0.0, 1.0 }
    };

    double crz = cos(z_rotation);
    double srz = sin(z_rotation);
    Eigen::Matrix4d rz {
        { crz, -srz, 0.0, 0.0 },
        { srz, crz, 0.0, 0.0 },
        { 0.0, 0.0, 1.0, 0.0 },
        { 0.0, 0.0, 0.0, 1.0 }
    };

    Eigen::Matrix4d t {
        { 1.0, 0.0, 0.0, x_translation },
        { 0.0, 1.0, 0.0, y_translation },
        { 0.0, 0.0, 1.0, z_translation },
        { 0.0, 0.0, 0.0, 1.0 }
    };

    Eigen::Matrix4d result = rx * ry * rz * t;

    return result;
}
