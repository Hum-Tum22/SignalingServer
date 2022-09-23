

#include "DeviceChannel.h"

void IDeviceChannel::updateDeviceChannel(IDeviceChannel* devchl)
{
    GBDeviceChannel* pGBchl = dynamic_cast<GBDeviceChannel*>(devchl);
    OnvifDeviceChannel* pOnvifDev = dynamic_cast<OnvifDeviceChannel*>(devchl);
    if (pGBchl)
    {
        /*auto iter = mDeviceMap.find(pGBDev->getDeviceId());
        if (iter != mDeviceMap.end())
            return iter->second;*/
    }
}