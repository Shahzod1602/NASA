from pathlib import Path
import wave
import numpy as np
for path in (Path(__file__).resolve().parents[1]/'SourceArt/Mars').glob('Radio_*.wav'):
    with wave.open(str(path),'rb') as w:
        sr=w.getframerate();channels=w.getnchannels()
        a=np.frombuffer(w.readframes(w.getnframes()),'<i2').astype(float)/32768
    if channels>1:a=a.reshape(-1,channels).mean(axis=1)
    rate=24000
    a=np.interp(np.arange(int(len(a)*rate/sr))*sr/rate,np.arange(len(a)),a)
    f=np.fft.rfftfreq(len(a),1/rate)
    a=np.tanh(np.fft.irfft(np.fft.rfft(a)*((f>350)&(f<3300)),len(a))*2)*.65
    chirp=np.sin(2*np.pi*1150*np.arange(1800)/rate)*np.hanning(1800)*.12
    a=np.concatenate([chirp,np.zeros(1200),a,np.zeros(800),chirp])
    with wave.open(str(path),'wb') as w:
        w.setparams((1,2,rate,len(a),'NONE','not compressed'))
        w.writeframes((np.clip(a,-.95,.95)*32767).astype('<i2').tobytes())
print('Mars radio filtered')
