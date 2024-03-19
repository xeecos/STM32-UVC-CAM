const { findByIds,WebUSBDevice ,usb} = require('usb');
const bmp = require('bmp-js');
const fs = require("fs")

let data = Buffer.alloc(640*480*4);

let device = findByIds(0x20B1, 0x1DE0);
async function transfer()
{
    let web = new WebUSBDevice(device);
    web.open();
    device.interfaces[0].claim();
    device.interfaces[1].claim();
    let epIn = device.interfaces[0].endpoints[0];
    // let epOut = device.interfaces[0].endpoints[1];
    // console.log(device.interfaces[0].endpoints);
    let image = [[],[]];
    let bufCount = 1;
    let successCount = 0;
    let failCount = 0;
    let totalCount = 0;
    let line = 0;
    let cout = 0;
    let time = Date.now();
    let imageIdx = 0;
    for(let i=0,count=640*480*4;i<count;i++)
    {
        data[i] = 0xff;
    }
    function encodeData()
    {
        let idx = 1-imageIdx;
        for(let i=0,count=640*480;i<count;i++)
        {
            let g = image[idx][i]&0xff;
            let n = i<<2;
            data[n+1] = g;
            data[n+2] = g;
            data[n+3] = g;
        }
        console.time("time");
        let img = bmp.encode({data,width:640,height:480});
        fs.writeFileSync("tmp.bmp",img.data);
        console.timeEnd("time");
    }
    // setInterval(()=>{
    // },1000);
    
    web.transferOut(2,Buffer.from([1,1,0x0,0x10])).then((res)=>{
    });
    web.transferOut(2,Buffer.from([1,2,0x15,0x19,0x15])).then((res)=>{
        console.log(res);
    });
    web.transferOut(2,Buffer.from([1,3,0x10])).then((res)=>{
        console.log(res);
    });
    // return;
    epIn.startPoll(1,bufCount*64);
    // return;
    epIn.on("data", function(buffer) { 
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
            if(line>100)
            {
                let t = Date.now()-time;
                console.log("encode:",t/1000, line);
                time = Date.now();
                imageIdx = 1 - imageIdx;

                if(line>479)
                {
                    successCount++;
                    encodeData();
                }
                else
                {
                    failCount++;
                }
                console.log("percent:",(successCount/(successCount+failCount)*100).toFixed(2),"success:", successCount, " fail:", failCount)
            }
            start = true;
            line = 0;
            cout = 0;
            totalCount = 0;
            for(let i=0;i<640*480;i++)
            {
                image[imageIdx][i] = 0xff;
            }
            return;
        }
        if(buffer.length>0)
        {
            for(let i=0;i<buffer.length;i++)
            {
                image[imageIdx][line*640+cout+i] = buffer[i];
                totalCount++;
            }
            cout+=buffer.length;
            if(cout>=640)
            {
                cout = 0;
                line++;
            }
            if(line>=480)
            {
                start = false;
            }
        }
    });
}
transfer();