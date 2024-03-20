const { findByIds,WebUSBDevice ,usb} = require('usb');
const bmp = require('bmp-js');
const fs = require("fs")
const Demosaic = require('demosaic');
 

let data = Buffer.alloc(640*480*4);

let device = findByIds(0x20B1, 0x1DE0);
async function transfer()
{
    let web = new WebUSBDevice(device);
    web.open();
    device.interfaces[0].claim();
    device.interfaces[1].claim();
    let epIn = device.interfaces[0].endpoints[0];
    let image = [[],[]];
    let bufCount = 1;
    let successCount = 0;
    let failCount = 0;
    let totalCount = 0;
    let line = 0;
    let cout = 0;
    let time = Date.now();
    let imageIdx = 0;
    let imageWidth = 640;
    let imageHeight = 480;
    function encodeData()
    {
        let idx = 1-imageIdx;
        
        let rgb = Demosaic.nearestNeighbour({data: Buffer.from(image[idx]), height:imageHeight, width:imageWidth, bayer:Demosaic.Bayer.RGGB});
        
        for(let i=0,count=imageWidth*imageHeight;i<count;i++)
        {
            // let g = image[idx][i]&0xff;
            let n = i<<2;
            data[n+1] = rgb[i*3];
            data[n+2] = rgb[i*3+1];
            data[n+3] = rgb[i*3+2];
        }
        console.time("time");
        let img = bmp.encode({data,width:imageWidth,height:imageHeight});
        fs.writeFileSync("tmp.bmp",img.data);
        console.timeEnd("time");
    }
    // setInterval(()=>{
    // },1000);
    let imageX = (640-imageWidth)/2+6;
    let imageY = (480-imageHeight)/2;
    web.transferOut(2,Buffer.from([1,1,imageX>>8,imageX&0xff,imageY>>8,imageY&0xff,imageWidth>>8,imageWidth&0xff,imageHeight>>8,imageHeight&0xff])).then((res)=>{
        console.log(res);
    });
    web.transferOut(2,Buffer.from([1,2,0x00,imageWidth==640?0x80:0x00])).then((res)=>{
        console.log(res);
    });
    web.transferOut(2,Buffer.from([1,3,0x0,0x10])).then((res)=>{
        console.log(res);
    });
    web.transferOut(2,Buffer.from([1,4,0x15,0x12,0x17])).then((res)=>{
        console.log(res);
    });
    web.transferOut(2,Buffer.from([1,5,0x20])).then((res)=>{
        console.log(res);
    });
    web.transferOut(2,Buffer.from([1,7,16])).then((res)=>{
        console.log(res);
    });
    web.transferOut(2,Buffer.from([1,8,1,1,0])).then((res)=>{
        console.log(res);
    });
    web.transferOut(2,Buffer.from([1,0,0x1])).then((res)=>{
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
            if(line>10)
            {
                let t = Date.now()-time;
                console.log("encode:",t/1000, line);
                time = Date.now();
                imageIdx = 1 - imageIdx;

                if(line>imageHeight-1)
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
            // for(let i=0;i<640*480;i++)
            // {
            //     image[imageIdx][i] = 0xff;
            // }
            return;
        }
        if(buffer.length>0)
        {
            for(let i=0;i<buffer.length;i++)
            {
                image[imageIdx][line*imageWidth+cout+i] = buffer[i];
                totalCount++;
            }
            cout+=buffer.length;
            if(cout>=imageWidth)
            {
                cout = 0;
                line++;
            }
            if(line>=imageHeight)
            {
                start = false;
            }
        }
    });
}
transfer();