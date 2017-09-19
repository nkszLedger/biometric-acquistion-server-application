#ifndef SHARED_H
#define SHARED_H

#include <QObject>

extern QString temp_path_global;
extern QString output_path_global;

enum biometricModality
{
    IRIS         = 1,
    FINGERPRINTS = 2,
    EAR2D        = 3,
    EAR3D        = 4,
    FOOTPRINTS   = 5,
    PALMPRINTS   = 6,
    MICROSCOPE   = 7
};


#endif // SHARED_H
