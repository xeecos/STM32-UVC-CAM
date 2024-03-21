const { Worker } = require('worker_threads');
const bmp = require('bmp-js');
const jpeg = require("jpeg-js");
const fs = require("fs")
const Demosaic = require('demosaic');
const sharedBuffer = new SharedArrayBuffer(640*480);
const worker = new Worker('./src/child.js', {workerData:sharedBuffer});

let buf = Buffer.from(sharedBuffer);
worker.on('message', (res)=>{
    let width = res.width, height = res.height;
    // if(!rgb)rgb = Buffer.alloc(width * height * 3);
    if(res.line!=res.height)return;
    console.time("time");
    let rgb = Demosaic.bilinear({data: Buffer.from(buf), width, height, bayer:Demosaic.Bayer.RGGB});
    let data = new Uint8Array(width * height * 4);
    for(let i=0,count=width*height;i<count;i++)
    {
        let n = i<<2;
        data[n] = rgb[i*3];
        data[n+1] = rgb[i*3+1];
        data[n+2] = rgb[i*3+2];
        data[n+3] = 0xff;
    }
    // let img = bmp.encode({data,width,height});
    let img = jpeg.encode({data,width,height},80);
    fs.writeFileSync("tmp.jpg",img.data);
    console.timeEnd("time");
});
worker.on('error', (err)=>{
    console.log(err);
});
worker.on('exit', (code) => {
    if (code !== 0)
        console.log(`stopped with  ${code} exit code`);
})
