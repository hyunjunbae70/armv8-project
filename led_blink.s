movz x0, #0x3F20, lsl #16
movz w1, #0x01C0
movz w2, #0x0040

ldr w3, [x0]
bic w3, w3, w1
orr w3, w3, w2
str w3, [x0]

loop:
    movz x4, #0x3F20, lsl #16
    add x4, x4, #0x1C
    movz w5, #0x4

    str w5, [x4]

    movz x6, #0x0073, lsl #16
on_time:
    subs x6, x6, #1
    b.ne on_time
    movz w7, #0x4

    movz x8, #0x3F20, lsl #16
    add x8, x8, #0x28

    str w7, [x8]

    movz x9, #0x0073, lsl #16   
off_time:
    subs w9, w9, #1
    b.ne off_time
    b loop
