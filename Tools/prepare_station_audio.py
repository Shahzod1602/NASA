"""Synthetic suit-radio switching sounds (no outdoor airborne audio)."""
from pathlib import Path
import numpy as np,wave
p=Path(__file__).resolve().parents[1]/'SourceArt/Immersion'
p.mkdir(exist_ok=True,parents=True)
sr=24000
rng=np.random.default_rng(733)
def write(name,x):
    with wave.open(str(p/(name+'.wav')),'wb') as w:
        w.setparams((1,2,sr,len(x),'NONE','not compressed'))
        w.writeframes((np.clip(x,-.95,.95)*32767).astype('<i2').tobytes())
def pulse(t,t0,f,volume=.22):
    d=t-t0
    return np.where(d>=0,np.sin(2*np.pi*f*d)*np.exp(-d*32),0)*volume
t=np.arange(int(sr*.65))/sr
click=pulse(t,.05,210,.3)+pulse(t,.22,330,.23)+pulse(t,.4,160,.2)
write('PowerContact',click)
t=np.arange(int(sr*4.5))/sr
static=rng.normal(0,.015,len(t))
static=np.convolve(static,np.ones(8)/8,'same')
seq=pulse(t,.08,620,.14)+pulse(t,.74,760,.14)+pulse(t,1.4,910,.14)+pulse(t,2.1,1120,.14)
write('PowerSequence',static+seq)
t=np.arange(int(sr*1.2))/sr
chime=np.sin(2*np.pi*620*t)*np.exp(-t*3)*.14+np.sin(2*np.pi*930*t)*np.exp(-t*4)*.11
write('PowerReady',chime)
print('POWER_AUDIO_READY')
