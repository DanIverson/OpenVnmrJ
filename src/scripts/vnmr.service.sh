[Unit]
Description=Acquisition Communication Daemon
ConditionPathExists=/vnmr/acqbin/acqpresent

[Service]
Type=simple
User=root
ExecStart=/vnmr/acqbin/ovjProcs
