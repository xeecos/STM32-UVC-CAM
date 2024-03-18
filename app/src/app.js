const { findByIds,WebUSBDevice ,usb} = require('usb');
const bmp = require('bmp-js');
const fs = require("fs")

let data = Buffer.alloc(640*480*4);

let device = findByIds(0x20B1, 0x1DE0);
async function transfer()
{
    device.open();
    device.interfaces[0].claim();
    let endpoint = device.interfaces[0].endpoints[0];
    let image = [[],[]];
    let start = false;
    let bufCount = 1;
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
    // encodeData();

    endpoint.startPoll(1,bufCount*64);
    let successCount = 0;
    let failCount = 0;
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
            if(line>100)
            {
                let t = Date.now()-time;
                console.log("encode:",t/1000, line);
                time = Date.now();
                imageIdx = 1 - imageIdx;

                if(line==480)
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
            return;
        }
        if(buffer.length>0)
        {
            for(let i=0;i<64*bufCount;i++)
            {
                image[imageIdx][line*640+cout+i] = buffer[i];
            }
            cout+=64*bufCount;
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