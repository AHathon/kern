.section .rodata
.balign 4
.global orbBmp
.global orbBmpEnd
.global iconsBmp
.global iconsBmpEnd
.global navBmp
.global navBmpEnd

orbBmp:
.incbin "resources/orb.bmp"
orbBmpEnd:

.balign 4
iconsBmp:
.incbin "resources/icons.bmp"
iconsBmpEnd:

.balign 4
navBmp:
.incbin "resources/nav.bmp"
navBmpEnd:
