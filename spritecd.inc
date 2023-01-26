; File: spritecd.inc
; Purpose: Sprite initialization and movement code

; offset allows using Y register to index both structures
OFFSET = SPRITE_X_LO - SPRITE_PATH_X_LO

;-------------------------------------------------------------------
; Initialize all sprite positions
;-------------------------------------------------------------------

; InitRing: initialize the sprite data for an entire ring.
;   path:    address of path point array
;   sprites: number of sprites on the path
;   index:   address of ZP start index into path point array
;
.macro InitRing path,sprites,index
    .scope
    lda     #<(path)            ; get path address (LO)
    sta     ZP_PATH_PTR_LO      ; set path address (LO)
    lda     #>(path)            ; get path address (HI)
    sta     ZP_PATH_PTR_HI      ; set path address (HI)

    stz     index               ; zero out index (LO)
    stz     index+1             ; zero out index (HI)

    ldx     #sprites            ; init loop counter
init_sprite:
    lda     #<SPRITE_BITMAP_ADDR_SHIFTED
    sta     VERA_data0

    lda     #(>SPRITE_BITMAP_ADDR_SHIFTED)|SPRITE_MODE_8_BPP
    sta     VERA_data0
 
    ldy     #0                  ; offset to path X,Y coordinate
    lda     (ZP_PATH_PTR),y     ; get X (LO)
    sta     VERA_data0          ; set X (LO)
 
    iny
    lda     (ZP_PATH_PTR),y     ; get X (HI)
    sta     VERA_data0          ; set X (HI)
 
    iny
    lda     (ZP_PATH_PTR),y     ; get Y (LO)
    sta     VERA_data0          ; set Y (LO)
 
    iny
    lda     (ZP_PATH_PTR),y     ; get Y (HI)
    sta     VERA_data0          ; set Y (HI)
 
    lda     #SPRITE_COLL_MASK_0|SPRITE_ZDEPTH_ABOVE_L1
    sta     VERA_data0

    lda     #SPRITE_HEIGHT_16_PIXELS|SPRITE_WIDTH_16_PIXELS
    sta     VERA_data0

    ; add size of 4 path point structures, to leave space between sprites
    lda     ZP_PATH_PTR_LO
    clc
    adc     #4*4
    sta     ZP_PATH_PTR_LO
    bcc     no_rollover
    inc     ZP_PATH_PTR_HI
no_rollover:

    dex                             ; done with this ring of sprites?
    bne     init_sprite             ; go if no
    .endscope
.endmacro

init_all_sprite_positions:
    SetupData0 VRAM_sprattr
    InitRing sprite_path_0_50_x_66,SPRITE_RING_COUNT_0,ZP_PATH_INDEX_0
    InitRing sprite_path_1_70_x_93,SPRITE_RING_COUNT_1,ZP_PATH_INDEX_1
    InitRing sprite_path_2_90_x_120,SPRITE_RING_COUNT_2,ZP_PATH_INDEX_2
    InitRing sprite_path_3_110_x_146,SPRITE_RING_COUNT_3,ZP_PATH_INDEX_3
    InitRing sprite_path_4_130_x_173,SPRITE_RING_COUNT_4,ZP_PATH_INDEX_4
    InitRing sprite_path_5_150_x_200,SPRITE_RING_COUNT_5,ZP_PATH_INDEX_5
    InitRing sprite_path_6_170_x_226,SPRITE_RING_COUNT_6,ZP_PATH_INDEX_6
    InitRing sprite_path_7_190_x_253,SPRITE_RING_COUNT_7,ZP_PATH_INDEX_7
    rts

;-------------------------------------------------------------------
; Update (move) all sprite positions
;-------------------------------------------------------------------

; MoveRing: move the sprite positions for an entire ring.
;   path:    address of path point array
;   end:     address after end of path point array
;   sprites: number of sprites on the path
;   index:   address of ZP start index into path point array
;   points:  number of X,Y coordinates in the path point array
;
.macro MoveRing path,end,sprites,index,points
    .scope
    ; increment the array index
    lda     index               ; get path array index (LO)
    inc                         ; advance to next index
    sta     index               ; set path array index (LO)
    bne     no_index_wrap       ; go if no overflow
    inc     index+1             ; update array index (HI)
no_index_wrap:

    ; check for end-of-ring
    cmp     #<(points)          ; compare next index (LO)
    bne     not_eor             ; go if not end-of-ring
    lda     index+1             ; get next index (HI)
    cmp     #>(points)          ; compare next index (HI)
    bne     not_eor             ; go if not end-of-ring
    stz     index               ; restart the index (LO)
    stz     index+1             ; restart the index (HI)
