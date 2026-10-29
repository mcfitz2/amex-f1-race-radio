import serial,time,sys
s=serial.Serial('/dev/cu.usbserial-8430',115200,timeout=1)
s.dtr=False; s.rts=True; time.sleep(0.1); s.rts=False
end=time.time()+float(sys.argv[1]); buf=b''
f=open(sys.argv[2],'w')
while time.time()<end:
    buf+=s.read(4096)
    while b'\n' in buf:
        l,buf=buf.split(b'\n',1); f.write(time.strftime('%H:%M:%S ')+l.decode(errors='replace')+'\n'); f.flush()
