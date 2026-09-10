# VOR Python test host

The same framed protocol is used by the VS simulator and the AT32 USB CDC port.
The simulator listens on `127.0.0.1:28730`; no virtual COM driver is required.

```powershell
python tools/vor_host/vor_host.py --transport tcp status
python tools/vor_host/vor_host.py --transport tcp load tools/vor_host/example_tasks.json
python tools/vor_host/vor_host.py --transport tcp start
python tools/vor_host/vor_host.py --transport tcp stop
```

For hardware, install `pyserial` and select the enumerated USB CDC port:

```powershell
python -m pip install pyserial
python tools/vor_host/vor_host.py --transport serial --port COM5 status
```

Task replacement is transactional: the previous list remains active until every
new item is accepted and `ReplaceCommit` succeeds. No heartbeat is sent, and a
serial/TCP disconnect does not stop an experiment. Screen and host controls are
both active; either can issue STOP.
