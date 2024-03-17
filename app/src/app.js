const { findByIds,WebUSBDevice ,usb} = require('usb');
const {encode} = require("jpeg-js");
const fs = require("fs")

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
    let prevLine = 0;
    let lines = [];
    let linesCount = [];
    let start = false;
    let bufCount = 1;
    let line = 0;
    let cout = 0;
    let time = Date.now();
    endpoint.startPoll(1,bufCount*64);
    endpoint.on("data", function(buffer) { 
        // if(buffer.length==2)
        // {
        //     return;
        // }
        // cout+=buffer.length;
        // if(cout>=300*1024)
        // {
        //     let t = Date.now()-time;
        //     console.log(t,":",cout/1024/1024/(t/1000),cout);
        //     time = Date.now();
        //     cout = 0;

        // }
        // return;
        if(buffer.length==2)
        {
            start = true;
            line = 0;
            cout = 0;
            return;
        }
        if(buffer.length>0)
        {
            for(let i=0;i<64*bufCount;i++)
            {
                lines[line*640+cout+i] = buffer[i];
            }
            cout+=64*bufCount;
            if(cout>=640)
            {
                cout = 0;
                line++;
            }
            if(line==480)
            {
                let t = Date.now()-time;
                console.log("encode:",t/1000);
                time = Date.now();
                let data = [];
                for(let i=0,count=640*480;i<count;i++)
                {
                    let g = lines[i]&0xff;
                    data.push(g);
                    data.push(g);
                    data.push(g);
                    data.push(0xff);
                }
                let jpegData = encode({width:640, height:480,data},80);
                fs.writeFileSync("tmp.jpeg",jpegData.data);
                line = 0;
                start = false;
            }
        }
    });
    return;
    while(1)
    {
        let buf = await getImage(endpoint);
        if(buf.length>0)
        {
            let line = (buf[0]<<8)+buf[1];
            if(prevLine==479&&line==0)
            {
                if(lines.length > 307100)
                {
                    console.log("encode")
                    let data = [];
                    for(let i=0;i<480;i++)
                    {
                        console.log(linesCount[i]);
                    }
                    for(let i=0,count=640*480;i<count;i++)
                    {
                        let g = lines[i]&0xff;
                        data.push(g);
                        data.push(g);
                        data.push(g);
                        data.push(0xff);
                    }
                    let jpegData = encode({width:640, height:480,data},80);
                    fs.writeFileSync("tmp.jpeg",jpegData.data);
                }
                console.log(Date.now(),":",lines.length);
    
                start = true;
                lines = [];
                for(let i=0;i<480;i++){
                    linesCount[i] = 0;
                }
            }
            if(start)
            {
                for(let i=2;i<64;i++)
                {
                    if(linesCount[line]<640)
                    {
                        lines[line*640+linesCount[line]] = buf[i];
                        linesCount[line]++;
                    }
                }
            }
            prevLine = line;
        }
        // if(buf[1]>1)
        // {
        //     console.log(buf);
        // }
        // if(buf[0]==0x2)
        // {
        //     console.log("tail:",i);
        // }
        // if(buf[0]==0x1)
        // {
        //     console.log("head:",i);
        // }
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