StartMonitor:
    lea (monitorWelcome).l,a1
    bsr ConPuts
.1
    bsr ConGetblocking
    cmp.b #'U',d0
    beq MonitorUpload
    cmp.b #'M',d0
    beq MonitorMemory
    cmp.b #'B',d0
    beq MonitorBoot
    bra .1
MonitorUpload:
    bra *
MonitorMemory:
    bra *
MonitorBoot:
    bra *    

monitorWelcome:
    dc.b "[U]pload S-records [M]emory [B]oot from media: ",0
    even

    include console.asm