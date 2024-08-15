# RBMC CFAM Daemon

This application interfaces with CFAM-S chips on IBM's redundant BMC system:

1. It writes scratchpad registers on the CFAM-S off of the local BMC.
2. It reads the scratchpad registers from the sibling BMC and puts the data on
   D-Bus.
