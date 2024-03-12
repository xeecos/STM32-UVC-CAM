const { getDeviceList } = require('usb');

const devices = getDeviceList();

for (const device of devices) {
    console.log(device); // Legacy device
}