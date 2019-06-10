#include "timestampler.h"
#include "devicehandler.h"


// THREE PHASES

//    -> phase 1 send 25 Msgs with 10ms period with ts -> the device overwrites its own with received-5ms if received-5ms > own
//    -> phase 2 measure complete propagation delay. the pc sends 25 Msgs where the device sends its own ts.
//    -> phase 3 we send corrected timestamps..



void TimeStampler::time_sync_msg_arrived(QByteArray *msg)
{

}

TimeStampler::TimeStampler(QObject *parent): m_lastTimeStamp(0), m_device_handler_arr(0)
{

}
