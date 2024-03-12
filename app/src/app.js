const { findByIds, WebUSBDevice} = require('usb');


const device = findByIds(0x20B1, 0x1DE0);

// async function createDevice()
// {
//     const webDevice = await WebUSBDevice.createInstance(device);

//     if (webDevice) {
//         console.log(webDevice); // WebUSB device
//     }
// }
// createDevice();

async function transfer()
{
    device.open();
    // device.detatchKernelDriver();
    let interface = device.interfaces[0];
    interface.claim();

    // let outEndpoint = endpoints[0];
    // let inEndpoint = endpoints[1];
    console.log(device.interfaces[0])
}
transfer();
// console.log(device)
// let endpoints = device.interfaces[0].endpoints;
// console.log(device);
// const { Context, Device, DeviceHandle, LibUvc } = require("uvc");

// async function createDevice()
// {
//     const libuvc = new LibUvc();
//     await libuvc.initialize();
    
//     const context = new Context(libuvc);
//     await context.initialize();
    
//     const device = await context.findDevice(0x20B1, 0x1DE0);

//     await device.initialize();
    
//     const deviceHandle = await device.open();
//     await deviceHandle.initialize();
// }
// createDevice();