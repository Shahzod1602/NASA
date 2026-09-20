"""Original synthetic suit-conducted audio; local Windows speech for the radio."""
from pathlib import Path
import wave
import numpy as np
p=Path(__file__).resolve().parents[1]/'SourceArt/Immersion'
rng=np.random.default_rng(71)
rate=24000
def write(name,a):
    with wave.open(str(p/(name+'.wav')),'wb') as w:
        w.setparams((1,2,rate,len(a),'NONE','not compressed'))
        w.writeframes((np.clip(a,-.95,.95)*32767).astype('<i2').tobytes())
def noise(n,cut):
    a=rng.normal(size=n);f=np.fft.rfftfreq(n,1/rate)
    return np.fft.irfft(np.fft.rfft(a)*np.exp(-(f/cut)**4),n)
t=np.arange(rate*5)/rate
env=np.sin(np.pi*np.clip(t/1.7,0,1))**2*.25+np.sin(np.pi*np.clip((t-2.2)/2.1,0,1))**2*.19
write('SuitBreath',noise(len(t),1600)*env)
for i in range(3):
    t=np.arange(int(rate*.42))/rate
    a=np.sin(2*np.pi*(75+i*9)*t)*np.exp(-t*23)*.27+noise(len(t),650)*np.exp(-t*17)*.3
    a*=np.minimum(t/.006,1)
    write('SuitStep'+str(i),a)
t=np.arange(int(rate*.26))/rate
write('RadioChirp',np.sin(2*np.pi*1150*t)*np.sin(np.pi*np.clip(t/.12,0,1))**2*.1)
for path in p.glob('Radio_*.wav'):
    with wave.open(str(path),'rb') as w:
        sr=w.getframerate();channels=w.getnchannels();a=np.frombuffer(w.readframes(w.getnframes()),'<i2').astype(float)/32768
    if channels>1:a=a.reshape(-1,channels).mean(axis=1)
    a=np.interp(np.arange(int(len(a)*rate/sr))*sr/rate,np.arange(len(a)),a)
    f=np.fft.rfftfreq(len(a),1/rate)
    a=np.fft.irfft(np.fft.rfft(a)*((f>350)&(f<3300)),len(a))
    a=np.tanh(a*2)*.65
    chirp=np.sin(2*np.pi*1150*np.arange(1800)/rate)*np.hanning(1800)*.12
    write(path.stem,np.concatenate([chirp,np.zeros(1200),a,np.zeros(800),chirp]))
print('Immersion audio generated')
