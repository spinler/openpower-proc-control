# RBMC CFAM Daemon

This application interfaces with CFAM-S chips on IBM's redundant BMC system:

1. It writes scratchpad registers on the CFAM-S off of the local BMC.
2. It reads the scratchpad registers from the sibling BMC and puts the data on
   D-Bus.

## CFAM-S ScratchPad Register Layout

| Field            | Register | Start Bit | Length |
| ---------------- | -------- | --------- | ------ |
| API Version      | 1        | 0         | 8      |
| BMC Position     | 1        | 8         | 1      |
| Role             | 1        | 9         | 2      |
| Red Enabled      | 1        | 11        | 1      |
| Failovers Paused | 1        | 12        | 1      |
| Provisioned      | 1        | 13        | 1      |
| BMC State        | 1        | 14        | 3      |
| Sibling Comms OK | 1        | 17        | 1      |
| Heartbeat        | 1        | 24        | 8      |
| FW Version       | 2        | 0         | 32     |
