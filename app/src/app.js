const bmp = require('bmp-js');
const fs = require("fs")
const Demosaic = require('demosaic');
const BF3003 = require("./bf3003");
// const Image = require("./jpeg.node");


BF3003.setup().then(
    (res)=>
    {
        let data = Buffer.alloc(640*480*4);
        BF3003.on("data",(res)=>
        {
            let width = BF3003.width;
            let height = BF3003.height;
            if(res.line!=height)return;
            console.time("time");
            // const rgbBuffer = Buffer.alloc(width * height * 3);
            // Image.demosaic(Buffer.from(res.data), width * height, rgbBuffer, 1, width, height);
            let rgb = Demosaic.bilinear({data: Buffer.from(res.data), height, width, bayer:Demosaic.Bayer.RGGB});
            console.timeEnd("time");
            for(let i=0,count=width*height;i<count;i++)
            {
                let n = i<<2;
                data[n] = 0xff;
                data[n+1] = rgb[i*3];
                data[n+2] = rgb[i*3+1];
                data[n+3] = rgb[i*3+2];
            }
            let img = bmp.encode({data,width,height});
            fs.writeFileSync("tmp.bmp",img.data);
        });

        BF3003.start();
    });