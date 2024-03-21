const { workerData, parentPort } = require('worker_threads');
const BF3003 = require("./bf3003");
BF3003.setup().then((res)=>
    {
        let buf = new Uint8Array(workerData);
        BF3003.on("data",(res)=>
        {
            let width = BF3003.width;
            let height = BF3003.height;
            for(let i=0,count=width*height;i<count;i++)
            {
                buf[i] = res.data[i];
            }
            parentPort.postMessage({ line:res.line, width, height });
        });

        BF3003.start();
    });