.section .data
_container_start:
.ascii SERVICE_MAGIC                    //magic
.byte 0x00
.balign 8, 0x0
.dword _container_end - _container_start //container size
.dword _payload_end - _payload_start     //code size
.word 0x20                              //start offset
.balign 32, 0x0

_payload_start:
    .incbin SERVICE_BIN
_payload_end:
.balign 16, 0x0
_container_end: