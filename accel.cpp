#include "common.h"

#include <windows.h>
#if DEBUG
#include <tchar.h>
#include <stdio.h>
#endif

bool accelDisabled = false;
INT mouseTresholds[3];
INT modifiedTresholds[3];

void saveAccel()
{
    SystemParametersInfo(SPI_GETMOUSE, 0, &mouseTresholds[0], SPIF_SENDCHANGE);
    modifiedTresholds[0] = mouseTresholds[0];
    modifiedTresholds[1] = mouseTresholds[1];
    modifiedTresholds[2] = 0;
    DBG_OUTT("saved");
}

void enableAccel()
{
    modifiedTresholds[2] = 1;
    SystemParametersInfo(SPI_SETMOUSE, 0, &modifiedTresholds[0], 0);
    modifiedTresholds[2] = 0;
    accelDisabled = false;
    DBG_OUTT("enabled");
}

void disableAccel()
{
    SystemParametersInfo(SPI_SETMOUSE, 0, &modifiedTresholds[0], 0);
    accelDisabled = true;
    DBG_OUTT("disabled");
}

void resetAccel()
{
    SystemParametersInfo(SPI_SETMOUSE, 0, &mouseTresholds[0], 0);
    accelDisabled = false;
    DBG_OUTT("reset");
}
