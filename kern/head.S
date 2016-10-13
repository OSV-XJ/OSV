/* just an interrupt entry point */
.text
.code64
#include <inc/intr.h>
	.macro intr_entry intrno
ENTRY(intr_noec_\intrno)
	/* presudo like a error code */
	pushq %rax
ENTRY(intr_ec_\intrno)
	subq $err_code, %rsp
	movq %rax, fr_rax(%rsp)
	movq %rbx, fr_rbx(%rsp)
	movq %rcx, fr_rcx(%rsp)
	movq %rdx, fr_rdx(%rsp)
	movq %rdi, fr_rdi(%rsp)
	movq %rsi, fr_rsi(%rsp)
	movq %rbp, fr_rbp(%rsp)
	movq %r8, fr_r8(%rsp)
	movq %r9, fr_r9(%rsp)
	movq %r10, fr_r10(%rsp)
	movq %r11, fr_r11(%rsp)
	movq %r12, fr_r12(%rsp)
	movq %r13, fr_r13(%rsp)
	movq %r14, fr_r14(%rsp)
	movq %r15, fr_r15(%rsp)

	/* prepare for the arguments */

	movq %rdi, %rdx
	movq %rsp, %rdi
	movq $\intrno, %rsi

	call intr_handler
ENTRY(intr_end_\intrno)
	.endm	

	intr_entry 0
	intr_entry 1
	intr_entry 2
	intr_entry 3
	intr_entry 4
	intr_entry 5
	intr_entry 6
	intr_entry 7
	intr_entry 8
	intr_entry 9
	intr_entry 10
	intr_entry 11
	intr_entry 12
	intr_entry 13
	intr_entry 14
	intr_entry 15
	intr_entry 16
	intr_entry 17
	intr_entry 18
	intr_entry 19
	intr_entry 20
	intr_entry 21
	intr_entry 22
	intr_entry 23
	intr_entry 24
	intr_entry 25
	intr_entry 26
	intr_entry 27
	intr_entry 28
	intr_entry 29
	intr_entry 30
	intr_entry 31
	intr_entry 32
	intr_entry 33
	intr_entry 34
	intr_entry 35
	intr_entry 36
	intr_entry 37
	intr_entry 38
	intr_entry 39
	intr_entry 40
	intr_entry 41
	intr_entry 42
	intr_entry 43
	intr_entry 44
	intr_entry 45
	intr_entry 46
	intr_entry 47
	intr_entry 48
	intr_entry 49
	intr_entry 50
	intr_entry 51
	intr_entry 52
	intr_entry 53
	intr_entry 54
	intr_entry 55
	intr_entry 56
	intr_entry 57
	intr_entry 58
	intr_entry 59
	intr_entry 60
	intr_entry 61
	intr_entry 62
	intr_entry 63
	intr_entry 64
	intr_entry 65
	intr_entry 66
	intr_entry 67
	intr_entry 68
	intr_entry 69
	intr_entry 70
	intr_entry 71
	intr_entry 72
	intr_entry 73
	intr_entry 74
	intr_entry 75
	intr_entry 76
	intr_entry 77
	intr_entry 78
	intr_entry 79
	intr_entry 80
	intr_entry 81
	intr_entry 82
	intr_entry 83
	intr_entry 84
	intr_entry 85
	intr_entry 86
	intr_entry 87
	intr_entry 88
	intr_entry 89
	intr_entry 90
	intr_entry 91
	intr_entry 92
	intr_entry 93
	intr_entry 94
	intr_entry 95
	intr_entry 96
	intr_entry 97
	intr_entry 98
	intr_entry 99
	intr_entry 100
	intr_entry 101
	intr_entry 102
	intr_entry 103
	intr_entry 104
	intr_entry 105
	intr_entry 106
	intr_entry 107
	intr_entry 108
	intr_entry 109
	intr_entry 110
	intr_entry 111
	intr_entry 112
	intr_entry 113
	intr_entry 114
	intr_entry 115
	intr_entry 116
	intr_entry 117
	intr_entry 118
	intr_entry 119
	intr_entry 120
	intr_entry 121
	intr_entry 122
	intr_entry 123
	intr_entry 124
	intr_entry 125
	intr_entry 126
	intr_entry 127
	intr_entry 128
	intr_entry 129
	intr_entry 130
	intr_entry 131
	intr_entry 132
	intr_entry 133
	intr_entry 134
	intr_entry 135
	intr_entry 136
	intr_entry 137
	intr_entry 138
	intr_entry 139
	intr_entry 140
	intr_entry 141
	intr_entry 142
	intr_entry 143
	intr_entry 144
	intr_entry 145
	intr_entry 146
	intr_entry 147
	intr_entry 148
	intr_entry 149
	intr_entry 150
	intr_entry 151
	intr_entry 152
	intr_entry 153
	intr_entry 154
	intr_entry 155
	intr_entry 156
	intr_entry 157
	intr_entry 158
	intr_entry 159
	intr_entry 160
	intr_entry 161
	intr_entry 162
	intr_entry 163
	intr_entry 164
	intr_entry 165
	intr_entry 166
	intr_entry 167
	intr_entry 168
	intr_entry 169
	intr_entry 170
	intr_entry 171
	intr_entry 172
	intr_entry 173
	intr_entry 174
	intr_entry 175
	intr_entry 176
	intr_entry 177
	intr_entry 178
	intr_entry 179
	intr_entry 180
	intr_entry 181
	intr_entry 182
	intr_entry 183
	intr_entry 184
	intr_entry 185
	intr_entry 186
	intr_entry 187
	intr_entry 188
	intr_entry 189
	intr_entry 190
	intr_entry 191
	intr_entry 192
	intr_entry 193
	intr_entry 194
	intr_entry 195
	intr_entry 196
	intr_entry 197
	intr_entry 198
	intr_entry 199
	intr_entry 200
	intr_entry 201
	intr_entry 202
	intr_entry 203
	intr_entry 204
	intr_entry 205
	intr_entry 206
	intr_entry 207
	intr_entry 208
	intr_entry 209
	intr_entry 210
	intr_entry 211
	intr_entry 212
	intr_entry 213
	intr_entry 214
	intr_entry 215
	intr_entry 216
	intr_entry 217
	intr_entry 218
	intr_entry 219
	intr_entry 220
	intr_entry 221
	intr_entry 222
	intr_entry 223
	intr_entry 224
	intr_entry 225
	intr_entry 226
	intr_entry 227
	intr_entry 228
	intr_entry 229
	intr_entry 230
	intr_entry 231
	intr_entry 232
	intr_entry 233
	intr_entry 234
	intr_entry 235
	intr_entry 236
	intr_entry 237
	intr_entry 238
	intr_entry 239
	intr_entry 240
	intr_entry 241
	intr_entry 242
	intr_entry 243
	intr_entry 244
	intr_entry 245
	intr_entry 246
	intr_entry 247
	intr_entry 248
	intr_entry 249
	intr_entry 250
	intr_entry 251
	intr_entry 252
	intr_entry 253
	intr_entry 254
	intr_entry 255

	.global intr_fr_pop
	intr_fr_pop:
	movq %rdi, %rsp

	movq fr_rax(%rsp), %rax
	movq fr_rbx(%rsp), %rbx
	movq fr_rcx(%rsp), %rcx
	movq fr_rdx(%rsp), %rdx
	movq fr_rdi(%rsp), %rdi
	movq fr_rsi(%rsp), %rsi
	movq fr_rbp(%rsp), %rbp
	movq fr_r8(%rsp), %r8
	movq fr_r9(%rsp), %r9
	movq fr_r10(%rsp), %r10
	movq fr_r11(%rsp), %r11
	movq fr_r12(%rsp), %r12
	movq fr_r13(%rsp), %r13
	movq fr_r14(%rsp), %r14
	movq fr_r15(%rsp), %r15

	addq $orig_rip, %rsp
	iretq


	
