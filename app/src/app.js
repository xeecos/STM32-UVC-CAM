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
async function getImage()
{
    return new Promise(resolve=>{

        let endpoint = device.interfaces[1].endpoint(0x81);
        console.log(endpoint)
        // endpoint.startPoll(3,64);
        // endpoint.on("data", function(data) { 
        //     console.log(data);
        // });
        resolve();
    })
}
async function transfer()
{
    // device.detatchKernelDriver();
    let dev = await WebUSBDevice.createInstance(device);
    dev.open();
    // device.detatchKernelDriver();
    // console.log("open:",device);
    // for(let i=0;i<device.interfaces.length; i++)
    // {
    //     // console.log(interface);
    // }

    // console.log(dev.configurations[0].interfaces[1].alternates[1].endpoints);
    dev.claimInterface(1).then(res=>{
        dev.selectAlternateInterface(1,0).then(async (res)=>{
            for(let i=0;i<1;i++)
            {
                await getImage();
                // await getImage();
                // let r = await dev.transferIn(1,64);
                // console.log(r.data);
                
                // dev.transferIn(1,64).then(res=>{
                //     console.log(res.data);
                // })
            }
        })
        
    });
    // dev.claimInterface(1).then(res=>{
    //     dev.selectAlternateInterface(1,1);
    // });
    // dev.controlTransferIn({
    //     requestType:"class",
    //     recipient:"interface",
    //     request:0x01,
    //     value:0x0001,
    //     index:1},64).then(res=>{
    //         console.log(res);
    //     });
    // dev.selectAlternateInterface(1,0).then(res=>{
    //     console.log(res);
    // })
    // dev.transferOut(1, "hello").then(res=>{
    //         console.log(res);
    //     });
    // dev.transferIn(0x81, 176).then(res=>{
    //     console.log(res);
    // })
    // let outEndpoint = endpoints[0];
    // let inEndpoint = endpoints[1];
    // console.log(device.interfaces[0])
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