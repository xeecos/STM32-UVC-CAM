const { findByIds,WebUSBDevice ,usb} = require('usb');


let device = findByIds(0x20B1, 0x1DE0);//046d:0825

// let device;//= findByIds(0x046d, 0x0825);
// const devices = getDeviceList();

// for (const d of devices) {
//     if(d.deviceDescriptor.idProduct==0x825)
//     {
//         device = d;
//     }
// }
// async function createDevice()
// {
//     const webDevice = await WebUSBDevice.createInstance(device);

//     if (webDevice) {
//         console.log(webDevice); // WebUSB device
//     }
// }
// createDevice();
// (async () => {
//     // Uses a blocking call, so is async

//     // Uses blocking calls, so is async
//     const webDevice = await WebUSBDevice.createInstance(device);

//     if (webDevice) {
//         webDevice.open();
//         console.log(webDevice.configurations[0].interfaces[1].alternates[1].endpoints); // WebUSB device
//         webDevice.transferIn(1,176).then(res=>{
//             console.log(res);
//         })
//     }
// })();
async function getImage(endpoint)
{
    return new Promise(resolve=>{
        endpoint.transfer(64,(err,data)=>{
            resolve(data);
        })
    })
}
async function transfer()
{
    device.open();
    device.interfaces[0].claim();
    let endpoint = device.interfaces[0].endpoints[0];
    // endpoint.startPoll();
    // endpoint.on("data", function(data) { 
    //     console.log(data);
    // });
    for(let i=0;i<10;i++)
    {
        console.log(await getImage(endpoint));
    }
}
transfer();
// console.log(0x41f1&0x80)
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