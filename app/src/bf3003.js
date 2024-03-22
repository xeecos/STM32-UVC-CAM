const { findByIds, WebUSBDevice } = require('usb');
const EventEmitter = require('node:events');
class BF3003
{
    constructor()
    {
        this._width = 640;
        this._height = 480;
        this._epOut = 2;
        this._emitter = new EventEmitter();
    }
    async setup(width=640,height=480)
    {
        let device = findByIds(0x20B1, 0x1DE0);
        if(!device)return 1;
        this._usb = new WebUSBDevice(device);
        this._usb.open();

        device.interfaces[0].claim();
        device.interfaces[1].claim();
        
        await this.setFrameSize(width,height);
        await this.setMode(1,1,1);
        await this.setDummy(this._width==640?0x40:0x00);
        await this.setExposure(0x10);
        await this.setFrequency(16);

        let debug = {time:Date.now(),successCount:0,failCount:0,enable:true};

        let line = 0;
        let imageIdx = 0;
        let imageBuf = [new Array(640*480),new Array(640*480)];
        let lineCount = 0;

        let epIn = device.interfaces[0].endpoints[0];
        epIn.startPoll(1,64);
        epIn.on("data", (buffer)=>{ 
            if(buffer.length==2)
            {
                if(line>1)
                {
                    this.emit("data",{line,data:imageBuf[imageIdx]});
                    imageIdx = 1 - imageIdx;

                    if(debug.enable)
                    {
                        let t = Date.now()-debug.time;
                        console.log("encode:",t/1000, line);
                        debug.time = Date.now();
                        if(line>this._height-1)
                        {
                            debug.successCount++;
                        }
                        else
                        {
                            debug.failCount++;
                        }
                        console.log("percent:",(debug.successCount/(debug.successCount+debug.failCount)*100).toFixed(2),"success:", debug.successCount, " fail:", debug.failCount);
                    }
                }
                line = 0;
                lineCount = 0;
                return;
            }
            if(buffer.length>0)
            {
                for(let i=0;i<buffer.length;i++)
                {
                    imageBuf[imageIdx][line*this._width+lineCount+i] = buffer[i];
                }
                lineCount+=buffer.length;
                if(lineCount>=this._width)
                {
                    lineCount = 0;
                    line++;
                }
            }
        });
        return 0;
    }
    get width()
    {
        return this._width;
    }
    get height()
    {
        return this._height;
    }
    emit(type,data)
    {
        this._emitter.emit(type, data);
    }
    on(type,callback)
    {
        this._emitter.on(type, callback);
    }
    setWindow(x,y,w,h)
    {
        return new Promise(resolve=>{
            this._width = w;
            this._height = h;
            this._usb.transferOut(this._epOut,Buffer.from([1,1,x>>8,x&0xff,y>>8,y&0xff,w>>8,w&0xff,h>>8,h&0xff])).then((res)=>{
                resolve(res);
            });
        })
    }
    setDummy(dummy=0x0)
    {
        return new Promise(resolve=>{
            this._usb.transferOut(this._epOut,Buffer.from([1,2,0x00,dummy])).then((res)=>{
                resolve(res);
            });
        })
    }
    setFrameSize(w,h)
    {
        let x = (640-w)/2+6;
        let y = (480-h)/2;
        return this.setWindow(x, y, w, h);
    }
    setExposure(exp=0x10)
    {
        return new Promise(resolve=>{
            this._usb.transferOut(this._epOut,Buffer.from([1,3,exp>>8,exp&0xff])).then((res)=>{
                resolve(res);
            });
        })
    }
    setGain(red=0x15, green=0x12, blue=0x17)
    {
        return new Promise(resolve=>{
            this._usb.transferOut(this._epOut,Buffer.from([1,4,red,green,blue])).then((res)=>{
                resolve(res);
            });
        })
    }
    setMode(gain=0,colorbalance=0,exposure=0)
    {
        return new Promise(resolve=>{
            this._usb.transferOut(this._epOut,Buffer.from([1, 8, gain, colorbalance, exposure])).then((res)=>{
                resolve(res);
            })
        });
    }
    setFrequency(freq=16)
    {
        return new Promise(resolve=>{
            this._usb.transferOut(this._epOut,Buffer.from([1,7,freq])).then((res)=>{
                resolve(res);
            });
        })
    }
    start()
    {
        return new Promise(resolve=>{
            console.log("start")
            this._usb.transferOut(this._epOut, Buffer.from([1,0,0x1])).then((res)=>{
                resolve(res);
            });
        });
    }
    stop()
    {
        return new Promise(resolve=>{
            this._usb.transferOut(this._epOut, Buffer.from([1,0,0x0])).then((res)=>{
                resolve(res);
            });
        });
    }
}
module.exports = new BF3003();