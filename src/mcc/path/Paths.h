#pragma once
#include "mcc/path/Config.h"
#include <string>

class QString;

namespace mccpath {

MCC_PATH_DECLSPEC QString qGetBinPath();
MCC_PATH_DECLSPEC QString qGetConfigPath();
MCC_PATH_DECLSPEC QString qGetDataPath();
MCC_PATH_DECLSPEC QString qGetLogsPath();
MCC_PATH_DECLSPEC QString qGetUiPath();
MCC_PATH_DECLSPEC QString qGetSoundsPath();

MCC_PATH_DECLSPEC std::string getBinPath();
MCC_PATH_DECLSPEC std::string getConfigPath();
MCC_PATH_DECLSPEC std::string getDataPath();
MCC_PATH_DECLSPEC std::string getLogsPath();
MCC_PATH_DECLSPEC std::string getUiPath();
MCC_PATH_DECLSPEC std::string getSoundsPath();
}
