#ifndef GLOBAL_H
#define GLOBAL_H

#include "devicecontroller.h"
#include "catchcontroller.h"
#include "timesynchandler.h"
#include "logfilehandler.h"
#include "deviceinfo.h"
#include "bluetoothbaseclass.h"
#include "ble_uart_types.h"
#include "deviceinterface.h"

#define VERBOSITY_LEVEL                     0
#define PRINT_THROUGHPUT                    1
#define WAIT_X_MS_BETWEEN_CHUNKS            250
#define CHANGE_CONN_PARAM_OF_OTHER_DEVICES  0
#define LOG_TRANSF_DATA_TO_FIL              0
#define PLOT_DATA                           1
#define ALLOW_WRITE_TO_FILE                 1
#define WRITE_BERNHARD_INFO_TO_LOG_FILE     1

#endif // GLOBAL_H