not_eor:

    ; multiply the path index by 4 (size of one X,Y point)
    lda     index+1             ; get index (HI)
    sta     ZP_TEMP_INDEX_HI    ; save temporary index (HI)
    lda     index               ; get index (LO)
    asl                         ; multiply index by 2
    rol     ZP_TEMP_INDEX_HI
    asl                         ; multiply index by 2
    rol     ZP_TEMP_INDEX_HI

    ; add to base path address to yield address in path
    clc
    adc     #<(path)            ; add path address (LO)
    sta     ZP_PATH_PTR_LO      ; set path address (LO)
    lda     ZP_TEMP_INDEX_HI    ; get index (HI)
    adc     #>(path)            ; add path address (HI)
    sta     ZP_PATH_PTR_HI      ; set path address (HI)
 
    ; move all of the sprites in this ring
    ldx     #sprites            ; init loop counter
move_sprite:
    lda     VERA_data0          ; skip 1 byte
    lda     VERA_data0          ; skip 1 byte
 
    ldy     #0                  ; offset to path X,Y coordinate
    lda     (ZP_PATH_PTR),y     ; get X (LO)
    sta     VERA_data0          ; set X (LO)
 
    iny
    lda     (ZP_PATH_PTR),y     ; get X (HI)
    sta     VERA_data0          ; set X (HI)
 
    iny
    lda     (ZP_PATH_PTR),y     ; get Y (LO)
    sta     VERA_data0          ; set Y (LO)
 
    iny
    lda     (ZP_PATH_PTR),y     ; get Y (HI)
    sta     VERA_data0          ; set Y (HI)
 
    lda     VERA_data0          ; skip 1 byte
    lda     VERA_data0          ; skip 1 byte
 
    ; add size of 4 path point structures, to leave space between sprites
    lda     ZP_PATH_PTR_LO
    clc
    adc     #4*4
    sta     ZP_PATH_PTR_LO
    bcc     no_rollover
    inc     ZP_PATH_PTR_HI
no_rollover:
 
    ; if the next path address goes past the end of the array, adjust it
    lda     ZP_PATH_PTR_HI      ; get new address (HI)
    cmp     #>(end)             ; compare new address (HI)
    bne     decide              ; go if no need to check LO
    lda     ZP_PATH_PTR_LO      ; get new address (LO)
    cmp     #<(end)             ; compare new address (LO)
decide:
    bcc     no_path_wrap        ; go if not past end of array
    and     #$0F                ; keep only the lower 4 bits
    ora     #(<(path))&$F0      ; include path address (bits 7:4)
    sta     ZP_PATH_PTR_LO      ; save adjusted path address (LO)
    lda     #>(path)            ; get path address (HI)
    sta     ZP_PATH_PTR_HI      ; reset path address (HI)
no_path_wrap:

    dex                         ; done with this ring of sprites?
    bne     move_sprite         ; go if no

    .endscope
.endmacro

RING_0_POINTS = ((end_sprite_path_0_50_x_66 - sprite_path_0_50_x_66) / 4)
RING_1_POINTS = ((end_sprite_path_1_70_x_93 - sprite_path_1_70_x_93) / 4)
RING_2_POINTS = ((end_sprite_path_2_90_x_120 - sprite_path_2_90_x_120) / 4)
RING_3_POINTS = ((end_sprite_path_3_110_x_146 - sprite_path_3_110_x_146) / 4)
RING_4_POINTS = ((end_sprite_path_4_130_x_173 - sprite_path_4_130_x_173) / 4)
RING_5_POINTS = ((end_sprite_path_5_150_x_200 - sprite_path_5_150_x_200) / 4)
RING_6_POINTS = ((end_sprite_path_6_170_x_226 - sprite_path_6_170_x_226) / 4)
RING_7_POINTS = ((end_sprite_path_7_190_x_253 - sprite_path_7_190_x_253) / 4)

update_all_sprite_positions:
    SetupData0 VRAM_sprattr
    MoveRing sprite_path_0_50_x_66,end_sprite_path_0_50_x_66,SPRITE_RING_COUNT_0,ZP_PATH_INDEX_0,RING_0_POINTS
    MoveRing sprite_path_1_70_x_93,end_sprite_path_1_70_x_93,SPRITE_RING_COUNT_1,ZP_PATH_INDEX_1,RING_1_POINTS
    MoveRing sprite_path_2_90_x_120,end_sprite_path_2_90_x_120,SPRITE_RING_COUNT_2,ZP_PATH_INDEX_2,RING_2_POINTS
    MoveRing sprite_path_3_110_x_146,end_sprite_path_3_110_x_146,SPRITE_RING_COUNT_3,ZP_PATH_INDEX_3,RING_3_POINTS
    MoveRing sprite_path_4_130_x_173,end_sprite_path_4_130_x_173,SPRITE_RING_COUNT_4,ZP_PATH_INDEX_4,RING_4_POINTS
    MoveRing sprite_path_5_150_x_200,end_sprite_path_5_150_x_200,SPRITE_RING_COUNT_5,ZP_PATH_INDEX_5,RING_5_POINTS
    MoveRing sprite_path_6_170_x_226,end_sprite_path_6_170_x_226,SPRITE_RING_COUNT_6,ZP_PATH_INDEX_6,RING_6_POINTS
    MoveRing sprite_path_7_190_x_253,end_sprite_path_7_190_x_253,SPRITE_RING_COUNT_7,ZP_PATH_INDEX_7,RING_7_POINTS
    rts
